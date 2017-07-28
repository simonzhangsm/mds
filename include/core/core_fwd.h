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
 * core_fwd.h
 *
 *  Created on: Oct 21, 2014
 *      Author: evank
 */

#ifndef CORE_FWD_H_
#define CORE_FWD_H_

#include <cstdint>
#include <limits>
#include <atomic>
#include <type_traits>
#include <iostream>

#include "mds_types.h"
#include "ruts/managed.h"
#include "ruts/ms_forward.h"
#include "mpgc/gc.h"
#include "mpgc/gc_cuckoo_map.h"
#include "mpgc/gc_stack.h"

namespace mds {
  namespace core {
    using namespace mpgc;

    using ruts::with_uniform_id;
    using ruts::uniform_key;

    enum class kind {
      BOOL,
      BYTE, UBYTE,
      SHORT, USHORT,
      INT, UINT,
      LONG, ULONG,
      FLOAT, DOUBLE,
      STRING,
      RECORD,
      BINDING,
      ARRAY,
      NAMESPACE,
      n_kinds
    };
    constexpr std::size_t n_kinds = static_cast<std::size_t>(kind::n_kinds);

    

    using timestamp_t = std::size_t;

    constexpr timestamp_t most_recent = std::numeric_limits<timestamp_t>::max();
    class unimplemented {};
    class control;

    using current_version_t = std::atomic<std::uint64_t>;
    class exportable : public gc_allocated {
      using gc_allocated::gc_allocated;
    };

    template <typename T, typename = void> struct is_exportable : std::false_type {};

    template <typename T>
    struct is_exportable<T, std::enable_if_t<std::is_base_of<exportable, T>::value>>
    : std::true_type
      {};

    /*
     * parent_view is used in merging.  If C extends P, a parent view child of C sees the parent
     * view of whatever view would see.
     */
    enum class view_type : unsigned char {
      live, snapshot
    };

    enum class mod_type : unsigned char {
      publishable, detached, read_only
    };

    class iso_context;
    class view;
    class publication_attempt;

    class snapshot;
    template <kind K> class record_field;
    class managed_type_base;
    template <kind K> class managed_type;
    class record_type;
    class conflict;
    using conflict_list = gc_threaded_list<gc_ptr<const conflict>>;
    class interned_string;
    template <std::size_t S> class interned_string_table;

    class managed_object : public exportable {
    public:
      using exportable::exportable;
    };
    class view_dependent_value : public managed_object {
    public:
      using managed_object::managed_object;
    };
    class managed_composite : public view_dependent_value {
    public:
      explicit managed_composite(gc_token &gc) : view_dependent_value{gc} {}
    };

    class managed_record;
    class managed_container;
    class managed_map;
    class managed_collection;
    class managed_list;
    class managed_set;

    class array_type_base;
    template <kind K> class array_type;
    class managed_array_base;
    template <kind K> class managed_array;

    // su - this avoids having a cyclical include between core_array and core_context
    using array_index_type = std::ptrdiff_t;

    template <typename T>
    constexpr bool is_value_type() {
      return std::is_arithmetic<T>() || std::is_base_of<managed_object,T>();
    }


    class incompatible_type_ex {};
    class incompatible_superclass_ex {};
    class unmodifiable_record_type_ex {};
    class incompatible_record_type_ex {
    public:
//      incompatible_record_type_ex() {
//        std::cerr << "Throwing incompatible_record_type_ex" << std::endl;
//      }
    };
    class unbound_name_ex {};
    class read_only_context_ex {};
    class unpublishable_context_ex {};

    template <kind K> struct kind_traits;
    template <kind K> using kind_val = typename kind_traits<K>::val_type;
    template <kind K> using kind_type = typename kind_traits<K>::type_t;
    template <kind K> using kind_field = typename kind_traits<K>::field_t;

    static constexpr std::size_t n_string_table_segments = 2;
    static constexpr std::size_t initial_string_table_capacity = 10000;
    using string_table_t = interned_string_table<n_string_table_segments>;
    template <> struct is_exportable<interned_string> : std::true_type {};

    class msv;
    template <kind K> class typed_msv;
    class value_chain;
    class task;
    class pending_rollup;
    class modified_value_chain;
    class modification;
    class blocking_mod;
    

    class name_space;
    class binding;

    template <kind K> class mod_condition;


    static constexpr std::size_t initial_record_type_table_capacity = 1000;
    using record_type_table_t = gc_cuckoo_map<gc_ptr<interned_string>, gc_ptr<const record_type>,
                                              ruts::hash1<gc_ptr<interned_string>>,
                                              ruts::hash2<gc_ptr<interned_string>>, 5>;



    template <typename T, typename Enable = void>
    struct managed_value_wrapper {
      using type = gc_ptr<T>;
    };
    template <typename T> using managed_value = typename managed_value_wrapper<T>::type;
    template <kind K> using kind_mv = managed_value<kind_val<K>>;

    template <typename T>
    struct managed_value_wrapper<T, std::enable_if_t<std::is_arithmetic<T>::value>>
    {
      using type = T;
    };

    template <>
    struct managed_value_wrapper<binding> {
      using type = binding;
    };

    template <typename T>
    struct vd_value {
      gc_ptr<T> value;
      gc_ptr<view> in_view;
      vd_value(const gc_ptr<T> &v, const gc_ptr<view> &b) : value{v}, in_view{b} {
        assert( (value == nullptr) == (in_view == nullptr) );
      }
      vd_value(nullptr_t)
        : value{nullptr}, in_view{nullptr}
      {}
      vd_value() : value{nullptr}, in_view{nullptr} {}
      static const auto &descriptor() {
        static gc_descriptor d =
	  GC_DESC(vd_value)
	  .template WITH_FIELD(&vd_value::value)
	  .template WITH_FIELD(&vd_value::in_view);
        return d;
      }

      bool operator ==(const vd_value &rhs) const {
        return value == rhs.value && in_view == rhs.in_view;
      }
      bool operator !=(const vd_value &rhs) const {
        return !((*this) == rhs);
      }
    };

    template <typename T, typename = void>
    struct managed_value_is_view_dependent : std::false_type
    {};

    template <typename T>
      struct managed_value_is_view_dependent<T, std::enable_if_t<std::is_base_of<managed_composite, T>::value>>
      : std::true_type
      {};
              

    template <typename T>
    struct managed_value_wrapper<T, std::enable_if_t<managed_value_is_view_dependent<T>::value>>
    {
      using type = vd_value<T>;
    };

    enum class validity : unsigned char {
      unchecked, valid, invalid
    };

    template <typename Fn>
    inline
    bool check_validity(validity &v, Fn &&fn) {
      if (v == validity::unchecked) {
        v = std::forward<Fn>(fn)() ? validity::valid : validity::invalid;
      }
      return v == validity::valid;
    }


  }
}

namespace std {
  template <typename C, typename Tr, typename T>
  basic_ostream<C,Tr> &
  operator <<(basic_ostream<C,Tr> &os, const mds::core::vd_value<T> &v) {
    return os << v.value << "{" << v.in_view << "}";
  }

  template <typename C, typename Tr>
  basic_ostream<C,Tr> &
  operator <<(basic_ostream<C,Tr> &os, mds::core::kind k) {
    using namespace mds::core;
    switch (k) {
    case kind::BOOL:
      return os << "BOOL";
    case kind::BYTE:
      return os << "BYTE";
    case kind::UBYTE:
      return os << "UBYTE";
    case kind::SHORT:
      return os << "SHORT";
    case kind::USHORT:
      return os << "USHORT";
    case kind::INT:
      return os << "INT";
    case kind::UINT:
      return os << "UINT";
    case kind::LONG:
      return os << "LONG";
    case kind::ULONG:
      return os << "ULONG";
    case kind::FLOAT:
      return os << "FLOAT";
    case kind::DOUBLE:
      return os << "DOUBLE";
    case kind::STRING:
      return os << "STRING";
    case kind::RECORD:
      return os << "RECORD";
    case kind::BINDING:
      return os << "BINDING";
    case kind::ARRAY:
      return os << "ARRAY";
    default:
      return os << "kind[" << static_cast<int>(k) << "]";
    }
  }

  template <typename C, typename Tr>
  basic_ostream<C,Tr> &
  operator <<(basic_ostream<C,Tr> &os, mds::core::view_type vt) {
    using namespace mds::core;
    switch (vt) {
    case view_type::live:
      return os << "live";
    case view_type::snapshot:
      return os << "snapshot";
    default:
      return os << "view_type[" << static_cast<int>(vt) << "]";
    }
  }

  template <typename C, typename Tr>
  basic_ostream<C,Tr> &
  operator <<(basic_ostream<C,Tr> &os, mds::core::mod_type mt) {
    using namespace mds::core;
    switch (mt) {
    case mod_type::publishable:
      return os << "full";
    case mod_type::detached:
      return os << "detached";
    case mod_type::read_only:
      return os << "read_only";
    default:
      return os << "mod_type[" << static_cast<int>(mt) << "]";
    }
  }

}



#endif /* CORE_FWD_H_ */
