/** @file cgp_atmega.cpp @brief Code-generator for ATmega microcontrollers */

/*
  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2016 Thomas Moor

*/

// my includes
#include "cgp_atmega.h"


/*
******************************************************************
******************************************************************
******************************************************************

ATmegaCodeGenerator implementation --- class mainenance

******************************************************************
******************************************************************
******************************************************************
*/


// Register derived class
FAUDES_REGISTERCODEGENERATOR("atm",ATmegaCodeGenerator);


// ATmegaCodeGenerator(void)
ATmegaCodeGenerator::ATmegaCodeGenerator(void) : EmbeddedcCodeGenerator() {
  FD_DCG("atmCodeGenerator(" << this << ")::atmCodeGenerator()");
}

// ATmegaCodeGenerator(void)
ATmegaCodeGenerator::~ATmegaCodeGenerator(void) {
  FD_DCG("atmCodeGenerator(" << this << ")::~atmCodeGenerator()");
}


// clear
void ATmegaCodeGenerator::Clear(void) {
  FD_DCG("atmCodeGenerator::Clear()");
  // call base
  EmbeddedcCodeGenerator::Clear();
  // my flavor of defaults
  mPrefix="fcg_";
  mWordType="unsigned char";
  mWordSize=8;
  mIntegerType="int";
  mIntegerSize=16;
  // my config parameter
  mATmegaProgmem=false;
  mATmegaPullups=false;
}

//DoReadTargetConfiguration(rTr)
void ATmegaCodeGenerator::DoReadTargetConfiguration(TokenReader& rTr) {
  FD_DCG("atmCodeGenerator::DoReadTargetConfiguration()");
  // base
  EmbeddedcCodeGenerator::DoReadTargetConfiguration(rTr);
  // avr options
  Token token;
  if(rTr.ExistsBegin("ATmegaProgmem")) {
    rTr.ReadBegin("ATmegaProgmem",token);
    mATmegaProgmem= token.AttributeIntegerValue("val");
    rTr.ReadEnd("ATmegaProgmem");
  }
  if(rTr.ExistsBegin("ATmegaPullups")) {
    rTr.ReadBegin("ATmegaPullups",token);
    mATmegaPullups= token.AttributeIntegerValue("val");
    rTr.ReadEnd("ATmegaPullups");
  }
}

//DoWriteTargetConfiguration(rTw)
void ATmegaCodeGenerator::DoWriteTargetConfiguration(TokenWriter& rTw) const{
  FD_DCG("atmCodeGenerator::DoWriteTargetConfiguration()");
  // base
  EmbeddedcCodeGenerator::DoWriteTargetConfiguration(rTw);
}


/*
******************************************************************
******************************************************************
******************************************************************

atmCodeGenerator implementation --- code organisation

******************************************************************
******************************************************************
******************************************************************
*/


// DoCompile()
void ATmegaCodeGenerator::DoCompile(void) {
  FD_DCG("atmCodeGenerator(" << this << ")::DoCompile()");
  // call base
  EmbeddedcCodeGenerator::DoCompile();
  // fix my code options
  mATmegaPgmReadInteger="";
  mATmegaPgmReadWord="";
  switch(mIntegerSize) {
    case 8:  mATmegaPgmReadInteger="pgm_read_byte"; break;
    case 16: mATmegaPgmReadInteger="pgm_read_word"; break;
  }
  switch(mWordSize) {
    case 8:  mATmegaPgmReadWord="pgm_read_byte"; break;
    case 16: mATmegaPgmReadWord="pgm_read_word"; break;
  }
  if(mATmegaProgmem) {
    if((mATmegaPgmReadInteger=="") || (mATmegaPgmReadWord==""))
      FCG_VERB0("ATmegaCodeGenerator::DoCompile(): target supports pgmread only for 8bit or 16bit data types --- ignoring option ATmegaProgmen ");
  }
  if(mATmegaPgmReadInteger=="") mATmegaProgmem=false;
  if(mATmegaPgmReadWord=="") mATmegaProgmem=false;
  if((mIntegerSize != 8) && (mIntegerSize != 16) && mArrayForTransitions) {
    FCG_ERR("atmCodeGenerator::DoCompile(): target supports only 8bit or 16bit integers in compiled transition arrays");
  }
  // my preferences
  if(mBitAddressArithmetic && mArrayForBitmasks && mATmegaProgmem) {
    FCG_VERB1("atmCodeGenerator::DoCompile(): with progmen we prefer bit-address maths over compiled bitmasks");
    mArrayForBitmasks = false;
  }
  // my preferences
  if(mEventNameLookup || mStateNameLookup) {
    FCG_VERB1("atmCodeGenerator::DoCompile(): symbol tables not recommended on this target due to memory restrictions");
  }
}

// DoGenerate()
void ATmegaCodeGenerator::DoGenerate(void) {
  FD_DCG("atmCodeGenerator(" << this << ")::DoGenerate()");
  // cut and paste from base
  mBitarrays.clear();
  // say hello
  Comment("************************************************");
  Comment("CodeGenerator: Target ATmega Microcontroller    ");
  Comment("************************************************");
  LineFeed(1);
  Comment(std::string("CompileDES ") + VersionString());
  Comment(std::string("Configuration: ") + Name());
  LineFeed(2+1);
  // snippets
  LiteralPrepend();
  // use base class for std semantics
  DoGenerateDeclarations();
  LineFeed(2);
  // use base class for std semantics
  Output() << "void " << mPrefix <<"cyclic(void) { ";
  LineFeed();
  DoGenerateResetCode();
  DoGenerateCyclicCode();
  Output() << "}; /* end function " << mPrefix <<"cyclic() */";
  LineFeed();
  LineFeed(2+1);
  // extra from ecCodeGenerator: provide timer decrement interface to host application
  DecrementTimers();
  // extra from atmCodeGenerator: initialise ports
  InitialisePorts();
  // snippets
  LiteralAppend();
  // done
  Comment("************************************************");
  Comment("CodeGenerator: Generated Code Ends Here         ");
  Comment("************************************************");
}


// code blocks: initialise ports
void ATmegaCodeGenerator::InitialisePorts(void){
  // figure output pins on ports A,B,C,D,E,F
  bool outexists= false;
  std::map<char, std::set< std::string > > outbits;
  ActionAddressIterator ait=ActionAddressesBegin();
  for(;ait!=ActionAddressesEnd();++ait) {
    // strict syntax check for set/clr actions, otherwise we cannot handle bit operations
    if(!ait->second.mSetClr) continue;
    if(ait->second.mAddress.size()!=3)  {
      FCG_ERR("atmCodeGenerator::InitialisePorts(): unkown output port [" << ait->second.mAddress << "]");
    }
    if(ait->second.mAddress[0]!='P')  {
      FCG_ERR("atmCodeGenerator::InitialisePorts(): unkown output port [" << ait->second.mAddress << "]");
    }
    char port = ait->second.mAddress[1];
    if((port < 'A') || (port > 'F'))  {
      FCG_ERR("atmCodeGenerator::InitialisePorts(): unkown output port [" << ait->second.mAddress << "]");
    }
    int pin = ait->second.mAddress[2] - '0';
    if((pin < 0) || (pin > 7)) {
      FCG_ERR("atmCodeGenerator::InitialisePorts(): unkown output port [" << ait->second.mAddress << "]");
    }
    outbits[port].insert(ait->second.mAddress);
    outexists= true;
  }
  // figure input pins on ports A,B,C,D,E,F
  bool inpexists= false;
  std::map<char, std::set< std::string > > inpbits;
  LineIterator lit=LinesBegin();
  for(;lit!=LinesEnd();++lit) {
    // weak syntax check for inputs, interpret as boolean expression if its not a port bit
    if(lit->second.mAddress.size()!=3) continue;
    if(lit->second.mAddress[0]!='P') continue;
    char port = lit->second.mAddress[1];
    if((port < 'A') || (port > 'F')) continue;
    int pin = lit->second.mAddress[2] - '0';
    if((pin < 0) || (pin > 7)) continue;
    inpbits[port].insert(lit->second.mAddress);
    inpexists= true;
  }
  // skip this section
  if((!outexists) && (!(inpexists && mATmegaPullups))) return;
  // configure ports
  Comment("************************************************");
  Comment("* initialise input/output pins                 *");
  Comment("************************************************");
  Output() << "void " << mPrefix <<"initpio(void) { ";
  LineFeed();
  IndentInc();
  std::map<char, std::set< std::string > >::iterator oit = outbits.begin();
  for(;oit!= outbits.end(); ++oit) {
    Output() << "DDR" << oit->first << " |= ";
    std::set< std::string >::iterator bit= oit->second.begin();
    while(true) {
      Output() << "( 1 << " << *bit << " )";
      ++bit;
      if(bit== oit->second.end()) break;
      Output() << " | ";
    }
    Output() << ";";
    LineFeed();
  }
  if(mATmegaPullups) {
    std::map<char, std::set< std::string > >::iterator iit = inpbits.begin();
    for(;iit!= inpbits.end(); ++iit) {
      Output() << "PORT" << iit->first << " |= ";
      std::set< std::string >::iterator bit= iit->second.begin();
      while(true) {
        Output() << "( 1 << " << *bit << " )";
        ++bit;
        if(bit== iit->second.end()) break;
        Output() << " | ";
      }
      Output() << ";";
      LineFeed();
    }
  }
  IndentDec();
  Output() << "};";
  LineFeed(1+2);
}

// output actions
void ATmegaCodeGenerator::RunActionSet(const std::string& address) {
  Output() << "PORT" << address[1] << " |= ( 1 << " << address << " );";
  LineFeed();
}
void ATmegaCodeGenerator::RunActionClr(const std::string& address) {
  Output() << "PORT" << address[1] << " &= ~( 1 << " << address << " );";
  LineFeed();
}

// read inputs
ATmegaCodeGenerator::AX ATmegaCodeGenerator::ReadInputLine(const std::string& address) {
  // if it is a port bit, convert to boolean expression
  if(address.size()==3)
  if(address[0]=='P')
  if(address[1]>='A')
  if(address[1]<='F')
  if(address[2]>='0')
  if(address[2]<='7') {
    std::string res = "( PIN" + address.substr(1,1) + " & ( 1 << " + address + " ) )";
    return AX(res);
  }
  // fallback to boolean expression
  std::string res = address;
  return AX(res);
}




/*
******************************************************************
******************************************************************
******************************************************************

atmCodeGenerator implementation --- atomic snippets

******************************************************************
******************************************************************
******************************************************************
*/

// implement pgm read for address-to-expression conversion
CodePrimitives::AX ATmegaCodeGenerator::TargetExpression(const AA& address) {
  // test case
  bool pgm_finteger = address.find("#PGM_FINTEGER# ")==0;
  bool pgm_fword = address.find("#PGM_FWORD# ")==0;
  // integer from pgm
  if(pgm_finteger) {
    size_t beg= address.find_first_of('#',1)+2;
    std::string addr=address.substr(beg);
    return AX( mATmegaPgmReadInteger + "( &( " + addr + " ) )");
  }
  // word from pgm
  if(pgm_fword) {
    size_t beg= address.find_first_of('#',1)+2;
    std::string addr=address.substr(beg);
    return AX( mATmegaPgmReadWord + "( &( " + addr + " ) )");
  }
  // base case: address matches expression
  return AX(TargetAddress(address));
}

std::string ATmegaCodeGenerator::TargetAddress(const AA& address) {
  // test case
  bool pgm_finteger = address.find("#PGM_FINTEGER# ")==0;
  bool pgm_fword = address.find("#PGM_FWORD# ")==0;
  // cannot deal with it
  if(pgm_finteger || pgm_fword) {
    FCG_ERR("ATmegaCodeGenerator: cannot convert pgm-address to target address")
  }
  std::string res=mPrefix+address;
  return res;
}


// const-int-array
void ATmegaCodeGenerator::CintarrayDeclare(const AA& address, int offset, const std::vector<int>& val) {
  if(val.size()==0) {
    FCG_ERR("atmCodeGenerator::Cintarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("atmCodeGenerator::Cwordarray(): const vector exceeds addres range");
    return;
  }
  if(mATmegaProgmem) {
    Output() << "const " << mIntegerType << " " << TargetAddress(address) << "[] PROGMEM = ";
    LineFeed();
  } else {
    Output() << "const " << mIntegerType << " " << TargetAddress(address) << "[] = ";
    LineFeed();
  }
  IndentInc();
  Output() << IntarrayConstant(offset,val) << ";";
  LineFeed();
  IndentDec();
}


// int array access by const
CodePrimitives::AA ATmegaCodeGenerator::CintarrayAccess(const AA& address, int index) {
 if(mATmegaProgmem) {
   return AA("#PGM_FINTEGER# " + TargetAddress(address) + "[" + ToStringInteger(index) + "]");
  } else {
   return AA(address + "[" + ToStringInteger(index) + "]");
  }
};

// int array access by expression
CodePrimitives::AA ATmegaCodeGenerator::CintarrayAccess(const AA& address, const AA &indexaddr){
  if(mATmegaProgmem) {
    return AA("#PGM_FINTEGER# " + TargetAddress(address) + "[" + TargetAddress(indexaddr) + "]");
  } else {
    return AA(address + "[" + TargetAddress(indexaddr) + "]");
  }
};


// const-word-array
void ATmegaCodeGenerator::CwordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val) {
  if(val.size()==0) {
    FCG_ERR("ecCodeGenerator::Cwordarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("ecCodeGenerator::Cwordarray(): const vector exceeds addres range");
    return;
  }
  if(mATmegaProgmem) {
    Output() << "const " << mWordType << " " << TargetAddress(address) << "[] PROGMEM = ";
    LineFeed();
  } else {
    Output() << "const " << mWordType << " " << TargetAddress(address) << "[] = ";
    LineFeed();
  }
  IndentInc();
  Output() << WordarrayConstant(offset,val) << ";";
  LineFeed();
  IndentDec();
}

// word-array access by const
CodePrimitives::AA ATmegaCodeGenerator::CwordarrayAccess(const AA& address, int index) {
  if(mATmegaProgmem) {
    return AA("#PGM_FWORD# " + TargetAddress(address) + "[" + ToStringInteger(index) + "]");
  } else {
    return AA(address + "[" + ToStringInteger(index) + "]");
  }
};

// word array access by expression
CodePrimitives::AA ATmegaCodeGenerator::CwordarrayAccess(const AA& address, const AA& indexaddr){
  if(mATmegaProgmem) {
    return AA("#PGM_FWORD# " + TargetAddress(address) + "[" + TargetAddress(indexaddr) + "]");
  } else {
    return AA(address + "[" + TargetAddress(indexaddr) + "]");
  }
};


// const string array
void ATmegaCodeGenerator::CstrarrayDeclare(const AA& address, int offset, const std::vector<std::string>& val) {
  if(val.size()==0) {
    FCG_ERR("ATmegaCodeGenerator::Cstrarrayy(): ignoring empty string array");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("ATmegaCodeGenerator::Cstrarray(): string array exceeds address range");
    return;
  }
  if(!mATmegaProgmem) {
    IndentInc();
    Output() << StrarrayConstant(offset,val) << ";";
    LineFeed();
    IndentDec();
    return;
  }
  // progmem: have individual strings first
  size_t vit;
  for(vit=offset; vit<val.size()+offset; ++vit) {
    Output() << "const char" << " " << TargetAddress(address.Sub(vit)) << "[] PROGMEM = ";
    Output() << StringConstant(val[vit-offset]) << ";";
    LineFeed();
  }
  // progmen: have array of pointers to individual strings
  Output() << "const char* const" << " " << TargetAddress(address) << "[] PROGMEM = ";
  IndentInc();
  std::stringstream strstr;
  int newline=5;
  strstr << "{" << std::endl;
  vit=0;
  while(true) {
    if(vit==val.size()+offset) break;
    if(vit < (size_t)offset)
      strstr << "0x0000";
    else
      strstr << TargetAddress(address.Sub(vit));
    ++vit;
    if(vit==val.size()+offset) break;
    strstr << ", ";
    newline--;
    if(newline==0) {
      strstr << std::endl;
      newline=5;
    }
  }
  strstr << " }";
  Output() << AX(strstr.str()) << ";";
  LineFeed();
  IndentDec();  
};


// const string array access not defined
CodePrimitives::AA  ATmegaCodeGenerator::CstrarrayAccess(const AA& address, int index) {
  (void) address;
  (void) index;
  FCG_ERR("ATmegaCodeGenerator::Cstrarray(): constant-str-arrays access not defined");
  return AA();
};
CodePrimitives::AA  ATmegaCodeGenerator::CstrarrayAccess(const AA& address, const AA& indexaddr){
  (void) address;
  (void) indexaddr;
  FCG_ERR("ATmegaCodeGenerator::Cstrarray(): constant-str-arrays access not defined");
  return AA();
};
