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

package com.hpl.mds.impl;

import com.hpl.mds.*;
import com.hpl.mds.usage.UsagePlan;
import com.hpl.mds.usage.UsageScope;


public class RecordArrayProxy<R extends ManagedRecord> extends ArrayProxy<R> {

  private static final NativeLibraryLoader NATIVE_LIB_LOADER = NativeLibraryLoader.getInstance();
  
  private final RecordArrayTypeProxy<R> type;
  private final RecordTypeProxy<R> eltType;

  private static native void release(long handle);
  private static native boolean isIdentical(long aHandle, long bHandle);
  private static native boolean isSameObject(long aHandle, long bHandle);
  private static native boolean isSameViewOfSameObject(long aHandle, long bHandle);
  private static native long createArray(long size, long typeHandle);

  private static native long getHandle(long handle, long index);
  private static native long peekHandle(long handle, long index);
  private static native long setHandle(long handle, long index, long val);
  private static native long getAndSetHandle(long handle, long index, long val);
  private static native long size(long handle);
  
  private static native String toString(long handle);

  /*

  @Override
  public String toString() {
    return toString(handleIndex());
  }
  */
 

  @Override
  void releaseHandleIndex(long index) {
    release(index); 
  }

  public enum FromHandle { FROM_HANDLE };

    
  protected RecordArrayProxy(FromHandle kwd, long handle, long size, RecordArrayTypeProxy<R> type) {
    super(handle, size);
    this.type = type;
    this.eltType = type.eltType();
  }
  protected RecordArrayProxy(FromHandle kwd, long handle, RecordArrayTypeProxy<R> type) {
    this(kwd, handle, -1, type);
  }


  @Override
  public ManagedArray.Type<R> type() {
    return type;
  }

  @Override
  public ManagedType<R> eltType() {
    return eltType;
  }

  @Override
  public long longSize() {
    if (size == -1) {
      size = size(handleIndex());
    }
    return size;
  }

  @Override
  public boolean isIdentical(ManagedComposite other) {
    if (other == this) {
      return true;
    } else if (other == null) {
      return false;
    } else if (!(other instanceof RecordArrayProxy)) {
      return false;
    }
    RecordArrayProxy<?> dc = (RecordArrayProxy<?>)other;
    return isIdentical(handleIndex(), dc.handleIndex());
  }


  @Override
  public boolean isSameObject(ManagedComposite other) {
    if (other == this) {
      return true;
    } else if (other == null) {
      return false;
    } else if (!(other instanceof RecordArrayProxy)) {
      return false;
    }
    RecordArrayProxy<?> dc = (RecordArrayProxy<?>)other;
    return isSameObject(handleIndex(), dc.handleIndex());
  }

  @Override
  public boolean isSameViewOfSameObject(ManagedComposite other) {
    if (other == this) {
      return true;
    } else if (other == null) {
      return false;
    } else if (!(other instanceof RecordArrayProxy)) {
      return false;
    }
    RecordArrayProxy<?> dc = (RecordArrayProxy<?>)other;
    return isSameViewOfSameObject(handleIndex(), dc.handleIndex());
  }

  
  @Override
  public R get(long index) {
	  return ManagedRecordProxy.fromHandle(getHandle(handleIndex(), index), eltType);
  }
  @Override
  public R peek(long index) {
	  return ManagedRecordProxy.fromHandle(peekHandle(handleIndex(), index), eltType);
  }
  @Override
  public R set(long index, R val) {
	  long old = setHandle(handleIndex(), index, ManagedRecordProxy.handleOf(val));
	  return ManagedRecordProxy.fromHandle(old, eltType);
  }
  @Override
  public R getAndSet(long index, R val) {
	  long old = getAndSetHandle(handleIndex(), index, ManagedRecordProxy.handleOf(val));
	  return ManagedRecordProxy.fromHandle(old, eltType);
  }
  
  @Override
  public RecordArrayProxy<R> bindName(Prior prior, Namespace ns, CharSequence name) {
    return (RecordArrayProxy<R>)type().bindIn(ns, name, this, prior);
  }

  static <R extends ManagedRecord>
    RecordArrayProxy<R> fromHandle(long handle, long size, RecordArrayTypeProxy<R> type) {
    RecordArrayProxy<R> a = new RecordArrayProxy<>(FromHandle.FROM_HANDLE, handle, size, type);
    return a;
  }      

  static <R extends ManagedRecord>
    RecordArrayProxy<R> fromHandle(long handle, RecordArrayTypeProxy<R> type) {
    return fromHandle(handle, -1, type);
  }

  public static <R extends ManagedRecord> long handleOf(ManagedArray<R> arr) {
    RecordArrayProxy<R> p = (RecordArrayProxy<R>)arr;
    return p == null ? 0 : p.handleIndex();  
  }
 @Override
  public ManagedArray<R> using(ManagedOrdered.Usage hint) {
    return Stub.notImplemented();
  }
  @Override
  public ManagedArray<R> inherentUsage(ManagedOrdered.Usage hint) {
    return Stub.notImplemented();
  }
  @Override
  public ManagedArray<R> usageDuring(UsageScope scope, ManagedOrdered.Usage hint) {
    return Stub.notImplemented();
  }
  @Override
  public UsagePlan usagePlan(ManagedOrdered.Usage hint) {
    return Stub.notImplemented();
  }
  @Override
  public ManagedArray<R> using(ManagedArray.Usage hint) {
    return Stub.notImplemented();
  }
  @Override
  public ManagedArray<R> inherentUsage(Usage hint) {
    return Stub.notImplemented();
  }
  @Override
  public ManagedArray<R> usageDuring(UsageScope scope, ManagedArray.Usage hint) {
    return Stub.notImplemented();
  }
  @Override
  public UsagePlan usagePlan(ManagedArray.Usage hint) {
    return Stub.notImplemented();
  }

}
