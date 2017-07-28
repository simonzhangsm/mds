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

import java.util.concurrent.ThreadLocalRandom;

import com.hpl.mds.annotations.Public;
import com.hpl.mds.annotations.RecordSchema;
import com.hpl.mds.annotations.Final;

@RecordSchema(name = "com.hpl.inventory.Inventory")
public interface InventorySchema {
	
	@Public @Final
	ListOfProducts products();
	
	static boolean add(Inventory.Private self, Product product) {
		return self.getProducts().add(product);
	}
	
	static boolean append(Inventory.Private self, Product product) {
		return self.getProducts().append(product);
	}
	
	static Product remove(Inventory.Private self, String productName) {
		return self.getProducts().remove(productName);
	}
	
	static void stockIn(Inventory.Private self, int units, String productName) {
		Product product = self.get(productName);
		if (product == null) {
			int value = ThreadLocalRandom.current().nextInt(1000, 100000);
			product = Product.create.record(productName, 0, value); 
			self.getProducts().add(product);
		}
		self.stockIn(units, product);
	}
	
	static void stockIn(Inventory.Private self, int units, Product product) {
		product.incCount(units);
	}
	
	static Sale orderOut(Inventory.Private self, int units, String productName) {
		return self.orderOut(units, self.get(productName));
	}
	
	static Sale orderOut(Inventory.Private self, int units, Product product) {
		if (product == null) {
			return new Sale(units, 0, 0, 0);
		}
		int nInStock = product.getCount();
		int n = Math.min(nInStock, units);
		int unitPrice = product.getValue();
		int totalPrice = n * unitPrice;
		
		product.decCount(n);
		product.incNbrSold(n);
		product.incRevenue(totalPrice);
		return new Sale(units, n, unitPrice, totalPrice);
	}
	
	static Product get(Inventory.Private self, String productName) {
		return self.getProducts().get(productName);
	}
	
	static void print(Inventory.Private self) {
        System.out.println("Inventory: size = " + self.getProducts().getSize()); 
        self.getProducts().forEach(Product::print);
	}
	
	static void printAll(Inventory.Private self) {
		self.getProducts().forEach(Product::printAll);
	}
	
	static Report generateReport(Inventory.Private self, String name, int nTop) {
		return Report.create.record(name, self, nTop);
	}

	static Report generateReport(Inventory.Private self, String name) {
		return self.generateReport(name, 3);
	}

	static void Inventory(Inventory.Constructing self) {
		self.setProducts(ListOfProducts.create.record());
	}
	
	
}










	/////////////////////////////////////////////////////////////////
	// analytics

//    /**
//     * calcRevenuePercentage
//     * 1. calculate TotalRevenues for all sales across all products
//     * 2. calculate revenuePercentage for each Product:
//     *    taking revenue for this product and TotalRevenues for all sales,
//     *    set revenuePercentage to int, rounded to nearest percentage
//     */
//    void calcRevenuePercentage() {
//    }
//
//    /**
//     * lowestValueProduct
//     * - returns Product with lowest dollar value;
//     */
//    Product lowestValueProduct() {
//        // TODO: implement
//        return null;
//    }
//
//    /**
//     * highestValueProduct
//     * - returns Product with highest dollar value;
//     */
//    Product highestValueProduct() {
//        // TODO: implement
//        return null;
//    }
//
//    Other operations:
//
//    Update operations:
//        decrement product Count
//            company delivers one or more of this product to a customer
//        increment product Count
//            company takes delivery of new stock of this product
//        increment product Value
//            increase retail price of this product
//        decrement product Value
//            decrease retail price of this product
//        increment product SaleValue
//            increase sale price of this product
//        decrement productSaleValue
//            decrease sale price of this product (further reductions!!)
//
//    Analytics Operations
//        Report all products on Sale (product SaleFlag = true)
//        Report all products in stock (product Count > 0)
//        Check for low inventory: percentage of products with Count < (CountTarget/2)
//        Generate order for resupplying inventory to target levels: order (CountTarget-Count) of product
//        Update AssetValue for each product (e.g. at the end of each day): Count * (Sale)Value
//        Calculate total value of assets: sum AssetValue for all products
//        Calculate average productCount
//        Calculate average productValue
//        Calculate average AssetValue
//        Calculate weighted average productValue (weighted by productCount)
//        Report total nbrSold for each product; sum nbrSold for all products
//        Report total revenue for each product; sum revenue for all products
//        Calculate revenue percentage for each product

