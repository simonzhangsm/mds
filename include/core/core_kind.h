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
 * core_kind.h
 *
 *  Created on: Oct 21, 2014
 *      Author: evank
 */

#ifndef CORE_KIND_H_
#define CORE_KIND_H_

#include "core/core_fwd.h"
#include <array>

namespace mds {
  namespace core {
    template <kind K, typename VT>
    struct kind_traits_val_only {
      using val_type = VT;
    };
    template <kind K, typename VT>
    struct kind_traits_base : public kind_traits_val_only<K,VT> {
      constexpr static kind k = K;
      using type_t = managed_type<K>;
      using field_t = record_field<K>;
    };
    template <> struct kind_traits<kind::BOOL> : kind_traits_base<kind::BOOL, bool_t> {};
    template <> struct kind_traits<kind::BYTE> : kind_traits_base<kind::BYTE, byte_t> {};
    template <> struct kind_traits<kind::UBYTE> : kind_traits_base<kind::UBYTE, ubyte_t> {};
    template <> struct kind_traits<kind::SHORT> : kind_traits_base<kind::SHORT, short_t> {};
    template <> struct kind_traits<kind::USHORT> : kind_traits_base<kind::USHORT, ushort_t> {};
    template <> struct kind_traits<kind::INT> : kind_traits_base<kind::INT, int_t> {};
    template <> struct kind_traits<kind::UINT> : kind_traits_base<kind::UINT, uint_t> {};
    template <> struct kind_traits<kind::LONG> : kind_traits_base<kind::LONG, long_t> {};
    template <> struct kind_traits<kind::ULONG> : kind_traits_base<kind::ULONG, ulong_t> {};
    template <> struct kind_traits<kind::FLOAT> : kind_traits_base<kind::FLOAT, float_t> {};
    template <> struct kind_traits<kind::DOUBLE> : kind_traits_base<kind::DOUBLE, double_t> {};
    template <> struct kind_traits<kind::RECORD> : kind_traits_base<kind::RECORD, managed_record> {
      using type_t = record_type;
    };
    template <> struct kind_traits<kind::STRING> : kind_traits_base<kind::STRING, interned_string> {};
    template <> struct kind_traits<kind::BINDING> : kind_traits_val_only<kind::BINDING, binding> {};
    template <> struct kind_traits<kind::ARRAY> : kind_traits_base<kind::ARRAY, managed_array_base> {
      using type_t = array_type_base;
    };
    template <> struct kind_traits<kind::NAMESPACE> : kind_traits_base<kind::NAMESPACE, name_space> {
      //      using type_t = array_type_base;
    };


    /*
     * A process local class (so can have virtual functions).
     *
     */
    struct kind_dispatch {
      static const std::array<std::unique_ptr<const kind_dispatch>,n_kinds> &table();
      static const kind_dispatch *lookup(kind k) {
        return table()[static_cast<std::size_t>(k)].get();
      }
      virtual ~kind_dispatch() {}
      /*
       * Called from mdb::same_type_as().  Already checked that kinds match.
       */
      virtual bool same_type(const gc_ptr<const managed_type_base> &lhs,
                             const gc_ptr<const managed_type_base> &rhs) const = 0;
    };

  }
}




#endif /* CORE_KIND_H_ */
