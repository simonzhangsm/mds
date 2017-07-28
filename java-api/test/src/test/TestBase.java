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

import com.hpl.erk.config.*;
import com.hpl.erk.RandomChoice;
import com.hpl.erk.util.Patterns;
import java.io.*;
import java.nio.file.*;
import java.util.*;

public class TestBase {
  static final RunContext testRC = RunContext.GLOBAL.subContext("test");

  static final ConfigParam<Path> dataDir = testRC.param(Path.class, "data_dir")
    .restrict(PathRestriction.is_dir())
    .restrict(PathRestriction.exists())
    .defaultVal(Paths.get("."))
    .help("Directory containing data files");
  static final ConfigParam<Path> productsFile = testRC.param(Path.class, "prod_file")
    .defaultVal(Paths.get("products.csv"))
    .help("CSV file describing products");
    

  static RandomChoice<Product> products = new RandomChoice<>();
  static Store store;

  private static int parseOr(int def, String s) {
    if (s == null || s.trim().isEmpty()) {
      return def;
    } else {
      return Integer.parseInt(s);
    }
  }

  private static double parseOr(double def, String s) {
    if (s == null || s.trim().isEmpty()) {
      return def;
    } else {
      return Double.parseDouble(s);
    }
  }

  static void initStore(Path csvPath) throws IOException {
    StoreBuilder sb = new StoreBuilder();
    Files.lines(csvPath)
      .skip(1)  // skip the header line
      .map(Patterns.COMMA_PATTERN::split)
      .forEach(fields -> {
          System.out.println(Arrays.asList(fields));
          String deptName = fields[0];
          String prodName = fields[1];
          double popularity = parseOr(1.0, fields[2]);
          int shelfLife = parseOr(0, fields[3]);
          int initial = parseOr(0, fields[4]);
          int restock_amt = parseOr(0, fields[5]);
          int restock_freq = parseOr(1, fields[6]);
          Product prod;
          if (shelfLife == 0) {
            prod = sb.addNonPerishable(deptName, prodName, initial);
          } else {
            prod = sb.addPerishable(deptName, prodName, shelfLife, initial);
          }
          products.add(prod, popularity);

        });

    store = sb.build();
  }

  static void initStore() throws IOException {
    Path dir = dataDir.getVal();
    Path fileName = productsFile.getVal();
    Path file = dir.resolve(fileName);
    initStore(file);
  }
  
  static Basket makeBasket(int n) {
    Map<Product, BasketItem> contents = new HashMap<>();
    System.out.format("Creating basket with %d items%n", n);
    for (int i=0; i<n; i++) {
      Product p = products.choose();
      // System.out.format("Chose %s%n", p);
      BasketItem bi = contents.get(p);
      if (bi == null) {
        bi = BasketItem.create.record(p, 1);
        contents.put(p, bi);
      } else {
        // System.out.format("Found %s%n", bi);
        bi.incQuantity();
      }
      // System.out.format("Line item now %s%n", bi);
    }
    BasketItem[] items = contents.values().stream().toArray(BasketItem[]::new);
    Basket b =  Basket.create.record(items);
    System.out.format("Basket is %s%n", b);
    return b;
  }

}