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

package com.hpl.erk.adt;

import java.util.ArrayList;
import java.util.IdentityHashMap;
import java.util.List;
import java.util.Map;

import com.hpl.erk.func.NullaryFunc;
import com.hpl.erk.func.UnaryFunc;
import com.hpl.erk.impl_helper.ObjectDescription;
import com.hpl.erk.util.CollUtils;
import com.hpl.erk.util.MapUtils;

public class ForClass<T> {
  private final List<Binding> bindings = new ArrayList<>();
  private Map<Class<?>, T> cache;
  private final NullaryFunc<? extends Map<Class<?>, T>> cacheCreator = MapUtils.<Class<?>, T>identityHashMapCreator();
  
  private class Binding {
    final Class<?> clss;
    final T val;
    private Binding(Class<?> clss, T val) {
      this.clss = clss;
      this.val = val;
    }
    @Override
    public String toString() {
      return String.format("%s=%s",clss,val);
    }
  }
  
  public T getForClass(Class<?> c) {
    if (MapUtils.maybeNullMap(cache).containsKey(c)) {
      return cache.get(c);
    }
    Binding best = null;
    for (Binding b : bindings) {
      if (b.clss.isAssignableFrom(c)) {
        if (best == null || best.clss.isAssignableFrom(b.clss)) {
          best = b;
        }
      }
    }
    T val = best == null ? null : best.val;
    cache = MapUtils.putIn(cache, c, val, cacheCreator);
    return val;
  }
  
  public T getFor(Object obj) {
    return getForClass(obj.getClass());
  }
  
  public ForClass<T> bind(Class<?> c, T val) {
    cache = null;
    bindings.add(new Binding(c, val));
    return this;
  }
  
  public ForClass<T> def(T val) {
    return bind(Object.class, val);
  }
  
  public ForClass<T> prune() {
    Map<Class<?>, Binding> map = new IdentityHashMap<>();
    for (Binding b : bindings) {
      map.put(b.clss, b);
    }
    if (map.size() < bindings.size()) {
      bindings.clear();
      bindings.addAll(map.values());
    }
    return this;
  }

  @Override
  public String toString() {
    final UnaryFunc<Binding,String> dropClassKwd = new UnaryFunc<Binding,String>() {
      @Override
      public String call(Binding b) {
        return String.format("%s=%s", b.clss.getName(), b.val);
      }
     };
    final UnaryFunc<Class<?>,String> dropKwd = new UnaryFunc<Class<?>,String>(){
      @Override
      public String call(Class<?> c) {
        return c.getName();
      }
    };
    return ObjectDescription.brackets()
        .classHead(this)
        .kvSep(": ")
        .add("binding", CollUtils.map(bindings, dropClassKwd))
        .add("cached", CollUtils.map(MapUtils.maybeNullMap(cache).keySet(), dropKwd))
        .toString();
    
  }
}
