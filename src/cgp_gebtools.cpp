/** @file cgp_gebtools.cpp @brief Code-generator GEB toolchain (IEC 61131-3 ST) */

/*
  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2010, 2016 Thomas Moor

*/

// my includes
#include "cgp_gebtools.h"


/*
******************************************************************
******************************************************************
******************************************************************

GebtoolsCodeGenerator implementation --- class

******************************************************************
******************************************************************
******************************************************************
*/


// Register derived class
FAUDES_REGISTERCODEGENERATOR("geb",GebtoolsCodeGenerator);


// GebtoolsCodeGenerator(void)
GebtoolsCodeGenerator::GebtoolsCodeGenerator(void) : Iec61131stCodeGenerator() {
  FD_DCG("GebtoolsCodeGenerator(" << this << ")::GebtoolsCodeGenerator()");
}


// GebtoolsCodeGenerator(void)
GebtoolsCodeGenerator::~GebtoolsCodeGenerator(void) {
  FD_DCG("GebtoolsCodeGenerator(" << this << ")::~GebtoolsCodeGenerator()");
}


// clear
void GebtoolsCodeGenerator::Clear(void) {
  FD_DCG("GebtoolsCodeGenerator::Clear()");
  // call base
  Iec61131stCodeGenerator::Clear();
  // my flavor of defaults
  mIecTypedArrayConstants=false;
}

//DoReadTargetConfiguration(rTr)
void GebtoolsCodeGenerator::DoReadTargetConfiguration(TokenReader& rTr) {
  FD_DCG("GebtoolsCodeGenerator::DoReadTargetConfiguration()");
  // base
  Iec61131stCodeGenerator::DoReadTargetConfiguration(rTr);
  // iec option
  Token token;
  if(rTr.ExistsBegin("IecTypedArrayConstants")) {
    rTr.ReadBegin("IecTypedArrayConstants",token);
    mIecTypedArrayConstants = token.AttributeIntegerValue("val");
    rTr.ReadEnd("IecTypedArrayConstants");
  }
}

//DoWriteTargetConfiguration(rTw)
void GebtoolsCodeGenerator::DoWriteTargetConfiguration(TokenWriter& rTw) const{
  FD_DCG("GebtoolsCodeGenerator::DoWriteTargetConfiguration()");
  // base
  Iec61131stCodeGenerator::DoWriteTargetConfiguration(rTw);
  // code option
  Token token;
  token.SetEmpty("IecTypedArrayConstants");
  token.InsAttributeBoolean("val",mIecTypedArrayConstants);
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


// code blocks: cyclic function block
void GebtoolsCodeGenerator::DoGenerateFunction(void){
  // array constants as types: outside POU for GEB IDE
  if(mIecTypedArrayConstants) {
    Comment("************************************************");
    Comment("* array types --- GEB IDE specific             *");
    Comment("************************************************");
    Output() << "TYPE";
    LineFeed(2);
    MuteMode('t');
    DeclareParallelState();
    DeclarePendingEvents();
    DeclareLargeCarray();
    DeclareSmallCarray();
    Comment("other type fixes");
    EventSetDeclare(AA("aux_locenabled"));
    EventSetDeclare(AA("aux_executables"));
    LineFeed(1);
    MuteMode('c');
    MuteCond('*');
    LineFeed(1);
    Output() << "END_TYPE";
    LineFeed(4);
  }
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
  LineFeed(1);
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
  LineFeed();
  Output() << "END_FUNCTION_BLOCK (* end of " << mPrefix <<"cyclic_fb *)";
  LineFeed();
  LineFeed(2+1);
}

// code blocks: function to host lookup tables
void GebtoolsCodeGenerator::DoGenerateLookups(void){
  // bail out
  if((!mEventNameLookup) && (!mStateNameLookup)) return;
  // array constants as types: outside POU for GEB IDE
  if(mIecTypedArrayConstants) {
    Comment("************************************************");
    Comment("* initialised array types --- GEB IDE specific *");
    Comment("************************************************");
    Output() << "TYPE";
    LineFeed(2);
    MuteMode('t');
    DeclareEventNameLookup();
    DeclareStateNameLookup();
    LineFeed(1);
    Output() << "END_TYPE";
    LineFeed(4);
    MuteMode('c');
    MuteCond('*');
  }
  // pass on to base
  Iec61131stCodeGenerator::DoGenerateLookups();
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



// const-int-array
void GebtoolsCodeGenerator::CintarrayDeclare(const AA& address,  int offset, const std::vector<int>& val) {
  if(val.size()==0) {
    FCG_ERR("GebtoolsCodeGenerator::Cintarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("GebtoolsCodeGenerator::Cwordarray(): const vector exceeds address range");
    return;
  }
  // let base handle
  if(!mIecTypedArrayConstants) {
    Iec61131stCodeGenerator::CintarrayDeclare(address,offset,val);
    return;
  }
  // geb specific special case: declare as type in first pass (mutemode 't')
  MuteCond('t');
  Output() << TargetAddress(AA(address+"_t")) << " : ARRAY[" << offset << ".." << val.size()+offset-1 << "] OF " << mIntegerType << " := ";
  LineFeed();
  IndentInc();
  Output() << IntarrayConstant(val) << ";";
  LineFeed();
  IndentDec();
  // geb specific special case: import on code generation (mutemode 'c')
  MuteCond('c');
  Output() << TargetAddress(address) << " : " << TargetAddress(AA(address+"_t")) << ";";
  LineFeed();
  // default to no mute
  MuteCond('*');
}


// const-word-array
void GebtoolsCodeGenerator::CwordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val) {
  if(val.size()==0) {
    FCG_ERR("GebtoolsCodeGenerator::Cwordarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("GebtoolsCodeGenerator::Cwordarray(): const vector exceeds addres range");
    return;
  }
  // let base handle
  if(!mIecTypedArrayConstants) {
    Iec61131stCodeGenerator::CwordarrayDeclare(address,offset,val);
    return;
  }
  // geb specific special case: declare as type in first pass (mutemode 't')
  MuteCond('t');
  Output() << TargetAddress(AA(address+"_t")) << " : ARRAY[" << offset << ".." << val.size()+offset-1 << "] OF " << mWordType << " := ";
  LineFeed();
  IndentInc();
  Output() << WordarrayConstant(val) << ";";
  LineFeed();
  IndentDec();
  // geb specific special case: import on code generation (mutemode 'c')
  MuteCond('c');
  Output() << TargetAddress(address) << " : " << TargetAddress(AA(address+"_t")) << ";";
  LineFeed();
  // default to no mute
  MuteCond('*');
}


// const-str-array
void GebtoolsCodeGenerator::CstrarrayDeclare(const AA& address, int offset, const std::vector<std::string>& val) {
  if(val.size()==0) {
    FCG_ERR("GebtoolsCodeGenerator::Cstrarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("GebtoolsCodeGenerator::Cstrarray(): const vector exceeds addres range");
    return;
  }
  // let base handle
  if(!mIecTypedArrayConstants) {
    Iec61131stCodeGenerator::CstrarrayDeclare(address,offset,val);
    return;
  }
  // figure string size
  size_t len=0;
  for(size_t i=0; i<val.size(); ++i)
    if(val[i].size()>len) len=val[i].size();
  // geb specific special case: declare as type in first pass (mutemode 't')
  MuteCond('t');
  // variant a): iec st compliant
  //Output() << TargetAddress(address) << "_t" << " : ARRAY[0.." << val.size()-1 << "] OF STRING[" << ToStringInteger(len) <<"] := ";
  // variant b): circumvent issue for early 3.xx GEB versions
  Output() << TargetAddress(address) << "_st" << " : STRING[" << ToStringInteger(len) <<"];";
  LineFeed();
  Output() << TargetAddress(AA(address+"_t")) << " : ARRAY[" << offset << ".." << val.size()+offset-1 << "] OF "<<
      TargetAddress(AA(address+"_st")) << " := ";
  LineFeed();
  IndentInc();
  Output() << StrarrayConstant(val) << ";";
  LineFeed();
  IndentDec();
  // geb specific special case: import on code generation (mutemode 'c')
  MuteCond('c');
  Output() << TargetAddress(address) << " : " << TargetAddress(AA(address+"_t")) << ";";
  LineFeed();
  // default to no mute
  MuteCond('*');
}

// int-array
void GebtoolsCodeGenerator::IntarrayDeclare(const AA& address, int offset, const std::vector<int>& val) {
  if(val.size()==0) {
    FCG_ERR("GebtoolsCodeGenerator::Intarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("GebtoolsCodeGenerator::Intarray(): const vector exceeds addres range");
    return;
  }
  // let base handle
  if(!mIecTypedArrayConstants) {
    Iec61131stCodeGenerator::IntarrayDeclare(address,offset,val);
    return;
  }
  // geb specific special case: declare as type in first pass (mutemode 't')
  MuteCond('t');
  Output() << TargetAddress(AA(address+"_t")) << " : ARRAY[" << offset << ".." << val.size()+offset-1 << "] OF " << mIntegerType << " := ";
  LineFeed();
  IndentInc();
  Output() << IntarrayConstant(val) << ";";
  LineFeed();
  IndentDec();
  // geb specific special case: import on code generation (mutemode 'c')
  MuteCond('c');
  Output() << TargetAddress(address) << " : " << TargetAddress(AA(address+"_t")) << ";";
  LineFeed();
  // default to no mute
  MuteCond('*');
}


// int-array
void GebtoolsCodeGenerator::IntarrayDeclare(const AA& address, int offset, int len) {
  if(len==0) {
    FCG_ERR("GebtoolsCodeGenerator::Intarray(): ignoring empty vector");
    return;
  }
  if(((unsigned long long) len+offset) >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("GebtoolsCodeGenerator::Intarray(): const vector exceeds addres range");
    return;
  }
  // let base handle
  if(!mIecTypedArrayConstants) {
    Iec61131stCodeGenerator::IntarrayDeclare(address,offset,len);
    return;
  }
  // geb specific special case: declare as type in first pass (mutemode 't')
  MuteCond('t');
  Output() << TargetAddress(AA(address + "_t")) << " : ARRAY[" << offset << ".." << len+offset-1 << "] OF " << mIntegerType << ";";
  LineFeed();
  // geb specific special case: import on code generation (mutemode 'c')
  MuteCond('c');
  Output() << TargetAddress(address) << " : " << TargetAddress(AA(address+"_t")) << ";";
  LineFeed();
  // default to no mute
  MuteCond('*');
}

// word-array
void GebtoolsCodeGenerator::WordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val) {
  if(val.size()==0) {
    FCG_ERR("GebtoolsCodeGenerator::Wordarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("GebtoolsCodeGenerator::Wordarray(): const vector exceeds addres range");
    return;
  }
  // let base handle
  if(!mIecTypedArrayConstants) {
    Iec61131stCodeGenerator::WordarrayDeclare(address,offset,val);
    return;
  }
  // geb specific special case: declare as type in first pass (mutemode 't')
  MuteCond('t');
  Output() << TargetAddress(AA(address + "_t")) << " : ARRAY[" << offset << ".." << val.size()+offset-1 << "] OF " << mWordType << " := ";
  LineFeed();
  IndentInc();
  Output() << WordarrayConstant(val) << ";";
  LineFeed();
  IndentDec();
  // geb specific special case: import on code generation (mutemode 'c')
  MuteCond('c');
  Output() << TargetAddress(address) << " : " << TargetAddress(AA(address+"_t")) << ";";
  LineFeed();
  // default to no mute
  MuteCond('*');
}

// word-array without initialisation
void GebtoolsCodeGenerator::WordarrayDeclare(const AA& address, int offset, int len) {
  // sanity tests
  if(len==0) {
    FCG_ERR("GebtoolsCodeGenerator::Wordarray(): ignoring empty vector");
    return;
  }
  if(((unsigned long long) len+offset ) >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("GebtoolsCodeGenerator::Wordarray(): const vector exceeds addres range");
    return;
  }
  // let base handle
  if(!mIecTypedArrayConstants) {
    Iec61131stCodeGenerator::WordarrayDeclare(address,offset,len);
    return;
  }
  // geb specific special case: declare as type in first pass (mutemode 't')
  MuteCond('t');
  Output() << TargetAddress(AA(address + "_t")) << " : ARRAY[" << offset << ".." << len+offset-1 << "] OF " << mWordType << ";";
  LineFeed();
  // geb specific special case: import on code generation (mutemode 'c')
  MuteCond('c');
  Output() << TargetAddress(address) << " : " << TargetAddress(AA(address+"_t")) << ";";
  LineFeed();
  // default to no mute
  MuteCond('*');
}






