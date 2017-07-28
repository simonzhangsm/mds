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

import com.hpl.mds.IsolationContext;
import com.hpl.mds.impl.TaskProxy;
import java.util.function.Supplier;
import java.util.function.BooleanSupplier;
import java.util.function.Consumer;
import java.util.function.LongConsumer;
import java.util.function.Function;
import java.util.Collection;
import com.hpl.erk.util.CollUtils;
import java.util.function.Predicate;

public interface Task {

  IsolationContext getContext();
  void dependsOn(Task... others);
  void dependsOn(Collection<? extends Task> others);
  
  void alwaysRedo();
  void cannotRedo();
  void establishAndRun(Runnable fn);
  <T> void establishAndAccept(Consumer<? super T> fn, T val);
  void establishAndAccept(LongConsumer fn, long val);
  <T> T establishAndGet(Supplier<T> fn);
  <T,R> R establishAndApply(Function<? super T, R> fn, T arg);
  
  static Task current() {
    return TaskProxy.current();
  }

  static void asTask(TaskOption opts, Runnable fn) {
    TaskProxy.run(opts, fn);
  }

  static void asTask(Runnable fn) {
    asTask(TaskOption.defaultOpts(), fn);
  }

  /*
   * This could return the value, without adding the dependency (by
   * having the runnable stash the value in a holder object and return
   * it) but that's probably more confusing than is warranted.  The
   * programmer can easily do that if desired.
   */
  public static void asTask(TaskOption opts, Supplier<?> fn) {
    asTask(opts, (Runnable)(() -> fn.get()));
  }

  public static void asTask(Supplier<?> fn) {
    asTask(TaskOption.defaultOpts(), fn);
  }

  /*
   * I was also going to add an inTask(fn) that took a supplier and
   * returned a value, implemented as computedValue(fn).get(), but
   * there's no real point.  Since the parent would wind up dependent
   * on the child, you might as well just call the function directly.
   */

  static <T> TaskComputed<T>
    computedValue(Function<? super T, ? extends T> fn)
  {
    return TaskProxy.computedValue(fn);
  }

  static <T> TaskComputed<T> computedValue(Supplier<? extends T> fn) {
    return computedValue(old -> fn.get());
  }

  /*
   * runnable() and consumer() are designed to take an existing
   * function and run it in a new task each time it is called.
   * consumer(), especially, is designed to be passed to
   * Iterable.forEach() or Stream.forEach() to allow independent
   * modification of elements.
   */
  static Runnable runnable(TaskOption opts, Runnable fn) {
    return ()->asTask(opts, fn);
  }

  static Runnable runnable(Runnable fn) {
    return runnable(TaskOption.defaultOpts(), fn);
  }

  static Runnable runnable(TaskOption opts, Supplier<?> fn) {
    return ()->asTask(opts, fn);
  }

  static Runnable runnable(Supplier<?> fn) {
    return runnable(TaskOption.defaultOpts(), fn);
  }

  static <T> Consumer<T> consumer(TaskOption opts, Consumer<? super T> fn) {
    return (x)->asTask(opts, ()->fn.accept(x));
  }

  static <T> Consumer<T> consumer(Consumer<? super T> fn) {
    return consumer(TaskOption.defaultOpts(), fn);
  }

  static LongConsumer consumer(TaskOption opts, LongConsumer fn) {
    return (x)->asTask(opts, ()->fn.accept(x));
  }

  static LongConsumer consumer(LongConsumer fn) {
    return consumer(TaskOption.defaultOpts(), fn);
  }

  static <T> Consumer<T> consumer(TaskOption opts, Function<? super T,?> fn) {
    return (x)->asTask(opts, ()->fn.apply(x));
  }

  static <T> Consumer<T> consumer(Function<? super T,?> fn) {
    return consumer(TaskOption.defaultOpts(), fn);
  }


  /*
   * inCurrent() creates a function bound to the current task.  It is
   * designed to create a function that can be submitted to an
   * executor pool, where you have no way of knowing what thread the
   * function will eventually run in.  Note that this function is not
   * independently re-runnable.  It is assumed that if the task is
   * re-run it will spawn a new set of functions.  If you want
   * re-runnable subtasks of the current task, say, e.g.,
   * `Task.inCurrent(Task.consumer(fn))`.  This will create a function
   * that first binds to the current task, then creates a subtask, and
   * finally calls `fn`.
   */

  static Runnable inCurrent(Runnable fn) {
    Task task = current();
    return ()->task.establishAndRun(fn);
  }

  static <T> Supplier<T> inCurrent(Supplier<? extends T> fn) {
    Task task = current();
    return ()->task.establishAndGet(fn);
  }
    
  static <T> Consumer<T> inCurrent(Consumer<? super T> fn) {
    Task task = current();
    return (x)->task.establishAndAccept(fn, x);
  }
    
  static LongConsumer inCurrent(LongConsumer fn) {
    Task task = current();
    return (x)->task.establishAndAccept(fn, x);
  }
    
  static <T,R> Function<T,R> inCurrent(Function<? super T, ? extends R> fn) {
    Task task = current();
    return (x)->task.establishAndApply(fn, x);
  }

  /*
   * Code is run after the task has been selected to be re-run for
   * conflict resolution but before any modified values are rolled
   * back.  Execution takes place in the top-level task of the
   * publishing context.  If the function returns false, conflict
   * resolution is abandoned.
   */
  void onPrepareForRedo(Predicate<? super TaskProxy> pred);

  default void onPrepareForRedo(Consumer<? super TaskProxy> fn) {
    onPrepareForRedo(t->{
        fn.accept(t);
        return true;
      });
  }
  
  default void onPrepareForRedo(Runnable fn) {
    onPrepareForRedo(t->{
        fn.run();
        return true;
      });
  }

  default void onPrepareForRedo(BooleanSupplier fn) {
    onPrepareForRedo(t->{
        return fn.getAsBoolean();
      });
        
  }

  
} // end class Task
