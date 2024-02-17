/** @file cfl_conflequiv.cpp

Abstractions that maintaine conflict-equivalence.

*/

/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2015  Michael Meyer and Thomnas Moor.
Exclusive copyright is granted to Klaus Schmidt

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

  
#include "cfl_conflequiv.h"
#include "cfl_bisimulation.h"
#include "cfl_graphfncts.h"
#include "cfl_parallel.h"
#include "cfl_regular.h"


/* 
The functions in this file implement automata transformations 
as proposed by Malik R and Flordal H in "Compositional verification in supervisory 
control", SIAM Journal of Control and Optimization, 2009. The implementation at hand
is based on the Bachelor Thesis Project of Michael Meyer, Friedrich-Alexander Universitaet 
Erlangen-Nuernberg, 2015.

The current status is still experimental. The individual rules are implemented,
for further testing and optimisation. The code so far does not use  
any advanced heuristics to apply the transformations is a strategic order. 
*/


namespace faudes {


// Merge equivalent classes, i.e. perform quotient abstraction
// (this implementation works fine with a small amount of small equiv classes)
void MergeEquivalenceClasses(
  Generator& rGen, 
  TransSetX2EvX1& rRevTrans,
  const std::list< StateSet >& rClasses
) 
{
  // iterators
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  TransSetX2EvX1::Iterator rit;
  TransSetX2EvX1::Iterator rit_end;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  // record for delayed delete
  StateSet delstates;
  // iterate classes
  std::list< StateSet >::const_iterator qit=rClasses.begin();
  for(;qit!=rClasses.end();++qit) {
    sit=qit->Begin();
    sit_end=qit->End();
    Idx q1=*(sit++);
    for(;sit!=sit_end;++sit){
      Idx q2=*sit;
      // merge outgoing transitions form q2 to q1
      tit = rGen.TransRelBegin(q2);
      tit_end = rGen.TransRelEnd(q2);
      for(;tit!=tit_end;++tit) { 
	 rGen.SetTransition(q1,tit->Ev,tit->X2);
	 rRevTrans.Insert(q1,tit->Ev,tit->X2);   
      }
      // merge incomming transitions form q2 to q1
      rit = rRevTrans.BeginByX2(q2);
      rit_end = rRevTrans.EndByX2(q2);
      for(;rit!=rit_end;++rit) { 
	rGen.SetTransition(rit->X1,rit->Ev,q1);
        rRevTrans.Insert(rit->X1,rit->Ev,q1);
      }
      // fix marking: if q2 was not marked, need to un-mark q1 (original by Michael Meyer)
      //if(!(rGen.ExistsMarkedState(q2)))
      //  rGen.ClrMarkedState(q1);
      // fix marking: if q2 was marked, so becomes  q1 (quatient by the books)
      if(rGen.ExistsMarkedState(q2))
        rGen.InsMarkedState(q1);
      // fix initial states: if q2 was initial, so becomes q1
      if((rGen.ExistsInitState(q2)))
        rGen.InsInitState(q1);
      // log for delete
      delstates.Insert(q2);
    }
  }
  // do delete
  rGen.DelStates(delstates);
  // optional: delete stes in rec transrel --- otherwise it is now invalid
}



// Compute extended transition relation '=>', i.e. relate each two states that 
// can be reached by one non-slilent event and an arbitrary amount of silent 
// events befor/after the non-silent event
//
// Note: currently all silent events are treated equivalent, as if they
// had been substituted by a single silent event "tau"; we should perform
// this substitution beforehand.
// Note: the below fixpoint iteration is simple but perhaps suboptimal; can a
// todostack perform better? See also the varuious implementations of ProjectNonDet

void ExtendedTransRel(const Generator& rGen, const EventSet& rSilentAlphabet, TransSet& rXTrans) {

  // HELPERS:
  TransSet::Iterator tit1;
  TransSet::Iterator tit1_end;
  TransSet::Iterator tit2;
  TransSet::Iterator tit2_end;
  TransSet newtrans;

  // initialize result with original transitionrelation
  rXTrans=rGen.TransRel();
  // loop for fixpoint
  while(true) {
    // accumulate new transitions
    newtrans.Clear();
    // loop over all transitions
    tit1=rXTrans.Begin();
    tit1_end=rXTrans.End();
    for(;tit1!=tit1_end; ++tit1) {
       // if it is silent add transition to non silent successor directly
       if(rSilentAlphabet.Exists(tit1->Ev)) {
         tit2=rXTrans.Begin(tit1->X2);
         tit2_end=rXTrans.End(tit1->X2);
         for(;tit2!=tit2_end; ++tit2) {
           if(!rSilentAlphabet.Exists(tit2->Ev)) // tmoor 18-09-2019
             newtrans.Insert(tit1->X1,tit2->Ev,tit2->X2);
         }
       }
       // if it is not silent add transition to silent successor directly
       else {
         tit2=rXTrans.Begin(tit1->X2);
         tit2_end=rXTrans.End(tit1->X2);
         for(;tit2!=tit2_end; ++tit2) {
           if(rSilentAlphabet.Exists(tit2->Ev)) 
	     newtrans.Insert(tit1->X1,tit1->Ev,tit2->X2);
         }
       }
    }
    // insert new transitions, break on fixpoint
    Idx tsz=rXTrans.Size();
    rXTrans.InsertSet(newtrans);
    if(tsz==rXTrans.Size()) break;
  }
}


// Performs observation equivalent abstraction; see cited literature 3.1
// Note: do we need to implement the test on the initial states? 
void ObservationEquivalentQuotient(Generator& g, const EventSet& silent){
  FD_DF("ObservationEquivalentQuotient(): prepare for t#"<<g.TransRelSize());

  // have extendend/reverse-ordered transition relations 
  TransSetX2EvX1 rtrans;
  g.TransRel().ReSort(rtrans);
  TransSet xtrans;
  ExtendedTransRel(g,silent,xtrans);
  FD_DF("ObservationEquivalentQuotient(): ext. trans t#"<<xtrans.Size());

  // this was in error --- ytang/tmoor 18-09-2019
  // remove silent events from extended transition relation
  // xtrans.RestrictEvents(g.Alphabet()-silent);

  // figure observation equivalent states
  std::list< StateSet > eqclasses;
  Generator xg(g);
  xg.InjectTransRel(xtrans);
  calcBisimulation(xg,eqclasses);

  // merge  classes
  FD_DF("ObservationEquivalentQuotient(): merging classes #"<< eqclasses.size());
  MergeEquivalenceClasses(g,rtrans,eqclasses);
 
  FD_DF("ObservationEquivalentQuotient(): done with t#"<<g.TransRelSize());
}



// Test for incoming equivalence '=_inc' based on reverse ext. transition relation
bool IsIncomingEquivalent(
  const TransSetX2EvX1& rRevXTrans, 
  const EventSet& rSilent, 
  const StateSet& rInitialStates, 
  Idx q1, Idx q2) 
{
  TransSetX2EvX1::Iterator rit1;
  TransSetX2EvX1::Iterator rit1_end;
  TransSetX2EvX1::Iterator rit2;
  TransSetX2EvX1::Iterator rit2_end;
  bool nonsilent=true;
  // condition 1: is reached from same state by the same std event
  rit1=rRevXTrans.BeginByX2(q1);
  rit1_end=rRevXTrans.EndByX2(q1);
  rit2=rRevXTrans.BeginByX2(q2);
  rit2_end=rRevXTrans.EndByX2(q2);
  while(true) {
    // skip silent
    for(;rit1!=rit1_end;++rit1) if(!rSilent.Exists(rit1->Ev)) break;
    for(;rit2!=rit2_end;++rit2) if(!rSilent.Exists(rit2->Ev)) break;
    // sense end of loop
    if(rit1==rit1_end) break;
    if(rit2==rit2_end) break;
    // mismatch
    if(rit1->X1!=rit2->X1) return false;
    if(rit1->Ev!=rit2->Ev) return false;
    nonsilent=true;
    // increment
    ++rit1;
    ++rit2;
  }
  // skip silent
  for(;rit1!=rit1_end;++rit1) if(!rSilent.Exists(rit1->Ev)) break;
  for(;rit2!=rit2_end;++rit2) if(!rSilent.Exists(rit2->Ev)) break;
  if(rit1!=rit1_end) return false;
  if(rit2!=rit2_end) return false;
  // condition 2: both or neither can be silently reached from an initial state 
  TransSetX2EvX1::Iterator rit;
  TransSetX2EvX1::Iterator rit_end;
  bool ini1=rInitialStates.Exists(q1);
  if(!ini1) {
    rit=rRevXTrans.BeginByX2(q1);
    rit_end=rRevXTrans.EndByX2(q1);
    for(;rit!=rit_end;++rit) {
      if(!rSilent.Exists(rit->Ev)) continue;
      if(rInitialStates.Exists(rit->X1)) {ini1=true; break;}
    }
  }
  bool ini2=rInitialStates.Exists(q2);
  if(!ini2) {
    rit=rRevXTrans.BeginByX2(q2);
    rit_end=rRevXTrans.EndByX2(q2);
    for(;rit!=rit_end;++rit) {
      if(!rSilent.Exists(rit->Ev)) continue;
      if(rInitialStates.Exists(rit->X1)) {ini2=true; break;}
    }
  }
  if(ini1!=ini2) return false;
  // condition 3: if all incomming are silent, must be reachable from initial state (redundant)
  if(!nonsilent && !ini1) return false;
  // all tests passt
  //FD_DF("IE PASSED");
  return true;
}

// Candidates for incomming equivalent states '=_inc'
// (this is meant as a filter in iterations)
void IncomingEquivalentCandidates(
  const TransSet& rXTrans, 
  const TransSetX2EvX1& rRevXTrans, 
  const EventSet& rSilent, 
  const StateSet& rInitialStates, 
  Idx q1,
  StateSet& rRes) 
{
  // iterators
  TransSetX2EvX1::Iterator rit;
  TransSetX2EvX1::Iterator rit_end;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  // prepare result
  rRes.Clear();
  // std case: one back and one forth via matching non-silent event
  bool stdcand=false;
  rit = rRevXTrans.BeginByX2(q1);
  rit_end = rRevXTrans.EndByX2(q1);
  for(;rit!=rit_end;++rit) {
    if(rSilent.Exists(rit->Ev)) continue;
    stdcand=true;
    tit = rXTrans.Begin(rit->X1,rit->Ev);
    tit_end = rXTrans.End(rit->X1,rit->Ev);
    for(;tit!=tit_end;++tit) 
      rRes.Insert(tit->X2);
  }
  // bail out if the std condition could be applied
  if(stdcand) return;
  // only reachable via silent events from initial state
  rit = rRevXTrans.BeginByX2(q1);
  rit_end = rRevXTrans.EndByX2(q1);
  for(;rit!=rit_end;++rit) {
    //if(!rSilent.Exists(tit->Ev)) continue; // all silent anyway
    rRes.Insert(rit->X1);
  }
  tit = rXTrans.Begin(q1);
  tit_end = rXTrans.End(q1);
  for(;tit!=tit_end;++tit) {
    if(!rSilent.Exists(tit->Ev)) continue;
    rRes.Insert(tit->X2);
  }
}





// Active events rule; see cited literature 3.2.1
// Note: this is a re-write of Michael Meyer's implementation to refer to the
// ext. transistion relation, as indicated by the literature.
void ActiveEventsRule(Generator& g, const EventSet& silent){
  FD_DF("ActiveEventsRule(): prepare for t#"<<g.TransRelSize());

  // iterators
  StateSet::Iterator sit1;
  StateSet::Iterator sit1_end;
  StateSet::Iterator sit2;
  StateSet::Iterator sit2_end;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  EventSet::Iterator eit;
  EventSet::Iterator eit_end;


  // have extendend/reverse-order transition relations 
  TransSetX2EvX1 rtrans;
  g.TransRel().ReSort(rtrans);
  TransSet xtrans;
  ExtendedTransRel(g,silent,xtrans);
  TransSetX2EvX1 rxtrans;
  xtrans.ReSort(rxtrans);
  FD_DF("ActiveEventsRule: ext. trans t#"<<xtrans.Size());

  // record equivalent states
  std::list< StateSet > eqclasses;
  StateSet eqclass;

  // inner loop variables
  StateSet candidates;
  EventSet active1;

  // iterate pairs of states
  StateSet states=g.States() - g.InitStates(); // todo: deal with initial states
  sit1=states.Begin();
  sit1_end=states.End();
  for(;sit1!=sit1_end;++sit1) {

    // find candidates for incoming equivalence
    IncomingEquivalentCandidates(xtrans,rxtrans,silent,g.InitStates(),*sit1,candidates);

    // q1 active events
    active1=xtrans.ActiveEvents(*sit1) - silent;
    // prepare set of equivalent states
    eqclass.Clear();

    // iterate candidates
    sit2=candidates.Begin();
    sit2_end=candidates.End();
    for(;sit2!=sit2_end;++sit2){
      // skip restriction/dealt with
      if(!states.Exists(*sit2)) continue;
      // skip doublets
      if(*sit2<=*sit1) continue;
      //FD_DF("ActiveEventsRule: test  "<< *sit1 << " with " << *sit2 );
      // insist in active standard events to match
      if(! ( active1 == xtrans.ActiveEvents(*sit2) - silent ) ) continue;

      /*
      // alternative implementation: makes no difference
      tit=xtrans.Begin(*sit2);
      tit_end=xtrans.End(*sit2);
      eit=active1.Begin();
      eit_end=active1.End();
      while(true) {
        // skip silent
        for(;tit!=tit_end;++tit) { if(!silent.Exists(tit->Ev)) break;}
	if(tit==tit_end) break;
        // sense mismatch
        if(tit->Ev != *eit) break; 
        // increment
	++tit;
        ++eit;
        // sense end of loop
        if(tit==tit_end) break;
	if(eit==eit_end) break;
      }
      for(;tit!=tit_end;++tit) { if(!silent.Exists(tit->Ev)) break;}
      if(eit!=eit_end) continue;
      if(tit!=tit_end) continue;        
      */

      // insist in active events to match, incl w-event, i.e. marking (not needed?)
      //if(! ( g.ExistsMarkedState(*sit1) == g.ExistsMarkedState(*sit2) ))
      // continue;
      //FD_DF("ActiveEventsRule: a-match  "<< *sit1 << " with " << *sit2 );
      // test incomming equivalence
      if(! IsIncomingEquivalent(rxtrans,silent,g.InitStates(),*sit1,*sit2))
	 continue;
      //FD_DF("ActiveEventsRule: record merge  "<< *sit1 << " with " << *sit2 );
      eqclass.Insert(*sit2);
      states.Erase(*sit2);
    }
    // record non-trivial equaivalence class
    if(eqclass.Size()>0){
      eqclass.Insert(*sit1);
      eqclasses.push_back(eqclass);
    }

  } // end: loop pais of states

  // iterate over all classes of equivalent states and merge
  FD_DF("ActiveEventsRule: merging classes #"<< eqclasses.size());
  MergeEquivalenceClasses(g,rtrans,eqclasses);
 
  FD_DF("ActiveEventsRule: done with t#"<<g.TransRelSize());
}


// Silent continuation rule; see cited literature 3.2.2
// Note: this is a rewrite of Michael Meyer's implementation to refer to the
// ext. transistion relation, as indicated by the literature.
// Note: assumes that silent SCCs have been eliminated before; see lit. Corollary 1, and MergeSilentSccs()
void SilentContinuationRule(Generator& g, const EventSet& silent){
  FD_DF("SilentContinuationRule(): prepare for t#"<<g.TransRelSize());

  // iterators
  StateSet::Iterator sit1;
  StateSet::Iterator sit1_end;
  StateSet::Iterator sit2;
  StateSet::Iterator sit2_end;

  // have extendend/reverse-ordered transition relations 
  TransSet xtrans;
  TransSetX2EvX1 rxtrans;
  TransSetX2EvX1 rtrans;
  ExtendedTransRel(g,silent,xtrans);
  xtrans.ReSort(rxtrans);
  g.TransRel().ReSort(rtrans);

  // record equivalent states
  std::list< StateSet > eqclasses;
  StateSet eqclass;

  // loop variables
  StateSet candidates;

  // iterate pairs of states
  StateSet states=g.States() - g.InitStates(); // todo: deal with initial states
  sit1=states.Begin();
  sit1_end=states.End();
  for(;sit1!=sit1_end;++sit1) {

    // must have at least one silent event enabled
    if((g.ActiveEventSet(*sit1)*silent).Size()==0) continue;

    // find candidates for incoming equivalence
    IncomingEquivalentCandidates(xtrans,rxtrans,silent,g.InitStates(),*sit1,candidates);
    if(candidates.Empty()) continue;

    // iterate candidates
    eqclass.Clear();
    sit2=candidates.Begin();
    sit2_end=candidates.End();
    for(;sit2!=sit2_end;++sit2){
      // skip restriction/dealt with
      if(!states.Exists(*sit2)) continue;
      // skip doublets
      if(*sit2<=*sit1) continue;

      // must have at least one silent event enabled
      if((g.ActiveEventSet(*sit2)*silent).Size()<=0)
        continue;
      
      //FD_DF("SilentContinuationRule: test  "<< *sit1 << " with " << *sit2 );
 
      // test incoming equivalence
      if(! IsIncomingEquivalent(rxtrans,silent,g.InitStates(),*sit1,*sit2))
	 continue;
      // record to merge
      //FD_DF("SilentContuationRule: record to merge  "<< *sit1 << " with " << *sit2 );
      eqclass.Insert(*sit2);
      states.Erase(*sit2);
    }

    // record equaivalent states
    if(eqclass.Size()>0){
       eqclass.Insert(*sit1);
       eqclasses.push_back(eqclass);
    }

  } // end: loop pais of states

  // iterate over all classes of equivalent states and merge
  FD_DF("SilentContinuationRule(): merging classes #"<< eqclasses.size());
  MergeEquivalenceClasses(g,rtrans,eqclasses);
 
  FD_DF("SilentContinuationRule(): done with t#"<<g.TransRelSize());
}


// Represent each silent SCC by one state and remove respective silent events.
// This transformation is observation equivalent and, hence, conflict equivalent (reference [19] in cited literature)
// The current implementation is based on Michael Meyer's with some additional performance considerations.
void MergeSilentSccs(Generator& g, const EventSet& silent){
  FD_DF("MergeSilentSccs(): prepare for t#"<<g.TransRelSize());
  StateSet delstates;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  // remove silent selfloops
  tit=g.TransRelBegin();
  tit_end=g.TransRelEnd();
  for(;tit!=tit_end;){
    if(tit->X1 == tit->X2)
      if(silent.Exists(tit->Ev)) {
        g.ClrTransition(tit++);
        continue;
      }
    ++tit;
  }
  // find silent SCCs
  EventSet avoid=g.Alphabet()-silent;
  SccFilter filter(SccFilter::FmEventsAvoid || SccFilter::FmIgnoreTrivial, avoid);
  StateSet sccroots;
  std::list< StateSet > scclist;
  ComputeScc(g,filter,scclist,sccroots);
  // set up state rename map
  std::map< Idx , Idx > sccxmap;
  while(!scclist.empty()) {
    const StateSet& scc=scclist.front();
    if(scc.Size()<=1) { scclist.pop_front(); continue;}
    // have new state to represent scc
    Idx newSt = g.InsState();
    // set marking status
    if( (scc * g.MarkedStates()).Size()>0 )
      g.SetMarkedState(newSt);
    // set initial status
    if( (scc * g.InitStates()).Size()>0 )
      g.SetInitState(newSt);
    // add to map 
    sit=scc.Begin();
    sit_end=scc.End();
    for(;sit!=sit_end;++sit) 
      sccxmap[*sit]=newSt;
    // add to delstates
    delstates.InsertSet(scc); 
    //remove from list
    scclist.pop_front();
  }
  // apply substitution
  if(sccxmap.size()>0){
    std::map<Idx,Idx>::iterator xit1;
    std::map<Idx,Idx>::iterator xit2;
    tit=g.TransRelBegin(); 
    while(tit!=g.TransRelEnd()) {
      Transition t= *tit;
      g.ClrTransition(tit++);
      xit1=sccxmap.find(t.X1);
      xit2=sccxmap.find(t.X2);
      if(xit1!=sccxmap.end()) t.X1=xit1->second;
      if(xit2!=sccxmap.end()) t.X2=xit2->second;
      g.SetTransition(t);
    }
  }
  // delete scc states
  g.DelStates(delstates);
  FD_DF("MergeSilentSccs(): done with t#"<<g.TransRelSize());
}


// Certain conflicts. see cited literature 3.2.3
// -- remove outgoing transitions from not coaccessible states
void RemoveNonCoaccessibleOut(Generator& g){
  StateSet notcoaccSet=g.States()-g.CoaccessibleSet();
  StateSet::Iterator sit=notcoaccSet.Begin();
  StateSet::Iterator sit_end=notcoaccSet.End();
  for(;sit!=sit_end;++sit){
    TransSetX1EvX2::Iterator tit=g.TransRelBegin(*sit);
    TransSetX1EvX2::Iterator tit_end=g.TransRelEnd(*sit);
    for(;tit!=tit_end;) g.ClrTransition(tit++);  
    //FD_DF("RemoveCertainConflictA: remove outgoing from state "<<*sit);
  }
}



// Certain conflicts. see cited literature 3.2.3
// -- remove outgoing transitions from states that block by a silent event 
void BlockingSilentEvent(Generator& g,const EventSet& silent){
  FD_DF("BlockingSilentEvent(): prepare for t#"<<g.TransRelSize());
  StateSet coacc=g.CoaccessibleSet();
  StateSet sblock;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  // loop all transitions to figure certain blocking states
  tit=g.TransRelBegin();
  tit_end=g.TransRelEnd();
  for(;tit!=tit_end;++tit) { 
    if(silent.Exists(tit->Ev)) 
      if(!coacc.Exists(tit->X2)) 
        sblock.Insert(tit->X1);
  }
  // unmark blocking states and eliminate possible future
  sit=sblock.Begin();
  sit_end=sblock.End();
  for(;sit!=sit_end;++sit) {
    g.ClrMarkedState(*sit);
    tit=g.TransRelBegin(*sit);
    tit_end=g.TransRelEnd(*sit);
    for(;tit!=tit_end;) 
      g.ClrTransition(tit++);
  }
  FD_DF("BlockingSilentEvent(): done with t#"<<g.TransRelSize());
}



// Certain conflicts. see cited literature 3.2.3
// -- merge all states that block to one representative
void MergeNonCoaccessible(Generator& g){
  StateSet notcoacc=g.States()-g.CoaccessibleSet();
  // bail out on trovial case
  if(notcoacc.Size()<2) return;
  // have a new state
  Idx qnc=g.InsState();
  // fix init status
  if((notcoacc * g.InitStates()).Size()>0)
    g.SetInitState(qnc);
  // fix transitions
  TransSet::Iterator tit=g.TransRelBegin();
  TransSet::Iterator tit_end=g.TransRelEnd();
  for(;tit!=tit_end;++tit){
    if(notcoacc.Exists(tit->X2))	
      g.SetTransition(tit->X1,tit->Ev,qnc);
  }
  // delete original not coacc 
  g.DelStates(notcoacc);
}

// Certain conflicts. see cited literature 3.2.3
// -- if a transition blocks, remove all transitions from the same state with the same label
void BlockingEvent(Generator& g,const EventSet& silent){
  FD_DF("BlockingEvent(): prepare for t#"<<g.TransRelSize());
  StateSet coacc=g.CoaccessibleSet();
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  TransSet::Iterator dit;
  TransSet::Iterator dit_end;
  TransSet deltrans;
  // loop all transitions to figure transitions to blocking states
  tit=g.TransRelBegin();
  tit_end=g.TransRelEnd();
  for(;tit!=tit_end;++tit) { 
    // silent events are treated by a separat rule;
    if(silent.Exists(tit->Ev)) continue;
    // look for transitions that block
    if(coacc.Exists(tit->X1)) continue;
    if(coacc.Exists(tit->X2)) continue;
    // consider all transitions with the same event to block
    dit=g.TransRelBegin(tit->X1,tit->Ev);
    dit_end=g.TransRelEnd(tit->X1,tit->Ev); 
    for(;dit!=dit_end;++dit) {
      // keep selfloops (Michael Meyer)
      if(dit->X1==dit->X2) continue;
      // rcord to delete later
      deltrans.Insert(*dit); 
    }
  }
  // delete transitions
  dit=deltrans.Begin();
  dit_end=deltrans.End();
  for(;dit!=dit_end;++dit) 
    g.ClrTransition(*dit);
  FD_DF("BlockingEvent(): done with t#"<<g.TransRelSize());
}



// Only silent incomming rule; see cited literature 3.2.4
// Note: input generator must be silent-SCC-free
// Note: this is a complete re-write and needs testing for more than one candidates
void OnlySilentIncoming(Generator& g, const EventSet& silent){
  FD_DF("OnlySilentIncoming(): prepare with t#"<<g.TransRelSize());

  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  TransSet::Iterator dit;
  TransSet::Iterator dit_end;
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;

  // figure states with only silent incomming transitions
  // note: Michael Meyer proposed to only consider states with at least two incomming 
  // events -- this was dropped in the re-write
  StateSet cand=g.States(); // - g.InitStates();
  tit=g.TransRelBegin();
  tit_end=g.TransRelEnd();
  for(;tit!=tit_end;++tit) 
    if(!silent.Exists(tit->Ev)) cand.Erase(tit->X2); 

  // bail out on trivial
  if(cand.Size()==0) return;

  // restrict candidates to "if marked, then have at least one non-self-loop silent outgoing transition"
  /*
  sit=cand.Begin();
  sit_end=cand.End();
  while(sit!=sit_end) {
    if(!g.ExistsMarkedState(*sit)) { ++sit; continue;}
    tit=g.TransRelBegin(*sit);
    tit_end=g.TransRelEnd(*sit);
    for(;tit!=tit_end;++tit) {   
      if(silent.Exists(tit->Ev) && (tit->X2!=*sit)) break;
    }
    if(tit!=tit_end) ++sit;
    else cand.Erase(sit++);
  }
  */

  // current code cannot handle multiple successive only-silent-incomming states 
  sit=cand.Begin();
  sit_end=cand.End();
  for(;sit!=sit_end;) {
    tit=g.TransRelBegin(*sit);
    tit_end=g.TransRelEnd(*sit);
    for(;tit!=tit_end;++tit)    
      if(cand.Exists(tit->X2)) break;
    if(tit==tit_end) ++sit;
    else cand.Erase(sit++);  
  }

  // bail out on trivial
  if(cand.Size()==0) return;
 
  // re-link outgoing transitions to predecessor
  // note: see the attempt we did for marking? this does not work
  // and is one reason why one insists in one silent outgoing event
  tit=g.TransRelBegin();
  tit_end=g.TransRelEnd();
  for(;tit!=tit_end;) {
    if(!cand.Exists(tit->X2))  {++tit; continue; }
    if(tit->X1==tit->X2)       {++tit; continue; } // self loop (cannot happen since silent-SCC-free)
    //bool marked = g.ExistsMarkedState(tit->X2);
    dit=g.TransRelBegin(tit->X2);
    dit_end=g.TransRelEnd(tit->X2);
    for(;dit!=dit_end;++dit) {
      g.SetTransition(tit->X1,dit->Ev,dit->X2);
      //if(!marked) g.ClrMarkedState(tit->X1);
    }
    g.ClrTransition(tit++);  
  }
  
  // make accessible
  g.Accessible();
  FD_DF("OnlySilentIncoming(): done with t#"<<g.TransRelSize());
}


// Only silent outgoing rule; see cited literature 3.2.5
// Note: input generator must be silent-SCC-free
void OnlySilentOutgoing(Generator& g,const EventSet& silent){
  FD_DF("OnlySilentOutgoing(): prepare for t#"<<g.TransRelSize());
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  TransSet::Iterator dit;
  TransSet::Iterator dit_end;
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  TransSetX2EvX1 rtrans;
  TransSetX2EvX1::Iterator rit;
  TransSetX2EvX1::Iterator rit_end;

  // figure states with only silent outgoing transitions (cannot handle initial states correctly)
  StateSet cand=g.States() - g.InitStates();
  tit=g.TransRelBegin();
  tit_end=g.TransRelEnd();
  for(;tit!=tit_end;++tit) 
    if(!silent.Exists(tit->Ev)) cand.Erase(tit->X1);


  // insist in at least one outgoing transition
  // (since we are silent-SCC-free, this will not be a self-loop)
  sit=cand.Begin();
  sit_end=cand.End();
  for(;sit!=sit_end;) {
    tit=g.TransRelBegin(*sit);
    tit_end=g.TransRelEnd(*sit);
    if(tit!=tit_end) ++sit;
    else cand.Erase(sit++);  
  }
				     

  // current code cannot handle multiple successive only-silent-outgoing states 
  sit=cand.Begin();
  sit_end=cand.End();
  for(;sit!=sit_end;) {
    tit=g.TransRelBegin(*sit);
    tit_end=g.TransRelEnd(*sit);
    for(;tit!=tit_end;++tit)    
      if(cand.Exists(tit->X2)) break;
    if(tit==tit_end) ++sit;
    else cand.Erase(sit++);  
  }

  // bail out on trivial
  if(cand.Size()==0) return;

  // re-link outgoing transitions of predecessor
  tit=g.TransRelBegin();
  tit_end=g.TransRelEnd();
  for(;tit!=tit_end;++tit) {
    if(!cand.Exists(tit->X2)) continue;
    dit=g.TransRelBegin(tit->X2);
    dit_end=g.TransRelEnd(tit->X2);
    for(;dit!=dit_end;++dit) {
      if(dit->X2==dit->X1) continue; // silent selfloop cannot happen
      g.SetTransition(tit->X1,tit->Ev,dit->X2);
    }  
  }

  // fix initial states
  sit=cand.Begin();
  sit_end=cand.End();
  for(;sit!=sit_end;++sit) 
    if(g.ExistsInitState(*sit)) 
      g.InsInitStates(g.SuccessorStates(*sit));
  
  // remove candidates
  g.DelStates(cand); 
  FD_DF("OnlySilentOutgoing(): done with t#"<<g.TransRelSize());
}
  




// apply all of the above repeatedly until a fixpoint is attained
void ConflictEquivalentAbstraction(vGenerator& rGen, const EventSet& rSilentEvents){
  FD_DF("ConflictEquivalentAbstraction(): prepare with t#"<<rGen.TransRelSize());
  while(true) {
    // track overall gain
    Idx sz0=rGen.Size();
    (void) sz0; // make compiler happy
    // good performance operations: go for fixpoint
    while(true) {
      Idx sz1=rGen.Size();
      FD_WPC(sz0, sz1, "ConflictEquivalentAbstraction: fixpoint iteration states #" << sz1);
      MergeNonCoaccessible(rGen);
      if(rSilentEvents.Size()>0){
        MergeSilentSccs(rGen,rSilentEvents);
        BlockingSilentEvent(rGen,rSilentEvents); 
        BlockingEvent(rGen,rSilentEvents);
        OnlySilentOutgoing(rGen,rSilentEvents); 
        OnlySilentIncoming(rGen,rSilentEvents);
      }
      if(rGen.Size()==sz1) break;
    }
    // break on large automata
    //if(rGen.TransRelSize()> 50000) break;
    // badly performing operations: give it a try
    Idx sz2=rGen.Size();
    ActiveEventsRule(rGen,rSilentEvents); // can cause tau-scc
    if(rSilentEvents.Size()>0) {
      MergeSilentSccs(rGen,rSilentEvents);
      SilentContinuationRule(rGen,rSilentEvents); // must not have tau-scc
    }
    if(rSilentEvents.Size()>0) {
      ObservationEquivalentQuotient(rGen,rSilentEvents); 
    }
    if(rGen.Size()==sz2) break;
    // break on slow progress
    //if(rGen.Size()> 1000 && rGen.Size()>0.95*sz0) break;
  }
  FD_DF("ConflictEquivalentAbstraction(): done with t#"<<rGen.TransRelSize());
}


// intentend user interface 
bool IsNonblocking(const GeneratorVector& rGenVec) {
  Idx i,j;

  // trivial cases
  if(rGenVec.Size()==0) return true;
  if(rGenVec.Size()==1) return IsNonblocking(rGenVec.At(0));

  // have a local copy of input generator
  GeneratorVector gvec=rGenVec;

  // figure silent events
  EventSet silent, all, shared;
  all=gvec.At(0).Alphabet();
  for(i=0;i<gvec.Size()-1;++i){
    for(j=i+1;j<gvec.Size();++j){
      const Generator& g1=gvec.At(i);
      const Generator& g2=gvec.At(j);
      shared=shared+g1.Alphabet()*g2.Alphabet();
    }
    all=all+gvec.At(i).Alphabet();
  }      
  silent=all-shared;

  // normalize for one silent event per generator
  for(i=0;i<gvec.Size();++i){
    Generator& gi=gvec.At(i);
    EventSet sili=gi.Alphabet()*silent;
    if(sili.Size()<=1) continue;
    Idx esi=*(sili.Begin());
    sili.Erase(esi);
    silent.EraseSet(sili);
    all.EraseSet(sili);
    TransSet::Iterator tit=gi.TransRelBegin();
    TransSet::Iterator tit_end=gi.TransRelEnd();
    for(;tit!=tit_end;) {
      if(!sili.Exists(tit->Ev)) {++tit; continue;}
      Transition t(tit->X1,esi,tit->X2);
      gi.ClrTransition(tit++);
      gi.SetTransition(t);
    }
    gi.InjectAlphabet(gi.Alphabet()-sili);
  } 

  // reduce all generators 
  for(i=0;i<gvec.Size();i++){
    Generator& g=gvec.At(i);
    FD_DF("Abstract generator " << g.Name());
    FD_DF("Silent events #" << (g.Alphabet()*silent).Size());
    ConflictEquivalentAbstraction(g,g.Alphabet()*silent);
    if(g.InitStates().Size()>0)
      if(g.MarkedStates().Size()==0){
        FD_DF("No marked states (A)");
        return false;
      }
  }

  // loop until resolved
  while(gvec.Size()>=2) {

    // candidat pairs with fewest transitions 'minT'
    Idx imin=0;
    for(i=1;i<gvec.Size();i++){
      if(gvec.At(i).TransRelSize()<gvec.At(imin).TransRelSize()) 
        imin=i;
    }
    
    // candidat pairs with most local events 'maxL'
    Idx jmin=0;
    Int score=-1;
    for(i=0;i<gvec.Size();i++){
      if(i==imin) continue;
      const Generator& gi=gvec.At(imin);
      const Generator& gj=gvec.At(i);
      EventSet aij=gi.Alphabet()+gj.Alphabet();
      EventSet shared;
      for(j=0;j<gvec.Size();j++){
        if(j==imin) continue;
        if(j==i) continue;
        shared=shared + gvec.At(j).Alphabet()*aij;
      }
      Int jscore= aij.Size()-shared.Size();
      if(jscore>score) {score=jscore; jmin=i;}
    }

    // candidat pairs with fewest states when composed 'minS'
    /*
    Idx jmin=0;
    Float score=-1;
    for(i=0;i<gvec.Size();i++){
      if(i==imin) continue;
      const Generator& gi=gvec.At(imin);
      const Generator& gj=gvec.At(i);
      Int jscore= gi.Size()*gj.Size()/((Float) gi.AlphabetSize()*gj.AlphabetSize()); // rough estimate
      if(jscore<score || score<0) {score=jscore; jmin=i;}
    }
    */

    // compose candidate pair
    Generator& gimin=gvec.At(imin);
    Generator& gjmin=gvec.At(jmin);
    FD_DF("Compose generator " << gimin.Name() << " and " << gjmin.Name());
    Parallel(gimin,gjmin,gimin);
    gvec.Erase(jmin);
    FD_DF("Composition done t#"<< gimin.TransRelSize());

    // update shared events
    EventSet silent, all, shared;
    all=gvec.At(0).Alphabet();
    for(i=0;i<gvec.Size()-1;++i){
      for(j=i+1;j<gvec.Size();++j){
        const Generator& g1=gvec.At(i);
        const Generator& g2=gvec.At(j);
        shared=shared+g1.Alphabet()*g2.Alphabet();
      }
      all=all+gvec.At(i).Alphabet();
    }      
    silent=all-shared;

    // normalize for one silent event per generator
    EventSet sili=gimin.Alphabet()*silent;
    if(sili.Size()>1) {
      Idx esi=*(sili.Begin());
      sili.Erase(esi);
      silent.EraseSet(sili);
      TransSet::Iterator tit=gimin.TransRelBegin();
      TransSet::Iterator tit_end=gimin.TransRelEnd();
      for(;tit!=tit_end;) {
        if(!sili.Exists(tit->Ev)) {++tit; continue;}
        Transition t(tit->X1,esi,tit->X2);
        gimin.ClrTransition(tit++);
        gimin.SetTransition(t);
      }
      gimin.InjectAlphabet(gimin.Alphabet()-sili);
    }

    // abstract
    if(gvec.Size()>1){
      FD_DF("Abstract generator " << gimin.Name());
      FD_DF("Silent events #" << (gimin.Alphabet()*silent).Size());
      ConflictEquivalentAbstraction(gimin,gimin.Alphabet()*silent);
      if(gimin.InitStates().Size()>0)
      if(gimin.MarkedStates().Size()==0){
        FD_DF("No marked states (A)");
       	  return false;
      }
    }
  
  }

  FD_DF("Testing final stage with t#" << gvec.At(0).TransRelSize() << "/s#" << gvec.At(0).Size());
  bool res=gvec.At(0).BlockingStates().Size()==0;
  FD_DF("IsNonblocking(): done");
  return res;
}




} // end namespace
