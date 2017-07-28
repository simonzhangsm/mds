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
 * core_msv.h
 *
 *  Created on: Oct 21, 2014
 *      Author: evank
 */

#ifndef CORE_TYPED_MSV_H_
#define CORE_TYPED_MSV_H_

#include "ruts/runtime_array.h"
#include "core/core_msv.h"
#include "core/core_coop.h"
#include <type_traits>

namespace mds {
  namespace core {

    enum class mod_ex_state {
      no_exception, divide_by_zero, guard_failed
        }; // mod_ex_state

    template <typename T>
    struct _default_op_traits {

      static bool modifies(modify_op op, const T &arg) {
        switch (op) {
        case modify_op::frozen_current:
        case modify_op::current_val:
          return false;
        case modify_op::set:
        case modify_op::roll_forward:
          return true;
        default:
          throw incompatible_modify_op{op};
        }
      }

      /*
       * Returns a pair containing a value and an indication of
       * whether an exception would occur.
       */
      static std::pair<T,mod_ex_state> compute(modify_op op, const T &old, const T &arg) {
        switch (op) {
        case modify_op::frozen_current:
        case modify_op::current_val:
        case modify_op::roll_forward:
          return std::make_pair(old, mod_ex_state::no_exception);
        case modify_op::set:
          return std::make_pair(arg, mod_ex_state::no_exception);
        default:
          throw incompatible_modify_op{op};
        }
      }
    }; // _default_op_traits

    template <typename T, typename = void>
    struct op_traits : _default_op_traits<T>
    {};

    template <typename T>
    struct op_traits<T, std::enable_if_t<std::is_arithmetic<T>::value>>
      : _default_op_traits<T>
    {
    private:
      using base = _default_op_traits<T>;
    public:
      static bool modifies(modify_op op, T arg) {
        switch (op) {
        case modify_op::add:
        case modify_op::sub:
          return arg != 0;
        case modify_op::mul:
        case modify_op::div:
          return arg != 1;
        default:
          return base::modifies(op, arg);
        }
      }

      static std::pair<T,mod_ex_state> compute(modify_op op, T old, T arg) {
        switch (op) {
        case modify_op::add:
          return std::make_pair(old+arg, mod_ex_state::no_exception);
        case modify_op::sub:
          return std::make_pair(old-arg, mod_ex_state::no_exception);
        case modify_op::mul:
          return std::make_pair(old*arg, mod_ex_state::no_exception);
        case modify_op::div:
          if (arg == 0) {
            return std::make_pair(old, mod_ex_state::divide_by_zero);
          }
          return std::make_pair(old/arg, mod_ex_state::no_exception);
        default:
          return base::compute(op, old, arg);
        }
      }
    }; //op_traits<arith>


    enum class known_guards {
      unbound
    };

    template <kind K>
    struct mod_condition : public gc_allocated_with_virtuals<mod_condition<K>, known_guards> {
      using base = gc_allocated_with_virtuals<mod_condition<K>, known_guards>;
    public:
      using val_type = kind_mv<K>;
      using typename base::discriminator_type;
      using typename base::virtuals_base;
      using base::call_virtual;

      static void init_vf_table(typename base::vf_table &);
      
      mod_condition(gc_token &gc, discriminator_type d)
        : base{gc, d}
      {}
      const static auto &descriptor() {
        static gc_descriptor d =
          GC_DESC(mod_condition)
          .template WITH_SUPER(base);
        return d;
      }
      struct virtuals : virtuals_base {
        virtual bool test(const mod_condition *self,
                          bool had_old_val,
                          const val_type &old_val,
                          const val_type &new_val) const = 0;
      };

      bool test(bool had_old_val,
                const val_type &old_val,
                const val_type &new_val) const
      {
        return call_virtual(this, &virtuals::test, had_old_val, old_val, new_val);
      }
    }; // mod_condition

    template <kind K>
    class unbound_mod_condition : public mod_condition<K> {
      using base = mod_condition<K>;
      using discriminator_type = typename base::discriminator_type;
      using val_type = typename base::val_type;
    public:
      static const discriminator_type
      discrim = known_guards::unbound;
      
      explicit unbound_mod_condition(gc_token &gc,
                                     discriminator_type d = discrim)
        : base{gc, d}
      {}

      const static auto &descriptor() {
        using this_class = unbound_mod_condition;
        static gc_descriptor d =
          GC_DESC(this_class)
          .template WITH_SUPER(base)
          ;
        return d;
      }

      struct virtuals : base::virtuals {
        using impl = unbound_mod_condition;
        bool test(const mod_condition<K> *self,
                  bool had_old_val,
                  const val_type &old_val,
                  const val_type &new_val) const override
        {
          return self->call_non_virtual(&impl::test_impl, had_old_val, old_val, new_val);
        }
      };

      bool test_impl(bool had_old_val, const val_type &old_val, const val_type &new_val) const
      {
        return !had_old_val;
      }
    }; // unbound_mod_condition

    template <kind K>
    gc_ptr<unbound_mod_condition<K>>
    is_unbound_guard()
    {
      static external_gc_ptr<unbound_mod_condition<K>>
        c = make_gc<unbound_mod_condition<K>>();
      return c;
    }

    template <kind K>
    struct typed_msv : msv {
      using val_type = kind_mv<K>;
      using mod_condition = core::mod_condition<K>;
      using unbound_mod_condition = core::unbound_mod_condition<K>;
      
      struct value : core::value {
        struct publish_t {};
        const val_type val;
        gc_ptr<value> prior;

        static bool need_single_modifier(const gc_ptr<task> &t,
                                         const gc_ptr<value> &v)
        {
          if (v == nullptr || t == nullptr) {
            return false;
          } else if (v->marks_publish()) {
            return true;
          } else {
            return t==v->write_task && v->single_modifier();
          }
        }
        
        value(gc_token &gc, timestamp_t time, const val_type &v, const gc_ptr<value> &p,
              const gc_ptr<task> &t = nullptr)
          : core::value{gc, time, false, need_single_modifier(t,p), t},
            val{v}, prior{p}
        {}
        value(gc_token &gc, timestamp_t time, publish_t, const gc_ptr<value> &p)
          : core::value{gc, time, true, false, nullptr}, val{}, prior{p}
        {}

        const static auto &descriptor() {
          static gc_descriptor d =
	    GC_DESC(value)
            .template WITH_SUPER(core::value)
	    .template WITH_FIELD(&value::val)
	    .template WITH_FIELD(&value::prior);
          return d;
        }
        static gc_ptr<value> for_publish(timestamp_t time, const gc_ptr<value> &n) {
          return make_gc<value>(time, publish_t{}, n);
        }

        static bool differs(const gc_ptr<value> &a, const gc_ptr<value> &b) {
          if (a == b) {
            return false;
          } else if (a == nullptr) {
            return b->val != val_type{};
          } else if (b == nullptr) {
            return a->val != val_type{};
          } else {
            return a->val != b->val;
          }
        }
      }; //  value



      struct value_chain : core::value_chain
      {
        value_chain(gc_token &gc, const gc_ptr<view> &v, discriminator_type d)
          : core::value_chain{gc, v, d}
        {}

        static gc_ptr<value_chain> for_view(const gc_ptr<view> &v,
                                            const gc_ptr<value_chain> &parent,
                                            const gc_ptr<msv> &in_msv);
        
        const static auto &descriptor() {
          static gc_descriptor d =
	    GC_DESC(value_chain)
            .template WITH_SUPER(core::value_chain)
            ;
          return d;
        }

        struct virtuals : core::value_chain::virtuals {
          virtual gc_ptr<value> value_at(const value_chain *self,
                                          timestamp_t ts) const = 0;
          virtual val_type frozen_read_in(value_chain *self,
                                          const gc_ptr<typed_msv> &msv) = 0;
          /*
           * If the op is other than frozen_current or current_val,
           * throws read_only_context_ex if in a read-only context.
           */
          virtual val_type modify_in(value_chain *self,
                                     const gc_ptr<typed_msv> &msv,
                                     modify_op op,
                                     const val_type &arg,
                                     ret_mode returning,
                                     const gc_ptr<mod_condition> &guard) = 0;
          virtual void add_conflicts_below(value_chain *self,
                                           const gc_ptr<blocking_mod> &bm,
                                           const gc_ptr<value_chain> &except) = 0;
        };

        gc_ptr<value> value_at(timestamp_t ts) const {
          return call_virtual(this, &virtuals::value_at, ts);
        }

        val_type frozen_read_in(const gc_ptr<typed_msv> &msv) {
          return call_virtual(this, &virtuals::frozen_read_in, msv);
        }

        val_type modify_in(const gc_ptr<typed_msv> &msv,
                           modify_op op, const val_type &arg,
                           ret_mode returning = ret_mode::resulting_val,
                           const gc_ptr<mod_condition> &guard = nullptr)
        {
          return call_virtual(this, &virtuals::modify_in, msv, op, arg, returning, guard);
        }

        void add_conflicts_below(const gc_ptr<blocking_mod> &bm,
                                 const gc_ptr<value_chain> &except) {
          call_virtual(this, &virtuals::add_conflicts_below, bm, except);
        }


      }; // value_chain

      class ross_vc;            // read-only snapshot
      class non_ross_vc;        // not a read-only snapshot
      class unpub_vc;           // unpublishable 
      class pub_vc;             // publishable
      class pub_live_vc;        // publishable live
      class pub_ss_vc;          // publishable snapshot
      class det_live_vc;        // detached live
      class det_ss_vc;          // detached snapshot
      class ro_live_vc;         // read-only live

      class modification;

      
    private:
      using vc_array_ptr_t = versioned_gc_ptr<gc_array<value_chain>, 1>;
      static constexpr typename vc_array_ptr_t::template flag_id<0> replaced_by_map{};
      using vc_map_ptr_t
      = gc_ptr<small_gc_cuckoo_map<ruts::uniform_key, gc_ptr<value_chain>>>;

      std::atomic<gc_ptr<value_chain>> _top_level_vc{nullptr};
      typename vc_array_ptr_t::atomic_pointer _vc_array;
      std::atomic<vc_map_ptr_t> _vc_map;

      gc_ptr<value_chain> insert_vc(const gc_ptr<view> &);
    public:
      typed_msv(gc_token &gc)
	: msv{gc} {
	}

      const static auto &descriptor() {
        static gc_descriptor d =
          GC_DESC(typed_msv)
          .template WITH_SUPER(msv)
          .template WITH_FIELD(&typed_msv::_top_level_vc)
          .template WITH_FIELD(&typed_msv::_vc_array)
          .template WITH_FIELD(&typed_msv::_vc_map)
          ;
        return d;
      }

      gc_ptr<value_chain>
      lookup_top_level(const gc_ptr<view> &v = top_level_view) {
        gc_ptr<value_chain> current = _top_level_vc;
        if (current != nullptr) {
          return current;
        }
        /*
         * otherwise, we create a new one and try to install it.
         */
        gc_ptr<value_chain> vc = value_chain::for_view(v, nullptr, GC_THIS);
        auto rr = ruts::try_change_value(_top_level_vc, nullptr, vc);
        /*
         * If we fail, we use whatever was there.
         */
        return rr.resulting_value();
      }

      /*
       * lookup(v) is guaranteed to return a value chain.
       */
      gc_ptr<value_chain> lookup(const gc_ptr<view> &v) {
        if (v == top_level_view) {
          return lookup_top_level(v);
        }
        /*
         * We check the map first, because if we have one, we know we
         * won't have to worry about the array.
         */
        vc_map_ptr_t m = _vc_map;
        if (m != nullptr) {
          gc_ptr<value_chain> vc = (*m)[v->uuid()];
          if (vc != nullptr) {
            assert(vc->get_view() == v);
            return vc;
          } 
        } else {
          gc_array_ptr<value_chain> a = _vc_array;
          if (a != nullptr && a.size() >= v->level()) {
            gc_ptr<value_chain> vc = a[v->level()-1];
            if (vc != nullptr && vc->get_view() == v) {
              return vc;
            }
          }
        }
        /*
         * if we get here, we didn't find one, so we need to try to
         * insert it.  We might discover that we have one as we go.
         */
        return insert_vc(v);
      }

      bool has_value(const gc_ptr<view> &v,
                     timestamp_t as_of = most_recent);
      val_type free_read(const gc_ptr<view> &v,
                         timestamp_t as_of = most_recent);
      val_type frozen_read(const gc_ptr<view> &v);
      void roll_forward(const gc_ptr<view> &v) {
        modify(v, modify_op::roll_forward, val_type{});
      }
      /*
       * If the op is other than frozen_current or current_val, throws
       * read_only_context_ex if the view is in a read-only context.
       */
      val_type modify(const gc_ptr<view> &v,
                      modify_op op,
                      const val_type &arg,
                      ret_mode returning = ret_mode::resulting_val,
                      const gc_ptr<mod_condition> &guard = nullptr);
      /*
       * If the op is other than frozen_current or current_val, throws
       * read_only_context_ex if the view is in a read-only context.
       */
      val_type write(const gc_ptr<view> &v,
                     const val_type &new_val,
                     ret_mode returning = ret_mode::resulting_val,
                     const gc_ptr<mod_condition> &guard = nullptr)
      {
        return modify(v, modify_op::set, new_val, returning, guard);
      }

      
      
    }; // typed_msv<K>

    
    template <kind K>
    inline
    gc_ptr<typed_msv<K>>
    msv::downcast() {
      gc_ptr<msv> me = GC_THIS;
      return std::static_pointer_cast<typed_msv<K>>(me);
    }


    template <kind K>
    bool
    typed_msv<K>::has_value(const gc_ptr<view> &v,
                            timestamp_t as_of)
      {
        process_rollups();
        if (_top_level_vc.load() == nullptr) {
          // if there are absolutely no VCs in this MSV, we know that
          // there's no VC corresponding to this view.

          return false;
        }
        /*
         * We could do this without actually creating the VCs if they
         * didn't exist, but then we'd have to walk the parentage and
         * worry about the the child being created after we noticed it
         * wasn't there and reading a value from an ancestor that was
         * never actually a correct view in the child.  (e.g., child
         * gets X, parent gets Y, child gets Z.  We can return either
         * X or Z, but Y is never correct.)  So we'll create it, in
         * order, to ensure that we give the right answer.
         */
        gc_ptr<value_chain> vc = lookup(v);
        gc_ptr<value> vn = vc->value_at(as_of);
        return vn != nullptr;
      }

    template <kind K>
    typename typed_msv<K>::val_type
    typed_msv<K>::free_read(const gc_ptr<view> &v,
                            timestamp_t as_of)
      {
        process_rollups();
        if (_top_level_vc.load() == nullptr) {
          // if there are absolutely no VCs in this MSV, we know that
          // there's no VC corresponding to this view.

          return val_type{};
        }
        /*
         * See comment in has_value()
         */
        gc_ptr<value_chain> vc = lookup(v);
        gc_ptr<value> vn = vc->value_at(as_of);
        return vn == nullptr ? val_type{} : vn->val;
      }

    template <kind K>
    typename typed_msv<K>::val_type
    typed_msv<K>::frozen_read(const gc_ptr<view> &v)
      {
        process_rollups();
        gc_ptr<value_chain> vc = lookup(v);
        return vc->frozen_read_in(GC_THIS);
      }

    template <kind K>
    typename typed_msv<K>::val_type
    typed_msv<K>::modify(const gc_ptr<view> &v,
                         modify_op op,
                         const val_type &arg,
                         ret_mode returning,
                         const gc_ptr<mod_condition> &guard)
      {
        /*
         * We don't process rollups here because we will do it in the
         * middle of the modification, after we block publication and
         * grab the timestamp.
         */
        gc_ptr<value_chain> vc = lookup(v);
        return vc->modify_in(GC_THIS, op, arg, returning, guard);
      }

    template <kind K>
    gc_ptr<typename typed_msv<K>::value_chain>
    typed_msv<K>::insert_vc(const gc_ptr<view> &v) {
      /*
       * We think we need to create one.  
       */
      vc_array_ptr_t old_array = _vc_array.contents();
      gc_ptr<value_chain> tlvc = lookup_top_level();
      gc_array_ptr<value_chain> new_vcs;
      gc_array_ptr<value_chain> old_vcs;
      auto level = v->level();
      /*
       * Does it go in the array slot?
       */
      if (old_array == nullptr && !old_array[replaced_by_map]) {
        /*
         * maybe.  Let's try.  If we're going to succeed, then none of
         * the ancestors have value chains, so we won't bother
         * checking.
         */
        new_vcs = make_gc_array<value_chain>(level);
        gc_ptr<value_chain> pvc = tlvc;
        std::size_t i = 0;
        for (auto a : v->ancestors) {
          gc_ptr<value_chain> new_vc = value_chain::for_view(a, pvc, GC_THIS);
          new_vcs[i++] = new_vc;
          pvc = new_vc;
        }
        gc_ptr<value_chain> result_vc = value_chain::for_view(v, pvc, GC_THIS);
        new_vcs[i] = result_vc;
        auto rr = _vc_array.change(old_array, new_vcs);
        if (rr) {
          /*
           * We installed it.
           */
          assert(result_vc != nullptr);
          return result_vc;
        } else {
          /*
           * Somebody else installed one (and maybe it got replaced by
           * a map).  
           */
          old_vcs = rr.resulting_value();
          /*
           * It might be that somebody already did the one we're
           * looking for or one of its children.  If it was replaced
           * by a map, it's size will show as zero.
           */
          if (old_vcs.size() > i) {
            gc_ptr<value_chain> vc = old_vcs[i];
            if (vc->get_view() == v) {
              assert(vc != nullptr);
              return vc;
            }
          }
        }
      }
      /*
       * It's not in the array.  Do we need to create a map?
       */
      vc_map_ptr_t current_map = _vc_map;
      if (current_map == nullptr) {
        /*
         * There's no map, so we'll assume we have to create one.  As
         * a rough estimate, we'll assume that there's no overlap
         * between the existing array and the VCs for the new view,
         * and then double it.
         */
        std::size_t cap = 2*(old_vcs.size()+level-1);
        auto new_map = make_gc<typename vc_map_ptr_t::element_type>(cap);
        /*
         * Everything from the old array goes in
         */
        gc_array_ptr<value_chain> vcs = old_array;
        for (auto vc : vcs) {
          (*new_map)[vc->get_view()->uuid()] = vc;
        }
        auto rr = ruts::try_change_value(_vc_map, nullptr, new_map);
        current_map = rr.resulting_value();
        /*
         * If we fail, a map has been installed, which necessarily has
         * the array elements copied over.
         */
      }
      /*
       * At this point, we know there's a map.  There might still be
       * an array, if the thread that installed the map died, so we'll
       * clear it and set the flag.  We can do this as an
       * unconditional set.
       */
      if (!old_array[replaced_by_map]) {
        vc_array_ptr_t ap = nullptr;
        ap[replaced_by_map] = true;

        _vc_array.fast_set(ap);
      }
      /*
       * first we check if the map contains the one we're looking for.
       */
      gc_ptr<value_chain> vc = (*current_map)[v->uuid()];
      if (vc != nullptr) {
        return vc;
      }
      if (new_vcs.empty()) {
        /*
         * if new_vcs is empty, we haven't created anything yet.  So
         * we'll walk through the ancestry list and create any that
         * aren't there.  Note that we might be racing another thread
         * doing the same thing, so we don't make any assumptions that
         * we're doing a suffix.
         */
        gc_ptr<value_chain> pvc = tlvc;
        for (auto a : v->ancestors) {
          vc = (*current_map)[a->uuid()];
          if (vc == nullptr) {
            vc = value_chain::for_view(a, pvc, GC_THIS);
            auto rr = current_map->put_new(a->uuid(), vc);
            pvc = rr.resulting_value;
          } else {
            pvc = vc;
          }
        }
        vc = (*current_map)[v->uuid()];
        if (vc != nullptr) {
          return vc;
        }
        vc = value_chain::for_view(v, pvc, GC_THIS);
        auto rr = current_map->put_new(v->uuid(), vc);
        assert(rr.resulting_value != nullptr);
        return rr.resulting_value;
      } else {
        /*
         * We have candidates in new_vcs.  As long as we can add them
         * to the map, we're good.  If we ever bump into one that
         * didn't work (because we found a one already there, we need
         * to create replacements for all the rest.
         */
        bool need_new = false;
        gc_ptr<value_chain> last_vc = nullptr;
        for (auto nvc : new_vcs) {
          gc_ptr<view> vcv = nvc->get_view();
          if (need_new) {
            nvc = value_chain::for_view(vcv, last_vc, GC_THIS);
          }
          auto rr = current_map->put_new(vcv->uuid(), nvc);
          if (!rr) {
            need_new = true;
          }
          last_vc = rr.resulting_value;
        }
        assert(last_vc != nullptr);
        return last_vc;
      }
    }
  }
}



#endif /* CORE_TYPED_MSV_H_ */
