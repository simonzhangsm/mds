/*
 *
 *  Managed Data Structures
 *  Copyright © 2016 Hewlett Packard Enterprise Development Company LP.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  As an exception, the copyright holders of this Library grant you permission
 *  to (i) compile an Application with the Library, and (ii) distribute the 
 *  Application containing code generated by the Library and added to the 
 *  Application during this compilation process under terms of your choice, 
 *  provided you also meet the terms and conditions of the Application license.
 *
 */

/*
 * iso_ctxt.h
 *
 *  Created on: Mar 16, 2015
 *      Author: evank
 */

#ifndef ISO_CTXT_H_
#define ISO_CTXT_H_

#include "mds_common.h"
#include <functional>
#include <tuple>
#include <memory>
#include <chrono>

namespace mds {
  class iso_ctxt;
  class pub_result;


  class no_value {};

  class publish_failed {};

  inline auto max_tries(std::size_t n) {
    return [n](){
      return [n, i=std::size_t{0}]() mutable {
        return ++i>=n;
      };
    };
  }

  inline auto operator "" _times(unsigned long long n) {
    return max_tries(n);
  }

  inline auto try_while(std::function<bool()> fn) {
    return [=](){
      return [=](){
        return !fn();
      };
    };
  }

  template <typename C, typename D>
  inline auto try_until(std::chrono::time_point<C,D> end) {
    return [=](){
      return [=](){
        auto now = C::now();
        return now > end;
      };
    };
  }

  template <typename R, typename P>
  inline auto try_for(std::chrono::duration<R,P> dur) {
    return [=](){
      return [end=std::chrono::high_resolution_clock::now()+dur](){
        auto now = std::chrono::high_resolution_clock::now();
        return now > end;
      };
    };
  }

  
  
  template <typename T>
  class iso_ctxt_opt {
  public:
    using control_type = std::function<bool(const T &)>;
    using opt_type = std::function<control_type()>;
  private:
    std::vector<opt_type> opts;
    static opt_type munge(const opt_type &fn) {
      return fn;
    }

    static opt_type munge(const std::function<std::function<bool()>()> &fn) {
      return [fn](){
        return [cfn=fn()](const T &){
          return cfn();
        };
      };
    }
  public:
    template <typename... Cond>
    iso_ctxt_opt(Cond&&...conds) 
      : opts({munge(std::forward<Cond>(conds))...})
    {}
  public:
    std::vector<control_type> controls() const {
      std::vector<control_type> res;
      if (!opts.empty()) {
        res.reserve(opts.size());
        for (const opt_type &opt : opts) {
          res.push_back(opt());
        }
      }
      return res;
    }
    void add(const iso_ctxt_opt &other) {
      opts.insert(opts.end(), other.opts.begin(), other.opts.end());
    }

    static bool check(const std::vector<control_type> &cs, const T &arg) {
      return !std::any_of(cs.begin(), cs.end(),
                          [&arg](const auto &c) {
                            return c(arg);
                          });
    }
  };


  class rerun_opts : public iso_ctxt_opt<iso_ctxt> {
  public:
    template <typename...Ts>
    rerun_opts(Ts&&...conds) : iso_ctxt_opt(std::forward<Ts>(conds)...) {}
  };

  template <typename...Cond>
  inline 
  rerun_opts
  rerun(Cond&&...conds) {
    return rerun_opts(std::forward<Cond>(conds)...);
  }

  class resolve_opts : public iso_ctxt_opt<pub_result> {
  public:
    template <typename...Ts>
    resolve_opts(Ts&&...conds) : iso_ctxt_opt(std::forward<Ts>(conds)...) {}
  };

  template <typename...Cond>
  inline 
  resolve_opts
  resolve(Cond&&...conds) {
    return resolve_opts(std::forward<Cond>(conds)...);
  }
  
  class task : public __delegator<api::task_handle>
  {
    using base = __delegator<api::task_handle>;
    using ctxt_handle_type = api::iso_context_handle;
    mutable ctxt_handle_type _ctxt;
    task(const handle_type &h) : base{h}, _ctxt{} {}
    friend class thread_local_data;
    friend class weak_handle<task>;
    friend class iso_ctxt;
    friend class pub_result;
    friend void ensure_thread_initialized();

    static task &_current() {
      static thread_local task t = handle_type::default_task();
      return t;
    }
    
    static void init_base_task() {
      handle_type::init_thread_base_task([]() {
          return current()._handle.pointer();
        });
    }


    void remember_and_call(std::function<void()> &&fn);

    struct _establish {
      _establish(const task &t) {
        _current() = t;
      }
      ~_establish() {
        _current() = handle_type::pop();
      }
    };

    template <typename Fn, typename...Args>
    auto establish_and_run(Fn&& fn, Args&&...args) const;
    // template <typename RT, typename...ATs>
    // static auto __task_fn(std::function<RT(ATs...)> fn) {
    //   return [=](const ATs &...args) {
    //     as_task([&]() {
    //         fn(args...);
    //       });
    //   };
    // }


  public:
    using task_fn_type = std::function<void()>;
    using pfr_fn_type = std::function<bool(const task &)>;
    class info;
    task() = default;
    iso_ctxt context() const;
    task parent() const {
      return _handle.get_parent();
    }

    void add_dependent(const task &other) {
      if (*this != other) {
        _handle.add_dependent(other._handle);
      }
    }

    void depends_on(task t) {
      t.add_dependent(*this);
    }

    template <typename...Tasks>
    void depends_on(task t1, task t2, Tasks...more) const {
      t1.add_dependent(*this);
      depends_on(t2, more...);
    }

    template <typename C>
    void depends_on_all(const C &tasks) const {
      for (task &t : tasks) {
        t.add_dependent(*this);
      }
    }

    void always_redo() {
      _handle.always_redo();
    }

    void cannot_redo() {
      _handle.cannot_redo();
    }

    static task current() {
      return _current();
    }

    bool needs_prepare_for_redo() const;

    void on_prepare_for_redo(const pfr_fn_type &pfr_fn);
    void on_prepare_for_redo(const std::function<bool()> &fn) {
      pfr_fn_type pf = [=](const task &){
        return fn();
      };
      on_prepare_for_redo(pf);
    }
    void on_prepare_for_redo(const std::function<void(const task &)> &fn) {
      pfr_fn_type pf = [=](const task &t){
        fn(t);
        return true;
      };
      on_prepare_for_redo(pf);
    }
    void on_prepare_for_redo(const std::function<void()> &fn) {
      pfr_fn_type pf = [=](const task &){
        fn();
        return true;
      };
      on_prepare_for_redo(pf);
    }

    std::shared_ptr<info> get_info() const;
    std::shared_ptr<info> lookup_info() const;

    // template <typename Fn, typename...Ts, typename I, I...indices>
    // void apply(Fn&& fn, std::tuple<Ts...> &params, const std::integer_sequence<I,indices...> &is) {
    //   std::forward<Fn>(fn)(std::get<indices>(params)...);
    // }

    static void as_task(std::function<void()> fn);

    // template <typename Fn>
    // constexpr static auto task_fn2(Fn &&fn) {
    //   return __task_fn(meta::as_function_t<Fn>(std::forward<Fn>(fn)));
    // }

    template <typename Fn>
    constexpr static auto task_fn(Fn &&fn) {
      return [fn=std::forward<Fn>(fn)](auto...args) {
        as_task([=]() {
            fn(args...);
          });
      };
    }

    
    template <typename T>
    class computed_val {
      class task_computed_base {
      public:
        virtual const T &get() const = 0;
      };


      class unpublishable : public task_computed_base {
        const T _val;
      public:
        template <typename Fn>
        unpublishable(Fn&& fn)
          : _val{std::forward<Fn>(fn)(T{})}
        {}

        const T &get() const {
          return _val;
        }
      };

      class publishable;

      std::shared_ptr<task_computed_base> _tc;
      template <typename X>
      using compatible = std::enable_if_t<std::is_assignable<T&,X>::value>;

      template <typename Fn>
      static std::shared_ptr<task_computed_base>
      create_concrete(Fn&& fn);
    public:
      computed_val() = default;
      computed_val(const computed_val &) = default;
      computed_val(computed_val &&) = default;
      template <typename X, typename = compatible<X>>
        computed_val(const computed_val<X> &rhs) : _tc{rhs._tc}
      {}
      template <typename X, typename = compatible<X>>
        computed_val(computed_val<X> &&rhs) : _tc{std::move(rhs._tc)}
      {}
      template <typename Fn,
                typename RT=std::result_of_t<Fn(const T &)>,
                typename=std::enable_if_t<std::is_assignable<T&,RT>::value>>
        computed_val(const Fn &fn)
        : _tc{create_concrete(fn)}
      {
        // std::cout << "Created a TC: " << _tc << std::endl;
      }
      // template <typename Fn, 
      //           typename RT=std::result_of_t<Fn()>,
      //           typename=std::enable_if_t<std::is_assignable<T&,RT>::value> >
      // computed_val(const Fn &fn)
      //   : computed_val([fn](const T &ignored) { return fn(); })
      // {}

      const T &get() const {
        return _tc->get();
      }
      operator const T&() const {
        return get();
      }
    };

    template <typename T, typename Fn>
    static computed_val<T> computed(Fn &&fn) {
      return computed_val<T>(std::forward<Fn>(fn));
    }
  }; // task

  inline void ensure_thread_initialized() {
    task::init_base_task();
  }

  template <typename Fn, typename...Args>
  auto task::establish_and_run(Fn&& fn, Args&&...args) const {
    ensure_thread_initialized();
    _establish c(_handle.push());
    return std::forward<Fn>(fn)(std::forward<Args>(args)...);
  }


  class publish_report {
  private:
    bool _succeeded = false;
  public:
    bool succeeded() const {
      return _succeeded;
    }
    virtual void reset() {
      _succeeded = false;
    }
    virtual void before_run(iso_ctxt &ctxt) {}
    virtual void before_resolve(const pub_result &pr) {}
    virtual void note_success() {
      _succeeded = true;
    }
    virtual void note_failure() {
      _succeeded = false;
    }
  };

  class report_opts {
    std::vector<std::shared_ptr<publish_report>> _reports;
    template <typename Fn, typename...Args>
    void delegate(Fn&& fn, Args&&...args) const {
      std::for_each(_reports.begin(), _reports.end(),
                    [fn=std::forward<Fn>(fn), &args...](const auto &rp) {
                      (rp.get()->*fn)(args...);
                    });
    }
  public:
    template <typename...Rs>
    report_opts(Rs&&...reports) : _reports({std::forward<Rs>(reports)...}) {}
    void add(const report_opts &other) {
      _reports.insert(_reports.end(), other._reports.begin(), other._reports.end());
    }
    void reset() const {
      delegate(&publish_report::reset);
    }
    void before_run(iso_ctxt &ctxt) const {
      delegate(&publish_report::before_run, ctxt);
    }
    void before_resolve(const pub_result &pr) const {
      delegate(&publish_report::before_resolve, pr);
    }
    void note_success() const {
      delegate(&publish_report::note_success);
    }
    void note_failure() const {
      delegate(&publish_report::note_failure);
    }
  };

  template <typename...Rs>
  inline report_opts report_to(Rs&&...reports) {
    return report_opts(std::forward<Rs>(reports)...);
  }



  class pub_result : public __delegator<api::publication_attempt_handle> {
    using base = __delegator<api::publication_attempt_handle>;
    pub_result(const handle_type &h) : base{h} {}
    friend class iso_ctxt;

  public:
    pub_result() = default;
    handle_type handle() const {
      return _handle;
    }
    bool succeeded() const {
      return _handle.succeeded();
    }
    operator bool() const {
      return succeeded();
    }
    iso_ctxt source_context() const;
    iso_ctxt target_context() const;
    long n_to_redo() const {
      return _handle.n_to_redo();
    }

    bool resolve(const report_opts &reports) const;

    void redo_tasks_by_start_time(std::vector<task> &) const;

    std::vector<task> redo_tasks_by_start_time() const {
      std::vector<task> tasks(n_to_redo());
      redo_tasks_by_start_time(tasks);
      return tasks;
    }

    bool redo(const task &task) const;

    bool prepare_for_redo() const {
      return _handle.prepare_for_redo();
    }
    
  };

  class iso_ctxt : public __delegator<api::iso_context_handle> {
    using base = __delegator<api::iso_context_handle>;
    iso_ctxt(const handle_type &h) : base{h} {}
    friend class pub_result;
    friend class task;
  public:
    using view_type = api::view_type;
    using mod_type = api::mod_type;
    iso_ctxt() = default;
    static const iso_ctxt &global() {
      static iso_ctxt c{handle_type::global()};
      return c;
    }
    static const iso_ctxt &for_process() {
      ensure_thread_initialized();
      static iso_ctxt c{handle_type::for_process()};
      return c;
    }
    static iso_ctxt current() {
      return task::current().context();
    }
    bool is_null() const {
      return _handle.is_null();
    }
    bool is_snapshot() const {
      return _handle.is_snapshot();
    }
    bool is_read_only() const {
      return _handle.is_read_only();
    }
    bool is_publishable() const {
      return _handle.is_publishable();
    }
    iso_ctxt parent() const {
      return is_null() ? iso_ctxt{} : _handle.parent();
    }

    task top_level_task() const {
      return _handle.top_level_task();
    }
    bool operator ==(const iso_ctxt &other) const {
      return _handle == other._handle;
    }
    bool operator !=(const iso_ctxt &other) const {
      return _handle != other._handle;
    }
    

    iso_ctxt create_nested(mod_type mt = mod_type::publishable,
                           view_type vt = view_type::live)

    {
      ensure_thread_initialized();
      return _handle.new_child(vt, mt);
    }
    iso_ctxt create_nested(view_type vt) {
      return create_nested(mod_type::publishable, vt);
    }
    iso_ctxt create_snapshot(mod_type mt = mod_type::publishable) {
      return create_nested(mt, view_type::snapshot);
    }
    iso_ctxt create_nested_detached(view_type vt = view_type::live) {
      return create_nested(mod_type::detached, vt);
    }
    iso_ctxt create_nested_read_only(view_type vt = view_type::live) {
      return create_nested(mod_type::read_only, vt);
    }
    iso_ctxt create_detached_snapshot() {
      return create_snapshot(mod_type::detached);
    }
    iso_ctxt create_read_only_snapshot() {
      return create_snapshot(mod_type::read_only);
    }

    static iso_ctxt nested_from_current(mod_type mt = mod_type::publishable,
                                        view_type vt = view_type::live)
    {
      return current().create_nested(mt, vt);
    }
    static iso_ctxt nested_from_current(view_type vt) {
      return nested_from_current(mod_type::publishable, vt);
    }
    static iso_ctxt snapshot_from_current(mod_type mt = mod_type::publishable) {
      return nested_from_current(mt, view_type::snapshot);
    }
    static iso_ctxt nested_detached_from_current(view_type vt = view_type::live) {
      return nested_from_current(mod_type::detached, vt);
    }
    static iso_ctxt nested_read_only_from_current(view_type vt = view_type::live) {
      return nested_from_current(mod_type::read_only, vt);
    }
    static iso_ctxt detached_snapshot_from_current() {
      return snapshot_from_current(mod_type::detached);
    }
    static iso_ctxt read_only_snapshot_from_current() {
      return snapshot_from_current(mod_type::read_only);
    }

    struct use : task::_establish {
      use(const iso_ctxt &c)
        : task::_establish([&c](){
            ensure_thread_initialized();
            return c.handle().push_prevailing();
          }()) {}
      use(const use &) = delete;
      use(use &&) = delete;
    };


    pub_result try_publish() {
      return _handle.publish();
    }

    pub_result publish(const resolve_opts &resolve_opts, const report_opts &reports) {
      using namespace std;
      pub_result pr = try_publish();
      if (!pr) {
        std::vector<task> tasks_to_redo;
        auto controls = resolve_opts.controls();
	while (resolve_opts::check(controls, pr)
               && pr.resolve(reports))
          {
            pr = try_publish();
            if (pr) {
              break;
            }
          }
      }
      return pr;
    }

    template <typename Fn, typename...Args>
    auto call(Fn&& fn, Args&&...args);

    template <typename Fn>
    iso_ctxt &run(Fn &&fn) {
      call(std::forward<Fn>(fn));
      return *this;
    }

    // run_then_publish(fn, puboptions...)

    template <typename Ret, typename...Args>
    auto bind(const std::function<Ret(Args...)> &fn) {
      iso_ctxt me = *this;
      return [=](Args&&...args) {
	return me.call([&]{
	    return fn(std::forward<Args>(args)...);
	  });
      };
    }

    template <typename Fn>
    static auto bind_to_current(Fn&& fn) {
      return current().bind(std::forward<Fn>(fn));
    }

    template <typename Fn, typename...Args>
    constexpr static bool returns_void() {
      // return std::is_void<std::result_of<Fn()> >::value;
      return std::is_void<decltype(std::forward<Fn>(std::declval<Fn>())
                                   (std::forward<Args>(std::declval<Args>())...))>::value;
    }


    template <typename Fn, typename...Args>
    static auto mask_void(Fn&& fn,
                          std::enable_if_t<!returns_void<Fn, Args...>(), bool>) {
      return std::forward<Fn>(fn);
    }

    template <typename Fn, typename...Args>
    static auto mask_void(Fn&& fn,
                          std::enable_if_t<returns_void<Fn, Args...>(), bool>)
    {
      return [&](auto&&...args) {
        std::forward<Fn>(fn)(std::forward<decltype(args)>(args)...);
        return no_value{};
      };
    }

    // template <typename Fn>
    // static auto mask_void(Fn&& fn, std::enable_if_t<!returns_void<Fn>(), bool>) {
    //   return std::forward<Fn>(fn);
    // }

    // template <typename Fn>
    // static auto mask_void(Fn&& fn, std::enable_if_t<returns_void<Fn>(), bool>) {
    //   return [&]() {
    //     std::forward<Fn>(fn)();
    //     return no_value{};
    //   };
    // }

    template <typename Fn, typename...Args>
    auto call_isolated__2(const resolve_opts &resolve_opts,
                          const report_opts &reports,
                          Fn&& fn,
                          Args&&... args)
    {
      reports.before_run(*this);
      auto val = call(mask_void<Fn,Args...>(std::forward<Fn>(fn), true),
                      std::forward<Args>(args)...);
      bool worked = publish(resolve_opts, reports);
      // std::cout << "publish " << (worked ? "succeeded" : "failed") << std::endl;
      return std::make_pair(val, worked);
    }
    
    template <typename Fn, typename...Args>
    auto call_isolated__(mod_type mt,
                         view_type vt,
                         const rerun_opts &rerun_opts,
                         const resolve_opts &resolve_opts,
                         const report_opts &reports,
                         Fn&& fn,
                         Args&&... args)
    {
      auto masked_fn = mask_void<Fn,Args...>(std::forward<Fn>(fn), true);
      iso_ctxt child = create_nested(mt, vt);
      if (mt != mod_type::publishable) {
        return std::make_pair(child.call(masked_fn, std::forward<Args>(args)...),
                              true);
      }
      reports.reset();
      auto res = child.call_isolated__2(resolve_opts, reports,
                                        masked_fn, std::forward<Args>(args)...);
      if (!res.second) {
        auto controls = rerun_opts.controls();
	while (!res.second && rerun_opts::check(controls, child)) {
          child = create_nested(mt, vt);
	  res = child.call_isolated__2(resolve_opts, reports,
                                       masked_fn, std::forward<Args>(args)...);
	}
      }
      if (res.second) {
        reports.note_success();
      } else {
        reports.note_failure();
      }
      return res;
    }

    template <typename...Xs>
    auto get_opts_and_call_isolated(mod_type mt, view_type vt,
                                    rerun_opts &rerun,
                                    resolve_opts &resolve,
                                    report_opts &reports,
                                    mod_type mt2, Xs&&...params)
    {
      return get_opts_and_call_isolated(mt2, vt, rerun, resolve, reports,
                                        std::forward<Xs>(params)...);
    }
                                     
    template <typename...Xs>
    auto get_opts_and_call_isolated(mod_type mt, view_type vt,
                                    rerun_opts &rerun,
                                    resolve_opts &resolve,
                                    report_opts &reports,
                                    view_type vt2, Xs&&...params)
    {
      return get_opts_and_call_isolated(mt, vt2, rerun, resolve, reports,
                                        std::forward<Xs>(params)...);
    }
                                     
    template <typename...Xs>
    auto get_opts_and_call_isolated(mod_type mt, view_type vt,
                                    rerun_opts &rerun,
                                    resolve_opts &resolve,
                                    report_opts &reports,
                                    const rerun_opts opts, Xs&&...params)
    {
      rerun.add(opts);
      return get_opts_and_call_isolated(mt, vt, rerun, resolve, reports,
                                        std::forward<Xs>(params)...);
    }
                                     
    template <typename...Xs>
    auto get_opts_and_call_isolated(mod_type mt, view_type vt,
                                    rerun_opts &rerun,
                                    resolve_opts &resolve,
                                    report_opts &reports,
                                    const resolve_opts opts, Xs&&...params)
    {
      resolve.add(opts);
      return get_opts_and_call_isolated(mt, vt, rerun, resolve, reports,
                                        std::forward<Xs>(params)...);
    }
                                     

    template <typename...Xs>
    auto get_opts_and_call_isolated(mod_type mt, view_type vt,
                                    rerun_opts &rerun,
                                    resolve_opts &resolve,
                                    report_opts &reports,
                                    const report_opts opts, Xs&&...params)
    {
      reports.add(opts);
      return get_opts_and_call_isolated(mt, vt, rerun, resolve, reports,
                                        std::forward<Xs>(params)...);
    }
                                     

    template <typename Fn, typename...Args>
    auto get_opts_and_call_isolated(mod_type mt, view_type vt,
                                    rerun_opts &rerun,
                                    resolve_opts &resolve,
                                    report_opts &reports,
                                    Fn&& fn, Args&&...args)
    {
      return call_isolated__(mt, vt, rerun, resolve, reports,
                             std::forward<Fn>(fn), std::forward<Args>(args)...);
    }
                                     

    template <typename...Xs>
    auto call_isolated_nothrow(Xs&&...params) {
      rerun_opts rerun;
      resolve_opts resolve;
      report_opts reports;
      return get_opts_and_call_isolated(mod_type::publishable, view_type::live,
                                        rerun, resolve, reports,
                                        std::forward<Xs>(params)...);
        
    }

    template <typename...Xs>
    auto call_isolated(Xs&&...params) {
      auto res = call_isolated_nothrow(std::forward<Xs>(params)...);
      if (!res.second) {
        throw publish_failed{};
      }
      return res.first;
    }

    template <typename Fn, typename...Args>
    auto call_detached(Fn &&fn, Args&&...args) {
      return create_nested_detached().call(std::forward<Fn>(fn),
                                           std::forward<Args>(args)...);
    }

    template <typename Fn, typename...Args>
    auto call_in_snapshot(Fn &&fn, Args&&...args) {
      return create_snapshot().call(std::forward<Fn>(fn),
                                    std::forward<Args>(args)...);
    }


    template <typename Fn, typename...Args>
    auto call_in_read_only_snapshot(Fn &&fn, Args&&...args) {
      return create_read_only_snapshot().call(std::forward<Fn>(fn),
                                              std::forward<Args>(args)...);
    }

  };

  inline iso_ctxt pub_result::source_context() const {
    return _handle.source_context();
  }

  inline iso_ctxt pub_result::target_context() const {
    return source_context().parent();
  }
  
  inline iso_ctxt task::context() const {
    if (is_null()) {
      return iso_ctxt{};
    }
    if (_ctxt.is_null()) {
      _ctxt = _handle.get_context();
    }
    return _ctxt;
  }

  inline void task::as_task(std::function<void()> fn) {
    task ct = _current();
    iso_ctxt cc = ct.context();
    if (cc.is_publishable()) {
      ensure_thread_initialized();
      task nt = handle_type::push_new();
      nt._ctxt = cc._handle;
      nt.remember_and_call(std::move(fn));
    } else {
      std::cout << "Context " << cc << " not publishable" << std::endl;
      fn();
    }
  }

  template <typename Fn>
  inline void as_task(Fn&& fn) {
    task::as_task(std::forward<Fn>(fn));
  }


  template <typename...Xs>
  inline
  auto isolated(Xs&&...params) {
    return iso_ctxt::current().call_isolated(std::forward<Xs>(params)...);
  }
  template <typename Fn>
  inline
  auto detached(Fn &&fn) {
    return iso_ctxt::current().call_detached(std::forward<Fn>(fn));
  }
  template <typename Fn>
  inline
  auto in_snapshot(Fn &&fn) {
    return iso_ctxt::current().call_in_snapshot(std::forward<Fn>(fn));
  }
  template <typename Fn>
  inline
  auto in_read_only_snapshot(Fn &&fn) {
    return iso_ctxt::current().call_in_snapshot(std::forward<Fn>(fn));
  }


  template <typename T>
  class task::computed_val<T>::publishable : public task_computed_base {
    T _val;
    mutable weak_handle<task> _task;
  public:
    template <typename Fn>
    void set_task(std::weak_ptr<publishable> wp,
                  Fn&& fn)
    {
      as_task([=, fn=std::forward<Fn>(fn)]() {
          std::shared_ptr<publishable> me = wp.lock();
          if (me != nullptr) {
            me->_task = task::current();
            me->_val = fn(me->_val);
          }
        });
    }

    const T &get() const {
      task t = _task.lock();
      if (t != nullptr) {
        t.add_dependent(task::current());
      }
      return _val;
    }

  };

  template <typename T>
  template <typename Fn>
  inline
  std::shared_ptr<typename task::template computed_val<T>::task_computed_base>
  task::computed_val<T>::create_concrete(Fn&& fn) {
    iso_ctxt ctxt = iso_ctxt::current();
    if (ctxt.is_publishable()) {
      auto res = std::make_shared<publishable>();
      res->set_task(res, std::forward<Fn>(fn));
      return res;
    } else {
      return std::make_shared<unpublishable>(std::forward<Fn>(fn));
    }
  }

  
  // class iso_ctxt::use {
  //   const iso_ctxt _old;
  // public:
  //   explicit use(const iso_ctxt &c)
  //     : _old(iso_ctxt::current())
  //   {
  //     iso_ctxt::current() = c;
  //   }
  //   explicit use(iso_ctxt &&c)
  //     : _old(iso_ctxt::current())
  //   {
  //     iso_ctxt::current() = std::move(c);
  //   }
  //   ~use() {
  //     iso_ctxt::current() = _old;
  //   }
  //   use(const use &) = delete;
  //   // Move would actually make sense, but it would require noting
  //   // invalidation and checking for it in the dtor.
  //   use(use &&other) = delete;
  // };

  template <typename Fn, typename...Args>
  inline
  auto iso_ctxt::call(Fn&& fn, Args&&...args) {
    use tc{*this};
    return std::forward<Fn>(fn)(std::forward<Args>(args)...);
  }

      
  template <typename Iter, typename Fn>
  void for_each_in_tasks(Iter from, Iter to, Fn&& fn) {
    std::for_each(from, to, task::task_fn(std::forward<Fn>(fn)));
  }
}

namespace std {
  template<> struct hash<mds::task> : mds::__intrinsic_hash<mds::task> {};
  template<> struct hash<mds::iso_ctxt> : mds::__intrinsic_hash<mds::iso_ctxt> {};
}


#endif /* ISO_CTXT_H_ */
