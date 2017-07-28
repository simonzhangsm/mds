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

import java.util.List;


public interface ManagedObject {
  interface ForManagedObject {}
  
  boolean isSameAs(ManagedObject other);
  
  boolean equals(Object obj);
  int hashCode();
  
  ManagedType<?> type();
  
  ManagedObject bindName(Prior prior, Namespace ns, CharSequence name);
  default ManagedObject bindName(Namespace ns, CharSequence name) {
    return bindName(Prior.wasAny(), ns, name);
  };

  
  default ManagedObject bindName(HName name) {
    return name.bind(this);
  }
  default ManagedObject bindName(CharSequence...segments) {
    return bindName(HName.from(segments));
  }
  default ManagedObject bindName(char sep, CharSequence...segments) {
    return bindName(HName.from(sep, segments));
  }
  default ManagedObject bindName(List<? extends CharSequence> segments) {
    return bindName(HName.from(segments));
  }
  default ManagedObject bindName(char sep, List<? extends CharSequence> segments) {
    return bindName(HName.from(sep, segments));
  }
  
  default ManagedObject bindName(Prior prior, HName name) {
    return name.bind(this, prior);
  }
  default ManagedObject bindName(Prior prior, CharSequence...segments) {
    return bindName(prior, HName.from(segments));
  }
  default ManagedObject bindName(Prior prior, char sep, CharSequence...segments) {
    return bindName(prior, HName.from(sep, segments));
  }
  default ManagedObject bindName(Prior prior, List<? extends CharSequence> segments) {
    return bindName(prior, HName.from(segments));
  }
  default ManagedObject bindName(Prior prior, char sep, List<? extends CharSequence> segments) {
    return bindName(prior, HName.from(sep, segments));
  }
  

  
   
}
