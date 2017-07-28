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

package com.hpl.mds;

import com.hpl.mds.impl.ProxyState;


public interface ManagedRecord extends ManagedComposite {
  
  interface StaticAccess {}
  
  
  public static <T extends ManagedRecord> T create() {
	  System.out.println("ManagedRecord.create invoked");
	  return null;
  }
  
  public static <T extends ManagedRecord> T create(ProxyState proxyState) {
	  System.out.println("ManagedRecord.create(ProxyState) invoked");
	  // return (T) new ManagedRecordBaseImpl(proxyState);
          return null;
  }

  String defaultToString();
  
  default String __OBJECT_METHOD_toString() {
    return defaultToString();
  }

  int defaultHashCode();

  default
    int __OBJECT_METHOD_hashCode() {
    return defaultHashCode();
  }

  boolean defaultEquals(Object obj);

  default
    boolean __OBJECT_METHOD_equals(Object obj) {
    return defaultEquals(obj);
  }
}

