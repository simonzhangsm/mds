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

package com.hpl.erk.iter;

import java.util.Iterator;
import java.util.List;

import com.hpl.erk.chain.Chain;
import com.hpl.erk.func.UnaryFunc;
import com.hpl.erk.tuple.Tuple;
import com.hpl.erk.tuple.Tuple2;
import com.hpl.erk.tuple.Tuple3;
import com.hpl.erk.tuple.Tuple4;
import com.hpl.erk.util.CollUtils;

public abstract class Lockstep<T> implements Iterable<T> {
  public enum EndWhen { 
    ALL_DONE {
      @Override
      boolean checkHasNext(Iterator<?>[] seqs) {
        for (Iterator<?> s : seqs) {
          if (s.hasNext()) {
            return true;
          }
        }
        return false;
      }
    }, 
    ANY_DONE {
      @Override
      boolean checkHasNext(Iterator<?>[] seqs) {
        for (Iterator<?> s : seqs) {
          if (!s.hasNext()) {
            return false;
          }
        }
        return true;
      }
    };
    abstract boolean checkHasNext(Iterator<?>[] seqs);
  };
  
  private final Iterable<?>[] seqs;
  private final EndWhen endWhen;

  protected abstract T makeTuple(Iterator<?>[] iters);
  
  protected Lockstep(EndWhen endWhen, Iterable<?>...seqs) {
    this.endWhen = endWhen;
    this.seqs = seqs;
  }

  @Override
  public Iterator<T> iterator() {
    return new Iterator<T>() {
      Iterator<?>[] iters = Chain.from(seqs).map(new UnaryFunc<Iterable<?>,Iterator<?>>() {
        @Override
        public Iterator<?> call(Iterable<?> val) {
          return val.iterator();
        }}).asArray(Iterator.class);
      @Override
      public boolean hasNext() {
        return endWhen.checkHasNext(iters);
      }

      @Override
      public T next() {
        return makeTuple(iters);
      }

      @Override
      public void remove() {
        throw new UnsupportedOperationException();
      }
    };
  }
  
  protected static <T> T next(Iterator<?>[] iters, int i) {
    @SuppressWarnings("unchecked")
    Iterator<T> iter = (Iterator<T>)iters[i];
    if (iter.hasNext()) {
      return iter.next();
    }
    return null;
  }

  public static <T1,T2>
  Lockstep<Tuple2<T1,T2>>
  over(EndWhen endWhen, Iterable<T1> s1, Iterable<T2> s2) {
    return new Lockstep<Tuple2<T1,T2>>(endWhen, s1, s2) {
      @Override
      protected Tuple2<T1,T2> makeTuple(Iterator<?>[] iters) {
        return Tuple.of(Lockstep.<T1>next(iters, 0),
                        Lockstep.<T2>next(iters, 1));
      }};
  }

  public static <T1,T2,T3>
  Lockstep<Tuple3<T1,T2,T3>>
  over(EndWhen endWhen, 
       Iterable<T1> s1, 
       Iterable<T2> s2,
       Iterable<T3> s3) {
    return new Lockstep<Tuple3<T1,T2,T3>>(endWhen, s1, s2, s3) {
      @Override
      protected Tuple3<T1,T2,T3> makeTuple(Iterator<?>[] iters) {
        return Tuple.of(Lockstep.<T1>next(iters, 0),
                        Lockstep.<T2>next(iters, 1),
                        Lockstep.<T3>next(iters, 2));
      }};
  }

  public static <T1,T2,T3,T4>
  Lockstep<Tuple4<T1,T2,T3,T4>>
  over(EndWhen endWhen, 
       Iterable<T1> s1, 
       Iterable<T2> s2,
       Iterable<T3> s3,
       Iterable<T4> s4) {
    return new Lockstep<Tuple4<T1,T2,T3,T4>>(endWhen, s1, s2, s3, s4) {
      @Override
      protected Tuple4<T1,T2,T3,T4> makeTuple(Iterator<?>[] iters) {
        return Tuple.of(Lockstep.<T1>next(iters, 0),
                        Lockstep.<T2>next(iters, 1),
                        Lockstep.<T3>next(iters, 2),
                        Lockstep.<T4>next(iters, 3));
      }};
  }
  
  public static <T1,T2> Lockstep<Tuple2<T1,T2>> 
  overAny(Iterable<T1> s1, Iterable<T2> s2) {
    return over(EndWhen.ALL_DONE, s1, s2);
  }
  public static <T1,T2> Lockstep<Tuple2<T1,T2>> 
  overAll(Iterable<T1> s1, Iterable<T2> s2) {
    return over(EndWhen.ANY_DONE, s1, s2);
  }

  public static <T1,T2,T3> Lockstep<Tuple3<T1, T2, T3>> 
  overAny(Iterable<T1> s1, Iterable<T2> s2, Iterable<T3> s3) {
    return over(EndWhen.ALL_DONE, s1, s2, s3);
  }
  public static <T1,T2,T3> Lockstep<Tuple3<T1, T2, T3>> 
  overAll(Iterable<T1> s1, Iterable<T2> s2, Iterable<T3> s3) {
    return over(EndWhen.ANY_DONE, s1, s2, s3);
  }

  public static <T1,T2,T3,T4> Lockstep<Tuple4<T1, T2, T3, T4>> 
  overAny(Iterable<T1> s1, Iterable<T2> s2, Iterable<T3> s3, Iterable<T4> s4) {
    return over(EndWhen.ALL_DONE, s1, s2, s3, s4);
  }
  public static <T1,T2,T3,T4> Lockstep<Tuple4<T1, T2, T3, T4>> 
  overAll(Iterable<T1> s1, Iterable<T2> s2, Iterable<T3> s3, Iterable<T4> s4) {
    return over(EndWhen.ANY_DONE, s1, s2, s3, s4);
  }

  public static void main(String[] args) {
    List<Integer> ints = CollUtils.listOf(1, 2, 3, 4, 5);
    List<String> strings = CollUtils.listOf("one", "two", "three");
    List<Double> doubles = CollUtils.listOf(0.5, 1.0, 1.5, 2.0);
    
    System.out.format("Stop after longest done:%n");
    for (Tuple2<Integer, String> tuple : Lockstep.overAny(ints, strings)) {
      System.out.format("  %s%n", tuple);
      System.out.format("    int: %s, string: %s%n", tuple.v1, tuple.v2);
    }
    
    System.out.format("Stop after shortest done:%n");
    for (Tuple3<Integer, String, Double> tuple : Lockstep.overAll(ints, strings, doubles)) {
      System.out.format("  %s%n", tuple);
      System.out.format("    int: %s, string: %s, double: %s%n", tuple.v1, tuple.v2, tuple.v3);
    }
  }
  
}
