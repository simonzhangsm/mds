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

package com.hpl.mds.test.field;

import com.hpl.mds.ManagedArray;
import com.hpl.mds.annotations.RecordSchema;
import com.hpl.mds.prim.ManagedBoolean;
import com.hpl.mds.prim.ManagedByte;
import com.hpl.mds.prim.ManagedDouble;
import com.hpl.mds.prim.ManagedFloat;
import com.hpl.mds.prim.ManagedInt;
import com.hpl.mds.prim.ManagedLong;
import com.hpl.mds.prim.ManagedShort;
import com.hpl.mds.prim.container.array.ManagedBooleanArray;
import com.hpl.mds.prim.container.array.ManagedByteArray;
import com.hpl.mds.prim.container.array.ManagedDoubleArray;
import com.hpl.mds.prim.container.array.ManagedFloatArray;
import com.hpl.mds.prim.container.array.ManagedIntArray;
import com.hpl.mds.prim.container.array.ManagedLongArray;
import com.hpl.mds.prim.container.array.ManagedShortArray;
import com.hpl.mds.string.ManagedString;
import com.hpl.mds.string.ManagedStringArray;
import com.hpl.mds.test.RecordPlainSimple;

@RecordSchema
public interface FieldsArraysSchema {

    ManagedBooleanArray myBooleanArray();

    ManagedArray<ManagedBoolean> myBooleanArray2();

    ManagedByteArray myByteArray();

    ManagedArray<ManagedByte> myByteArray2();

    ManagedDoubleArray myDoubleArray();

    ManagedArray<ManagedDouble> myDoubleArray2();

    ManagedFloatArray myFloatArray();

    ManagedArray<ManagedFloat> myFloatArray2();

    ManagedIntArray myIntArray();

    ManagedArray<ManagedInt> myIntArray2();

    ManagedLongArray myLongArray();

    ManagedArray<ManagedLong> myLongArray2();

    ManagedShortArray myShortArray();

    ManagedArray<ManagedShort> myShortArray2();

    ManagedStringArray myStringArray();

    ManagedArray<ManagedString> myStringArray2();

    ManagedArray<RecordPlainSimple> myRecordArray();
}
