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

package com.hpl.inventory;

import java.io.PrintStream;

import com.hpl.mds.annotations.RecordSchema;
import com.hpl.mds.annotations.Final;

import static com.hpl.inventory.Formatting.formatCurrency;

@RecordSchema(name = "com.hpl.inventory.Product")
public interface ProductSchema {
	@Final String name();
	int count();
	int countTarget();
	int value();
	int saleValue();
	boolean saleFlag();
	int assetValue();
	int nbrSold();
	int revenue();
	float revenuePercentage();
	Product next();
	Product prev();
	
	static void print(Product.Private self) {
		self.print(System.out);
	}
	static void print(Product.Private self, PrintStream out) {
		self.printName(out);
	}
	
	static void printName(Product.Private self) {
		self.printName(System.out);
	}
	static void printName(Product.Private self, PrintStream out) {
        out.println("Product: name = " + self.getName());
	}
	
	static void printAll(Product.Private self) {
		self.printAll(System.out);
	}
	static void printAll(Product.Private self, PrintStream out) {
	        System.out.println("Product: name = " + self.getName());

	        System.out.println("     count = " + self.getCount());
	        System.out.println("     countTarget = " + self.getCountTarget());
	        System.out.format( "     value = %s%n", formatCurrency(self.getValue())); 
	        System.out.format( "     saleValue = %s%n", formatCurrency(self.getSaleValue()));
	        System.out.println("     saleFlag = " + self.isSaleFlag());
	        System.out.format( "     assetValue = %s%n", formatCurrency(self.getAssetValue()));
	        System.out.println("     nbrSold = " + self.getNbrSold());
	        System.out.format( "     revenue = %s%n", formatCurrency(self.getRevenue())); 
	        System.out.println("     revenuePercentage = " + self.getRevenuePercentage() + "%");
	}
	
	static void Product(Product.Constructing self, String name, int count, int value) {
		self.setName(name);
		self.setCount(count);
		self.setValue(value);
	}
	
}
