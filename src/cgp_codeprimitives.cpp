/** @file cgp_codeprimitives.cpp @brief Code-generator with abstract types and operations */

/*

  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2010, 2016, 2017, Thomas Moor

*/

// my includes
#include "cgp_codeprimitives.h"
#include <cctype>

/*e
******************************************************************
******************************************************************
******************************************************************

CodePrimitives implementation --- maintaine class

******************************************************************
******************************************************************
******************************************************************
*/


// CodePrimitives(void)
CodePrimitives::CodePrimitives(void) : CodeGenerator() {
  FD_DCG("CodePrimitives(" << this << ")::CodePrimitives()");
}

// CodePrimitives(void)
CodePrimitives::~CodePrimitives(void) {
  FD_DCG("CodePrimitives(" << this << ")::~CodePrimitives()");
}


// Clear()
void CodePrimitives::Clear(void) {
  FD_DCG("CodePrimitives(" << this << ")::Clear()");
  // call base
  CodeGenerator::Clear();
  // clear my configuration parameters
  mPrefix="faudes_";
  mWordType="word_type";
  mWordSize=8;
  mIntegerType="int_type";
  mIntegerSize=8;
  mArrayForTransitions=HasCintarray();
  mMaintainStateIndices=false;
  mArrayForBitmasks=HasCwordarray();
  mBitAddressArithmetic= HasIntmaths() && !mArrayForBitmasks;
  mArrayForBitarray= HasWordarray();
  mBisectionForBitfind=true;
  mArrayForState=false;
  mEventsetsForPerformance=false;
  mLoopPendingInputs=false;
  mLoopEnabledOutputs=false;
  mStrictEventSynchronisation=false;
  mEventNameLookup=false;
  mStateNameLookup=false;
  mEventExecutionHook="";
  mStateUpdateHook="";
  mLiteralPrepend="";
  mLiteralAppend="";
}


//DoReadTargetConfiguration(rTr)
void CodePrimitives::DoReadTargetConfiguration(TokenReader& rTr) {
  FD_DCG("CodePrimitives::DoReadTargetConfiguration()");
  Token token;
  // call base
  CodeGenerator::DoReadTargetConfiguration(rTr);
  // prefix
  if(rTr.ExistsBegin("Prefix")) {
    rTr.ReadBegin("Prefix",token);
    mPrefix=token.AttributeStringValue("val");
    if(mPrefix!="") {
      if(*mPrefix.rbegin()!='_') mPrefix=mPrefix+"_";
    }
    rTr.ReadEnd("Prefix");
  }
  // integral target type
  if(rTr.ExistsBegin("WordType")) {
    rTr.ReadBegin("WordType",token);
    mWordType=token.AttributeStringValue("val");
    rTr.ReadEnd("WordType");
  }
  // wordsize
  if(rTr.ExistsBegin("WordSize")) {
    rTr.ReadBegin("WordSize",token);
    mWordSize=token.AttributeIntegerValue("val");
    rTr.ReadEnd("WordSize");
  }
  // unsigned integral type
  if(rTr.ExistsBegin("IntegerType")) {
    rTr.ReadBegin("IntegerType",token);
    mIntegerType=token.AttributeStringValue("val");
    rTr.ReadEnd("IntegerType");
  }
  // intsize
  if(rTr.ExistsBegin("IntegerSize")) {
    rTr.ReadBegin("IntegerSize",token);
    mIntegerSize=token.AttributeIntegerValue("val");
    rTr.ReadEnd("IntegerSize");
  }
  // code option
  if(rTr.ExistsBegin("ArrayForTransitions")) {
    rTr.ReadBegin("ArrayForTransitions",token);
    mArrayForTransitions= token.AttributeIntegerValue("val");
    rTr.ReadEnd("ArrayForTransitions");
  }
  // code option
  if(rTr.ExistsBegin("MaintainStateIndices")) {
    rTr.ReadBegin("MaintainStateIndices",token);
    mMaintainStateIndices= token.AttributeIntegerValue("val");
    rTr.ReadEnd("MaintainStateindices");
  }
  // code option
  if(rTr.ExistsBegin("ArrayForBitmasks")) {
    rTr.ReadBegin("ArrayForBitmasks",token);
    mArrayForBitmasks= token.AttributeIntegerValue("val");
    rTr.ReadEnd("ArrayForBitmasks");
  }
  // code option
  if(rTr.ExistsBegin("ArrayForBitarray")) {
    rTr.ReadBegin("ArrayForBitarray",token);
    mArrayForBitarray= token.AttributeIntegerValue("val");
    rTr.ReadEnd("ArrayForBitarray");
  }
  // code option
  if(rTr.ExistsBegin("BitAddressArithmetic")) {
    rTr.ReadBegin("BitAddressArithmetic",token);
    mBitAddressArithmetic= token.AttributeIntegerValue("val");
    rTr.ReadEnd("BitAddressArithmetic");
  }
  // code option
  if(rTr.ExistsBegin("BisectionForBitfind")) {
    rTr.ReadBegin("BisectionForBitfind",token);
    mBisectionForBitfind= token.AttributeIntegerValue("val");
    rTr.ReadEnd("BisectionForBitfind");
  }
  // code option
  if(rTr.ExistsBegin("ArrayForState")) {
    rTr.ReadBegin("ArrayForState",token);
    mArrayForState= token.AttributeIntegerValue("val");
    rTr.ReadEnd("ArrayForState");
  }
  // code option
  if(rTr.ExistsBegin("EventsetsForPerformance")) {
    rTr.ReadBegin("EventsetsForPerformance",token);
    mEventsetsForPerformance= token.AttributeIntegerValue("val");
    rTr.ReadEnd("EventsetsForPerformance");
  }
  // code option
  if(rTr.ExistsBegin("LoopPendingInputs")) {
    rTr.ReadBegin("LoopPendingInputs",token);
    mLoopPendingInputs= token.AttributeIntegerValue("val");
    rTr.ReadEnd("LoopPendingInputs");
  }
  // code option
  if(rTr.ExistsBegin("LoopEnabledOutputs")) {
    rTr.ReadBegin("LoopEnabledOutputs",token);
    mLoopEnabledOutputs= token.AttributeIntegerValue("val");
    rTr.ReadEnd("LoopEnabledOutputs");
  }
  // code option
  if(rTr.ExistsBegin("StrictEventSynchronisation")) {
    rTr.ReadBegin("StrictEventSynchronisation",token);
    mStrictEventSynchronisation= token.AttributeIntegerValue("val");
    rTr.ReadEnd("StrictEventSynchronisation");
  }
  // code option
  if(rTr.ExistsBegin("EventNameLookup")) {
    rTr.ReadBegin("EventNameLookup",token);
    mEventNameLookup= token.AttributeIntegerValue("val");
    rTr.ReadEnd("EventNameLookup");
  }
  // code option
  if(rTr.ExistsBegin("StateNameLookup")) {
    rTr.ReadBegin("StateNameLookup",token);
    mStateNameLookup= token.AttributeIntegerValue("val");
    rTr.ReadEnd("StateNameLookup");
  }
  // code option
  if(rTr.ExistsBegin("EventExecutionHook")) {
    rTr.ReadBegin("EventExecutionHook",token);
    mEventExecutionHook= token.AttributeStringValue("val");
    rTr.ReadEnd("EventExecutionHook");
  }
  // code option
  if(rTr.ExistsBegin("StateUpdateHook")) {
    rTr.ReadBegin("StateUpdateHook",token);
    mStateUpdateHook= token.AttributeStringValue("val");
    rTr.ReadEnd("StateUpdateHook");
  }
  // literal
  if(rTr.ExistsBegin("IncludeBefore"))
    rTr.ReadVerbatim("IncludeBefore",mLiteralPrepend);
  // literal
  if(rTr.ExistsBegin("IncludeAfter"))
    rTr.ReadVerbatim("IncludeAfter",mLiteralAppend);
  FD_DCG("CodePrimitives::DoReadTargetConfiguration(): done");
}



//DoWriteTargetConfiguration(rTr,rLabel)
void CodePrimitives::DoWriteTargetConfiguration(TokenWriter& rTw) const {
  // call base
  CodeGenerator::DoWriteTargetConfiguration(rTw);
  // write my configuration parameters
  Token token;
  // prefix
  token.SetEmpty("Prefix");
  token.InsAttributeString("val",mPrefix);
  rTw.Write(token);
  // word type
  token.SetEmpty("WordType");
  token.InsAttributeString("val",mWordType);
  rTw.Write(token);
  // wordsize
  token.SetEmpty("WordSize");
  token.InsAttributeInteger("val",mWordSize);
  rTw.Write(token);
  // integer type
  token.SetEmpty("IntegerType");
  token.InsAttributeString("val",mIntegerType);
  rTw.Write(token);
  // integersize
  token.SetEmpty("IntegerSize");
  token.InsAttributeInteger("val",mIntegerSize);
  rTw.Write(token);
  // code option
  token.SetEmpty("ArrayForTransitions");
  token.InsAttributeBoolean("val",mArrayForTransitions);
  rTw.Write(token);
  // code option
  token.SetEmpty("MaintainStateIndices");
  token.InsAttributeBoolean("val",mMaintainStateIndices);
  rTw.Write(token);
  // code option
  token.SetEmpty("ArrayForBitmasks");
  token.InsAttributeBoolean("val",mArrayForBitmasks);
  // code option
  token.SetEmpty("ArrayForBitarray");
  token.InsAttributeBoolean("val",mArrayForBitarray);
  rTw.Write(token);
  // code option
  token.SetEmpty("BitAddressArithmetic");
  token.InsAttributeBoolean("val",mBitAddressArithmetic);
  rTw.Write(token);
  // code option
  token.SetEmpty("BisectionForBitfind");
  token.InsAttributeBoolean("val",mBisectionForBitfind);
  rTw.Write(token);
  // code option
  token.SetEmpty("ArrayForState");
  token.InsAttributeBoolean("val",mArrayForState);
  rTw.Write(token);
  // code option
  token.SetEmpty("EventsetsForPerformance");
  token.InsAttributeBoolean("val",mEventsetsForPerformance);
  rTw.Write(token);
  // code option
  token.SetEmpty("LoopPendingInputs");
  token.InsAttributeBoolean("val",mLoopPendingInputs);
  rTw.Write(token);
  // code option
  token.SetEmpty("LoopEnabledOutputs");
  token.InsAttributeBoolean("val",mLoopEnabledOutputs);
  rTw.Write(token);
  // code option
  token.SetEmpty("StrictEventSynchronisation");
  token.InsAttributeBoolean("val",mStrictEventSynchronisation);
  rTw.Write(token);
  // code option
  token.SetEmpty("EventNameLookup");
  token.InsAttributeBoolean("val",mEventNameLookup);
  rTw.Write(token);
  // code option
  token.SetEmpty("StateNameLookup");
  token.InsAttributeBoolean("val",mStateNameLookup);
  // code option
  token.SetEmpty("EventExecutionHook");
  token.InsAttributeString("val",mEventExecutionHook);
  rTw.Write(token);
  // code option
  token.SetEmpty("StateUpdateHook");
  token.InsAttributeString("val",mStateUpdateHook);
  rTw.Write(token);
  // preamble/main
  if(mLiteralPrepend.size()>0)
    rTw.WriteVerbatim("IncludeBefore",mLiteralPrepend);
  if(mLiteralAppend.size()>0)
    rTw.WriteVerbatim("IncludeAfter",mLiteralAppend);
}

// DoCompile()
void CodePrimitives::DoCompile(void) {
  // figure which generators provide statenames
  mExistStateNames=false;
  mHasStateNames.assign(Size(),false);
  for(size_t gid=0; gid<Size(); ++gid) {
    const faudes::Generator& gen=At(gid);
    int mem=0;
    faudes::StateSet::Iterator sit=gen.StatesBegin();
    for(;sit!=gen.StatesEnd();++sit)
      mem += (int) gen.StateName(*sit).size();
    if(mem>0) {
      mHasStateNames[gid]=true;
      mExistStateNames=true;
    }
    // report
    if(mem>0) {
      FCG_VERB0("CodeGenerator::Compile(): [" << gen.Name() << "] state names amount to #"
        << mem << " bytes");
    }
  }
  // figure which state index scheme to use
  if((!mStateUpdateHook.empty()) && (!mMaintainStateIndices)) {
    FCG_VERB1("CodePrimitives::Compile(): state update hook implies to maintain state indices as provided");
    mMaintainStateIndices=true;
  }
  if(mStateNameLookup && !HasCstrarray()) {
    FCG_VERB1("CodePrimitives::Compile(): cannot generate lookup tables for the specified platforms");
    mStateNameLookup=false;
  }
  if(mStateNameLookup && !mExistStateNames) {
    FCG_VERB1("CodePrimitives::Compile(): no state names found --- will generate dummy loopkups only");
  }
  // set up address scheme flags
  mUsingVectorAddressStates.assign(Size(),true);
  if(mStateNameLookup) {
    for(size_t gid=0; gid<Size(); ++gid) {
      if(mHasStateNames[gid]) mUsingVectorAddressStates[gid]=false;
    }
  }
  if(mMaintainStateIndices) {
    mUsingVectorAddressStates.assign(Size(),false);
  }
  // have consecutive states when possible/sensible
  if(!mMaintainStateIndices) {
    for(size_t git=0; git<Size(); ++git) {
      if(At(git).MaxStateIndex()==At(git).States().Size()) continue;
      if((mArrayForTransitions && !mUsingVectorAddressStates[git]) || (mStateNameLookup && mHasStateNames[git]))
       FCG_VERB0("CodeGenerator::Generator(): generator [" << At(git).Name() << "] " <<
           "has non-consecutive state indices --- applying MinStateIndex()");
       mGenerators[git].MinStateIndex();
    }
  }
  // produce warnings for non-consecutive states where relevant
  for(size_t git=0; git<Size(); ++git) {
    int gap = At(git).MaxStateIndex() - At(git).States().Size();
    if(gap==0) continue;
    if(mArrayForTransitions && !mUsingVectorAddressStates[git]) {
      FCG_VERB0("CodeGenerator::Generator(): generator [" << At(git).Name() << "] " <<
          "has non-consecutive state indices --- this will introduce an inefficient address table with #"
             << gap << " dummy entries");
    }
    if(mStateNameLookup && mHasStateNames[git]) {
      FCG_VERB0("CodeGenerator::Generator(): generator [" << At(git).Name() << "] " <<
         "has non-consecutive state indices --- this will introduce an inefficient symboltable with #"
             << gap << " dummy entries");      continue;
    }
  }

  // call base, incl transitionvector setup
  CodeGenerator::DoCompile();

  // continue test conformance
  FD_DCG("CodePrimitives(" << this << ")::DoCompile()");
  // check and fix configuration
  if(mArrayForTransitions && !HasCintarray()) {
    FCG_VERB1("CodePrimitives::Compile(): cannot generate compiled arrays for the specified target");
    mArrayForTransitions=false;
  }
  if(mArrayForBitmasks && !HasCwordarray()) {
    FCG_VERB1("CodePrimitives::Compile(): cannot generate compiled bit-masks for the specified target");
    mArrayForBitmasks=false;
  }
  if(mArrayForBitmasks && (mWordSize>8))
    FCG_VERB1("CodePrimitives::Compile(): compiled bitmasks not recommended for word-size #" << mWordSize);
  if(mArrayForBitmasks && (mWordSize>16)) {
    FCG_VERB1("CodePrimitives::Compile(): reject compiled bitmasks for word-size #" << mWordSize);
    mArrayForBitmasks = false;
  }
  if(mBitAddressArithmetic  && !HasIntmaths()) {
    FCG_VERB1("CodePrimitives::Compile(): bit address-maths not available for the specified target");
    mBitAddressArithmetic=false;
  }
  if(mEventsetsForPerformance && !( (mBitAddressArithmetic || mArrayForBitmasks) && mArrayForBitarray )) {
    FCG_VERB1("CodePrimitives::Compile(): reject option eventsets-for-performance for specified target");
    mEventsetsForPerformance = false;
  }
  /*
  if(mLoopEnabledOutputs && !mEventsetsForPerformance) {
    FCG_VERB1("CodePrimitives::Compile(): option loop-enabled-outputs suggests eventsets-for-performance");
  }
  */
  if(mLoopEnabledOutputs && !mLoopPendingInputs) {
    FCG_VERB1("CodePrimitives::Compile(): option loop-enabled-outputs implies loop-enabled-inputs");
  }
  if(mLoopEnabledOutputs) {
    if(mLastOutputEvent+1+mInternalEvents.Size() != mUsedEvents.Size()) {
      FCG_ERR("CodePrimitives::Compile(): option loop-enabled-outputs requires high priority for output events");
    }
  }
  if(mEventNameLookup && !HasCstrarray()) {
    FCG_VERB1("CodePrimitives::Compile(): cannot generate lookup tables for the specified platform");
    mEventNameLookup=false;
  }
  if((!mStateUpdateHook.empty()) && (!HasIntarray())) {
    FCG_VERB1("CodePrimitives::Compile(): cannot implement state-update hook for the specified platform");
    mStateUpdateHook="";
  }
  if((!mStateUpdateHook.empty()) && (!mArrayForState)) {
    FCG_VERB1("CodePrimitives::Compile(): state-update hook implies array representation of overall state");
    mArrayForState=true;
  }
}

/*
******************************************************************
******************************************************************
******************************************************************

CodePrimitives implementation: internal data organisation

******************************************************************
******************************************************************
******************************************************************
*/


// mangle an arbitrary string to a valid target symbol
std::string CodePrimitives::TargetSymbol(const std::string& str) {
  // could have a static lookup here ... avoid doublets aso .. use synthetic symbols
  std::string res;
  std::string::const_iterator sit=str.begin();
  for(;sit!=str.end();++sit){
    if(std::isalpha(*sit)) { res.push_back(*sit); continue; };
    if(std::isdigit(*sit) && res.size()>0) { res.push_back(*sit); continue; }
    if(res.size()>0) res.push_back('_');
  }
  return res;
}


// effective target state index
int CodePrimitives::StateTargetIdx(size_t git, Idx idx) {
  // if we compile with transition array, let base respond for vetor address when applicable
  if(mArrayForTransitions) return CodeGenerator::StateTargetIdx(git,idx);
  // else default to original index, which is consecutive starting with 1
  return idx;
}

// retrieve faudes state idx
Idx CodePrimitives::StateFaudesIdx(size_t git, int idx) {
  // if we compile with transition array, let base respond for vetor addrss when applicable
  if(mArrayForTransitions) return CodeGenerator::StateFaudesIdx(git,idx);
  // else default to original index, which is consecutive starting with 1
  return idx;
}



/*
******************************************************************
******************************************************************
******************************************************************

CodePrimitives implementation --- generate code blocks

******************************************************************
******************************************************************
******************************************************************
*/


// code blocks: literal prepend from configuration
void CodePrimitives::LiteralPrepend(void){
  if(mLiteralPrepend.size()==0) return;
  Comment("************************************************");
  Comment("* prepend code snippet from configuration      *");
  LineFeed(2);
  Output() << mLiteralPrepend;
  LineFeed(2+2);
}


// code blocks: variables (global)
void CodePrimitives::DeclareStatus(void){
  Comment("status (0x00 <> waiting, 0x01<>executing, 0x02<>err)");
  WordDeclare(AA("status"));
  LineFeed(1);
}

// code blocks: variables (global)
void CodePrimitives::DeclareReset(void){
  Comment("external reset");
  BooleanDeclare(AA("reset"), false);
  LineFeed(1);
}

// code blocks: variables (global)
void CodePrimitives::DeclareRecentEvent(void){
  Comment("recent event (0<>none)");
  IntegerDeclare(AA("recent_event"));
  LineFeed(1);
}


// code blocks: variables (opt global)
void CodePrimitives::DeclareParallelState(void){
  Comment("parallel state");
  if(mArrayForState) {
    IntarrayDeclare(AA("parallel_state"),0,Size());
  } else {
    for(Idx gid=0; gid< Size(); gid++)
      IntegerDeclare(AA("parallel_state").Sub(gid));
  }
  LineFeed(1);
}

// code blocks: variables (opt global)
void CodePrimitives::DeclarePendingEvents(void){
  // std event sets
  Comment("pending input events incl timer elapse");
  EventSetDeclare(AA("pending_events"));
  LineFeed(1);
  Comment("events enabled by all generators");
  EventSetDeclare(AA("enabled_events"));
  LineFeed(1);
}

// code blocks: variables (local)
void CodePrimitives::DeclareLoopState(void){
  // exec event
  Comment("executed event (-1<>init, 0<>none)");
  IntegerDeclare(AA("exec_event"),-1);
  LineFeed(1);
  // scheduled event
  Comment("scheduled event (0<>none)");
  IntegerDeclare(AA("sched_event"));
  LineFeed(1);
  // edge detection
  if(LinesBegin()!=LinesEnd()){
    Comment("line levels");
    LineIterator lit=LinesBegin();
    for(;lit!=LinesEnd();++lit) {
      AA baddr = AA("line_level").Sub(lit->second.mBitAddress);
      BooleanDeclare(baddr);
    }
    LineFeed(1);
  }
  // track loop count
  if(mLoopEnabledOutputs || mLoopPendingInputs) {
    Comment("loop flag");
    BooleanDeclare(AA("first_loop"));
    LineFeed(1);
  }
}

// code blocks: variables (local)
void CodePrimitives::DeclareTimers(void){
  if(TimersBegin()!=TimersEnd()) {
    Comment("timer states");
    TimerIterator tit= TimersBegin();
    for(;tit!=TimersEnd();++tit)
      TimerDeclare(AA(tit->second.mAddress),tit->second.mInitialValue);
    LineFeed(1);
  }
}

// code blocks: variables (local temp)
void CodePrimitives::DeclareAux(void){
  // additional aux variables
  if(mArrayForTransitions) {
    Comment("parse generator");
    EventSetDeclare(AA("aux_locenabled"));
    IntegerDeclare(AA("aux_parsetrans"));
    IntegerDeclare(AA("aux_parseevent"));
    LineFeed(1);
  }
  // aux enabled
  Comment("enabled events that can be executed");
  EventSetDeclare(AA("aux_executables"));
  LineFeed(1);
  // additional aux variables
  Comment("aux maths and stack variables ");
  WordDeclare(AA("aux_wordret"));
  if(LinesBegin()!=LinesEnd()){
    BooleanDeclare(AA("aux_edge"));
  }
  if(mBitAddressArithmetic || mArrayForBitmasks) {
    IntegerDeclare(AA("aux_wordaddr"));
    IntegerDeclare(AA("aux_bitaddr"));
  }
  if(mBitAddressArithmetic || mArrayForBitmasks || mLoopPendingInputs || mLoopPendingInputs) {
    WordDeclare(AA("aux_bitmask"));
  }
  LineFeed(1);
}

// code blocks: variables (large const)
void CodePrimitives::DeclareLargeCarray(void){
  // transition vector
  if(mArrayForTransitions) {
    for(size_t git=0; git<Size(); ++git) {
      const Generator& gen = At(git);
      Comment("generator [" + gen.Name() + "]");
      if(!mUsingVectorAddressStates[git]) {
        // set up statevector
        std::vector< int > statevect(gen.MaxStateIndex(),0);
        IndexSet::Iterator sit= gen.StatesBegin();
        for(;sit!=gen.StatesEnd();++sit)
          statevect[*sit-1]=mStateVectorAddress[git][*sit];
        // declare state vector
        CintarrayDeclare(AA("generator_states").Sub(git),1,statevect);
      }
      // declare transition vector
      CintarrayDeclare(AA("generator_transitions").Sub(git),0,mTransitionVector[git]);
      LineFeed(1);
    }
  }
}

// code blocks: variables (small const)
void CodePrimitives::DeclareSmallCarray(void){
  // bitmaks vector
  if(mArrayForBitmasks) {
    Comment("mask vectors");
    if(EventBitMaskSize()>mWordSize)
      CintarrayDeclare(AA("wordaddr_vector"),0,mWordAddressVector);
    CwordarrayDeclare(AA("bitmask_vector"),0,mBitMaskVector);
    LineFeed(1);
  }
}

// code blocks: symbol tables as array declarations
void CodePrimitives::DeclareEventNameLookup(void){
  if(!mEventNameLookup) return;
  std::vector<std::string> evlookup;\
  evlookup.resize(mUsedEvents.Size());
  faudes::EventSet::Iterator eit=mUsedEvents.Begin();
  for(;eit!=mUsedEvents.End();++eit) {
    int tidx=EventTargetIdx(*eit);
    if(((int) evlookup.size())<tidx) evlookup.resize(tidx);
    evlookup[tidx-1]=mUsedEvents.SymbolicName(*eit);
  }
  Comment("event name lookup table");
  CstrarrayDeclare(AA("event_lookup"),1,evlookup);
  LineFeed(1);
}

// code blocks: symbol tables as array declarations
void CodePrimitives::DeclareStateNameLookup(void){
  if(!mStateNameLookup) return;
  for(size_t gid=0; gid<Size(); ++gid) {
    if(!mHasStateNames[gid]) continue;
    const faudes::Generator& gen=At(gid);
    std::vector<std::string> stlookup;\
    faudes::StateSet::Iterator sit=gen.StatesBegin();
    for(;sit!=gen.StatesEnd();++sit) {
      int tidx=StateTargetIdx(gid,*sit);
      if(((int) stlookup.size())<tidx) stlookup.resize(tidx);
      std::string name=gen.StateName(*sit);
      if(name=="") name="#"+ToStringInteger(tidx);
      stlookup[tidx-1]=name;

    }
    Comment("state name lookup for generator [" + At(gid).Name() + "]");
    CstrarrayDeclare(AA("state_lookup").Sub(gid),1,stlookup);
    LineFeed(1);
  }
}

// code blocks: reset
void CodePrimitives::ResetState(void) {
  // say hello
  Comment("************************************************");
  Comment("* reset internal state                         *");
  LineFeed(1);
  Comment("set internal reset flag");
  IfTrue(TargetExpression(AA("reset")));
  IndentInc();
  IntegerAssign(AA("exec_event"),-1);
  IndentDec();
  IfEnd();
  LineFeed();
  Comment("do reset core");
  IfTrue(IntegerIsEq(AA("exec_event"),-1));
  Comment("clear status");
  IntegerAssign(AA("status"),0);
  Comment("set initial state");
  for(Idx gid=0; gid< Size(); gid++) {
    AA statevar;
    if(mArrayForState)
      statevar=IntarrayAccess(AA("parallel_state"), gid);
    else
      statevar= AA("parallel_state").Sub(gid);
    IntegerAssign(statevar,StateTargetIdx(gid,*At(gid).InitStatesBegin()));
  }
  Comment("clear scheduled event");
  IntegerAssign(AA("sched_event"),0);
  Comment("clear recent event");
  IntegerAssign(AA("recent_event"),0);
  Comment("clear pending/enabled events");
  EventSetClear(AA("pending_events"));
  EventSetClear(AA("enabled_events"));
  if(TimersBegin()!=TimersEnd()) {
    Comment("reset timer");
    TimerIterator tit= TimersBegin();
    for(;tit!=TimersEnd();++tit) {
      TimerStop(AA(tit->second.mAddress));
      TimerReset(AA(tit->second.mAddress) ,tit->second.mInitialValue);
    }
  }
  InsertExecHooks();
  IfEnd();
  LineFeed(1);
}

// code blocks: reset
void CodePrimitives::ResetReturn(void) {
    // say hello
    Comment("************************************************");
    Comment("* bail out on external reset                   *");
    LineFeed(1);
    IfTrue(TargetExpression(AA("reset")));
    IndentInc();
    FunctionReturn();
    IndentDec();
    IfEnd();
    LineFeed(1+2);
}

// code blocks: inputs
void CodePrimitives::SenseInputs(void){
  // skip this section
  if(FlagsBegin()==FlagsEnd())
    if(LinesBegin()==LinesEnd())
      return;
  // say hello
  Comment("************************************************");
  Comment("* sense input events                           *");
  LineFeed(1);
  // my iterators
  EventSet::Iterator eit;
  LineIterator lit;
  FlagIterator fit;
  // reset edges, sense statics
  if(LinesBegin()!=LinesEnd()) {
    Comment("init: reset all line data and generate statics");
    IfTrue(IntegerIsEq(AA("exec_event"),-1));
    lit=LinesBegin();
    for(;lit!=LinesEnd();++lit) {
      AA baddr = AA("line_level").Sub(lit->second.mBitAddress);
      BooleanAssign(baddr,ReadInputLine(AA(lit->second.mAddress))); // this is actually an expresssion
      if(lit->second.mPosStatics.Empty() && lit->second.mNegStatics.Empty()) continue;
      if(!lit->second.mPosStatics.Empty())
        IfTrue(TargetExpression(baddr));
      eit=lit->second.mPosStatics.Begin();
      for(;eit!=lit->second.mPosStatics.End();++eit) {
        Comment("positive value: trigger init event [" + EventName(*eit) + "]");
        EventSetInsert(AA("pending_events"),*eit);
      }
      if(!lit->second.mPosStatics.Empty())
      if(lit->second.mNegStatics.Empty())
        IfEnd();
      if(!lit->second.mPosStatics.Empty())
      if(!lit->second.mNegStatics.Empty())
        IfElse();
      if(lit->second.mPosStatics.Empty())
      if(!lit->second.mNegStatics.Empty())
        IfFalse(TargetExpression(baddr));
      eit=lit->second.mNegStatics.Begin();
      for(;eit!=lit->second.mNegStatics.End();++eit) {
        Comment("negative value: trigger init event [" + EventName(*eit) + "]");
        EventSetInsert(AA("pending_events"),*eit);
      }
      if(!lit->second.mNegStatics.Empty())
        IfEnd();
    }
    IfEnd();
    LineFeed(1);
    Comment("normal operation: read lines and detect edges");
    IfTrue(IntegerIsNotEq(AA("exec_event"),-1));
    lit=LinesBegin();
    for(;lit!=LinesEnd();++lit) {
      AA baddr = AA("line_level").Sub(lit->second.mBitAddress);
      Comment("read line [" + lit->second.mAddress + "]");
      BooleanAssign(AA("aux_edge"),ReadInputLine(lit->second.mAddress));
      IfTrue(BooleanIsNotEq(AA("aux_edge"),baddr));
      if(!lit->second.mPosEvents.Empty()) {
        IfTrue(TargetExpression(AA("aux_edge")));
        eit=lit->second.mPosEvents.Begin();
        for(;eit!=lit->second.mPosEvents.End();++eit) {
          Comment("positive edge: trigger input event [" + EventName(*eit) + "]");
          EventSetInsert(AA("pending_events"),*eit);
          IntegerAssign(AA("sched_event"),0);
        }
        IfEnd();
      }
      if(!lit->second.mNegEvents.Empty()) {
        IfTrue(TargetExpression(baddr));
        eit=lit->second.mNegEvents.Begin();
        for(;eit!=lit->second.mNegEvents.End();++eit) {
          Comment("negative edge trigger input event [" + EventName(*eit) + "]");
          EventSetInsert(AA("pending_events"),*eit);
          IntegerAssign(AA("sched_event"),0);
        }
        IfEnd();
      }
      BooleanAssign(baddr,TargetExpression(AA("aux_edge")));
      IfEnd();
    }
    IfEnd();
    LineFeed(1);
  }
  // detect flags
  if(FlagsBegin()!=FlagsEnd()) {
    //Comment("scanning flags");
    fit=FlagsBegin();
    for(;fit!=FlagsEnd();++fit) {
      Comment("testing flag [" + fit->second.mAddress + "]");
      IfTrue(InputExpression(fit->second.mAddress));
      eit=fit->second.mEvents.Begin();
      for(;eit!=fit->second.mEvents.End();++eit) {
        Comment("trigger input event [" + EventName(*eit) + "]");
        EventSetInsert(AA("pending_events"),*eit);
        IntegerAssign(AA("sched_event"),0);
      }
      IfEnd();
    }
    LineFeed(1);
  }
  LineFeed(2);
}


// code blocks: sense timer elapse
void CodePrimitives::SenseTimerElapse(void){
  // skip this section
  if(TimersBegin()==TimersEnd()) return;
  // say hello
  Comment("************************************************");
  Comment("* sense timer elapse                           *");
  LineFeed(1);
  // loop all timer definitions
  TimerIterator tit= TimersBegin();
  for(;tit!=TimersEnd();++tit) {
    IfTrue(TimerIsElapsed(AA(tit->second.mAddress)));
    TimerStop(AA(tit->second.mAddress));
    EventSetInsert(AA("pending_events"),EventIndex(tit->second.mElapseEvent));
    IntegerAssign(AA("sched_event"),0);
    IfEnd();
  }
  LineFeed(1+2);
}


// code blocks: loop enabled events
void CodePrimitives::BeginExecutionLoop(void){
  if(! (mLoopEnabledOutputs || mLoopPendingInputs)) return;
  Comment("************************************************");
  Comment("* event execution loop                         *");
  LineFeed(1);
  Comment("clear status to waiting");
  IntegerAssign(AA("status"),0);
  Comment("clear event report");
  IntegerAssign(AA("recent_event"),0);
  Comment("set entry flag");
  BooleanAssign(AA("first_loop"),true);
  LoopBegin();
  IndentDec();
  LineFeed(1+2);
}


// code blocks: enabled events
void CodePrimitives::UpdateEnabledBySwitching(void){
  Comment("************************************************");
  Comment("* update enabled events after execution        *");
  LineFeed(1);
  // doit: set up enabled events
  Comment("if event was executed (and on initialisation)");
  IfTrue(IntegerIsNotEq(AA("exec_event"),0));
  Comment("set all to enabled");
  EventSetFull(AA("enabled_events"));
  LineFeed(1);
  // for each generator
  for(Idx gid=0; gid< Size(); gid++) {
    Comment("restricting enabled events by [" + At(gid).Name() + "]");
    AA statevar;
    if(mArrayForState)
      statevar=IntarrayAccess(AA("parallel_state"), gid);
    else
      statevar= AA("parallel_state").Sub(gid);
    SwitchBegin(statevar);
    // cases for each state
    StateSet::Iterator sit;
    for(sit=At(gid).StatesBegin(); sit!=At(gid).StatesEnd(); sit++) {
      EventSet disset= At(gid).Alphabet();
      disset.EraseSet(At(gid).ActiveEventSet(*sit));
      if(disset.Empty()) continue;
      SwitchCase(statevar,*sit);
      EventSetErase(AA("enabled_events"),disset);
      SwitchBreak();
    }
    SwitchEnd();
    LineFeed(1);
  } // end: for each generator
  Comment("dispose event");
  IntegerAssign(AA("exec_event"),0);
  IntegerAssign(AA("sched_event"),0);
  IfEnd();
  LineFeed(1+2);
}


// code blocks: enabled events
void CodePrimitives::UpdateEnabledByInterpreter(void){
  if(!mArrayForTransitions)
    FCG_ERR("CodePrimitives::UpdateEnabledByInterpreter(): vector representation not available");
  Comment("************************************************");
  Comment("* update enabled events after execution        *");
  LineFeed(1);
  // doit: set up enabled events
  Comment("if event was executed (and on init)");
  IfTrue(IntegerIsNotEq(AA("exec_event"),0));
  LineFeed(1);
  // special case: only one generator
  if(Size()==1) {
  EventSet empty;
    Comment("setup enabled events by [" + At(0).Name() + "]");
    AA statevar;
    if(mArrayForState)
      statevar=IntarrayAccess(AA("parallel_state"),0);
    else
      statevar= AA("parallel_state").Sub(0);
    AA gentrans= AA("generator_transitions").Sub(0);
    AA genstates= AA("generator_states").Sub(0);
    EventSetAssign(AA("enabled_events"),empty);
    if(mUsingVectorAddressStates[0]) {
      IntegerAssign(AA("aux_parsetrans"),TargetExpression(statevar));
    } else {
      IntegerAssign(AA("aux_parsetrans"),TargetExpression(CintarrayAccess(genstates,statevar)));
    }
    LoopBegin();
    IntegerAssign(AA("aux_parseevent"),TargetExpression(CintarrayAccess(gentrans,AA("aux_parsetrans"))));
    LoopBreak(IntegerIsEq(AA("aux_parseevent"),0));
    EventSetInsert(AA("enabled_events"),AA("aux_parseevent"),At(0).Alphabet());
    IntegerIncrement(AA("aux_parsetrans"),2);
    LoopEnd();
  }
  // general case
  if(Size()>1) {
    Comment("set all to enabled");
    //EventSetFull(AA("enabled_events"));
    EventSetAssign(AA("enabled_events"),mUsedEvents);
    LineFeed(1);
    // for each generator
    for(Idx gid=0; gid< Size(); gid++) {
      Comment("setup enabled events by [" + At(gid).Name() + "]");
      AA statevar;
      if(mArrayForState)
        statevar=IntarrayAccess(AA("parallel_state"), gid);
      else
        statevar= AA("parallel_state").Sub(gid);
      AA gentrans= AA("generator_transitions").Sub(gid);
      AA genstates= AA("generator_states").Sub(gid);
      EventSetAssign(AA("aux_locenabled"),Alphabet()- At(gid).Alphabet());
      if(mUsingVectorAddressStates[gid]) {
        IntegerAssign(AA("aux_parsetrans"),TargetExpression(statevar));
      } else {
        IntegerAssign(AA("aux_parsetrans"),TargetExpression(CintarrayAccess(genstates,statevar)));
      }
      LoopBegin();
      IntegerAssign(AA("aux_parseevent"),TargetExpression(CintarrayAccess(gentrans,AA("aux_parsetrans"))));
      LoopBreak(IntegerIsEq(AA("aux_parseevent"),0));
      EventSetInsert(AA("aux_locenabled"),AA("aux_parseevent"),At(gid).Alphabet());
      IntegerIncrement(AA("aux_parsetrans"),2);
      LoopEnd();
      Comment("restrict enabled events");
      EventSetRestrict(AA("enabled_events"),AA("aux_locenabled"));
      LineFeed(1);
    } // end: for each generator
  }
  Comment("dispose event");
  IntegerAssign(AA("exec_event"),0);
  IntegerAssign(AA("sched_event"),0);
  IfEnd();
  LineFeed(1+2);
}

// wrapper
void CodePrimitives::UpdateEnabled(void){
  if(mArrayForTransitions)
    UpdateEnabledByInterpreter();
  else
    UpdateEnabledBySwitching();
}


// code blocks: enabled events
void CodePrimitives::ScheduleEvent(void){
  Comment("************************************************");
  Comment("* schedule next event to execute               *");
  LineFeed(1);
  IfTrue(IntegerIsEq(AA("sched_event"),0));
  LineFeed(1);
  Comment("setup candidate set to \"pending or internal\"");
  EventSetUnion(AA("aux_executables"),AA("pending_events"),mInternalEvents + mOutputEvents);
  Comment("restrict candidate set by \"enabled\"");
  EventSetRestrict(AA("aux_executables"),AA("enabled_events"));
  LineFeed(1);
  Comment("find highest priority event (lowest bitaddress)");
  EventSetFindHighestPriority(AA("aux_executables"),AA("sched_event"));
  LineFeed(1);
  Comment("remove scheduled event from pending events");
  IfTrue(IntegerIsGreater(AA("sched_event"),0));
  EventSetErase(AA("pending_events"),AA("sched_event"));
  IfEnd();
  LineFeed(1);
  Comment("detect sync error");
  IfTrue(IntegerIsEq(AA("sched_event"),0));
  EventSetIsNotEmpty(AA("pending_events"),AA("aux_wordret"));
  IfTrue(WordIsNotEq(AA("aux_wordret"),0));
  WordOr(AA("status"),WordConstant(0x02));
  IfEnd();
  IfEnd();
  LineFeed(1);
  if(!mStrictEventSynchronisation) {
    Comment("cancel pending events if no event could be scheduled ");
    IfTrue(IntegerIsEq(AA("sched_event"),0));
    EventSetClear(AA("pending_events"));
    IfEnd();
  }
  LineFeed(1);
  IfEnd();
  LineFeed(1+2);
}


// code blocks: break loop
void CodePrimitives::BreakExecutionLoop(void){
  if(! (mLoopEnabledOutputs || mLoopPendingInputs)) return;
  Comment("************************************************");
  Comment("* break execution loop                         *");
  LineFeed(1);
  LoopBreak(IntegerIsEq(AA("sched_event"),0));
  Idx max=mLastInputEvent+1;
  if(mLoopEnabledOutputs) max=mLastOutputEvent+1;
  if(max<mUsedEvents.Size()) {
    IfFalse(TargetExpression(AA("first_loop")));
    LoopBreak(IntegerIsGreater(AA("sched_event"),max));
    IfEnd();
  }
  BooleanAssign(AA("first_loop"),false);
  LineFeed(1+2);
}

// execution hooks (dummy)
void CodePrimitives::InsertExecHooks() {
  if((mEventExecutionHook!="") || (mStateUpdateHook!=""))
    FCG_ERR("CodePrimitives::InsertExecHooks(): no hooks supported by target platform");
}


// code blocks: execute event
void CodePrimitives::ExecuteEventBySwitching(void){
  Comment("************************************************");
  Comment("* execute scheduled event                      *");
  LineFeed(1);
  IfTrue(IntegerIsNotEq(AA("sched_event"),0));
  LineFeed(1);
  IntegerAssign(AA("exec_event"),TargetExpression(AA("sched_event")));
  IntegerAssign(AA("sched_event"),0);
  WordOr(AA("status"),WordConstant(0x01));
  LineFeed(1);
  for(Idx gid=0; gid< Size(); gid++) {      
    Comment("execute event for [" + At(gid).Name() + "]");
    // loop states
    AA statevar;
    if(mArrayForState)
      statevar=IntarrayAccess(AA("parallel_state"), gid);
    else
      statevar= AA("parallel_state").Sub(gid);
    SwitchBegin(statevar);
    StateSet::Iterator sit;
    for(sit=At(gid).StatesBegin(); sit!=At(gid).StatesEnd(); sit++) {
      // cases for each state
      SwitchCase(statevar,*sit);      
      // consolidate sub-cases aka same target state for multiple events
      std::map< int , int > casedata;
      std::set< std::set< int > > casesets;
      TransSet::Iterator tit;
      for(tit=At(gid).TransRelBegin(*sit); tit!=At(gid).TransRelEnd(*sit); tit++)
        casedata[EventTargetIdx(tit->Ev)]= tit->X2;
      ConsolidateCaseSets<int>(casedata,casesets);
      // loop transitions
      SwitchBegin(AA("exec_event"));
      std::set< std::set<int> >::iterator cit=casesets.begin();
      for(; cit!=casesets.end(); ++cit) {
        SwitchCases(AA("exec_event"),*cit);
        IntegerAssign(statevar,casedata[*cit->begin()]);
        SwitchBreak();
      }
      SwitchEnd();
      SwitchBreak();
    }
    SwitchEnd();
    LineFeed(1);
  } // end for each generator
  // record
  Comment("record");
  IntegerAssign(AA("recent_event"),TargetExpression(AA("exec_event")));
  // call hooks
  InsertExecHooks();
  // done
  IfEnd();
  LineFeed(1+2);
}

// code blocks: execute event
void CodePrimitives::ExecuteEventByInterpreter(void){
  Comment("************************************************");
  Comment("* execute scheduled event                      *");
  LineFeed(1);
  if(!mArrayForTransitions) {
    FCG_ERR("CodePrimitives::ExecuteByInterpreter(): vector representation not available");
  }
  // execute by array interpreter
  IfTrue(IntegerIsNotEq(AA("sched_event"),0));
  LineFeed(1);
  IntegerAssign(AA("exec_event"),TargetExpression(AA("sched_event")));
  IntegerAssign(AA("sched_event"),0);
  WordOr(AA("status"),WordConstant(0x01));
  LineFeed(1);
  for(Idx gid=0; gid< Size(); gid++) {
    Comment("execute event for [" + At(gid).Name() + "]");
    // vector to find event
    AA statevar;
    if(mArrayForState)
      statevar=IntarrayAccess(AA("parallel_state"), gid);
    else
      statevar= AA("parallel_state").Sub(gid);
    AA gentrans= AA("generator_transitions").Sub(gid);
    AA genstates= AA("generator_states").Sub(gid);

    if(mUsingVectorAddressStates[gid]) {
      IntegerAssign(AA("aux_parsetrans"),TargetExpression(statevar));
    } else {
      IntegerAssign(AA("aux_parsetrans"),TargetExpression(CintarrayAccess(genstates,statevar)));
    }
    LoopBegin();
    IntegerAssign(AA("aux_parseevent"),TargetExpression(CintarrayAccess(gentrans,AA("aux_parsetrans"))));
    LoopBreak(IntegerIsEq(AA("aux_parseevent"),TargetExpression(AA("exec_event"))));
    LoopBreak(IntegerIsEq(AA("aux_parseevent"),0));
    IntegerIncrement(AA("aux_parsetrans"),2);
    LoopEnd();
    IfTrue(IntegerIsGreater(AA("aux_parseevent"),0));
    IntegerIncrement(AA("aux_parsetrans"));
    IntegerAssign(statevar,TargetExpression(CintarrayAccess(gentrans,AA("aux_parsetrans"))));
    IfEnd();
    LineFeed(1);
  } // end for each generator
  // record
  Comment("record");
  IntegerAssign(AA("recent_event"),TargetExpression(AA("exec_event")));
  // call hooks
  InsertExecHooks();
  // done
  IfEnd();
  LineFeed(1+2);
}

// wrapper
void CodePrimitives::ExecuteEvent(void){
  if(mArrayForTransitions)
    ExecuteEventByInterpreter();
  else
    ExecuteEventBySwitching();
}

// code blocks: excute timers
void CodePrimitives::OperateTimers(void){
  //skip this section
  if(TimerActionsBegin()==TimerActionsEnd()) return;
  Comment("************************************************");
  Comment("* operate timers                               *");
  LineFeed(1);
  // consolidate cases aka same timer actions for multiple event
  std::map< int , TimerAction> casedata;
  std::set< std::set< int > > casesets;
  for(int tev=1; tev<=EventBitMaskSize(); ++tev) {
    // test for valid record, i.e., whether switch case exists
    std::string ev=EventName(EventFaudesIdx(tev));
    TimerActionIterator eit= mTimerActions.find(ev);
    if(eit==mTimerActions.end()) continue;
    // record
    casedata[tev]= eit->second;
  }
  ConsolidateCaseSets<TimerAction>(casedata,casesets);
  // if an event was executed
  IfTrue(IntegerIsGreater(AA("exec_event"),0));
  // switch events
  SwitchBegin(AA("exec_event"));
  // loop over merged switch cases
  std::set< std::set< int > >::iterator cit;
  for(cit=casesets.begin(); cit!=casesets.end(); ++cit) {
    SwitchCases(AA("exec_event"),*cit);
    TimerAction tac = casedata[*cit->begin()];
    std::set< std::string >::iterator ait;
    ait = tac.mTimerStops.begin();
    for(; ait != tac.mTimerStops.end(); ++ait) {
      TimerStop(AA(*ait));
    }
    ait = tac.mTimerResets.begin();
    for(; ait != tac.mTimerResets.end(); ++ait) {
      TimerIterator tit = mTimers.find(*ait);
      if(tit==mTimers.end()) {
        FCG_ERR("CodePrimitives: internal error on timer records -- sorry");
      }
      TimerReset(AA(*ait),tit->second.mInitialValue);
    }
    ait = tac.mTimerStarts.begin();
    for(; ait != tac.mTimerStarts.end(); ++ait) {
      TimerStart(AA(*ait));
    }
    SwitchBreak();
  }
  SwitchEnd();
  IfEnd();
  LineFeed(1+2);
}


// code blocks: execute outputs
void CodePrimitives::OperateOutputs(void){
  Comment("************************************************");
  Comment("* operate outputs                              *");
  LineFeed(1);
  // test whether exec is an output
  IfTrue(IntegerIsGreater(AA("exec_event"),mLastInputEvent+1));
  if(mLastOutputEvent+1< (int) mUsedEvents.Size())
    IfTrue(IntegerIsLess(AA("exec_event"),mLastOutputEvent+2));
  SwitchBegin(AA("exec_event"));
  for(EventSet::Iterator eit=mOutputEvents.Begin(); eit!=mOutputEvents.End(); eit++) {
    const std::vector<AttributeCodeGeneratorEvent::OutputAction>& actions = mAlphabet.Attribute(*eit).mActions;
    if(actions.size()==0) continue;
    SwitchCase(AA("exec_event"),EventTargetIdx(*eit));
    Comment("outputs for [" + EventName(*eit) + "]");
    for(size_t i=0; i<actions.size(); i++) {
      if(actions[i].mSet) RunActionSet(actions[i].mAddress);
      if(actions[i].mClr) RunActionClr(actions[i].mAddress);
      if(actions[i].mExe) RunActionExe(AX(actions[i].mAddress));
    }
    SwitchBreak();
  }
  SwitchEnd();
  if(mLastOutputEvent+1< (int) mUsedEvents.Size())
    IfEnd();
  IfEnd();
  LineFeed(1+2);
}


// code blocks: loop enabled outputs
void CodePrimitives::EndExecutionLoop(void){
  if(! (mLoopEnabledOutputs || mLoopPendingInputs)) return;
  Comment("************************************************");
  Comment("* end execution loop                           *");
  LineFeed(1);
  IndentInc();
  LoopEnd();
  LineFeed(1+2);
}


// code blocks: literal append from configuration
void CodePrimitives::LiteralAppend(void){
  if(mLiteralAppend.size()==0) return;
  Comment("************************************************");
  Comment("* append code snippet from configuration       *");
  LineFeed(2);
  Output() << mLiteralAppend;
  LineFeed(2+2);
}


// DoGenerate()
void CodePrimitives::DoGenerateDeclarations(void) {
  FD_DCG("CodePrimitives(" << this << ")::DoGenerateDeclarations()");
  Comment("************************************************");
  Comment("* declaration of variables and constants       *");
  LineFeed(1);
  DeclareStatus();
  DeclareReset();
  DeclareRecentEvent();
  DeclareParallelState();
  DeclarePendingEvents();
  DeclareLoopState();
  DeclareTimers();
  DeclareAux();
  DeclareSmallCarray();
  DeclareLargeCarray();
  DeclareEventNameLookup();
  DeclareStateNameLookup();
}

// DoGenerateResetCode()
void CodePrimitives::DoGenerateResetCode(void) {
  FD_DCG("CodePrimitives(" << this << ")::DoGenerateResetCode()");
  Comment("************************************************");
  Comment("* executor core cyclic code: reset/initialise  *");
  Comment("************************************************");
  LineFeed(1+1);
  ResetState();
  LineFeed(1+1);
  ResetReturn();
}

// DoGenerateCyclicCode()
void CodePrimitives::DoGenerateCyclicCode(void) {
  FD_DCG("CodePrimitives(" << this << ")::DoGenerateCyclicCode()");
  Comment("************************************************");
  Comment("* executor core cyclic code: begin             *");
  Comment("************************************************");
  LineFeed(1+2);
  SenseInputs();
  SenseTimerElapse();
  BeginExecutionLoop();
  UpdateEnabled();
  ScheduleEvent();
  BreakExecutionLoop();
  ExecuteEvent();
  OperateTimers();
  OperateOutputs();
  EndExecutionLoop();
  LineFeed(1);
  Comment("************************************************");
  Comment("* executor core cyclic code: end               *");
  Comment("************************************************");
}

// DoGenerate()
void CodePrimitives::DoGenerate(void) {
  // clear my data
  mBitarrays.clear();
  // generate my modules
  LiteralPrepend();
  DoGenerateDeclarations();
  LineFeed(2);
  DoGenerateCyclicCode();
  LineFeed(2+1);
  LiteralAppend();
}



/*
******************************************************************
******************************************************************
******************************************************************

CodePrimitives implementation --- generate atomic snippets

******************************************************************
******************************************************************
******************************************************************
*/

// cosmetics (base only records the text, need to rimplement in derived classes
void CodePrimitives::Comment(const std::string& text) {
  CodeGenerator::Comment(text);
}

// declaration template (optional)
void CodePrimitives::VariableDeclare(const std::string& laddr, const std::string& ltype) {
  (void) laddr;
  (void) ltype;
  FCG_ERR("CodePrimitives::VariableDeclare(): not implemented");
}

// declaration template (optional)
void CodePrimitives::VariableDeclare(const std::string& laddr, const std::string& ltype, const std::string& lval) {
  (void) laddr;
  (void) ltype;
  (void) lval;
  FCG_ERR("CodePrimitives::VariableDeclare(): not implemented");
}

// integer maths
void CodePrimitives::IntegerDecrement(const AA& address, int val) {
    IntegerIncrement(address,-val);
}

// integer maths
CodePrimitives::AX CodePrimitives::IntegerQuotient(const AX& expression, int val) {
  (void) expression;
  (void) val;
  FCG_ERR("CodePrimitives::Integer(): maths  not available");
  return AX();
  }
CodePrimitives::AX CodePrimitives::IntegerRemainder(const AX& expression, int val) {
  (void) expression;
  (void) val;
  FCG_ERR("CodePrimitives::Integer(): maths not available");
  return AX();
}
CodePrimitives::AX CodePrimitives::IntegerBitmask(const AX& expression) {
  (void) expression;
  FCG_ERR("CodePrimitives::Integer(): maths not available");
  return AX();
}
bool CodePrimitives::HasIntmaths(void) {
  return false;
}
CodePrimitives::AX CodePrimitives::IntegerIsEq(const AA& address, int val) {
  (void) address;
  (void) val;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
  return AX();
}
CodePrimitives::AX CodePrimitives::IntegerIsEq(const AA& address, const AX& expression) {
  (void) address;
  (void) expression;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
  return AX();
}
CodePrimitives::AX CodePrimitives::IntegerIsNotEq(const AA& address, int val) {
  (void) address;
  (void) val;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
  return AX();
}
CodePrimitives::AX CodePrimitives::IntegerIsNotEq(const AA& address, const AX& expression) {
  (void) address;
  (void) expression;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
  return AX();
}
CodePrimitives::AX CodePrimitives::IntegerIsGreater(const AA& address, int val) {
  (void) address;
  (void) val;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
  return AX();
}
CodePrimitives::AX CodePrimitives::IntegerIsLess(const AA& address, int val) {
  (void) address;
  (void) val;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
  return AX();
}


// word-of-bits (optional)
void CodePrimitives::WordDeclare(const AA& address) {
  (void) address;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
}
void CodePrimitives::WordDeclare(const AA& address, word_t val) {
  (void) address;
  (void) val;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
}
void CodePrimitives::WordAssign(const AA& address, word_t val) {
  (void) address;
  (void) val;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
} 
void CodePrimitives::WordAssign(const AA& address, const AX& expression) {
  (void) address;
  (void) expression;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
}
void CodePrimitives::WordOr(const AA& address, word_t val) {
  (void) address;
  (void) val;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
}
void CodePrimitives::WordOr(const AA& address, const AX& expression) {
  (void) address;
  (void) expression;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
}
void CodePrimitives::WordOr(const AA& address, const AA& op1, const AA& op2) {
  (void) address;
  (void) op1;
  (void) op2;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
}
void CodePrimitives::WordOr(const AA& address, const AA& op1, word_t op2) {
  (void) address;
  (void) op1;
  (void) op2;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
}
void CodePrimitives::WordAnd(const AA& address, word_t val) {
  (void) address;
  (void) val;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
} 
void CodePrimitives::WordAnd(const AA& address, const AX& expression) {
  (void) address;
  (void) expression;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
}
void CodePrimitives::WordAnd(const AA& address, const AA& op1, const AA& op2) {
  (void) address;
  (void) op1;
  (void) op2;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
}
void CodePrimitives::WordAnd(const AA& address, const AA& op1, word_t op2) {
  (void) address;
  (void) op1;
  (void) op2;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
}
void CodePrimitives::WordNand(const AA& address, const AX& expression) {
  (void) address;
  (void) expression;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
}
CodePrimitives::AX CodePrimitives::WordIsEq(const AA& address, word_t val) {
  (void) address;
  (void) val;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
  return AX();
}
CodePrimitives::AX CodePrimitives::WordIsNotEq(const AA& address, word_t val) {
  (void) address;
  (void) val;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
  return AX();
}
CodePrimitives::AX CodePrimitives::WordIsBitSet(const AA& address, int idx) {
  (void) address;
  (void) idx;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
  return AX();
}
CodePrimitives::AX CodePrimitives::WordIsBitClr(const AA& address, int idx) {
  (void) address;
  (void) idx;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
  return AX();
}
CodePrimitives::AX CodePrimitives::WordIsMaskSet(const AA& address, word_t idx) {
  (void) address;
  (void) idx;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
  return AX();
}
CodePrimitives::AX CodePrimitives::WordConstant(word_t val) {
  (void) val;
  FCG_ERR("CodePrimitives::Word(): word-of-bits type not available");
  return AX();
}


// fallback implementation of booleans as integers
void CodePrimitives::BooleanDeclare(const AA& address) {
  IntegerDeclare(address);
}
void CodePrimitives::BooleanDeclare(const AA& address, int val) {
  IntegerDeclare(address,val ? 1 : 0);
}
void CodePrimitives::BooleanAssign(const AA& address, const AX& expression) {
  IntegerAssign(address,expression);
}
void CodePrimitives::BooleanAssign(const AA& address, int val) {
  IntegerAssign(address, val ? 1 : 0);
}
CodePrimitives::AX CodePrimitives::BooleanIsEq(const AA& op1, const AA& op2) {
  return IntegerIsEq(op1,TargetExpression(op2));
}
CodePrimitives::AX CodePrimitives::BooleanIsNotEq(const AA& op1, const AA& op2) {
  return IntegerIsNotEq(op1,TargetExpression(op2));
}


/** @} */


// default const-int-array: not supported
void CodePrimitives::CintarrayDeclare(const AA& address, int offset, const std::vector<int>& val) {
  (void) address;
  (void) val;
  (void) offset;
  FCG_ERR("CodePrimitives::Cintarray(): constant-int-arrays not defined");
};
CodePrimitives::AA CodePrimitives::CintarrayAccess(const AA& address, int index) {
  (void) address;
  (void) index;
  FCG_ERR("CodePrimitives::Cintarray(): constant-int-arrays not defined");
  return AA();
};  
CodePrimitives::AA CodePrimitives::CintarrayAccess(const AA& address, const AA& indexaddr){
  (void) address;
  (void) indexaddr;
  FCG_ERR("CodePrimitives::Cintarray(): constant-int-arrays not defined");
  return AA();
};
bool CodePrimitives::HasCintarray(void) {
  return false;
};

// default const-word-array: not supported
void CodePrimitives::CwordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val) {
  (void) address;
  (void) offset;
  (void) val;
  FCG_ERR("CodePrimitives::Cwordarray(): constant-word-arrays not defined");
};
CodePrimitives::AA CodePrimitives::CwordarrayAccess(const AA& address, int index) {
  (void) address;
  (void) index;
  FCG_ERR("CodePrimitives::Cwordarray(): constant-word-arrays not defined");
  return AA();
};
CodePrimitives::AA CodePrimitives::CwordarrayAccess(const AA& address, const AA& indexaddr){
  (void) address;
  (void) indexaddr;
  FCG_ERR("CodePrimitives::Cwordarray(): constant-word-arrays not defined");
  return AA();
};
bool CodePrimitives::HasCwordarray(void) {
  return false;
};


// default word-array: not supported
void CodePrimitives::WordarrayDeclare(const AA& address, int offset, int len) {
  (void) address;
  (void) offset;
  (void) len;
  FCG_ERR("CodePrimitives::Wordarray(): word-arrays not available");
};
void CodePrimitives::WordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val) {
  (void) address;
  (void) offset;
  (void) val;
  FCG_ERR("CodePrimitives::Wordarray(): word-arrays not available");
};
CodePrimitives::AA CodePrimitives::WordarrayAccess(const AA& address, int index) {
  (void) address;
  (void) index;
  FCG_ERR("CodePrimitives::Wordarray(): word-arrays not available");
  return AA();
};
CodePrimitives::AA CodePrimitives::WordarrayAccess(const AA& address, const AA& indexaddr){
  (void) address;
  (void) indexaddr;
  FCG_ERR("CodePrimitives::Wordarray(): word-arrays not available");
  return AA();
};
bool CodePrimitives::HasWordarray(void) {
  return false;
};


/// default int-array: not supported
void CodePrimitives::IntarrayDeclare(const AA& address, int offset, int len) {
  (void) address;
  (void) offset;
  (void) len;
  FCG_ERR("CodePrimitives::Intarray(): int-arrays not available");
};
void CodePrimitives::IntarrayDeclare(const AA& address, int offset, const std::vector<int>& val) {
  (void) address;
  (void) offset;
  (void) val;
  FCG_ERR("CodePrimitives::Intarray(): int-arrays not available");
};
CodePrimitives::AA CodePrimitives::IntarrayAccess(const AA& address, int index) {
  (void) address;
  (void) index;
  FCG_ERR("CodePrimitives::Intarray(): int-arrays not available");
  return AA();
};
CodePrimitives::AA CodePrimitives::IntarrayAccess(const AA& address, const AA& indexaddr){
  (void) address;
  (void) indexaddr;
  FCG_ERR("CodePrimitives::Intarray(): int-arrays not available");
  return AA();
};
bool CodePrimitives::HasIntarray(void) {
  return false;
};

// default const-string-array: not supported
CodePrimitives::AX CodePrimitives::StringConstant(const std::string &val) {
  (void) val;
  FCG_ERR("CodePrimitives::Cstrarray(): constant-str-arrays not defined");
  return AX();
};
void CodePrimitives::CstrarrayDeclare(const AA& address, int offset, const std::vector<std::string>& val) {
  (void) address;
  (void) offset;
  (void) val;
  FCG_ERR("CodePrimitives::Cstrarray(): constant-str-arrays not defined");
};
CodePrimitives::AA CodePrimitives::CstrarrayAccess(const AA& address, int index) {
  (void) address;
  (void) index;
  FCG_ERR("CodePrimitives::Cstrarray(): constant-str-arrays not defined");
  return AA();
};
CodePrimitives::AA CodePrimitives::CstrarrayAccess(const AA& address, const AA& indexaddr){
  (void) address;
  (void) indexaddr;
  FCG_ERR("CodePrimitives::Cstrarray(): constant-str-arrays not defined");
  return AA();
};
bool CodePrimitives::HasCstrarray(void) {
  return false;
};


// declare bit array as record of words
void CodePrimitives::BitarrayDeclare(const AA& address, int blen) {
  // figure dimension
  int wlen = (blen + mWordSize - 1) / mWordSize;
  // declare as array or words
  if(mArrayForBitarray) {
    WordarrayDeclare(address, 0, wlen);
  }
  // declare as array separate words
  if(!mArrayForBitarray) {
    for(int i=0; i<wlen; i++) {
      AA baddr= address.Sub(i);
      WordDeclare(baddr);
    }
  }
  // record (dummy value)
  bitarray_rec newbitarray;
  newbitarray.blen= blen;
  mBitarrays[address]=newbitarray;
}  

// declare bit array as record of words
void CodePrimitives::BitarrayDeclare(const AA& address, const std::vector<bool>& val) {
  // figure dimension
  int blen = (int) val.size();
  int wlen = (blen + mWordSize - 1) / mWordSize;
  // declare as array or words
  if(mArrayForBitarray) {
    WordarrayDeclare(address, 0, WordVectorFromBitVector(val));
  }
  // declare as array separate words
  if(!mArrayForBitarray) {
    for(int i=0; i<wlen; i++) {
      AA baddr = address.Sub(i);
      WordDeclare(baddr,WordFromBitVector(val,i));
    }
  }
  // record
  bitarray_rec newbitarray;
  newbitarray.blen= blen;
  newbitarray.value= val;
  mBitarrays[address]=newbitarray;
}  

// bitarray
void CodePrimitives::BitarrayAssign(const AA& address, const std::vector<bool>& val) {
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  // loop words
  for(int i=0; i<wlen; ++i) {
    AA waddr;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,i);
    else waddr = address.Sub(i);
    word_t wval=WordFromBitVector(val,i);
    WordAssign(waddr,wval);
  }
}

// bitarray
void CodePrimitives::BitarrayAssign(const AA& address, const AA& otherarray) {
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  // must match
  if(blen!=mBitarrays[otherarray].blen) {
    FCG_ERR("CodePrimitives(): BitarrayAssign(): internal error");
  }
  // loop words
  for(int i=0; i<wlen; ++i) {
    AA waddr;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,i);
    else waddr = address.Sub(i);
    AA oaddr;
    if(mArrayForBitarray) oaddr= WordarrayAccess(address,i);
    else oaddr = address.Sub(i);
    WordAssign(waddr,TargetExpression(oaddr));
  }
}

// bitarray
void CodePrimitives::BitarrayClear(const AA& address){
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  // loop all words
  for(int i=0; i<wlen; i++) {
    AA waddr;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,i);
    else waddr = address.Sub(i);
    WordAssign(waddr, 0x0UL);
  }
}  

// bitarray
void CodePrimitives::BitarrayFull(const AA& address){
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  int bllen = blen - (wlen-1) * mWordSize;
  // loop all words
  for(int i=0; i<wlen; i++) {
    AA waddr;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,i);
    else waddr = address.Sub(i);
    word_t val= ~0x0UL;
    if(i==wlen-1) val= (1ULL<<bllen)-1;
    WordAssign(waddr, val);
  }
}  

// bitarray
void CodePrimitives::BitarrayOrAllWords(const AA& address, const AA& result) {
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  int bllen = blen - (wlen-1) * mWordSize;
  // one word case
  if(wlen==1) {
    AA waddr;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,0);
    else waddr = address.Sub(0);
    WordAssign(result,TargetExpression(waddr));
    return;
  }
  // loop all words
  for(int i=0; i<wlen; i++) {
    AA waddr;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,i);
    else waddr = address.Sub(i);
    if(i==wlen-1) WordAnd(waddr,WordConstant((1ULL<<bllen)-1));
    if(i==0) WordAssign(result,TargetExpression(waddr));
    if(i>0) WordOr(result,TargetExpression(waddr));
  }
}

// bitarray set by const index
void CodePrimitives::BitarraySetBit(const AA& address, int index){
  // figure word/bit
  int windex=index / mWordSize;
  int bindex= index % mWordSize;
  // set
  AA waddr;
  if(mArrayForBitarray) waddr= WordarrayAccess(address,windex);
  else waddr = address.Sub(windex);
  WordOr(waddr,WordConstant( 1UL << bindex));
}  

// bitarray set by var index
void CodePrimitives::BitarraySetBit(const AA& address, const AA& indexaddr, int offset, const std::vector<bool>& hint){
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  bool gotaddress=false;
  // use precompiled masks to obtain address
  if(mArrayForBitmasks && (wlen > 1)) {
    IntegerAssign(AA("aux_bitaddr"),TargetExpression(indexaddr));
    IntegerDecrement(AA("aux_bitaddr"));
    IntegerAssign(AA("aux_wordaddr"),TargetExpression(CintarrayAccess(AA("wordaddr_vector"),AA("aux_bitaddr"))));
    IntegerAssign(AA("aux_bitmask"),TargetExpression(CwordarrayAccess(AA("bitmask_vector"),AA("aux_bitaddr"))));
    gotaddress=true;
  }
  // compute address
  if((!gotaddress) && mBitAddressArithmetic && (wlen > 1)) {
    IntegerAssign(AA("aux_bitaddr"),TargetExpression(indexaddr));
    IntegerDecrement(AA("aux_bitaddr"),offset);
    IntegerAssign(AA("aux_wordaddr"),IntegerQuotient(TargetExpression(AA("aux_bitaddr")),mWordSize));
    IntegerAssign(AA("aux_bitaddr"),IntegerRemainder(TargetExpression(AA("aux_bitaddr")),mWordSize));
    IntegerAssign(AA("aux_bitmask"),IntegerBitmask(TargetExpression(AA("aux_bitaddr"))));
    gotaddress=true;
  }
  // use precompiled masks, simple case
  if((!gotaddress) && mArrayForBitmasks && (wlen == 1)) {
    IntegerAssign(AA("aux_bitaddr"),TargetExpression(indexaddr));
    IntegerDecrement(AA("aux_bitaddr"),offset);
    IntegerAssign(AA("aux_bitmask"),TargetExpression(CwordarrayAccess(AA("bitmask_vector"),AA("aux_bitaddr"))));
    gotaddress=true;
  }
  // compute address, simple case
  if((!gotaddress) && mBitAddressArithmetic && (wlen == 1)) {
    IntegerAssign(AA("aux_bitaddr"),TargetExpression(indexaddr));
    IntegerDecrement(AA("aux_bitaddr"),offset);
    IntegerAssign(AA("aux_wordaddr"),IntegerQuotient(TargetExpression(AA("aux_bitaddr")),mWordSize));
    IntegerAssign(AA("aux_bitaddr"),IntegerRemainder(TargetExpression(AA("aux_bitaddr")),mWordSize));
    IntegerAssign(AA("aux_bitmask"),IntegerBitmask(TargetExpression(AA("aux_bitaddr"))));
    gotaddress=true;
  }  
  // perform with word array
  if(gotaddress && mArrayForBitarray) {
    WordOr(WordarrayAccess(address, AA("aux_wordaddr")),TargetExpression(AA("aux_bitmask")));
    return;
  }
  // perform by switch
  if(gotaddress && (wlen  > 1)) {
    SwitchBegin(AA("aux_wordaddr"));
    for(int i=0; i<wlen; i++) {
      if(hint.size()>0)
        if(WordFromBitVector(hint,i)==0) continue;
      SwitchCase(AA("aux_wordadd"),i);
      AA waddr = address.Sub(i);
      WordOr(waddr, TargetExpression(AA("aux_bitmask")));
      SwitchBreak();
    }
    SwitchEnd();
    return;
  }
  // perform simple case
  if(gotaddress && (wlen  == 1)) {
    AA waddr = address.Sub(0);
    WordOr(waddr, TargetExpression(AA("aux_bitmask")));
    return;
  }
  // fallback: switch using the provided hint
  SwitchBegin(indexaddr);
  int baddr;
  for(baddr=0;baddr<blen;baddr++) {
    if(hint.size()>0)
      if(!hint[baddr])
        continue;
    SwitchCase(indexaddr,baddr+offset);
    BitarraySetBit(address,baddr);
    SwitchBreak();
  }
  SwitchEnd();
}

// bitarray test set by var index
void CodePrimitives::BitarrayIsBitSet(const AA& address, const AA& indexaddr, const AA& result, int offset, const std::vector<bool>& hint){
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  bool gotaddress=false;
  // use precompiled masks to obtain address
  if(mArrayForBitmasks && (wlen > 1)) {
    IntegerAssign(AA("aux_bitaddr"),TargetExpression(indexaddr));
    IntegerDecrement(AA("aux_bitaddr"),offset);
    IntegerAssign(AA("aux_wordaddr"),TargetExpression(CintarrayAccess(AA("wordaddr_vector"),AA("aux_bitaddr"))));
    IntegerAssign(AA("aux_bitmask"),TargetExpression(CwordarrayAccess(AA("bitmask_vector"),AA("aux_bitaddr"))));
    gotaddress=true;
  }
  // compute address
  if((!gotaddress) && mBitAddressArithmetic && (wlen > 1)) {
    IntegerAssign(AA("aux_bitaddr"),TargetExpression(indexaddr));
    IntegerDecrement(AA("aux_bitaddr"),offset);
    IntegerAssign(AA("aux_wordaddr"),IntegerQuotient(TargetExpression(AA("aux_bitaddr")),mWordSize));
    IntegerAssign(AA("aux_bitaddr"),IntegerRemainder(TargetExpression(AA("aux_bitaddr")),mWordSize));
    IntegerAssign(AA("aux_bitmask"),IntegerBitmask(TargetExpression(AA("aux_bitaddr"))));
    gotaddress=true;
  }
  // use precompiled masks, simple case
  if((!gotaddress) && mArrayForBitmasks && (wlen == 1)) {
    IntegerAssign(AA("aux_bitaddr"),TargetExpression(indexaddr));
    IntegerDecrement(AA("aux_bitaddr"),offset);
    IntegerAssign(AA("aux_bitmask"),TargetExpression(CwordarrayAccess(AA("bitmask_vector"),AA("aux_bitaddr"))));
    gotaddress=true;
  }
  // compute address, simple case
  if((!gotaddress) && mBitAddressArithmetic && (wlen == 1)) {
    IntegerAssign(AA("aux_bitaddr"),TargetExpression(indexaddr));
    IntegerDecrement(AA("aux_bitaddr"),offset);
    IntegerAssign(AA("aux_wordaddr"),IntegerQuotient(TargetExpression(AA("aux_bitaddr")),mWordSize));
    IntegerAssign(AA("aux_bitaddr"),IntegerRemainder(TargetExpression(AA("aux_bitaddr")),mWordSize));
    IntegerAssign(AA("aux_bitmask"),IntegerBitmask(TargetExpression(AA("aux_bitaddr"))));
    gotaddress=true;
  }
  // perform with word array
  if(gotaddress && mArrayForBitarray) {
    WordAnd(result,WordarrayAccess(address, AA("aux_wordaddr")),AA("aux_bitmask"));
  }
  // perform by switch
  if(gotaddress && (wlen  > 1)) {
    SwitchBegin(AA("aux_wordaddr"));
    for(int i=0; i<wlen; i++) {
      if(hint.size()>0)
        if(WordFromBitVector(hint,i)==0) continue;
      SwitchCase(AA("aux_wordadd"),i);
      AA waddr = address.Sub(i);
      WordAnd(result,waddr,AA("aux_bitmask"));
      SwitchBreak();
    }
    SwitchEnd();
    return;
  }
  // perform simple case
  if(gotaddress && (wlen  == 1)) {
    AA waddr = address.Sub(0);
    WordAnd(result,waddr,AA("aux_bitmask"));
    return;
  }
  // fallback: switch not implemented
  FCG_ERR("CodePrimitives::Bitarray(): BitAddressArithmetic or ArrayForBitmasks required for test by variable");
}

// bitarray
void CodePrimitives::BitarrayClrBit(const AA& address, int index){
  // figure word/bit
  int windex=index / mWordSize;
  int bindex= index % mWordSize;
  // clear
  AA waddr;
  if(mArrayForBitarray) waddr= WordarrayAccess(address,windex);
  else waddr = address.Sub(windex);
  WordNand(waddr,WordConstant( 1UL << bindex));
}  

// bitarray clear bit by index
void CodePrimitives::BitarrayClrBit(const AA& address, const AA& indexaddr, int offset, const std::vector<bool>& hint){
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  bool gotaddress=false;
  // use precompiled masks to obtain address
  if(mArrayForBitmasks && (wlen > 1)) {
      IntegerAssign(AA("aux_bitaddr"),TargetExpression(indexaddr));
    IntegerDecrement(AA("aux_bitaddr"),offset);
    IntegerAssign(AA("aux_wordaddr"),TargetExpression(CintarrayAccess(AA("wordaddr_vector"),AA("aux_bitaddr"))));
    IntegerAssign(AA("aux_bitmask"),TargetExpression(CwordarrayAccess(AA("bitmask_vector"),AA("aux_bitaddr"))));
    gotaddress=true;
  }
  // compute address
  if((!gotaddress) && mBitAddressArithmetic && (wlen > 1)) {
    IntegerAssign(AA("aux_bitaddr"),TargetExpression(indexaddr));
    IntegerDecrement(AA("aux_bitaddr"),offset);
    IntegerAssign(AA("aux_wordaddr"),IntegerQuotient(TargetExpression(AA("aux_bitaddr")),mWordSize));
    IntegerAssign(AA("aux_bitaddr"),IntegerRemainder(TargetExpression(AA("aux_bitaddr")),mWordSize));
    IntegerAssign(AA("aux_bitmask"),IntegerBitmask(TargetExpression(AA("aux_bitaddr"))));
    gotaddress=true;
  }
  // use precompiled masks, simple case
  if((!gotaddress) && mArrayForBitmasks && (wlen == 1)) {
    IntegerAssign(AA("aux_bitaddr"),TargetExpression(indexaddr));
    IntegerDecrement(AA("aux_bitaddr"),offset);
    IntegerAssign(AA("aux_bitmask"),TargetExpression(CwordarrayAccess(AA("bitmask_vector"),AA("aux_bitaddr"))));
    gotaddress=true;
  }
  // compute address, simple case
  if((!gotaddress) && mBitAddressArithmetic && (wlen == 1)) {
    IntegerAssign(AA("aux_bitaddr"),TargetExpression(indexaddr));
    IntegerDecrement(AA("aux_bitaddr"),offset);
    IntegerAssign(AA("aux_wordaddr"),IntegerQuotient(TargetExpression(AA("aux_bitaddr")),mWordSize));
    IntegerAssign(AA("aux_bitaddr"),IntegerRemainder(TargetExpression(AA("aux_bitaddr")),mWordSize));
    IntegerAssign(AA("aux_bitmask"),IntegerBitmask(TargetExpression(AA("aux_bitaddr"))));
    gotaddress=true;
  }
  // perform with word array
  if(gotaddress && mArrayForBitarray) {
    WordNand(WordarrayAccess(address, AA("aux_wordaddr")),TargetExpression(AA("aux_bitmask")));
    return;
  }
  // perform by switch
  if(gotaddress && (wlen  > 1)) {
    SwitchBegin(AA("aux_wordaddr"));
    for(int i=0; i<wlen; i++) {
      if(hint.size()>0)
        if(WordFromBitVector(hint,i)==0) continue;
      SwitchCase(AA("aux_wordadd"),i);
      AA waddr = address.Sub(i);
      WordNand(waddr, TargetExpression(AA("aux_bitmask")));
      SwitchBreak();
    }
    SwitchEnd();
    return;
  }
  // perform simple case
  if(gotaddress && (wlen  == 1)) {
    AA waddr = address.Sub(0);
    WordNand(waddr, TargetExpression(AA("aux_bitmask")));
    return;
  }
  // fallback: switch using the provided hint
  SwitchBegin(indexaddr);
  int baddr;
  for(baddr=0;baddr<blen;baddr++) {
    if(hint.size()>0)
      if(!hint[baddr])
        continue;
    SwitchCase(indexaddr,baddr+offset);
    BitarrayClrBit(address,baddr);
    SwitchBreak();
  }
  SwitchEnd();
}

// bitarray
void CodePrimitives::BitarrayOr(const AA& address, const std::vector<bool>& val) {
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  // iterate words
  for(int w=0; w<wlen; ++w) {
    word_t wval=WordFromBitVector(val,w);
    if(wval==0) continue;
    AA waddr;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,w);
    else waddr = address.Sub(w);
    WordOr(waddr,wval);
  }
}


// bitarray
void CodePrimitives::BitarrayOr(const AA& address, const AA &op1, const std::vector<bool>& op2) {
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  // iterate words
  for(int w=0; w<wlen; ++w) {
    word_t wval=WordFromBitVector(op2,w);
    AA waddr;
    AA wop1;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,w);
    else waddr = address.Sub(w);
    if(mArrayForBitarray) wop1= WordarrayAccess(op1,w);
    else wop1 = op1.Sub(w);
    if(wval!=0)
      WordOr(waddr,wop1,wval);
    else
      WordAssign(waddr,TargetExpression(wop1));
  }
}

// bitarray
void CodePrimitives::BitarrayAnd(const AA& address, const std::vector<bool>& val) {
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  // iterate words
  for(int w=0; w<wlen; ++w) {
    word_t wval=WordFromBitVector(val,w);
    if(wval== (1UL << mWordSize) - 1) continue;
    AA waddr;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,w);
    else waddr = address.Sub(w);
    WordAnd(waddr,wval);
  }
}

// intersect
void CodePrimitives::BitarrayAnd(const AA& address, const AA& otherarray) {
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  // iterate words
  for(int w=0; w<wlen; ++w) {
    AA waddr;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,w);
    else waddr = address.Sub(w);
    AA oaddr;
    if(mArrayForBitarray) oaddr= WordarrayAccess(otherarray,w);
    else oaddr = otherarray.Sub(w);
    WordAnd(waddr,TargetExpression(oaddr));
  }
}

// intersect
void CodePrimitives::BitarrayAnd(const AA& address, const AA &op1, const std::vector<bool>& op2) {
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  // iterate words
  for(int w=0; w<wlen; ++w) {
    word_t wval=WordFromBitVector(op2,w);
    AA waddr;
    AA wop1;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,w);
    else waddr = address.Sub(w);
    if(mArrayForBitarray) wop1= WordarrayAccess(op1,w);
    else wop1 = op1.Sub(w);
    if(wval!= (1UL << mWordSize) - 1)
      WordAnd(waddr,wop1,wval);
    else
      WordAssign(waddr,TargetExpression(wop1));
  }
}

// intersect
void CodePrimitives::BitarrayAnd(const AA& address, const AA& op1, const AA& op2) {
  // lookup
  int blen= mBitarrays[address].blen;
  int wlen = (blen + mWordSize - 1) / mWordSize;
  // iterate words
  for(int w=0; w<wlen; ++w) {
    AA waddr;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,w);
    else waddr = address.Sub(w);
    AA op1addr;
    if(mArrayForBitarray) op1addr= WordarrayAccess(op1,w);
    else op1addr = op1.Sub(w);
    AA op2addr;
    if(mArrayForBitarray) op2addr= WordarrayAccess(op2,w);
    else op2addr = op2.Sub(w);
    WordAnd(waddr,op1addr,op2addr);
  }
}

// find lowest index with bit set (result as an address)
void CodePrimitives::BitarrayFindFirst(const AA& address, const AA& result, int offset) {
  // prepare result
  IntegerAssign(result,offset-1);
  // lookup
  int blen= mBitarrays[address].blen;
  // iterate words
  for(int w=0; w< (blen + mWordSize -1) / mWordSize; ++w) {
    AA waddr;
    if(mArrayForBitarray) waddr= WordarrayAccess(address,w);
    else waddr = address.Sub(w);
    int ifcnt=0;
    /* as of the second word we only search if nothing found so far */
    if(w>0) {
      IfTrue(IntegerIsEq(result,offset-1));
      ++ifcnt;
    }
    /* search the current word only if non-zero */
    IfTrue(WordIsNotEq(waddr,0));
    ++ifcnt;
    /* simple case: only one bit left to search */
    if(w*mWordSize + 1 == blen) {
      IntegerAssign(result,w*mWordSize+offset);
      for(;ifcnt>0;--ifcnt) IfEnd();
      continue;
    }
    /* naive approach O(mWordSize)*/
    if(!mBisectionForBitfind) {
      for(int b=0; b<mWordSize;++b) {
        int index=w*mWordSize + b;
        if(index>=blen) break;
        if(b==0) {
          IfTrue(WordIsBitSet(waddr,b));
          ++ifcnt;
        } else {
          IfElseIfTrue(WordIsBitSet(waddr,b));
        }
        IntegerAssign(result,index+offset);
      }
    }
    /* interval search O(log(mWordSize)) */
    if(mBisectionForBitfind) {
      int wsize=mWordSize;
      int bleft=blen-w*mWordSize;
      if(bleft>wsize) bleft=wsize;
      while(wsize/2>=bleft) wsize=wsize/2;
      bool hl=true;
      int pos=0;
      int size=wsize;
      while(size<=wsize) {
        /* break on out of range */
        if(!(pos<bleft)) break;
        /* figure range to process */
        if(hl) {
          /* restrict size to remaining bits */
          while(pos+size/2>=bleft) size=size/2;
          /* process bit range [pos,pos+size) */
          // FD_WARN("range [" << pos << ",  " << pos+size << ")");
	}
        /* subdivide to process lower half */
        if(hl && (size>2)) {
          word_t ltest= (1UL << size/2) -1;
          ltest=ltest<<pos;
          IfTrue(WordIsMaskSet(waddr,ltest));
          ++ifcnt;
          size=size/2;
          continue;
        }
        /* resolve range of size 2 */
        if(hl && (size==2)) {
          IfTrue(WordIsMaskSet(waddr,(1UL << pos)));
          IntegerAssign(result, w*mWordSize + pos + offset);
          IfElse();
          IntegerAssign(result,w*mWordSize + pos + 1 + offset);
          IfEnd();
          size=2*size;
          hl=false;
          continue;
        }
        /* resolve range of size 1 */
        if(hl && (size==1)) {
          IntegerAssign(result, w*mWordSize + pos + offset);
          size=2*size;
          hl=false;
          continue;
        }
        /* sense lower half done, set up for for higher half */
        if((!hl) && ((pos % size) == 0)) {
          size=size/2;
          pos+=size;
          hl=true;
          if(pos<bleft) IfElse();
          continue;
        }
        /* sense higher half done, return to parent */
        if((!hl) && ((pos % size) != 0)) {
          --ifcnt;
          IfEnd();
          pos-=size/2;
          size=size*2;
	}
      }
    }
    /* close open ifs */
    for(;ifcnt>0;--ifcnt)
      IfEnd();
  } // end: loop words
}


// event set
void CodePrimitives::EventSetDeclare(const AA& address) {
  BitarrayDeclare(address,EventBitMaskSize());
}

// event set
void CodePrimitives::EventSetDeclare(const AA& address, const EventSet& evset) {
  std::vector<bool> evmask = EventBitMask(evset);
  BitarrayDeclare(address,evmask);
}

// event set
void CodePrimitives::EventSetClear(const AA& address) {
  BitarrayClear(address);
}

// event set
void CodePrimitives::EventSetFull(const AA& address) {
  BitarrayFull(address);
}

// set is empty
void CodePrimitives::EventSetIsNotEmpty(const AA& address, const AA& result) {
  BitarrayOrAllWords(address,result);
}

// event insert
void CodePrimitives::EventSetAssign(const AA& address, const EventSet& evset) {
  std::vector<bool> evmask = EventBitMask(evset);
  BitarrayAssign(address,evmask);
}

// event insert
void CodePrimitives::EventSetInsert(const AA& address, const EventSet& evset) {
  std::vector<bool> evmask = EventBitMask(evset);
  BitarrayOr(address,evmask);
}

// event insert
void CodePrimitives::EventSetInsert(const AA& address, const AA& evaddrexpr) {
  BitarraySetBit(address,evaddrexpr,1);
};

// event insert, candidats known
void CodePrimitives::EventSetInsert(const AA& address, const AA& evaddrexpr, const EventSet& hint) {
  std::vector<bool> evmask = EventBitMask(hint);
  BitarraySetBit(address,evaddrexpr,1,evmask);
};

// event insert
void CodePrimitives::EventSetInsert(const AA& address, Idx ev) {
  BitarraySetBit(address,EventBitAddress(ev));
}

// event erase
void CodePrimitives::EventSetErase(const AA& address, const EventSet& evset) {
  std::vector<bool> evmask = EventBitMask(evset);
  for(size_t i=0; i<evmask.size(); i++) evmask[i]= ! evmask[i];
  BitarrayAnd(address,evmask);
}

// event erase
void CodePrimitives::EventSetErase(const AA& address, Idx ev) {
  BitarrayClrBit(address,EventBitAddress(ev));
}

// event erase
void CodePrimitives::EventSetErase(const AA& address, const AA& evaddr) {
  BitarrayClrBit(address,evaddr,1);
};

// event test
void CodePrimitives::EventSetExists(const AA& address, const AA& evaddr, const AA& result, const EventSet& hint) {
  std::vector<bool> evmask = EventBitMask(hint);
  BitarrayIsBitSet(address,evaddr,result,1,evmask);
};

// event erase, candidats known
void CodePrimitives::EventSetErase(const AA& address, const AA& evaddr, const EventSet& hint) {
  std::vector<bool> evmask = EventBitMask(hint);
  BitarrayClrBit(address,evaddr,1,evmask);
};

// event restrict
void CodePrimitives::EventSetRestrict(const AA& address, const AA& otherset) {
  BitarrayAnd(address,otherset);
}

// event union
void CodePrimitives::EventSetUnion(const AA& address, const AA& op1, const EventSet& op2) {
  std::vector<bool> evmask = EventBitMask(op2);
  BitarrayOr(address,op1,evmask);
}

// event intersection
void CodePrimitives::EventSetIntersection(const AA& address, const AA& op1, const EventSet& op2) {
  std::vector<bool> evmask = EventBitMask(op2);
  BitarrayAnd(address,op1,evmask);
}

// event set find lowest index (i.e. max priority)
void CodePrimitives::EventSetFindHighestPriority(const AA& address, const AA& result) {
  BitarrayFindFirst(address,result,1);
}



// no relevant defaults in conditionals/loops
void CodePrimitives::IfTrue(const AX& expression) {
  (void) expression;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
}
// no relevant defaults in conditionals/loops
void CodePrimitives::IfFalse(const AX& expression) {
  (void) expression;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
}
// no relevant defaults in conditionals/loops
void CodePrimitives::IfWord(const AX& expression) {
  (void) expression;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
}
void CodePrimitives::IfElse(void) {
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
}
// no relevant defaults in conditionals/loops
void CodePrimitives::IfElseIfTrue(const AX& expression) {
  (void) expression;
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
}
void CodePrimitives::IfEnd(void) {
  FCG_ERR("CodePrimitives::Conditionals(): construct not available");
}
void CodePrimitives::LoopBegin(void) {
  FCG_ERR("CodePrimitives::Loops(): construct not available");
}
void CodePrimitives::LoopBreak(const AX& expression) {
  (void) expression;
  FCG_ERR("CodePrimitives::Loops(): construct not available");
}
void CodePrimitives::LoopEnd(void) {
  FCG_ERR("CodePrimitives::Loops(): construct not available");
}
void CodePrimitives::FunctionReturn(void) {
  FCG_ERR("CodePrimitives::FunctionReturn(): construct not available");
}

// switch defaults to conditionsl
void CodePrimitives::SwitchBegin(const AA& address){
  (void) address;
  FCG_ERR("CodePrimitives::Conditionals(): switch not available");
}
void CodePrimitives::SwitchCase(const AA& address, int val){
  (void) address;
  (void) val;
  FCG_ERR("CodePrimitives::Conditionals(): switch not available");
}
void CodePrimitives::SwitchCases(const AA& address, int from, int to){
  (void) address;
  (void) from;
  (void) to;
  FCG_ERR("CodePrimitives::Conditionals(): switch not available");
}
void CodePrimitives::SwitchCases(const AA& address, const std::set< int > & vals){
  (void) address;
  (void) vals;
  FCG_ERR("CodePrimitives::Conditionals(): switch not available");
}
void CodePrimitives::SwitchBreak(void){
  FCG_ERR("CodePrimitives::Conditionals(): switch not available");
}  
void CodePrimitives::SwitchEnd(void){
  FCG_ERR("CodePrimitives::Conditionals(): switch not available");
}
bool CodePrimitives::HasMultiCase(void){
  return false;
}


// output actions (not available)
void CodePrimitives::RunActionSet(const std::string& address) {
  (void) address;
  FCG_ERR("CodePrimitives::RunAction(): action not available");
}
void CodePrimitives::RunActionClr(const std::string& address) {
  (void) address;
  FCG_ERR("CodePrimitives::RunAction(): action not available");
}
void CodePrimitives::RunActionExe(const AX& expression) {
  (void) expression;
  FCG_ERR("CodePrimitives::RunAction(): action not available");
}


// read inputs (treat as native expression)
CodePrimitives::AX CodePrimitives::ReadInputLine(const std::string& address) {
  return AX(address);
}
CodePrimitives::AX CodePrimitives::InputExpression(const std::string& expression) {
  return AX(expression);
}


// timer (not available)
void CodePrimitives::TimerDeclare(const AA& address, const std::string &litval) {
  (void) address;
  (void) litval;
  FCG_ERR("CodePrimitives::Timer(): not available");
}
void CodePrimitives::TimerStart(const AA& address) {
  (void) address;
  FCG_ERR("CodePrimitives::Timer(): not available");
}
void CodePrimitives::TimerStop(const AA& address) {
  (void) address;
  FCG_ERR("CodePrimitives::Timer(): not available");
}
void CodePrimitives::TimerReset(const AA& address, const std::string &litval) {
  (void) address;
  (void) litval;
  FCG_ERR("CodePrimitives::Timer(): not available");
}
CodePrimitives::AX CodePrimitives::TimerIsElapsed(const AA& address) {
  (void) address;
  FCG_ERR("CodePrimitives::Timer(): not available");
  return AX();
}

