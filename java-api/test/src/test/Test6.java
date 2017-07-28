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

package test;

import com.hpl.erk.RandomChoice;
import com.hpl.erk.config.*;
import com.hpl.erk.config.ex.*;
import java.io.*;
import java.util.*;
import java.util.function.*;
import java.util.stream.*;
import org.apache.log4j.Logger;
import com.hpl.mds.*;
import static com.hpl.mds.MDS.*;

public class Test6 {
  static final Logger log = Logger.getLogger(Test6.class);
  static final RunContext rc = RunContext.GLOBAL.subContext("test6").activate();

  static void trace(String which, Namespace ns, String key) {
    System.out.format("%s = %s%n", which, ns.lookupString(key));
  }

  public static void main(String[] args)
    throws ConfigErrorsSeen
  {
    args = RunConfig.process(Test6.class, args);

    final String path = "path";
    final String key = "key";
    final String outerVal = "Outer";
    final String innerVal = "Inner";

    Namespace gns = Namespace.fromPath(path);
    gns.bind(key, outerVal);
    trace("GNS", gns, key);
    IsolationContext c = IsolationContext.nestedDetachedFromCurrent();
    Namespace lns = c.call(()->{
        System.out.format("--- In detached context%n");
        Namespace local = Namespace.fromPath(path);
        trace("GNS", gns, key);
        trace("LNS", local, key);
        System.out.format("--- Binding%n");
        gns.bind(key, innerVal);
        trace("GNS", gns, key);
        trace("LNS", local, key);
        System.out.format("--- Returning from detached context%n");
        return local;
      });
    trace("GNS", gns, key);
    trace("LNS", lns, key);

  }
}
