/** @file cgp_embeddedc.cpp @brief Code-generator for target C  */

/*
  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2010, 2016 Thomas Moor

*/

// my includes
#include "cgp_embeddedc.h"


/*
******************************************************************
******************************************************************
******************************************************************

EmbeddedcCodeGenerator implementation

******************************************************************
******************************************************************
******************************************************************
*/

// Register derived class
FAUDES_REGISTERCODEGENERATOR("ec",EmbeddedcCodeGenerator);


// EmbeddedcCodeGenerator(void)
EmbeddedcCodeGenerator::EmbeddedcCodeGenerator(void) : CodePrimitives() {
  FD_DCG("EmbeddedcCodeGenerator(" << this << ")::EmbeddedcCodeGenerator()");
}


// EmbeddedcCodeGenerator(void)
EmbeddedcCodeGenerator::~EmbeddedcCodeGenerator(void) {
  FD_DCG("EmbeddedcCodeGenerator(" << this << ")::~EmbeddedcCodeGenerator()");
}

// clear
void EmbeddedcCodeGenerator::Clear(void) {
  FD_DCG("EmbeddedcCodeGenerator::Clear()");
  // call base
  CodePrimitives::Clear();
  // my flavor of defaults
  mPrefix="fcg_";
  mWordType="unsigned char";
  mWordSize=8;
  mIntegerType="int";
  mIntegerSize=16;
}

//DoReadTargetConfiguration(rTr)
void EmbeddedcCodeGenerator::DoReadTargetConfiguration(TokenReader& rTr) {
  FD_DCG("EmbeddedcCodeGenerator::DoReadTargetConfiguration()");
  // base
  CodePrimitives::DoReadTargetConfiguration(rTr);
}

//DoWriteTargetConfiguration(rTw)
void EmbeddedcCodeGenerator::DoWriteTargetConfiguration(TokenWriter& rTw) const{
  FD_DCG("EmbeddedcCodeGenerator::DoWriteTargetConfiguration()");
  // base
  CodePrimitives::DoWriteTargetConfiguration(rTw);
}

// DoCompile()
void EmbeddedcCodeGenerator::DoCompile(void) {
  // call base
  CodePrimitives::DoCompile();
  // my preferences
  if(mBitAddressArithmetic && mArrayForBitmasks && (mWordSize<=8)) {
    FCG_VERB1("EmbeddedcCodeGenerator::Compile(): prefer compiled bitmasks over bit-address maths");
    mBitAddressArithmetic = false;
  }  
}


// DoGenerate()
void EmbeddedcCodeGenerator::DoGenerate(void) {
  FD_DCG("EmbeddedcCodeGenerator(" << this << ")::DoGenerate()");
  // cut and paste from base
  mBitarrays.clear();
  // say hello
  Comment("************************************************");
  Comment("CodeGenerator: Target Embedded C                ");
  Comment("************************************************");
  LineFeed(1);
  Comment(std::string("CompileDES ") + VersionString());
  Comment(std::string("Configuration: ") + Name());
  LineFeed(2+1);
  // base class snippets
  LiteralPrepend();
  DoGenerateDeclarations();
  // cyclic function
  LineFeed(2);
  Output() << "void " << mPrefix << "cyclic(void) { ";
  LineFeed();
  DoGenerateResetCode();
  DoGenerateCyclicCode();
  Output() << "}; /* end function " << mPrefix << "cyclic() */";
  LineFeed();
  LineFeed(2+1);
  // extra: provide timer decrement interface to host application
  DecrementTimers();
  // snippets
  LiteralAppend();
  Comment("************************************************");
  Comment("CodeGenerator: Generated Code Ends Here         ");
  Comment("************************************************");
}



// code blocks: organise time decrement as function to be periodically
// invoked by the host application
void EmbeddedcCodeGenerator::DecrementTimers(void){
  // timer update function
  if(TimersBegin()!=TimersEnd()) {
    Comment("************************************************");
    Comment("* update timer states                          *");
    Comment("************************************************");
    LineFeed(2);
    Output() << "void " << mPrefix <<"timerdec(int dec) { ";
    LineFeed();
    IndentInc();
    TimerIterator tit= TimersBegin();
    for(;tit!=TimersEnd();++tit) {
      Output() << "if( " << TargetAddress(AA("timer_" + tit->second.mAddress + "_run")) << " ) "
        << TargetAddress(AA("timer_" + tit->second.mAddress + "_cnt")) << " -= dec;";
      LineFeed();
    }
    IndentDec();
    Output() << "};";
    LineFeed();
    LineFeed(1+2);
  }
}

// generate code
void EmbeddedcCodeGenerator::InsertExecHooks(void) {
  if((mEventExecutionHook!="") || (mStateUpdateHook!="")) {
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
}

// generate code
void EmbeddedcCodeGenerator::Comment(const std::string& text) {
  // write comment
  if(!mMuteComments) {
    Output() << "/* " << text << " */";
    LineFeed();
  }
  // call base to record muted comments
  CodePrimitives::Comment(text);
}

// address transformation
std::string EmbeddedcCodeGenerator::TargetAddress(const AA& address) {
  // should so some mangeling to replace non-ascii / funny characters
  std::string res=mPrefix+address;
  return res;
}

// address transformation
CodePrimitives::AX EmbeddedcCodeGenerator::TargetExpression(const AA& address) {
  return AX(TargetAddress(address));
}

// generate code: integer
void EmbeddedcCodeGenerator::IntegerDeclare(const AA& address){
  Output() << "static " << mIntegerType << " " << TargetAddress(address) << ";";
  LineFeed();
}  

// generate code: integer
void EmbeddedcCodeGenerator::IntegerDeclare(const AA& address, int val){
  Output() << "static " << mIntegerType << " " << TargetAddress(address) << "=" << val << ";";
  LineFeed();
}  

// generate code: integer
void EmbeddedcCodeGenerator::IntegerAssign(const AA& address, int val){
  Output() << TargetAddress(address) << " = " << IntegerConstant(val) << ";";
  LineFeed();
}  

// generate code: integer
void EmbeddedcCodeGenerator::IntegerAssign(const AA& address, const AX& expression){
  Output() << TargetAddress(address) << " = " << expression << ";";
  LineFeed();
}  

// generate code: integer
void EmbeddedcCodeGenerator::IntegerIncrement(const AA& address, int val) {
  if(val==1)
    Output() << TargetAddress(address) << "++;";
  else if(val==-1)
    Output() << TargetAddress(address) << "--;";
  else if(val!=0)
    Output() << TargetAddress(address) << " += " << IntegerConstant(val) << ";";
  if(val!=0)
    LineFeed();
}

// integer maths
EmbeddedcCodeGenerator::AX EmbeddedcCodeGenerator::IntegerQuotient(const AX& expression, int val) {
  return AX(expression + " / " + IntegerConstant(val));
} 

// integer maths
EmbeddedcCodeGenerator::AX EmbeddedcCodeGenerator::IntegerRemainder(const AX& expression, int val) {
  return AX(expression + " % " + IntegerConstant(val));
}

// integer maths
CodePrimitives::AX EmbeddedcCodeGenerator::IntegerBitmask(const AX& expression) {
  return AX("(" + WordConstant(1) + " << " + expression + ")" );
}

// integer compare
CodePrimitives::AX EmbeddedcCodeGenerator::IntegerIsEq(const AA& address, int val) {
  return AX(TargetAddress(address) + " == " + IntegerConstant(val));
}

// integer compare
CodePrimitives::AX EmbeddedcCodeGenerator::IntegerIsEq(const AA& address, const AX& expression) {
  return AX(TargetAddress(address) + " == " + expression);
}

// integer compare
CodePrimitives::AX EmbeddedcCodeGenerator::IntegerIsNotEq(const AA& address, const AX& expression) {
  return AX(TargetAddress(address) + " != " + expression);
}

// integer compare
CodePrimitives::AX EmbeddedcCodeGenerator::IntegerIsNotEq(const AA& address, int val) {
  return AX(TargetAddress(address) + " != " + IntegerConstant(val));
}

// integer compare
CodePrimitives::AX EmbeddedcCodeGenerator::IntegerIsGreater(const AA& address, int val) {
  return AX(TargetAddress(address) + " > " + IntegerConstant(val));
}

// integer compare
CodePrimitives::AX EmbeddedcCodeGenerator::IntegerIsLess(const AA& address, int val) {
  return AX(TargetAddress(address) + " < " + IntegerConstant(val));
}

// integer maths
bool EmbeddedcCodeGenerator::HasIntmaths(void) {
  return true;
}

// generate code: integer
CodePrimitives::AX EmbeddedcCodeGenerator::IntegerConstant(int val) {
  AX res(ToStringInteger(val));
  if(mIntegerType == "char") return res;
  if(mIntegerType == "int8_t") return res;
  if(mIntegerType == "int") return res;
  if(mIntegerType == "int16_t") return res;
  if(mIntegerType == "long") return AX(res + "L");
  if(mIntegerType == "int32_t") return AX(res + "L");
  if(mIntegerType == "long long") return AX(res + "LL");
  FCG_ERR("EmbeddedcCodeGenerator: unsupported integer data type [" << mIntegerType << "]");
  return res;
}

// generate code: word
void EmbeddedcCodeGenerator::WordDeclare(const AA& address){
  Output() << "static " << mWordType << " " << TargetAddress(address) << ";";
  LineFeed();
}  

// generate code: word
void EmbeddedcCodeGenerator::WordDeclare(const AA& address, word_t val){
  Output() << "static " << mWordType << " " << TargetAddress(address) << "=" << val << ";";
  LineFeed();
}  

// generate code: word
void EmbeddedcCodeGenerator::WordAssign(const AA& address, word_t val){
  Output() << TargetAddress(address) << " = " << WordConstant(val) << ";";
  LineFeed();
}  

// generate code: word
void EmbeddedcCodeGenerator::WordAssign(const AA& address, const AX& expression){
  Output() << TargetAddress(address) << " = " << expression << ";";
  LineFeed();
}  

// generate code: word
void EmbeddedcCodeGenerator::WordOr(const AA& address, word_t val) {
  Output() << TargetAddress(address) << " |= " << WordConstant(val) << ";";
  LineFeed();
}  

// generate code: word
void EmbeddedcCodeGenerator::WordOr(const AA& address, const AX& expression) {
  Output() << TargetAddress(address) << " |= " << expression << ";";
  LineFeed();
}  

// generate code: word
void EmbeddedcCodeGenerator::WordOr(const AA& address, const AA& op1, const AA& op2) {
  Output() << TargetAddress(address) << " = " << TargetExpression(op1) << " | " << TargetExpression(op2) << ";";
  LineFeed();
}  

// generate code: word
void EmbeddedcCodeGenerator::WordOr(const AA& address, const AA& op1, word_t op2) {
  Output() << TargetAddress(address) << " = " << TargetExpression(op1) << " | " << WordConstant(op2) << ";";
  LineFeed();
}

// generate code: word
void EmbeddedcCodeGenerator::WordAnd(const AA& address, word_t val) {
  Output() << TargetAddress(address) << " &= " << WordConstant(val) << ";";
  LineFeed();
}  

// generate code: word
void EmbeddedcCodeGenerator::WordAnd(const AA& address, const AX& expression) {
  Output() << TargetAddress(address) << " &= " << expression << ";";
  LineFeed();
}  

// generate code: word
void EmbeddedcCodeGenerator::WordAnd(const AA& address, const AA& op1, const AA& op2) {
  Output() << TargetAddress(address) << " = " << TargetExpression(op1) << " & " << TargetExpression(op2) << ";";
  LineFeed();
}  

// generate code: word
void EmbeddedcCodeGenerator::WordAnd(const AA& address, const AA& op1, word_t op2) {
  Output() << TargetAddress(address) << " = " << TargetExpression(op1) << " & " << WordConstant(op2) << ";";
  LineFeed();
}

// generate code: word
void EmbeddedcCodeGenerator::WordNand(const AA& address, const AX& expression) {
  Output() << TargetAddress(address) << " &= ~(" << expression << ");";
  LineFeed();
}  

// generate code: word
CodePrimitives::AX EmbeddedcCodeGenerator::WordIsBitSet(const AA& address, int idx) {
  return AX("( " + TargetAddress(address) + " & " + WordConstant(0x01<<idx) + " ) != 0");
}

// generate code: word
CodePrimitives::AX EmbeddedcCodeGenerator::WordIsBitClr(const AA& address, int idx) {
  return AX("( " + TargetAddress(address) + " & " + WordConstant(0x01<<idx) + " ) == 0");
}

// generate code: word
CodePrimitives::AX EmbeddedcCodeGenerator::WordIsMaskSet(const AA& address, word_t mask) {
  return AX("( " + TargetAddress(address) + " & " + WordConstant(mask) + " ) != 0");
}

// generate code: conditionals
CodePrimitives::AX EmbeddedcCodeGenerator::WordIsEq(const AA& address, word_t val) {
  return AX(TargetAddress(address) + " == " + WordConstant(val));
}

// generate code: conditionals
CodePrimitives::AX EmbeddedcCodeGenerator::WordIsNotEq(const AA& address, word_t val) {
  return AX(TargetAddress(address) + " != " + WordConstant(val));
}

// generate code: word
EmbeddedcCodeGenerator::AX EmbeddedcCodeGenerator::WordConstant(word_t val) {
  AX res;
  std::stringstream sstr;
  sstr << "0x" << std::setbase(16) << std::setfill('0'); 
  // special case: zero
  if(val==0) sstr << std::setw(2);
  // special case: char
  if(mWordType == "unsigned char") { sstr << std::setw(2); val &=0xff; }
  // do stream
  if(mWordType == "unsigned char") sstr << val;
  else if(mWordType == "uint8_t") sstr << ((uint8_t) val) << "U";
  else if(mWordType == "unsigned short") sstr << ((unsigned short) val) << "U";
  else if(mWordType == "unsigned int") sstr << ((unsigned int) val) << "U";
  else if(mWordType == "uint16_t") sstr << ((uint16_t) val) << "U";
  else if(mWordType == "unsigned long") sstr << ((unsigned long) val) << "UL";
  else if(mWordType == "uint32_t") sstr << ((uint32_t) val) << "UL";
  else if(mWordType == "unsigned long long") sstr << ((unsigned long long) val) << "ULL";
  else FCG_ERR("EmbeddedcCodeGenerator: unsupported word data type [" << mWordType <<"]");
  sstr >> res;
  return res;
}

// generate code: string constant
EmbeddedcCodeGenerator::AX EmbeddedcCodeGenerator::StringConstant(const std::string& val) {
  AX res;
  res.append(1,'\"');
  for(std::size_t i=0; i<val.length(); ++i) {
    char c=val.at(i);
    // terminate
    if(c==0) break;
    // quote double quote
    if(c=='"') { res.append("\\\""); continue; }
    // quote back slash
    if(c=='\\') { res.append("\\\\"); continue; }
    // printable ascii
    if((c>=0x20) && (c<0x7f)) { res.append(1,c); continue; };
    // error on other (alternative: use octal encoding to support utf8)
    FCG_ERR("EmbeddedcCodeGenerator: non-printable ascii or other encoding unsupported [" << val <<"]");
  }
  res.append(1,'\"');
  return res;
}

// const-int array (Embedded C only helper)
EmbeddedcCodeGenerator::AX EmbeddedcCodeGenerator::IntarrayConstant(int offset, const std::vector<int>& val) {
  std::stringstream strstr;
  int newline=20;
  if(val.size()+offset<25) newline=25;
  strstr << "{ " ;
  size_t vit=0;
  while(true) {
    if(vit==val.size()+offset) break;
    if(vit< (size_t) offset)
      strstr << IntegerConstant(0);
    else
      strstr << IntegerConstant(val[vit-offset]);
    ++vit;
    if(vit==val.size()+offset) break;
    strstr << ", ";
    newline--;
    if(newline==0) {
      strstr << std::endl << "  ";
      newline=20;
    }
  }
  strstr << " }";
  return AX(strstr.str());
}

// const-int-array
void EmbeddedcCodeGenerator::CintarrayDeclare(const AA& address, int offset, const std::vector<int>& val) {
  if(val.size()==0) {
    FCG_ERR("EmbeddedcCodeGenerator::Cintarray(): ignoring empty const vector");
    return;
  }
  if(val.size()>= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("EmbeddedcCodeGenerator::Cintarray(): const vector exceeds addres range");
    return;
  }
  //const std::vector<int> *pval=&val;
  if(offset>0) {
    std::vector<int> oval(val.size()+offset,0);
    for(size_t i=0; i<val.size(); ++i)
      oval[offset+i]=val[i];
    //pval=&oval;
  }
  Output() << "const " << mIntegerType << " " << TargetAddress(address) << "[] = ";
  LineFeed();
  IndentInc();
  Output() << IntarrayConstant(offset,val) << ";";
  LineFeed();
  IndentDec();
}

// int array access by const
EmbeddedcCodeGenerator::AA EmbeddedcCodeGenerator::CintarrayAccess(const AA& address, int index) {
  return AA(address + "[" + ToStringInteger(index) + "]");
};  

// int array access by expression
EmbeddedcCodeGenerator::AA EmbeddedcCodeGenerator::CintarrayAccess(const AA& address, const AA& indexaddr){
  return AA(address + "[" + TargetAddress(indexaddr) + "]");
};

// tell parent class about this feature
bool EmbeddedcCodeGenerator::HasCintarray(void) {
  return true;
};


// const-word array (Embedded C only helper)
EmbeddedcCodeGenerator::AX EmbeddedcCodeGenerator::WordarrayConstant(int offset, const std::vector<word_t>& val) {
  std::stringstream strstr;
  int newline=20;
  if(val.size()+offset<25) newline=25;
  strstr << "{ " ;
  size_t vit=0;
  while(true) {
    if(vit==val.size()+offset) break;
    if(vit < (size_t) offset)
      strstr << WordConstant(0);
    else
      strstr << WordConstant(val[vit-offset]);
    ++vit;
    if(vit==val.size()+offset) break;
    strstr << ", ";
    newline--;
    if(newline==0) {
      strstr << std::endl << "  ";
      newline=20;
    }
  }
  strstr << " }";
  return AX(strstr.str());
}

// const-word-array
void EmbeddedcCodeGenerator::CwordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val) {
  if(val.size()==0) {
    FCG_ERR("EmbeddedcCodeGenerator::Cwordarray(): ignoring empty const vector");
    return;
  }
  if(val.size()>= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("EmbeddedcCodeGenerator::Cwordarray(): const vector exceeds addres range");
    return;
  }
  Output() << "const " << mWordType << " " << TargetAddress(address) << "[] = ";
  IndentInc();
  Output() << WordarrayConstant(offset,val) << ";";
  LineFeed();
  IndentDec();
}

// word-array access by const
EmbeddedcCodeGenerator::AA EmbeddedcCodeGenerator::CwordarrayAccess(const AA& address, int index) {
  return AA(address + "[" + ToStringInteger(index) + "]");
};  

// word array access by expression
EmbeddedcCodeGenerator::AA EmbeddedcCodeGenerator::CwordarrayAccess(const AA& address, const AA& indexaddr){
  return AA(address + "[" + TargetAddress(indexaddr) + "]");
};

// tell parent class about this feature
bool EmbeddedcCodeGenerator::HasCwordarray(void) {
  return true;
};

// int-array
void EmbeddedcCodeGenerator::IntarrayDeclare(const AA& address, int offset, const std::vector<int>& val) {
  if(val.size()==0) {
    FCG_ERR("EmbeddedcCodeGenerator::Intarray(): ignoring empty const vector");
    return;
  }
  if(val.size()>= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("EmbeddedcCodeGenerator::Intarray(): vector exceeds address range");
    return;
  }
  Output() << mIntegerType << " " << TargetAddress(address) << "[] = ";
  IndentInc();
  Output() << IntarrayConstant(offset,val) << ";";
  LineFeed();
  IndentDec();
}

// int-array
void EmbeddedcCodeGenerator::IntarrayDeclare(const AA& address, int offset, int len) {
  if(len==0) {
    FCG_ERR("EmbeddedcCodeGenerator::Intarray(): ignoring empty const vector");
    return;
  }
  if(len+offset>= (1LL << (mIntegerSize-1))) {
    FCG_ERR("EmbeddedcCodeGenerator::Intarray(): const vector exceeds address range");
    return;
  }
  Output() << mIntegerType << " " << TargetAddress(address) << "[" << IntegerConstant(len+offset) << "];";
  LineFeed();
}

// int-array access by const
EmbeddedcCodeGenerator::AA EmbeddedcCodeGenerator::IntarrayAccess(const AA& address, int index) {
  return AA(address + "[" + ToStringInteger(index) + "]");
};

// int array access by expression
EmbeddedcCodeGenerator::AA EmbeddedcCodeGenerator::IntarrayAccess(const AA& address, const AA& indexaddr){
  return AA(address + "[" + TargetAddress(indexaddr) + "]");
};

// tell parent class about this feature
bool EmbeddedcCodeGenerator::HasIntarray(void) {
  return true;
};

// word-array
void EmbeddedcCodeGenerator::WordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val) {
  if(val.size()==0) {
    FCG_ERR("EmbeddedcCodeGenerator::Wordarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("EmbeddedcCodeGenerator::Wordarray(): const vector exceeds addres range");
    return;
  }
  Output() << mWordType << " " << TargetAddress(address) << "[] = ";
  IndentInc();
  Output() << WordarrayConstant(offset,val) << ";";
  LineFeed();
  IndentDec();
}

// word-array
void EmbeddedcCodeGenerator::WordarrayDeclare(const AA& address, int offset, int len) {
  if(len==0) {
    FCG_ERR("EmbeddedcCodeGenerator::Wordarray(): ignoring empty const vector");
    return;
  }
  if(len+offset>= (1LL << (mIntegerSize-1))) {
    FCG_ERR("EmbeddedcCodeGenerator::Wordarray(): const vector exceeds addres range");
    return;
  }
  Output() << mWordType << " " << TargetAddress(address) << "[" << IntegerConstant(len+offset) << "];";
  LineFeed();
}

// word-array access by const
EmbeddedcCodeGenerator::AA EmbeddedcCodeGenerator::WordarrayAccess(const AA& address, int index) {
  return AA(address + "[" + ToStringInteger(index) + "]");
};

// word array access by expression
EmbeddedcCodeGenerator::AA EmbeddedcCodeGenerator::WordarrayAccess(const AA& address, const AA& indexaddr){
  return AA(address + "[" + TargetAddress(indexaddr) + "]");
};

// tell parent class about this feature
bool EmbeddedcCodeGenerator::HasWordarray(void) {
  return true;
};

// literal const-string array (Embedded C only helper)
EmbeddedcCodeGenerator::AX EmbeddedcCodeGenerator::StrarrayConstant(int offset, const std::vector<std::string>& val) {
  std::stringstream strstr;
  int newline;
  if(val.size()+offset <4) {
    newline=4;
    strstr << "{ ";
  } else {
    newline=1;
    strstr << "{" << std::endl;
  }
  size_t vit=0;
  while(true) {
    if(vit==val.size()+offset) break;
    if(vit < (size_t)offset)
      strstr << StringConstant("");
    else
      strstr << StringConstant(val[vit-offset]);
    ++vit;
    if(vit==val.size()+offset) break;
    strstr << ", ";
    newline--;
    if(newline==0) {
      strstr << std::endl;
      newline=1;
    }
  }
  strstr << " }";
  return AX(strstr.str());
}

// const string array
void EmbeddedcCodeGenerator::CstrarrayDeclare(const AA& address, int offset, const std::vector<std::string>& val) {
  if(val.size()==0) {
    FCG_ERR("EmbeddedcCodeGenerator::Cstrarrayy(): ignoring empty string array");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("EmbeddedcCodeGenerator::Cstrarray(): string array exceeds address range");
    return;
  }
  Output() << "const char* const" << " " << TargetAddress(address) << "[] = ";
  IndentInc();
  Output() << StrarrayConstant(offset,val) << ";";
  LineFeed();
  IndentDec();
};

// const string array
EmbeddedcCodeGenerator::AA EmbeddedcCodeGenerator::CstrarrayAccess(const AA& address, int index) {
  return AA(address + "[" + ToStringInteger(index) + "]");
};

// const string array
EmbeddedcCodeGenerator::AA EmbeddedcCodeGenerator::CstrarrayAccess(const AA& address, const AA& indexaddr){
  return AA(address + "[" + TargetAddress(indexaddr) + "]");
};

// tell parent about this feature
bool EmbeddedcCodeGenerator::HasCstrarray(void) {
  return true;
};


// generate code: conditionals
void EmbeddedcCodeGenerator::IfTrue(const AX& expression) {
  Output() << "if( " << expression << " ) {";
  LineFeed();
  IndentInc();
}

// generate code: conditionals
void EmbeddedcCodeGenerator::IfFalse(const AX& expression) {
  Output() << "if( ! ( " << expression << " ) ) {";
  LineFeed();
  IndentInc();
}

// generate code: conditionals
void EmbeddedcCodeGenerator::IfWord(const AX& expression) {
  Output() << "if( " << expression << " ) {";
  LineFeed();
  IndentInc();
}

// generate code: conditionals
void EmbeddedcCodeGenerator::IfElse(void) {
  IndentDec();
  Output() << "} else {";
  LineFeed();
  IndentInc();
}

// generate code: conditionals
void EmbeddedcCodeGenerator::IfElseIfTrue(const AX& expression) {
  IndentDec();
  Output() << "} else if( " << expression << " ) {";
  LineFeed();
  IndentInc();
}

// generate code: conditionals
void EmbeddedcCodeGenerator::IfEnd(void) {
  IndentDec();
  Output() << "};";
  LineFeed();
}

// switch control
void EmbeddedcCodeGenerator::SwitchBegin(const AA& address){
  Output() << "switch(" << TargetAddress(address) << ") {";
  LineFeed();
}

// switch control
void EmbeddedcCodeGenerator::SwitchCase(const AA& address, int val){
  (void) address;
  Output() << "case " << IntegerConstant(val) << ":";
  LineFeed();
  IndentInc();
}

// switch control
void EmbeddedcCodeGenerator::SwitchCases(const AA& address, int from, int to){
  // simplify to singleton
  if(from==to) {
    SwitchCase(address, from);
    return;
  }
  // do range (quiet for empty)
  for(int val=from; val<=to; ++val) {
    Output() << "case " << IntegerConstant(val) << ":";
    LineFeed();
  }
  IndentInc();
}

// switch control
void EmbeddedcCodeGenerator::SwitchCases(const AA& address, const std::set< int >& vals){
  (void) address;
  // do set (quiet for empty)
  std::set< int >::const_iterator vit=vals.begin();
  for(; vit!=vals.end(); ++ vit) {
    Output() << "case " << IntegerConstant(*vit) << ":";
    LineFeed();
  }
  IndentInc();
}

// switch control
void EmbeddedcCodeGenerator::SwitchBreak(void){
  Output() << "break;";
  LineFeed();
  IndentDec();
}  

// switch control
void EmbeddedcCodeGenerator::SwitchEnd(void){
  Output() << "default:";
  LineFeed();
  IndentInc();
  Output() << "break;";
  LineFeed();
  IndentDec();
  Output() << "};";
  LineFeed();
}

// switch control
bool EmbeddedcCodeGenerator::HasMultiCase(void) {
  return true;
}

// loops
void EmbeddedcCodeGenerator::LoopBegin(void) {
  Output() << "while(1) {";
  LineFeed();
  IndentInc();
}

// loops
void EmbeddedcCodeGenerator::LoopBreak(const AX& expression) {
  Output() << "if( " << expression << " ) break;";
  LineFeed();
}


// loops
void EmbeddedcCodeGenerator::LoopEnd(void) {
  IndentDec();
  Output() << "};";
  LineFeed();
}

void EmbeddedcCodeGenerator::FunctionReturn(void) {
  Output() << "return;";
  LineFeed();
}


// timer
void EmbeddedcCodeGenerator::TimerDeclare(const AA& address, const std::string& litval) {
  // read/check ftu time format
  char* next;
  const char* first = litval.c_str();
  strtol(first,&next,10);
  bool err=false;
  if(*next!='f') err=true;
  if(!err) if(*(++next)!='t') err=true;
  if(!err) if(*(++next)!='u') err=true;
  if(!err) if(*(++next)!=0) err=true;
  if(err) {
      FCG_ERR("EmbeddedcCodeGenerator: missmatched time literal "+ litval);
  }
  Output() << "static int " << TargetAddress(AA("timer_"+address+"_cnt")) << ";";
  LineFeed();
  Output() << "static char " << TargetAddress(AA("timer_"+address+"_run")) << ";";
  LineFeed();
}
void EmbeddedcCodeGenerator::TimerStart(const AA& address) {
  Output() << TargetAddress(AA("timer_"+address+"_run")) << " = 1;";
  LineFeed();
}
void EmbeddedcCodeGenerator::TimerStop(const AA& address) {
  Output() << TargetAddress(AA("timer_"+address+"_run")) << " = 0;";
  LineFeed();
}
void EmbeddedcCodeGenerator::TimerReset(const AA& address, const std::string& litval) {
  int val=(int) strtol(litval.c_str(),NULL,10);
  Output() << TargetAddress(AA("timer_"+address+"_cnt")) << " = " << IntegerConstant(val) << ";";
  LineFeed();
}
CodePrimitives::AX EmbeddedcCodeGenerator::TimerIsElapsed(const AA& address) {
  return AX(TargetAddress(AA("timer_"+address+"_run")) +  " && (" + TargetAddress(AA("timer_"+address+"_cnt")) + " <= 0)");
}


// output actions 
void EmbeddedcCodeGenerator::RunActionSet(const std::string& address) {
  Output() << address << " = 1;" ;
  LineFeed();
}
void EmbeddedcCodeGenerator::RunActionClr(const std::string& address) {
  Output() << address << " = 0;" ;
  LineFeed();
}
void EmbeddedcCodeGenerator::RunActionExe(const AX& expression) {
  Output() << expression << ";";
  LineFeed();
}



