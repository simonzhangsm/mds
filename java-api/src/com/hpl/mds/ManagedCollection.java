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

import java.util.Collection;
import java.util.Iterator;
import java.util.Objects;

import com.hpl.mds.impl.Stub;
import com.hpl.mds.usage.Operation;
import com.hpl.mds.usage.UsagePlan;
import com.hpl.mds.usage.UsageScope;

public interface ManagedCollection<M extends ManagedObject> extends ManagedContainer, Collection<M> {
  public static final Operation<ForManagedCollection> ITERATE = Stub.notImplemented();
  
  public static interface Type<M extends ManagedObject, C extends ManagedCollection<M>> extends ManagedType<C> {
    public ManagedType<M> eltType();
  }
  
  public interface ForManagedCollection extends ManagedContainer.ForManagedContainer {}
  
  public interface UsageOps<UF extends ForManagedCollection, U extends UsageOps<UF, U>> extends ManagedContainer.UsageOps<UF, U> {
  }
  
  public interface Usage extends UsageOps<ForManagedCollection, Usage> {}
  
  static Usage usage() {
    // TODO
    return Stub.notImplemented();
  }
  
  
  
  
  

  public ManagedType<M> eltType();
  
  @Override public default int size() {
    return ManagedContainer.super.size();
  }

  @Override
  public default boolean isEmpty() {
    return longSize() == 0;
  }

  @Override
  public default boolean containsAll(Collection<?> c) {
    Objects.requireNonNull(c);
    for (Object o : c) {
      if (!contains(o)) {
        return false;
      }
    }
    return true;
  }

  @Override
  public default boolean addAll(Collection<? extends M> c) {
    Objects.requireNonNull(c);
    boolean changed = false;
    for (M m : c) {
      if (add(m)) {
        changed = true;
      }
    }
    return changed;
  }

  @Override
  public default boolean removeAll(Collection<?> c) {
    Objects.requireNonNull(c);
    boolean changed = false;
    for (Object o : c) {
      if (remove(o)) {
        changed = true;
      }
    }
    return changed;
  }

  @Override
  public default boolean retainAll(Collection<?> c) {
    Objects.requireNonNull(c);
    boolean changed = false;
    for (Iterator<M> iter = iterator(); iter.hasNext();) {
      M elt = iter.next();
      if (!c.contains(elt)) {
        iter.remove();
        changed = true;
      }
    }
    return changed;
  }

  ManagedCollection<M> using(Usage hint);
  ManagedCollection<M> inherentUsage(Usage hint);
  ManagedCollection<M> usageDuring(UsageScope scope, Usage hint);
  UsagePlan usagePlan(Usage hint);

  default ManagedCollection<M> using(ManagedContainer.Usage hint) {
    return using(hint.cast(Usage.class));
  }
  default ManagedCollection<M> inherentUsage(ManagedContainer.Usage hint) {
    return inherentUsage(hint.cast(Usage.class));
  }
  default ManagedCollection<M> usageDuring(UsageScope scope, ManagedContainer.Usage hint) {
    return usageDuring(scope, hint.cast(Usage.class));
  }
  default UsagePlan usagePlan(ManagedContainer.Usage hint) {
    return usagePlan(hint.cast(Usage.class));
  }


}
