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
 * core_control_block.cpp
 *
 *  Created on: Nov 13, 2014
 *      Author: evank
 */

#include "core/mds_core.h"
#include "mpgc/gc.h"

namespace mds {
  namespace core {
    bool process_registered = false;

    current_version_t *current_version;
    external_gc_ptr<view> top_level_view;
    external_gc_ptr<iso_context> global_context;
    external_gc_ptr<string_table_t> string_table;
    external_gc_ptr<record_type_table_t> record_type_table;
    external_gc_ptr<name_space> global_namespace;
    std::size_t *next_task_number;
    
    external_gc_ptr<const managed_type<kind::BOOL>> managed_bool_type;
    external_gc_ptr<const managed_type<kind::BYTE>> managed_byte_type;
    external_gc_ptr<const managed_type<kind::UBYTE>> managed_ubyte_type;
    external_gc_ptr<const managed_type<kind::SHORT>> managed_short_type;
    external_gc_ptr<const managed_type<kind::USHORT>> managed_ushort_type;
    external_gc_ptr<const managed_type<kind::INT>> managed_int_type;
    external_gc_ptr<const managed_type<kind::UINT>> managed_uint_type;
    external_gc_ptr<const managed_type<kind::LONG>> managed_long_type;
    external_gc_ptr<const managed_type<kind::ULONG>> managed_ulong_type;
    external_gc_ptr<const managed_type<kind::FLOAT>> managed_float_type;
    external_gc_ptr<const managed_type<kind::DOUBLE>> managed_double_type;
    external_gc_ptr<const managed_type<kind::STRING>> managed_string_type;

    class control : public gc_allocated {
      friend bool register_process();
      const static mpgc::persistent_root_key gc_root_key;

      current_version_t _current_version { 0 };
      gc_ptr<iso_context> _global_context = make_gc<iso_context>(iso_context::private_ctor{}, iso_context::global);
      gc_ptr<view> _top_level_view = make_gc<view>(_global_context, nullptr);
      gc_ptr<string_table_t> _string_table = make_gc<string_table_t>(initial_string_table_capacity);
      gc_ptr<record_type_table_t> _record_type_table = make_gc<record_type_table_t>(initial_record_type_table_capacity);
      gc_ptr<name_space> _global_namespace = make_gc<name_space>();
      std::size_t _next_task_number;

      gc_ptr<const managed_type<kind::BOOL>> _mtype_bool = make_gc<managed_type<kind::BOOL>>();
      gc_ptr<const managed_type<kind::BYTE>> _mtype_byte = make_gc<managed_type<kind::BYTE>>();
      gc_ptr<const managed_type<kind::UBYTE>> _mtype_ubyte = make_gc<managed_type<kind::UBYTE>>();
      gc_ptr<const managed_type<kind::SHORT>> _mtype_short = make_gc<managed_type<kind::SHORT>>();
      gc_ptr<const managed_type<kind::USHORT>> _mtype_ushort = make_gc<managed_type<kind::USHORT>>();
      gc_ptr<const managed_type<kind::INT>> _mtype_int = make_gc<managed_type<kind::INT>>();
      gc_ptr<const managed_type<kind::UINT>> _mtype_uint = make_gc<managed_type<kind::UINT>>();
      gc_ptr<const managed_type<kind::LONG>> _mtype_long = make_gc<managed_type<kind::LONG>>();
      gc_ptr<const managed_type<kind::ULONG>> _mtype_ulong = make_gc<managed_type<kind::ULONG>>();
      gc_ptr<const managed_type<kind::FLOAT>> _mtype_float = make_gc<managed_type<kind::FLOAT>>();
      gc_ptr<const managed_type<kind::DOUBLE>> _mtype_double = make_gc<managed_type<kind::DOUBLE>>();
      gc_ptr<const managed_type<kind::STRING>> _mtype_string = make_gc<managed_type<kind::STRING>>();

    public:

      control(gc_token &gc) : gc_allocated(gc) {}
      // QUESTION:  Can I do this in registration and then simply assume that it's all initialized and
      // just use static vars?
      static const auto &descriptor() {
        static gc_descriptor d =
	  GC_DESC(control)
	  .WITH_FIELD(&control::_current_version)
	  .WITH_FIELD(&control::_global_context)
	  .WITH_FIELD(&control::_top_level_view)
	  .WITH_FIELD(&control::_string_table)
	  .WITH_FIELD(&control::_record_type_table)
	  .WITH_FIELD(&control::_global_namespace)
	  .WITH_FIELD(&control::_next_task_number)
	  .WITH_FIELD(&control::_mtype_bool)
	  .WITH_FIELD(&control::_mtype_byte)
	  .WITH_FIELD(&control::_mtype_ubyte)
	  .WITH_FIELD(&control::_mtype_short)
	  .WITH_FIELD(&control::_mtype_ushort)
	  .WITH_FIELD(&control::_mtype_int)
	  .WITH_FIELD(&control::_mtype_uint)
	  .WITH_FIELD(&control::_mtype_long)
	  .WITH_FIELD(&control::_mtype_ulong)
	  .WITH_FIELD(&control::_mtype_double)
	  .WITH_FIELD(&control::_mtype_float)
	  .WITH_FIELD(&control::_mtype_string)
          ;
        return d;
      }

    };

    const mpgc::persistent_root_key control::gc_root_key("MDS Control Block Root Key");


    external_gc_ptr<control> control_block = nullptr;

    bool register_process() {
      mpgc::initialize();//GC allocator initialization
      bool already_registered = true;
      static std::once_flag done;
      std::call_once(done, [&](){
        control_block = mpgc::persistent_roots().find_or_create<control>(control::gc_root_key);
        control &cb = *control_block;
	
        current_version = &cb._current_version;
        top_level_view = cb._top_level_view;
	assert(top_level_view.value().is_valid());
        global_context = cb._global_context;
	assert(global_context.value().is_valid());
        string_table = cb._string_table;
	assert(string_table.value().is_valid());
        record_type_table = cb._record_type_table;
	assert(record_type_table.value().is_valid());
        global_namespace = cb._global_namespace;
	assert(global_namespace.value().is_valid());
        next_task_number = &cb._next_task_number;
        managed_bool_type = cb._mtype_bool;
	assert(managed_bool_type.value().is_valid());
        managed_byte_type = cb._mtype_byte;
	assert(managed_byte_type.value().is_valid());
        managed_ubyte_type = cb._mtype_ubyte;
	assert(managed_ubyte_type.value().is_valid());
        managed_short_type = cb._mtype_short;
	assert(managed_short_type.value().is_valid());
        managed_ushort_type = cb._mtype_ushort;
	assert(managed_ushort_type.value().is_valid());
        managed_int_type = cb._mtype_int;
	assert(managed_int_type.value().is_valid());
        managed_uint_type = cb._mtype_uint;
	assert(managed_uint_type.value().is_valid());
        managed_long_type = cb._mtype_long;
	assert(managed_long_type.value().is_valid());
        managed_ulong_type = cb._mtype_ulong;
	assert(managed_ulong_type.value().is_valid());
        managed_float_type = cb._mtype_float;
	assert(managed_float_type.value().is_valid());
        managed_double_type = cb._mtype_double;
	assert(managed_double_type.value().is_valid());
        managed_string_type = cb._mtype_string;
	assert(managed_string_type.value().is_valid());

        already_registered = false;
        process_registered = true;
        /*
         * TODO: Probably need some sort of call to atomic_thread_fence to ensure that these
         * writes actually make it out to the memory before anybody starts to read them.
         * At the very least, all of the others need to make it out before the set of
         * process_registered does.
         */
      });
      return already_registered;
    }

    bool unregister_process() {
      bool already_unregistered = true;
      static std::once_flag done;
      std::call_once(done, [&](){
        already_unregistered = false;
      });
      return already_unregistered;

    }


  }
}
