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

import java.util.Objects;

import com.hpl.mds.*;

import com.hpl.mds.callbacks.ChangeHandler;
import com.hpl.mds.callbacks.ChangeHandlerRemovalHook;
import com.hpl.mds.callbacks.FieldChange;

public abstract class FieldProxy<RT extends ManagedRecord, FT extends ManagedObject> extends Proxy implements Field<RT,FT> {
  
  protected ManagedStringProxy name_;
  protected RecordTypeProxy<RT> recordType_;
  
	protected FieldProxy(long handleIndex, RecordTypeProxy<RT> recType, ManagedStringProxy name) {
          super(handleIndex, null);
		recordType_ = recType;
		name_ = name;
	}
	
	@Override
	public FT apply(RT rec) {
	  return get(rec);
	}

	

  @Override
  public final boolean change(RT rec, FT expected, FT val) {
    return Stub.notImplemented();
  }

  @Override
  public final boolean change(RT rec, Holder<FT> holdsExpected, FT val) {
    return Stub.notImplemented();
  }

  @Override
  public final
    ChangeHandlerRemovalHook afterChange(ChangeHandler<? super FieldChange<? super RT, ? super FT>> handler) {
    return Stub.notImplemented();
  }

}
