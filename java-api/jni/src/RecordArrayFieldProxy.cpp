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

/* C++ code implementing native methods of Java class:
 *   com.hpl.mds.impl.RecordArrayFieldProxy
 */

#include <jni.h>
#include "mds_core_api.h"                              // MDS Core API
#include "mds-debug.h"                            // #define dout cout
#include "mds_jni.h"                              // MDS Java API JNI common fns

using namespace mds;
using namespace mds::api;
using namespace mds::jni;

extern "C"
{

  JNIEXPORT
  jlong
  JNICALL
  Java_com_hpl_mds_impl_RecordArrayFieldProxy_createFieldIn (
      JNIEnv *jEnv, jclass, jlong recTypeHIndex, jlong nameHIndex,
      jlong valTypeHIndex)
  {
    ensure_thread_initialized(jEnv);
    return exception_handler_wr (jEnv, [=]
      {
	indexed<record_type_handle> rec_type
	  { recTypeHIndex};
	indexed<interned_string_handle> name
	  { nameHIndex};
	indexed<array_type_handle<kind::RECORD>> val_type
	  { valTypeHIndex};
	indexed<record_field_handle<kind::ARRAY>> h
	  { val_type->field_in(*rec_type, *name, true)};
	return h.return_index();
      });
  }

  JNIEXPORT
  jlong
  JNICALL
  Java_com_hpl_mds_impl_RecordArrayFieldProxy_valTypeHandle (JNIEnv *jEnv,
							     jclass,
							     jlong hIndex)
  {
    ensure_thread_initialized(jEnv);
    return exception_handler_wr (
	jEnv, [=]
	  {
	    // TODO: move downcast into handler
	indexed<record_field_handle<kind::ARRAY>> h
	  { hIndex};
	auto tp = h->field_type();
	auto tpdc = tp.pointer()->downcast<kind::RECORD>();
	const_array_type_handle<kind::RECORD> tph(tpdc);
	indexed<const_array_type_handle<kind::RECORD>> type(tph);
	return type.return_index();
      });
  }

  JNIEXPORT
  jlong
  JNICALL
  Java_com_hpl_mds_impl_RecordArrayFieldProxy_getValueHandle (JNIEnv *jEnv,
							      jclass,
							      jlong hIndex,
							      jlong recHIndex)
  {
    ensure_thread_initialized(jEnv);
    return exception_handler_wr (
	jEnv, [=]
	  {
	    indexed<record_field_handle<kind::ARRAY>> h
	      { hIndex};
	    indexed<managed_record_handle> rec
	      { recHIndex};
	    // TODO: move downcast into handler
            auto v = h->frozen_read(*rec);
            managed_array_handle<kind::RECORD> vdc;
            if (v != nullptr)
              {
                managed_array_handle<kind::RECORD> vdc2(v.pointer()->downcast<kind::RECORD>(),
                                                        v.view());
                vdc = vdc2;
              }
            indexed<managed_array_handle<kind::RECORD>> val(vdc);
        
            return val.return_index();
          });
  }

  JNIEXPORT
  jlong
  JNICALL
  Java_com_hpl_mds_impl_RecordArrayFieldProxy_peekValueHandle (JNIEnv *jEnv,
                                                               jclass,
                                                               jlong hIndex,
                                                               jlong recHIndex)
  {
    ensure_thread_initialized(jEnv);
    return exception_handler_wr (
	jEnv, [=]
	  {
	    indexed<record_field_handle<kind::ARRAY>> h
	      { hIndex};
	    indexed<managed_record_handle> rec
	      { recHIndex};
	    // TODO: move downcast into handler
            auto v = h->free_read(*rec);
            managed_array_handle<kind::RECORD> vdc;
            if (v != nullptr)
              {
                managed_array_handle<kind::RECORD> vdc2(v.pointer()->downcast<kind::RECORD>(),
                                                        v.view());
                vdc = vdc2;
              }
            indexed<managed_array_handle<kind::RECORD>> val(vdc);
        
            return val.return_index();
          });
  }

  JNIEXPORT
  jlong
  JNICALL
  Java_com_hpl_mds_impl_RecordArrayFieldProxy_setValueHandle (JNIEnv *jEnv,
							      jclass,
							      jlong hIndex,
							      jlong recHIndex,
							      jlong valArg)
  {
    ensure_thread_initialized(jEnv);
    return exception_handler_wr (
	jEnv,
	[=]
	  {
	    indexed<record_field_handle<kind::ARRAY>> h
	      { hIndex};
	    indexed<managed_record_handle> rec
	      { recHIndex};
	    indexed<managed_array_handle<kind::RECORD>> val
	      { valArg};
	    auto v = h->write(*rec, *val);
	    managed_array_handle<kind::RECORD> vdc;
	    if (v != nullptr)
	      {
		managed_array_handle<kind::RECORD> vdc2(v.pointer()->downcast<kind::RECORD>(),
		    v.view());
		vdc = vdc2;
	      }
	    indexed<managed_array_handle<kind::RECORD>> old(vdc);
	    return old.return_index();
	  });
  }

  JNIEXPORT
  jboolean
  JNICALL
  Java_com_hpl_mds_impl_RecordArrayFieldProxy_initFinal (JNIEnv *jEnv,
                                                         jclass,
                                                         jlong hIndex,
                                                         jlong recHIndex,
                                                         jlong valArg)
  {
    ensure_thread_initialized(jEnv);
    return exception_handler_wr (
	jEnv,
	[=]
	  {
	    indexed<record_field_handle<kind::ARRAY>> h
	      { hIndex};
	    indexed<managed_record_handle> rec
	      { recHIndex};
	    indexed<managed_array_handle<kind::RECORD>> val
	      { valArg};
	    return h->write_initial(*rec, *val);
	  });
  }

  JNIEXPORT
  jlong
  JNICALL
  Java_com_hpl_mds_impl_RecordArrayFieldProxy_getAndSetValueHandle (JNIEnv *jEnv,
                                                                    jclass,
                                                                    jlong hIndex,
                                                                    jlong recHIndex,
                                                                    jlong valArg)
  {
    ensure_thread_initialized(jEnv);
    return exception_handler_wr (
	jEnv,
	[=]
	  {
	    indexed<record_field_handle<kind::ARRAY>> h
	      { hIndex};
	    indexed<managed_record_handle> rec
	      { recHIndex};
	    indexed<managed_array_handle<kind::RECORD>> val
	      { valArg};
	    auto v = h->write(*rec, *val, ret_mode::prior_val);
	    managed_array_handle<kind::RECORD> vdc;
	    if (v != nullptr)
	      {
		managed_array_handle<kind::RECORD> vdc2(v.pointer()->downcast<kind::RECORD>(),
		    v.view());
		vdc = vdc2;
	      }
	    indexed<managed_array_handle<kind::RECORD>> old(vdc);
	    return old.return_index();
	  });
  }

  JNIEXPORT jboolean JNICALL
  Java_com_hpl_mds_impl_RecordArrayFieldProxy_changeValueHandle (JNIEnv *jEnv,
								 jclass, jlong,
                                                                 jlong,
								 jlong, jlong,
								 jobject);

}
