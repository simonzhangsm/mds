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

import com.hpl.mds.annotations.*;
import com.hpl.mds.*;
import static com.hpl.mds.MDS.*;


@RecordSchema
@TypeName(name="mds-test-%1$s")
public interface NonPerishableProductSchema extends ProductSchema {
  int stockOnHand();

  static void NonPerishableProduct(NonPerishableProduct.Constructing self,
                                   int num, String name, Department dept,
                                   int initialStock)
  {
    self.superInit(num, name, dept);
    self.setStockOnHand(initialStock);
  }

  static void addStock(NonPerishableProduct.Private self, int n, Pause.Button pause) {
    /*
     * We do this unnecessarily in an isolated block to allow testing
     * of two-level conflicts.
     */
    String tag = String.format("Adding to %s: ", self.getName());
    isolated(Options.reportTo(new Trace(tag)),
             () -> {
               System.out.format("Adding %d of %s%n", n, self);
               self.incStockOnHand(n);
               System.out.format("Now %s%n", self);
               Pause.on(pause, String.format("tag: ", tag));
             });
  }

  static int removeStock(NonPerishableProduct.Private self, int n) {
    /*
     * We need to make sure that it doesn't go below zero.  We could
     * do this in an isolated block here, but we assume it will be
     * part of a larger transaction.
     */
    int current = self.getStockOnHand();
    if (current < n) {
      n = current;
    }
    self.setStockOnHand(current-n);
    self.incNbrSold(n);
    return n;
  }

  static int inStock(NonPerishableProduct.Private self) {
    return self.getStockOnHand();
  }
}
