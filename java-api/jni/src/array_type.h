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

#include "mds-debug.h"
#include "mds_core_api.h"                           // MDS Core API
#include "mds_jni.h"

using namespace mds;
using namespace mds::api;
using namespace mds::jni;

namespace mds
{
  namespace jni
  {
    namespace array_type
    {

      template<kind KIND>
	inline api_type<kind::LONG>
	lookup_handle (api_type<kind::LONG> nsHIndex, api_type<kind::LONG> nameHIndex)
	{
	  indexed<namespace_handle> ns
	    { nsHIndex };
	  indexed<interned_string_handle> name
	    { nameHIndex };
	  indexed<managed_array_handle<KIND>> val
	    { ns->lookup (*name, managed_array_handle_by_kind<KIND> ()) };
	  return val.return_index ();

	}

      template<kind KIND>
	inline api_type<kind::BOOL>
	bind_handle (api_type<kind::LONG> nsHIndex, api_type<kind::LONG> nameHIndex,
		     api_type<kind::LONG> valHandle)
	{
	  indexed<namespace_handle> ns
	    { nsHIndex };
	  indexed<interned_string_handle> name
	    { nameHIndex };
	  indexed<managed_array_handle<KIND>> val
	    { valHandle };
	  return ns->bind<kind::ARRAY> (*name, *val);
	}

      template<kind KIND>
	inline api_type<kind::BOOL>
	is_same_as (api_type<kind::LONG> aHIndex, api_type<kind::LONG> bHIndex)
	{
	  indexed<array_type_handle<KIND>> a
	    { aHIndex };
	  indexed<array_type_handle<KIND>> b
	    { bHIndex };
	  return a->is_same_as (*b);
	}

      template<kind KIND>
	inline api_type<kind::LONG>
	create_array (api_type<kind::LONG> size)
	{
	  indexed<managed_array_handle<KIND>> arr
	    { managed_array_handle_by_kind<KIND> ().create_array (size) };
	  return arr.return_index ();
	}

    }
  }
}
