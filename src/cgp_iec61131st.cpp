/** @file cgp_iec61131st.cpp @brief Code-generator for target IEC 61131-3 ST  */

/*
  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2010, 2016 Thomas Moor

*/

// my includes
#include "cgp_iec61131st.h"


/*
******************************************************************
******************************************************************
******************************************************************

Iec611311stGenerator implementation --- class maintenance

******************************************************************
******************************************************************
******************************************************************
*/


// Register derived class
FAUDES_REGISTERCODEGENERATOR("st",Iec61131stCodeGenerator);


// Iec61131stCodeGenerator(void)
Iec61131stCodeGenerator::Iec61131stCodeGenerator(void) : CodePrimitives() {
  FD_DCG("Iec61131stCodeGenerator(" << this << ")::Iec61131stCodeGenerator()");
}


// Iec61131stCodeGenerator(void)
Iec61131stCodeGenerator::~Iec61131stCodeGenerator(void) {
  FD_DCG("Iec61131stCodeGenerator(" << this << ")::~Iec61131stCodeGenerator()");
}


// clear
void Iec61131stCodeGenerator::Clear(void) {
  FD_DCG("Iec61131stCodeGenerator::Clear()");
  // call base
  CodePrimitives::Clear();
  // my flavor of defaults
  mPrefix="fcg_";
  mWordType="BYTE";
  mWordSize=8;
  mIntegerType="INT";
  mIntegerSize=16;
  mIecDeclarePhysical="VAR_EXTERNAL";
  mLiteralCyclic="";
}

//DoReadTargetConfiguration(rTr)
void Iec61131stCodeGenerator::DoReadTargetConfiguration(TokenReader& rTr) {
  FD_DCG("Iec61131stCodeGenerator::DoReadTargetConfiguration()");
  // base
  CodePrimitives::DoReadTargetConfiguration(rTr);
  // iec option
  Token token;
  if(rTr.ExistsBegin("IecDeclarePhysical")) {
    rTr.ReadBegin("IecDeclarePhysical",token);
    mIecDeclarePhysical= token.AttributeStringValue("val");
    rTr.ReadEnd("IecDeclarePhysical");
  }
  // iec option
  if(rTr.ExistsBegin("IncludeCyclic"))
    rTr.ReadVerbatim("IncludeCyclic",mLiteralCyclic);
  if(rTr.ExistsBegin("IecTimeOperators")) {
    rTr.ReadBegin("IecTimeOperators",token);
    mHasIecTimeOperators= token.AttributeIntegerValue("val");
    rTr.ReadEnd("IecTimeOperators");
  }
}

//DoWriteTargetConfiguration(rTw)
void Iec61131stCodeGenerator::DoWriteTargetConfiguration(TokenWriter& rTw) const{
  FD_DCG("Iec61131stCodeGenerator::DoWriteTargetConfiguration()");
  // base
  CodePrimitives::DoWriteTargetConfiguration(rTw);
  // code option
  Token token;
  token.SetEmpty("IecDeclarePhysical");
  token.InsAttributeString("val",mIecDeclarePhysical);
  rTw.Write(token);
  // code option
  if(mLiteralCyclic.size()>0)
    rTw.WriteVerbatim("IncludeCyclic",mLiteralCyclic);
  token.SetEmpty("IecTimeOperators");
  token.InsAttributeBoolean("val",mHasIecTimeOperators);
  rTw.Write(token);
}



/*
******************************************************************
******************************************************************
******************************************************************

Iec611311stGenerator implementation --- code organisation

******************************************************************
******************************************************************
******************************************************************
*/

// DoCompile()
void Iec61131stCodeGenerator::DoCompile(void) {
  // call base
  CodePrimitives::DoCompile();
  // my preferences
  if(mBitAddressArithmetic && mArrayForBitmasks) {
    FCG_VERB1("Iec61131stCodeGenerator::Compile(): prefer compiled bitmasks over bit-address maths");
    mBitAddressArithmetic = false;
  }
  // set array for state for returnvalue
  if(mStateNameLookup &&  !mArrayForState) {
    FCG_VERB1("Iec61131stCodeGenerator::Compile(): using array for state as return value datatype");
    mArrayForState = true;
  }
}


// DoGenerate()
void Iec61131stCodeGenerator::DoGenerate(void) {
  // cut and paste from base
  mBitarrays.clear();
  // doit: say hello
  Comment("************************************************");
  Comment("CodeGenerator: Target IEC 61131 Structured Text ");
  Comment("************************************************");
  LineFeed(1);
  Comment(std::string("CompileDES ") + VersionString());
  Comment(std::string("Configuration: ") + Name());
  LineFeed(2+1);
  // snippets
  LiteralPrepend();
  // doit: function block to host cyclic code
  DoGenerateFunction();
  // doit: functions to host lookup tables
  DoGenerateLookups();
  // snippets
  LiteralAppend();
  Comment("************************************************");
  Comment("CodeGenerator: Generated Code Ends Here         ");
  Comment("************************************************");
}



// code blocks: cyclic function block
void Iec61131stCodeGenerator::DoGenerateFunction(void){
  Comment("************************************************");
  Comment("* function block to host cyclic code           *");
  Comment("************************************************");
  LineFeed(2);
  Output() << "FUNCTION_BLOCK " << mPrefix <<"cyclic_fb";
  LineFeed(2);
  Output() << "VAR_INPUT";
  LineFeed(2);
  DeclareReset();
  Output() << "END_VAR";
  LineFeed(2);
  Output() << "VAR_OUTPUT";
  LineFeed(2);
  DeclareStatus();
  if(mEventNameLookup) DeclareRecentEvent();
  if(mEventNameLookup) DeclarePendingEvents();
  if(mStateNameLookup) DeclareParallelState();
  Output() << "END_VAR";
  LineFeed(2);
  Output() << "VAR";
  LineFeed(2);
  if(!mEventNameLookup) DeclareRecentEvent();
  if(!mEventNameLookup) DeclarePendingEvents();
  if(!mStateNameLookup) DeclareParallelState();
  DeclareLoopState();
  DeclareTimers();
  DeclareAux();
  DeclareSmallCarray();
  DeclareLargeCarray();
  DeclareSystime();
  Output() << "END_VAR";
  LineFeed(2);
  if(CountImportSymbolicIo()>0) {
    Output() << "VAR_EXTERNAL";
    LineFeed(2);
    DeclareImportSymbolicIo();
    Output() << "END_VAR";
    LineFeed(2);
  }
  if(CountImportPhysicalIo()>0) {
    Output() << mIecDeclarePhysical;
    LineFeed(2);
    DeclareImportPhysicalIo();
    Output() << "END_VAR";
    LineFeed(2);
  }
  LiteralCyclic();
  DoGenerateResetCode();
  DecrementTimers();
  DoGenerateCyclicCode();
  LineFeed(1+2);
  Comment("************************************************");
  Comment("* end of cyclic function block                 *");
  LineFeed(1);
  Output() << "END_FUNCTION_BLOCK";
  LineFeed();
  LineFeed(2+1);
}


// code blocks: function to host lookup tables
void Iec61131stCodeGenerator::DoGenerateLookups(void){
  if(mEventNameLookup) {
    Comment("************************************************");
    Comment("* function to host the event name loopup table *");
    Comment("************************************************");
    LineFeed(2);
    Output() << "FUNCTION " << mPrefix <<"event_lookup_f" << " : STRING";
    LineFeed(2);
    Output() << "VAR_INPUT";
    LineFeed();
    Output() << "IDX : " << mIntegerType <<";";
    LineFeed();
    Output() << "END_VAR";
    LineFeed(2);
    Output() << "VAR CONSTANT";
    LineFeed();
    DeclareEventNameLookup();
    Output() << "END_VAR";
    LineFeed(2);
    Output() << mPrefix <<"event_lookup_f"  << " := " << mPrefix << "event_lookup[IDX]" << ";";
    LineFeed(2);
    Output() << "END_FUNCTION";
    LineFeed();
    LineFeed(2+1);
  }
  if(mStateNameLookup) {
    Comment("************************************************");
    Comment("* function to host state name loopup tables    *");
    Comment("************************************************");
    LineFeed(2);
    Output() << "FUNCTION " << mPrefix <<"state_lookup_f" << " : STRING";
    LineFeed(2);
    Output() << "VAR_INPUT";
    LineFeed();
    Output() << "GID : " << mIntegerType <<";";
    LineFeed();
    Output() << "IDX : " << mIntegerType <<";";
    LineFeed();
    Output() << "END_VAR";
    LineFeed(2);
    Output() << "VAR CONSTANT";
    LineFeed();
    DeclareStateNameLookup();
    Output() << "END_VAR";
    LineFeed(2);
    Output() << mPrefix << "state_lookup_f:='';";
    LineFeed();
    Output() << "CASE GID OF";
    LineFeed();
    for(size_t gid=0; gid<Size(); ++gid) {
      if(mHasStateNames[gid])
        Output() << ToStringInteger(gid) << ": "
                 << mPrefix <<"state_lookup_f"  << " := " << mPrefix << "state_lookup_" << ToStringInteger(gid) << "[IDX]" << ";";
      LineFeed();
    }
    Output() << "END_CASE;";
    LineFeed();
    Output() << "IF LEN(" << mPrefix << "state_lookup_f) = 0 THEN";
    LineFeed();
    Output() << "  " << mPrefix <<"state_lookup_f"  << " := CONCAT('s'," << mIntegerType << "_TO_STRING(IDX))" << ";";
    LineFeed();
    Output() << "END_IF;";
    LineFeed(2);
    Output() << "END_FUNCTION";
    LineFeed();
    LineFeed(2+1);
  }
}


// extend base reset by iec timers
void Iec61131stCodeGenerator::ResetState(void) {
  FD_DCG("Iec61131stCodeGenerator(" << this << ")::StateReset()");
  // call base
  CodePrimitives::ResetState();
  // have iec extras
  UpdateSystime();
}



// code blocks: extend base timer declaration
void Iec61131stCodeGenerator::DeclareTimers(void){
  // call base
  CodePrimitives::DeclareTimers();
  // extend
  if(TimersBegin()!=TimersEnd()) {
    Comment("elapsed time since last invokation");
    VariableDeclare(TargetAddress(AA("time_elapsed")),"TIME");
    LineFeed();
  }
}


// code blocks: import interface variables
int Iec61131stCodeGenerator::CountImportPhysicalIo(void){
  // bail out on unconfigured
  if(mIecDeclarePhysical=="") return 0;
  // figure count
  int iocnt=0;
  LineIterator lit=LinesBegin();
  for(;lit!=LinesEnd();++lit) {
    std::string lineaddr= lit->second.mAddress;
    if(lineaddr.size()<1) continue; // paranoia
    if(lineaddr.at(0)=='%') iocnt++;
  }
  ActionAddressIterator ait=ActionAddressesBegin();
  for(;ait!=ActionAddressesEnd();++ait) {
    if(!ait->second.mSetClr) continue;
    std::string actaddr= ait->second.mAddress;
    if(actaddr.size()<1) continue; // paranoia
    if(actaddr.at(0)=='%') ++iocnt;
  }
  return iocnt;
}


void Iec61131stCodeGenerator::DeclareImportPhysicalIo(void){
  Comment("import physical i/o addresses");
  LineIterator lit=LinesBegin();
  for(;lit!=LinesEnd();++lit) {
    std::string lineaddr= lit->second.mAddress;
    if(lineaddr.size()<1) continue; // paranoia
    if(lineaddr.at(0)=='%') {
      Output() << "AT " << lineaddr << " : BOOL;";
      LineFeed();
    }
  }
  ActionAddressIterator ait=ActionAddressesBegin();
  for(;ait!=ActionAddressesEnd();++ait) {
    if(!ait->second.mSetClr) continue;
    std::string actaddr= ait->second.mAddress;
    if(actaddr.size()<1) continue; // paranoia
    if(actaddr.at(0)=='%') {
      Output() << "AT " << actaddr << " : BOOL;";
      LineFeed();
    }
  }
  LineFeed(1);
}

// code blocks: import interface variables
int Iec61131stCodeGenerator::CountImportSymbolicIo(void){
  // figure count
  int iocnt=0;
  LineIterator lit=LinesBegin();
  for(;lit!=LinesEnd();++lit) {
    std::string lineaddr= lit->second.mAddress;
    if(lineaddr.size()<1) continue; // paranoia
    if(lineaddr.at(0)!='%') iocnt++;
  }
  ActionAddressIterator ait=ActionAddressesBegin();
  for(;ait!=ActionAddressesEnd();++ait) {
    if(!ait->second.mSetClr) continue;
    std::string actaddr= ait->second.mAddress;
    if(actaddr.size()<1) continue; // paranoia
    if(actaddr.at(0)!='%') ++iocnt;
  }
  return iocnt;
}

void Iec61131stCodeGenerator::DeclareImportSymbolicIo(void){
  Comment("import i/o variables and addresses");
  LineIterator lit=LinesBegin();
  for(;lit!=LinesEnd();++lit) {
    std::string lineaddr= lit->second.mAddress;
    if(lineaddr.size()<1) continue; // paranoia
    if(lineaddr.at(0)!='%') {
      VariableDeclare(lineaddr,"BOOL");
      LineFeed();
    }
  }
  ActionAddressIterator ait=ActionAddressesBegin();
  for(;ait!=ActionAddressesEnd();++ait) {
    if(!ait->second.mSetClr) continue;
    std::string actaddr= ait->second.mAddress;
    if(actaddr.size()<1) continue; // paranoia
    if(actaddr.at(0)!='%') {
      VariableDeclare(actaddr,"BOOL");
      LineFeed();
    }
  }
  LineFeed(1);
}


// code blocks: literal prepend from configuration
void Iec61131stCodeGenerator::LiteralCyclic(void){
  if(mLiteralCyclic.size()==0) return;
  Comment("************************************************");
  Comment("* extra cyclic code from configuration         *");
  LineFeed();
  Output() << mLiteralCyclic;
  LineFeed(2);
  Comment("* end of extra code from configuration         *");
  Comment("************************************************");
  LineFeed(1+1);
}

// generate code
void Iec61131stCodeGenerator::InsertExecHooks(void) {
  if((mEventExecutionHook!="") || (mStateUpdateHook!=""))
    Comment("call hooks");
  if(mEventExecutionHook!="") {
    std::string hook = mEventExecutionHook + "(" + TargetExpression(AA("exec_event")) + ");";
    Output() << hook;
    LineFeed(1);
  }
  if(mStateUpdateHook!="") {
    std::string hook = mStateUpdateHook + "(" + TargetAddress(AA("parallel_state")) + ");";
    Output() << hook;
    LineFeed(1);
  }
  LineFeed(1);
}


// code blocks: organise time decrement as inline code snippet
void Iec61131stCodeGenerator::DecrementTimers(void){
  if(TimersBegin()==TimersEnd()) return;
  // timer update function
  Comment("************************************************");
  Comment("* update timer states                          *");
  LineFeed(2);
  Output() << "IF " << TargetAddress(AA("time_elapsed")) << " > " << TimeConstant(0) << " THEN";
  LineFeed();
  IndentInc();
  TimerIterator tit= TimersBegin();
  for(;tit!=TimersEnd();++tit) {
    AA cnt("timer_" + tit->second.mAddress + "_cnt");
    AA run("timer_" + tit->second.mAddress + "_run");
    Output() << "IF " << TargetAddress(AA(run)) << " THEN";
    IndentInc();
    LineFeed();
    Output() << "IF " << TargetAddress(cnt) << " > " << TargetAddress(AA("time_elapsed")) << " THEN";
    IndentInc();
    LineFeed();
    if(mHasIecTimeOperators) {
      Output() << TargetAddress(cnt) << " := " << TargetAddress(cnt) << " - " << TargetAddress(AA("time_elapsed")) << ";";
    } else {
      Output() << TargetAddress(cnt) << " := SUB_TIME(" << TargetAddress(cnt) << ", " << TargetAddress(AA("time_elapsed")) << ");";
    }
    IndentDec();
    LineFeed();
    Output() << "ELSE";
    LineFeed();
    IndentInc();
    Output() << TargetAddress(cnt) << " := TIME#0s;";
    IndentDec();
    LineFeed();
    Output() << "END_IF;";
    IndentDec();
    LineFeed();
    Output() << "END_IF;";
    LineFeed();
  }
  IndentDec();
  Output() << "END_IF;";
  LineFeed();
  LineFeed(1+2);

}

// code blocks: mimique systime by iec timer
void Iec61131stCodeGenerator::DeclareSystime(void) {
  if(TimersBegin()==TimersEnd()) return;
  Comment("iec timer to simulate system time");
  VariableDeclare(TargetAddress(AA("systime_ton")),"TON");
  VariableDeclare(TargetAddress(AA("systime_recent")),"TIME");
  VariableDeclare(TargetAddress(AA("systime_now")),"TIME");
  LineFeed();
}

// code blocks: mimique systime by iec timer
void Iec61131stCodeGenerator::UpdateSystime(void) {
  if(TimersBegin()==TimersEnd()) return;
  Comment("do reset/track systime");
  Output() << "IF " << mPrefix << "exec_event = -1 " << " THEN" << std::endl;
  Output() << "  " << mPrefix << "systime_ton(IN:=false);" << std::endl;
  Output() << "  " << mPrefix << "systime_recent := TIME#0ms;" << std::endl;
  Output() << "END_IF;" << std::endl;
  Output() << mPrefix << "systime_ton(IN:=true, PT:=TIME#2h, ET=>" << mPrefix << "systime_now);" << std::endl;
  if(mHasIecTimeOperators) {
    Output() << mPrefix << "time_elapsed := " << mPrefix << "systime_now - " << mPrefix << "systime_recent;" << std::endl;
  } else {
    Output() << mPrefix << "time_elapsed := SUB_TIME(" << mPrefix << "systime_now," << mPrefix << "systime_recent);" << std::endl;
  }
  Output() << "IF " << mPrefix << "systime_now < TIME#1h THEN" << std::endl;
  Output() << "  " << mPrefix << "systime_recent := " << mPrefix << "systime_now;" << std::endl;
  Output() << "ELSE" << std::endl;
  Output() << "  " << mPrefix <<  "systime_ton(IN:=false, PT:=TIME#2h);" << std::endl;
  Output() << "  " << mPrefix <<  "systime_ton(IN:=true, PT:=TIME#2h);" << std::endl;
  Output() << "  " << mPrefix <<  "systime_recent := TIME#0ms;" << std::endl;
  Output() << "END_IF;" << std::endl;
  LineFeed(1);
}



/*
******************************************************************
******************************************************************
******************************************************************

Iec611311stGenerator implementation --- code primitives

******************************************************************
******************************************************************
******************************************************************
*/


// generate code
void Iec61131stCodeGenerator::Comment(const std::string& text) {
  // write comment
  if(!mMuteComments) {
    Output() << "(* " << text << " *)";
    LineFeed();
  }
  // call base to record muted comments
  CodePrimitives::Comment(text);
}

// declaration template
void Iec61131stCodeGenerator::VariableDeclare(const std::string& laddr, const std::string& ltype) {
  if((ltype != mIntegerType) && (ltype != mWordType) && (ltype != "BOOL") && (ltype != "STRING") && (ltype != "TIME") && (ltype != "TON"))
    FCG_ERR("Iec61131stCodeGenerator::VariableDeclare(): unsupported type [" << ltype << "]");
  Output() << laddr << " : " << ltype << ";";
  LineFeed();
}

void Iec61131stCodeGenerator::VariableDeclare(const std::string& laddr, const std::string& ltype, const std::string& lval) {
  if((ltype != mIntegerType) && (ltype != mWordType) && (ltype != "BOOL") && (ltype != "STRING") && (ltype != "TIME"))
    FCG_ERR("Iec61131stCodeGenerator::VariableDeclare(): unsupported type [" << ltype << "]");
  Output() << laddr << " : " << ltype << " := " << lval << ";";
  LineFeed();
}

// address transformation
std::string Iec61131stCodeGenerator::TargetAddress(const AA& address) {
  std::string res;
  // cosmetic: use uppercase plain names for input/output variables
  if(address=="reset") res="RESET";
  if(address=="status") res="STATUS";
  if(mEventNameLookup) {
    if(address=="recent_event") res="RECENT_EVENT";
    if(address.find("pending_events")==0)
    if(address!="pending_events_t")
      res="PENDING_EVENTS"+address.substr(14);
    if(address.find("enabled_events")==0)
    if(address!="enabled_events_t")
      res="ENABLED_EVENTS"+address.substr(14);
  }
  if(mStateNameLookup) {
    if(address.find("parallel_state")==0)
    if(address!="parallel_state_t")
      res="PARALLEL_STATE"+address.substr(14);
  }
  // base case
  if(res=="") res=mPrefix+address;
  return res;
}

// address transformation
CodePrimitives::AX Iec61131stCodeGenerator::TargetExpression(const AA& address) {
  return AX(TargetAddress(address));
}

// generate code: integer
void Iec61131stCodeGenerator::IntegerDeclare(const AA& address){
  VariableDeclare(TargetAddress(address),mIntegerType);
}  

// generate code: integer
void Iec61131stCodeGenerator::IntegerDeclare(const AA& address, int val){
  VariableDeclare(TargetAddress(address),mIntegerType,IntegerConstant(val));
}  

// generate code: integer
void Iec61131stCodeGenerator::IntegerAssign(const AA& address, int val){
  Output() << TargetAddress(address) << " := " << IntegerConstant(val) << ";";
  LineFeed();
}  

// generate code: integer
void Iec61131stCodeGenerator::IntegerAssign(const AA& address, const AX& expression){
  Output() << TargetAddress(address) << " := " << expression << ";";
  LineFeed();
}  

// generate code: integer
void Iec61131stCodeGenerator::IntegerIncrement(const AA& address, int val) {
  if(val>0)
    Output() << TargetAddress(address) << " := " << TargetAddress(address) << " + " << IntegerConstant(val) << ";";
  else if(val<0)
    Output() << TargetAddress(address) << " := " << TargetAddress(address) << " - " << IntegerConstant(-val) << ";";
  if(val!=0)
    LineFeed();
}

// integer maths
Iec61131stCodeGenerator::AX Iec61131stCodeGenerator::IntegerQuotient(const AX& expression, int val) {
  return AX(expression + " / " + IntegerConstant(val));
} 

// integer maths
Iec61131stCodeGenerator::AX Iec61131stCodeGenerator::IntegerRemainder(const AX& expression, int val) {
  return AX(expression + " MOD " + IntegerConstant(val));
}

// integer maths
CodePrimitives::AX Iec61131stCodeGenerator::IntegerBitmask(const AX& expression) {
  return AX("SHL( IN:=" + WordConstant(1) + " , N:=" + expression + " )" );
}

// integer maths
bool Iec61131stCodeGenerator::HasIntmaths(void) {
  return true;
}

// generate code: integer
CodePrimitives::AX Iec61131stCodeGenerator::IntegerConstant(int val) {
  std::string res(ToStringInteger(val));
  if(mIntegerType == "INT") return AX(res);
  if(mIntegerType == "DINT") return AX("DINT#" + res);
  if(mIntegerType == "LINT") return AX("LINT#" + res);
  FCG_ERR("Iec61131stCodeGenerator: unsupported integer data type");
  return AX(res);
}

// generate code: conditionals
CodePrimitives::AX Iec61131stCodeGenerator::IntegerIsEq(const AA& address, int val) {
  return AX(TargetAddress(address) + " = " + IntegerConstant(val));
}

// generate code: conditionals
CodePrimitives::AX Iec61131stCodeGenerator::IntegerIsEq(const AA& address, const AX& expression) {
  return AX(TargetAddress(address) + " = " + expression);
}

// generate code: conditionals
CodePrimitives::AX Iec61131stCodeGenerator::IntegerIsNotEq(const AA& address, int val) {
  return AX(TargetAddress(address) + " <> " + IntegerConstant(val));
}

// generate code: conditionals
CodePrimitives::AX Iec61131stCodeGenerator::IntegerIsNotEq(const AA& address, const AX& expression) {
  return AX(TargetAddress(address) + " != " + expression);
}

// generate code: conditionals
CodePrimitives::AX Iec61131stCodeGenerator::IntegerIsGreater(const AA& address, int val) {
  return AX(TargetAddress(address) + " > " + IntegerConstant(val));
}

// generate code: conditionals
CodePrimitives::AX Iec61131stCodeGenerator::IntegerIsLess(const AA& address, int val) {
  return AX(TargetAddress(address) + " < " + IntegerConstant(val));
}



// generate code: word
void Iec61131stCodeGenerator::WordDeclare(const AA& address){
  VariableDeclare(TargetAddress(address),mWordType);
}

// generate code: word
void Iec61131stCodeGenerator::WordDeclare(const AA& address, word_t val){
  VariableDeclare(TargetAddress(address),mWordType,WordConstant(val));
}

// generate code: word
void Iec61131stCodeGenerator::WordAssign(const AA& address, word_t val){
  Output() << TargetAddress(address) << " := " << WordConstant(val) << ";";
  LineFeed();
}

// generate code: word
void Iec61131stCodeGenerator::WordAssign(const AA& address, const AX& expression){
  Output() << TargetAddress(address) << " := " << expression << ";";
  LineFeed();
}


// generate code: word
void Iec61131stCodeGenerator::WordOr(const AA& address, word_t val) {
  Output() << TargetAddress(address) << " := " << TargetExpression(address) << " OR " << WordConstant(val) << ";";
  LineFeed();
}  

// generate code: word
void Iec61131stCodeGenerator::WordOr(const AA& address, const AX& expression) {
  Output() << TargetAddress(address) << " := " << TargetExpression(address) << " OR " <<  expression << ";";
  LineFeed();
}  

// generate code: word
void Iec61131stCodeGenerator::WordOr(const AA& address, const AA& op1, const AA& op2) {
  Output() << TargetAddress(address) << " := " << TargetExpression(op1) << " OR " << TargetExpression(op2) << ";";
  LineFeed();
}  

// generate code: word
void Iec61131stCodeGenerator::WordOr(const AA& address, const AA& op1, word_t op2) {
  Output() << TargetAddress(address) << " := " << TargetExpression(op1) << " OR " << WordConstant(op2) << ";";
  LineFeed();
}

// generate code: word
void Iec61131stCodeGenerator::WordAnd(const AA& address, word_t val) {
  Output() << TargetAddress(address) << " := " << TargetAddress(address) << " AND " << WordConstant(val) << ";";
  LineFeed();
}  

// generate code: word
void Iec61131stCodeGenerator::WordAnd(const AA& address, const AX& expression) {
  Output() << TargetAddress(address) << " := " << TargetAddress(address) << " AND " << expression << ";";
  LineFeed();
}  

// generate code: word
void Iec61131stCodeGenerator::WordAnd(const AA& address, const AA& op1, const AA& op2) {
  Output() << TargetAddress(address) << " := " << TargetAddress(op1) << " AND " << TargetAddress(op2) << ";";
  LineFeed();
}  

// generate code: word
void Iec61131stCodeGenerator::WordAnd(const AA& address, const AA& op1, word_t op2) {
  Output() << TargetAddress(address) << " := " << TargetExpression(op1) << " AND " << WordConstant(op2) << ";";
  LineFeed();
}

// generate code: word
void Iec61131stCodeGenerator::WordNand(const AA& address, const AX& expression) {
  Output() << TargetAddress(address) << " := " << TargetAddress(address) << " AND (NOT " << expression << " );";
  LineFeed();
}  

// generate code: conditionals
CodePrimitives::AX Iec61131stCodeGenerator::WordIsBitSet(const AA& address, int idx) {
  return AX("( " + TargetAddress(address) + " AND " + WordConstant(0x01<<idx) + " ) <> " + WordConstant(0));
}

// generate code: conditionals
CodePrimitives::AX Iec61131stCodeGenerator::WordIsBitClr(const AA& address, int idx) {
  return AX("( " + TargetAddress(address) + " AND " + WordConstant(0x01<<idx) + " ) == " + WordConstant(0));
}

/// generate code: conditionals
CodePrimitives::AX Iec61131stCodeGenerator::WordIsMaskSet(const AA& address, word_t mask) {
  return AX("( " + TargetAddress(address) + " AND " + WordConstant(mask) + " ) <> " + WordConstant(0));
}

// generate code: conditionals
CodePrimitives::AX Iec61131stCodeGenerator::WordIsEq(const AA& address, word_t val) {
  return AX(TargetAddress(address) + " = " + WordConstant(val));
}

// generate code: conditionals
CodePrimitives::AX Iec61131stCodeGenerator::WordIsNotEq(const AA& address, word_t val) {
  return AX("NOT ( " + TargetAddress(address) + " = " + WordConstant(val) + " )");
}

// generate code: word
Iec61131stCodeGenerator::AX Iec61131stCodeGenerator::WordConstant(word_t val) {
  // have neet zero
  if(val==0) return AX( mWordType + "#0");
  // construct typed hex representation
  std::stringstream sstr;
  sstr << mWordType << "#16#" << std::setbase(16) << std::setfill('0');
  if(mWordType == "BYTE") sstr << std::setw(2) << (val & 0xff);
  else if(mWordType == "WORD") sstr << std::setw(4) << (val & 0xffff);
  else if(mWordType == "DWORD") sstr << std::setw(8) << (val & 0xffffffff);
  else if(mWordType == "LWORD") sstr << std::setw(16) << (val & 0xffffffffffffffff);
  else FCG_ERR("Iec61131stCodeGenerator: unsupported word data type");
  AX res(sstr.str());
  return res;
}


// generate code: bool
void Iec61131stCodeGenerator::BooleanDeclare(const AA& address){
  VariableDeclare(TargetAddress(address),"BOOL");
}

// generate code: bool
void Iec61131stCodeGenerator::BooleanDeclare(const AA& address, int val){
  std::string valstr;
  if(val) valstr="true"; else valstr="false";
  VariableDeclare(TargetAddress(address),"BOOL",valstr);
}

// generate code: bool
void Iec61131stCodeGenerator::BooleanAssign(const AA& address, int val){
  std::string valstr;
  if(val) valstr="true"; else valstr="false";
  Output() << TargetAddress(address) << " := " << valstr << ";";
  LineFeed();
}

// generate code: bool
void Iec61131stCodeGenerator::BooleanAssign(const AA& address, const AX& expression){
  Output() << TargetAddress(address) << " := " << expression << ";";
  LineFeed();
}

// generate code: bool
CodePrimitives::AX Iec61131stCodeGenerator::BooleanIsNotEq(const AA& op1,const AA& op2) {
  return AX(TargetAddress(op1) + " <> " + TargetAddress(op2));
}

// generate code: bool
CodePrimitives::AX Iec61131stCodeGenerator::BooleanIsEq(const AA& op1,const AA& op2) {
  return AX(TargetAddress(op1) + " = " + TargetAddress(op2));
}


// const-int array (local iec only helper)
Iec61131stCodeGenerator::AX Iec61131stCodeGenerator::IntarrayConstant(const std::vector<int>& val) {
  std::stringstream strstr;
  int newline=20;
  if(val.size()<25) newline=25;
  strstr << "[ " ;
  size_t vit=0;
  while(true) {
    if(vit==val.size()) break;
    strstr << IntegerConstant(val[vit]);
    ++vit;
    if(vit==val.size()) break;
    strstr << ", ";
    newline--;
    if(newline==0) {
      strstr << std::endl << "  ";
      newline=20;
    }
  }
  strstr << " ]";
  return AX(strstr.str());
}

// const-int-array
void Iec61131stCodeGenerator::CintarrayDeclare(const AA& address, int offset, const std::vector<int>& val) {
  if(val.size()==0) {
    FCG_ERR("Iec61131stCodeGenerator::Cintarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("Iec61131stCodeGenerator::Cwordarray(): const vector exceeds address range");
    return;
  }
  // declare with initialiser
  Output() << TargetAddress(address) << " : ARRAY[" << offset << ".." << val.size()+offset-1 << "] OF " << mIntegerType << " := ";
  LineFeed();
  IndentInc();
  Output() << IntarrayConstant(val) << ";";
  LineFeed();
  IndentDec();
}


// int array access by const
CodePrimitives::AA Iec61131stCodeGenerator::CintarrayAccess(const AA& address, int index) {
  return AA(address + "[" + ToStringInteger(index) + "]");
};  

// int array access by expression
CodePrimitives::AA Iec61131stCodeGenerator::CintarrayAccess(const AA& address, const AA& indexaddr){
  return AA(address + "[" + TargetAddress(indexaddr) + "]");
};

// tell parent class about this feature
bool Iec61131stCodeGenerator::HasCintarray(void) {
  return true;
};


// const-word array
Iec61131stCodeGenerator::AX Iec61131stCodeGenerator::WordarrayConstant(const std::vector<word_t>& val) {
  std::stringstream strstr;
  int newline=8;
  if(val.size()<11) newline=15;
  strstr << "[ " ;
  size_t vit=0;
  while(true) {
    if(vit==val.size()) break;
    strstr << WordConstant(val[vit]);
    ++vit;
    if(vit==val.size()) break;
    strstr << ", ";
    newline--;
    if(newline==0) {
      strstr << std::endl << "  ";
      newline=8;
    }
  }
  strstr << " ]";
  return AX(strstr.str());
}

// const-word-array
void Iec61131stCodeGenerator::CwordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val) {
  if(val.size()==0) {
    FCG_ERR("Iec61131stCodeGenerator::Cwordarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("Iec61131stCodeGenerator::Cwordarray(): const vector exceeds addres range");
    return;
  }
  // declare with initialiser
  Output() << TargetAddress(address) << " : ARRAY[" << offset << ".." << val.size()+offset-1 << "] OF " << mWordType << " := ";
  LineFeed();
  IndentInc();
  Output() << WordarrayConstant(val) << ";";
  LineFeed();
  IndentDec();
}


// word-array access by const
CodePrimitives::AA Iec61131stCodeGenerator::CwordarrayAccess(const AA& address, int index) {
  return AA(address + "[" + ToStringInteger(index) + "]");
};  

// word array access by expression
CodePrimitives::AA Iec61131stCodeGenerator::CwordarrayAccess(const AA& address, const AA& indexaddr){
  return AA(address + "[" + TargetAddress(indexaddr) + "]");
};

// tell parent class about this feature
bool Iec61131stCodeGenerator::HasCwordarray(void) {
  return true;
};

// generate code: string constant
Iec61131stCodeGenerator::AX Iec61131stCodeGenerator::StringConstant(const std::string& val) {
  AX res;
  res.append(1,'\'');
  for(std::size_t i=0; i<val.length(); ++i) {
    char c=val.at(i);
    // terminate
    if(c==0) break;
    // quote quote
    if(c=='\'') { res.append("$'"); continue; }
    // quote dollar
    if(c=='$') { res.append("$$"); continue; }
    // printable ascii
    if((c>=0x20) && (c<0x7f)) { res.append(1,c); continue; };
    // error on other (alternative: use octal encoding to support utf8)
    FCG_ERR("EmbeddedcCodeGenerator: non-printable ascii or other encoding unsupported [" << val <<"]");
  }
  res.append(1,'\'');
  return res;
}

// literal const-string array
Iec61131stCodeGenerator::AX Iec61131stCodeGenerator::StrarrayConstant(const std::vector<std::string>& val) {
  std::stringstream strstr;
  int newline;
  if(val.size()<4) {
    newline=4;
    strstr << "[ ";
  } else {
    newline=1;
    strstr << "[ " << std::endl;
  }
  size_t vit=0;
  while(true) {
    if(vit==val.size()) break;
    strstr << StringConstant(val[vit]);
    ++vit;
    if(vit==val.size()) break;
    strstr << ", ";
    newline--;
    if(newline==0) {
      strstr << std::endl;
      newline=1;
    }
  }
  strstr << " ]";
  return AX(strstr.str());
}


// const-string-array
void Iec61131stCodeGenerator::CstrarrayDeclare(const AA& address, int offset, const std::vector<std::string>& val) {
  if(val.size()==0) {
    FCG_ERR("Iec61131stCodeGenerator::Cstrarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset>= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("Iec61131stCodeGenerator::Cstrarray(): const vector exceeds addres range");
    return;
  }
  // figure string size
  size_t len=0;
  for(size_t i=0; i<val.size(); ++i)
    if(val[i].size()>len) len=val[i].size();
  // declare with initialiser
  Output() << TargetAddress(address) << " : ARRAY[" << offset << ".." << val.size()+offset-1 << "] OF STRING[" << ToStringInteger(len) <<"] := ";
  LineFeed();
  IndentInc();
  Output() << StrarrayConstant(val) << ";";
  LineFeed();
  IndentDec();
}


// string-array access by const
CodePrimitives::AA Iec61131stCodeGenerator::CstrarrayAccess(const AA& address, int index) {
  return AA(address + "[" + ToStringInteger(index) + "]");
};

// string array access by expression
CodePrimitives::AA Iec61131stCodeGenerator::CstrarrayAccess(const AA& address, const AA& indexaddr){
  return AA(address + "[" + TargetAddress(indexaddr) + "]");
};

// tell parent class about this feature
bool Iec61131stCodeGenerator::HasCstrarray(void) {
  return true;
};

// int-array
void Iec61131stCodeGenerator::IntarrayDeclare(const AA& address, int offset, const std::vector<int>& val) {
  if(val.size()==0) {
    FCG_ERR("Iec61131stCodeGenerator::Intarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("Iec61131stCodeGenerator::Intarray(): const vector exceeds addres range");
    return;
  }
  // declare with initialiser
  Output() << TargetAddress(address) << " : ARRAY[" << offset << ".." << val.size()+offset-1 << "] OF " << mIntegerType << " := ";
  LineFeed();
  IndentInc();
  Output() << IntarrayConstant(val) << ";";
  LineFeed();
  IndentDec();
}


// int-array
void Iec61131stCodeGenerator::IntarrayDeclare(const AA& address, int offset, int len) {
  if(len==0) {
    FCG_ERR("Iec61131stCodeGenerator::Intarray(): ignoring empty const vector");
    return;
  }
  if(len+offset >= (1LL << (mIntegerSize-1))) {
    FCG_ERR("Iec61131stCodeGenerator::Intarray(): const vector exceeds addres range");
    return;
  }
  Output() << TargetAddress(address) << " : ARRAY[" << offset << ".." << len+offset-1 << "] OF " << mIntegerType << ";";
  LineFeed();
}

// word-array access by const
Iec61131stCodeGenerator::AA Iec61131stCodeGenerator::IntarrayAccess(const AA& address, int index) {
  return AA(address + "[" + ToStringInteger(index) + "]");
};

// word array access by expression
Iec61131stCodeGenerator::AA Iec61131stCodeGenerator::IntarrayAccess(const AA& address, const AA& indexaddr){
  return AA(address + "[" + TargetAddress(indexaddr) + "]");
};

// tell parent class about this feature
bool Iec61131stCodeGenerator::HasIntarray(void) {
  return true;
};


// word-array
void Iec61131stCodeGenerator::WordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val) {
  if(val.size()==0) {
    FCG_ERR("Iec61131stCodeGenerator::Wordarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("Iec61131stCodeGenerator::Wordarray(): const vector exceeds addres range");
    return;
  }
  // declare with initialiser
  Output() << TargetAddress(address) << " : ARRAY[" << offset << ".." << val.size()+offset-1 << "] OF " << mWordType << " := ";
  LineFeed();
  IndentInc();
  Output() << WordarrayConstant(val) << ";";
  LineFeed();
  IndentDec();
}


// word-array
void Iec61131stCodeGenerator::WordarrayDeclare(const AA& address, int offset, int len) {
  if(len==0) {
    FCG_ERR("Iec61131stCodeGenerator::Wordarray(): ignoring empty const vector");
    return;
  }
  if(len>= (1LL << (mIntegerSize-1))) {
    FCG_ERR("Iec61131stCodeGenerator::Wordarray(): const vector exceeds addres range");
    return;
  }
  Output() << TargetAddress(address) << " : ARRAY[" << offset << ".." << len+offset-1 << "] OF " << mWordType << ";";
  LineFeed();
}

// word-array access by const
Iec61131stCodeGenerator::AA Iec61131stCodeGenerator::WordarrayAccess(const AA& address, int index) {
  return AA(address + "[" + ToStringInteger(index) + "]");
};

// word array access by expression
Iec61131stCodeGenerator::AA Iec61131stCodeGenerator::WordarrayAccess(const AA& address, const AA& indexaddr){
  return AA(address + "[" + TargetAddress(indexaddr) + "]");
};

// tell parent class about this feature
bool Iec61131stCodeGenerator::HasWordarray(void) {
  return true;
};



// event set
/*
void Iec61131stCodeGenerator::EventSetDeclareImport(const AA& address) {
  // same as declare
  EventSetDeclare(address);
}
*/

// generate code: conditionals
void Iec61131stCodeGenerator::IfTrue(const AX& expression) {
  Output() << "IF " << expression << " THEN";
  LineFeed();
  IndentInc();
}

// generate code: conditionals
void Iec61131stCodeGenerator::IfFalse(const AX& expression) {
  Output() << "IF NOT " << expression << " THEN";
  LineFeed();
  IndentInc();
}

// generate code: conditionals
void Iec61131stCodeGenerator::IfWord(const AX& expression) {
  Output() << "IF ( " << expression << " ) <> " << WordConstant(0) << " THEN";
  LineFeed();
  IndentInc();
}

// generate code: conditionals
void Iec61131stCodeGenerator::IfElse(void) {
  IndentDec();
  Output() << "ELSE";
  LineFeed();
  IndentInc();
}

// generate code: conditionals
void Iec61131stCodeGenerator::IfElseIfTrue(const AX& expression) {
  IndentDec();
  Output() << "ELSIF " << expression << " THEN";
  LineFeed();
  IndentInc();
}

// generate code: conditionals
void Iec61131stCodeGenerator::IfEnd(void) {
  IndentDec();
  Output() << "END_IF;";
  LineFeed();
}

// switch control
void Iec61131stCodeGenerator::SwitchBegin(const AA& address){
  Output() << "CASE " << TargetAddress(address) << " OF";
  LineFeed();
}

// switch control
void Iec61131stCodeGenerator::SwitchCase(const AA& address, int val){
  (void) address;
  Output() << IntegerConstant(val) << ":";
  LineFeed();
  IndentInc();
}

// switch control
void Iec61131stCodeGenerator::SwitchCases(const AA& address, int from, int to){
  // bail out on empty
  if(from>to) {
    FCG_VERB0("CodeGenerator: WARNING: inconsistent empty range of switch-cases");
    IndentInc();
    return;
  }
  // simplify to singleton
  if(from==to) {
    SwitchCase(address, from);
    return;
  }
  // do range
  Output() << IntegerConstant(from) << " .. " << IntegerConstant(to) << ":";
  LineFeed();
  IndentInc();
}

// switch control
void Iec61131stCodeGenerator::SwitchCases(const AA& address, const std::set< int >& vals){
  // bail out on empty
  if(vals.empty()) {
    FCG_VERB0("CodeGenerator: WARNING: inconsistent empty range of switch-cases");
    IndentInc();
    return;
  }
  // simplify to range
  int from=*vals.begin();
  int to= *(--vals.end());
  if(to+1-from== (int) vals.size()) {
    SwitchCases(address,from,to);
    return;
  }
  // do set
  std::set< int >::const_iterator vit=vals.begin();
  for(; vit!=vals.end(); ++ vit) {
    if(vit!=vals.begin()) Output() << ", ";
    Output() << IntegerConstant(*vit);
  }
  Output() << ":";
  LineFeed();
  IndentInc();
}

// switch control
void Iec61131stCodeGenerator::SwitchBreak(void){
  IndentDec();
}  

// switch control
void Iec61131stCodeGenerator::SwitchEnd(void){
  Output() << "END_CASE;";
  LineFeed();
}

// switch control
bool Iec61131stCodeGenerator::HasMultiCase(void) {
  return true;
}

// loops
void Iec61131stCodeGenerator::LoopBegin(void) {
  Output() << "WHILE true DO";
  LineFeed();
  IndentInc();
}

// loops
void Iec61131stCodeGenerator::LoopBreak(const AX& expression) {
  Output() << "IF " << expression << " THEN EXIT; END_IF;";
  LineFeed();
}

// loops
void Iec61131stCodeGenerator::LoopEnd(void) {
  IndentDec();
  Output() << "END_WHILE;";
  LineFeed();
}

// loops
void Iec61131stCodeGenerator::FunctionReturn(void) {
  Output() << "RETURN;";
  LineFeed();
}

// timer
void Iec61131stCodeGenerator::TimerDeclare(const AA& address, const std::string &val) {
  // todo: type check time literal
  (void) val;
  VariableDeclare(TargetAddress(AA("timer_"+address+"_cnt")),"TIME");
  VariableDeclare(TargetAddress(AA("timer_"+address+"_run")),"BOOL");
}
void Iec61131stCodeGenerator::TimerStart(const AA& address) {
  Output() << TargetAddress(AA("timer_"+address+"_run")) << " := true;";
  LineFeed();
}
void Iec61131stCodeGenerator::TimerStop(const AA& address) {
  Output() << TargetAddress(AA("timer_"+address+"_run")) << " := false;";
  LineFeed();
}
void Iec61131stCodeGenerator::TimerReset(const AA& address, const std::string& litval) {
  Output() << TargetAddress(AA("timer_"+address+"_cnt")) << " := " << litval << ";";
  LineFeed();
}

CodePrimitives::AX Iec61131stCodeGenerator::TimerIsElapsed(const AA& address) {
  return AX(TargetAddress(AA("timer_"+address+"_run")) + " AND (" +
            TargetAddress(AA("timer_"+address+"_cnt")) + " <= " + TimeConstant(0) + ")");
}

CodePrimitives::AX Iec61131stCodeGenerator::TimeConstant(int val) {
  std::string res("TIME#" + ToStringInteger(val) + "ms");
  return AX(res);
}


// output actions 
void Iec61131stCodeGenerator::RunActionSet(const std::string& address) {
  Output() << address << " := true;" ;
  LineFeed();
}
void Iec61131stCodeGenerator::RunActionClr(const std::string& address) {
  Output() << address << " := false;" ;
  LineFeed();
}
void Iec61131stCodeGenerator::RunActionExe(const AX& expression) {
  Output() << expression << ";";
  LineFeed();
}

Iec61131stCodeGenerator::IECVariableType Iec61131stCodeGenerator::CurrentVariableType(void) {
  return mCurrentVariableType;
}

void Iec61131stCodeGenerator::CurrentVariableType(const IECVariableType & type) {
  mCurrentVariableType = type;
}


