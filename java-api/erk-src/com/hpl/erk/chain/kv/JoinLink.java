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

package com.hpl.erk.chain.kv;

import com.hpl.erk.chain.Chain;
import com.hpl.erk.coroutines.ChainedCoroutine;
import com.hpl.erk.func.Pair;
import com.hpl.erk.util.ObjUtils;

public abstract class JoinLink<Head, InK, InV, Out> extends Chain<Head, Out> {
  protected KeyValChain<Head, ? extends InK, ? extends InV> pred;
	
  public JoinLink(KeyValChain<Head, ? extends InK, ? extends InV> pred) {
    super(pred.complete);
    this.pred = pred;
  }
  
  /**
   * Called after a link is constructed.  Here is where an eager link does its 
   * magic if it's complete.  We can't do it in the constructor, since we aren't
   * fully constructed at EagerLink's constructor.
   * @return
   */
  public void activate() {
  }
  
  public abstract class LinkCoroutine extends ChainedCoroutine<Pair<? extends InK, ? extends InV>, Out> {
    public LinkCoroutine() {
      super(pred.iterator());
    }
  }


  
  public abstract class Context extends Chain<Head,Out>.Context {
    protected final KeyValChain<Head,? extends InK, ? extends InV>.Context source;
    public Context(KeyValChain<Head, ? extends InK, ? extends InV>.Context source) {
        super();
        this.source = source;
    }
    protected Context() {
      this(pred.createContext());
    }
    
  }
  


  @Override
  public <H, T extends Head> Chain<H, Out> prepend(Chain<H, T> chain) {
    @SuppressWarnings("unchecked")
    JoinLink<H, InK, InV, Out> clone = (JoinLink<H, InK, InV, Out>)clone();
    clone.complete = chain.complete;
    clone.pred = pred.prepend(chain);
    clone.activate();
    return clone;
  }
  
  @Override
  protected JoinLink<Head,InK,InV,Out> clone() {
    try {
      return ObjUtils.castClone(this, super.clone());
    } catch (CloneNotSupportedException e) {
      return null;
    }
  }


}
