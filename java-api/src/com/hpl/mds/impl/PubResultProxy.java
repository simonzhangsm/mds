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

import java.util.HashSet;
import java.util.Set;
import java.util.Map;
import java.util.Arrays;
import java.util.Collection;

import org.apache.log4j.Logger;


public class PubResultProxy extends Proxy implements PubResult {

  private static final NativeLibraryLoader NATIVE_LIB_LOADER = NativeLibraryLoader.getInstance();

  private static final Logger log = Logger.getLogger(PubResultProxy.class);

  private static final Proxy.Table<PubResultProxy> 
    proxyTable = new Proxy.Table<>(PubResultProxy::release);
	

  private static native void release(long h);
  private static native boolean succeeded(long hindex);
  private static native long sourceContextIndex(long hindex);
  private static native long nToRedo(long hindex);
  private static native long[] redoTasksByStartTime(long hindex);
  private static native boolean prepareForRedo(long hindex);

  private IsoContextProxy sourceContext;
  private IsoContextProxy targetContext;
  private TaskProxy[] redoTasks;

  private PubResultProxy(long h) {
    super(h, proxyTable);
  }

  long getHandle() {
    return handleIndex_;
  }

  @Override
  void releaseHandleIndex(long index) {
    proxyTable.release(index);
  }

  public static PubResultProxy fromHandle(long handleIndex) {
    return proxyTable.fromIndex(handleIndex, PubResultProxy::new);
  }

  @Override
  public IsoContextProxy sourceContext() {
    if (sourceContext == null) {
      sourceContext = IsoContextProxy.fromHandle(sourceContextIndex(handleIndex_));
    }
    return sourceContext;
  }

  @Override
  public IsoContextProxy targetContext() {
    if (targetContext == null) {
      targetContext = (IsoContextProxy)sourceContext().parent();
    }
    return targetContext;
  }

  @Override
  public boolean succeeded() {
    return succeeded(handleIndex_);
  }

  @Override
  public long nToRedo() {
    return nToRedo(handleIndex_);
  }

  @Override
  public TaskProxy[] redoTasksByStartTime() {
    if (redoTasks == null) {
      long[] rth = redoTasksByStartTime(handleIndex_);
      if (rth == null) {
        redoTasks = new TaskProxy[0];
      } else {
        int n = rth.length;
        redoTasks = new TaskProxy[n];
        for (int i=0; i<n; i++) {
          redoTasks[i] = TaskProxy.fromHandle(rth[i]);
        }
      }
    }
    return redoTasks;
  }
  
  @Override
  public boolean prepareForRedo() {
    return prepareForRedo(handleIndex_);
  }
  
  @Override
  public boolean redo(Task task) {
    TaskProxy tp = (TaskProxy)task;
    IsoContextProxy ctxt = tp.getContext();
    Map<TaskProxy, Runnable> taskMap = IsoContextProxy.redoableTasks.get(ctxt);
    if (taskMap == null) {
      return false;
    }
    Runnable fn = taskMap.get(tp);
    if (fn == null) {
      return false;
    }
    redo(tp, fn);
    return true;
  }

  private void redo(TaskProxy task, Runnable fn) {
    task.rerun(fn);
  }

  @Override
  public boolean resolve(Collection<PublishReport> reports) {
    TaskProxy[] tasks = redoTasksByStartTime();
    // System.out.format("Got the array: %s%n", Arrays.toString(tasks));
    if (tasks == null || tasks.length == 0) {
      // System.out.format("Nothing to resolve%n");
      return true;
    }
    reports.forEach(r -> r.beforeResolve(tasks));
    // System.out.format("Trying to resolve%n");
    // System.out.format("Tasks to resolve: %s%n", Arrays.toString(tasks));
    
    IsoContextProxy ctxt = sourceContext();
    Map<TaskProxy, Runnable> taskMap = IsoContextProxy.redoableTasks.get(ctxt);
    if (taskMap == null) {
      return false;
    }
    boolean needTaskPrepare = false;
    for (TaskProxy t : tasks) {
      if (!taskMap.containsKey(t)) {
        return false;
      }
      if (t.needsPrepareForRedo()) {
        needTaskPrepare = true;
      }
    }
    /*
     * We do this in two separate loops so that we fail faster if
     * there are any tasks that can't be redone.
     *
     * At the moment, we're in the parent context.  Since we're
     * delegating to user code out of our control, we force any
     * prepareForRedo() code to take place in the top-level task of
     * the child context.  
     */
    if (needTaskPrepare) {
      TaskProxy tltask = ctxt.topLevelTask();
      if (!tltask.establishAndGet(()->{
            for (TaskProxy t : tasks) {
              if (!t.prepareForRedo()) {
                return false;
              }
            }
            return true;
          }))
        {
          return false;
        }
    }
    
    if (!prepareForRedo()) {
      return false;
    }
    /*
     * For now, we're going to simply do them linearly in the same
     * thread.  Eventually, we'll need to pay attention to the bounds
     * and get the necessary parallelism going.
     */
    for (TaskProxy t : tasks) {
      /*
       * TODO: It's tempting to call ctxt.hasConflicts() here and
       * short circuit, returning true, if we find that the context
       * already has conflicts.  If we did that, we'd need to keep
       * track of the tasks that still had to be re-run, since we
       * wouldn't necessarily get them the next time (because we've
       * cleared their modifications), probably by actually taking
       * these from a queue in the context itself, popped off after
       * the task finished.  Any new tasks in the next PubResult would
       * have to be added, in order, at the front of the queue, since
       * the conceit is that we reran everything we didn't actually
       * rerun at the beginning of the resolution phase.
       *
       * The problem is that some of the new ones might also be in the
       * carry-over list, and, more importantly, some of the new ones
       * might subsume some of the ones in the carry-over list.  So
       * the right thing to do is probably to maintain a set in the
       * core context itself and take them out after each resolution.
       * When we get a failure and compute the new set, we just dump
       * the old set in, but necessarily later than anything being
       * added now.
       *
       * For now, I'm not going to short circuit.
       */
      redo(t, taskMap.get(t));
    }
    return true;
  }
}
