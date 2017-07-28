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

package com.hpl.mds.generator;

import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Set;

import com.hpl.erk.config.RunConfig;
import com.hpl.erk.config.ex.ConfigErrorsSeen;
import com.hpl.mds.generator.Types.ValueType;
import com.hpl.erk.config.ConfigParam;


public class Generator {
  public static final Path templateBase = Paths.get("templates");

  static ConfigParam<Set<String>> etP = RunConfig.setParam(String.class, "templates")
    .defaultVal(Collections.emptySet());

  static boolean allowExplicit(String templateName) {
    Set<String> epVal = etP.getVal();
    boolean val = (epVal == null || epVal.isEmpty() || epVal.contains(templateName));
    // System.out.format("Checking '%s' against '%s': %s%n", templateName, epVal, val);
    return val;
  }
  
  public static void generateTypeSpecific(String templateName, Collection<? extends ValueType> forTypes) throws IOException {
    if (!allowExplicit(templateName)) {
      return;
    }
    try (Template.ForType template = new Template.ForType(templateName)) {
      for (ValueType type : forTypes) {
        template.generate(type);
      }
    }
  }
  public static void generateTypeSpecific(String templateName, ValueType... forTypes) throws IOException {
    generateTypeSpecific(templateName, Arrays.asList(forTypes));
  }
  
  public static void generate(String templateName) throws IOException {
    if (!allowExplicit(templateName)) {
      return;
    }
    try (Template template = new Template(templateName)) {
      template.generate();
    }
  }
  
  public static void main(String[] args) throws IOException, ConfigErrorsSeen {
    args = RunConfig.process(Generator.class, args, Template.class);
    generateTypeSpecific("ManagedFoo", Types.maskedTypes);
    generateTypeSpecific("ManagedFooImpl", Types.primTypes);
    generateTypeSpecific("ManagedFooType", Types.maskedTypes);
    generateTypeSpecific("FooArrayType", Types.maskedTypes);
    generateTypeSpecific("FooArrayField", Types.maskedTypes);
    generateTypeSpecific("FooArrayFieldProxy", Types.maskedTypes);
    generateTypeSpecific("FooField", Types.maskedTypes);
    generateTypeSpecific("FooFieldProxy", Types.maskedTypes);
    generateTypeSpecific("FooArrayProxy", Types.maskedTypes);
    generateTypeSpecific("FooFieldChange", Types.maskedTypes);
    generateTypeSpecific("ManagedFooSet", Types.maskedTypes);
    generateTypeSpecific("ManagedFooArray", Types.maskedTypes);
    generateTypeSpecific("ManagedFooList", Types.maskedTypes);
    generateTypeSpecific("ManagedMapToFoo", Types.maskedTypes);
    generateTypeSpecific("ManagedStringToFooMap", Types.maskedTypes);
    generateTypeSpecific("FooModifier", Types.primTypes);
    generateTypeSpecific("FooIndexedModifier", Types.primTypes);
    generateTypeSpecific("FooKeyedModifier", Types.primTypes);
    generateTypeSpecific("FooPredicate", Types.primTypes);
    generateTypeSpecific("FooConsumer", Types.primTypes);
    generateTypeSpecific("FooSupplier", Types.primTypes);
    generateTypeSpecific("ToFooFunction", Types.primTypes);
    generateTypeSpecific("FooHolder", Types.primTypes);
    generate("ManagedArray");
    generate("ManagedType");
    generate("ManagedList");
    generate("ManagedSet");
    generate("ManagedMap");
    generate("ManagedMapFromString");
    generate("RecordType");
    generate("Field");
    generate("Namespace");
    generate("HName");
    generateTypeSpecific("JNI_FooArrayProxy", Types.maskedTypes);
    generateTypeSpecific("JNI_FooArrayFieldProxy", Types.maskedTypes);
    generateTypeSpecific("JNI_FooArrayType", Types.maskedTypes);
    generateTypeSpecific("JNI_FooFieldProxy", Types.maskedTypes);
    generateTypeSpecific("JNI_ManagedFooType", Types.maskedTypes);
    generate("IsolationContext");
    generate("MDS");
    generate("PubResultProxy");
    generate("JNI_pr_merge_result");
  }

}
