/** @file cgp_kinetis.cpp @brief Code-generator for Freescale Kinetis K20 microcontrollers */

/*
  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2016 Thomas Moor

*/

// my includes
#include "cgp_kinetis.h"


/*
******************************************************************
******************************************************************
******************************************************************

KinetisCodeGenerator implementation --- class mainenance

******************************************************************
******************************************************************
******************************************************************
*/


// Register derived class
FAUDES_REGISTERCODEGENERATOR("k20",KinetisCodeGenerator);


// KinetisCodeGenerator(void)
KinetisCodeGenerator::KinetisCodeGenerator(void) : EmbeddedcCodeGenerator() {
  FD_DCG("KinetisCodeGenerator(" << this << ")::KinetisCodeGenerator()");
}

// KinetisCodeGenerator(void)
KinetisCodeGenerator::~KinetisCodeGenerator(void) {
  FD_DCG("KinetisCodeGenerator(" << this << ")::~KinetisCodeGenerator()");
}


// clear
void KinetisCodeGenerator::Clear(void) {
  FD_DCG("KinetisCodeGenerator::Clear()");
  // call base
  EmbeddedcCodeGenerator::Clear();
  // my flavor of defaults
  mPrefix="fcg_";
  mWordType="uint32_t";
  mWordSize=32;
  mIntegerType="int16_t";
  mIntegerSize=16;
  // my config parameter
  mKinetisOutputControl="";
  mKinetisInputControl="";
}

//DoReadTargetConfiguration(rTr)
void KinetisCodeGenerator::DoReadTargetConfiguration(TokenReader& rTr) {
  FD_DCG("KinetisCodeGenerator::DoReadTargetConfiguration()");
  // base
  EmbeddedcCodeGenerator::DoReadTargetConfiguration(rTr);
  // k20 options
  Token token;
  if(rTr.ExistsBegin("KinetisOutputControl")) {
    rTr.ReadBegin("KinetisOutputControl",token);
    mKinetisOutputControl= token.AttributeStringValue("val");
    rTr.ReadEnd("KinetisOutputControl");
  }
  if(rTr.ExistsBegin("KinetisInputControl")) {
    rTr.ReadBegin("KinetisInputControl",token);
    mKinetisInputControl= token.AttributeStringValue("val");
    rTr.ReadEnd("KinetisInputControl");
  }
}

//DoWriteTargetConfiguration(rTw)
void KinetisCodeGenerator::DoWriteTargetConfiguration(TokenWriter& rTw) const{
  FD_DCG("KinetisCodeGenerator::DoWriteTargetConfiguration()");
  // base
  EmbeddedcCodeGenerator::DoWriteTargetConfiguration(rTw);
  // k20 opts
  Token token;
  token.SetEmpty("KinetisOutputControl");
  token.InsAttributeString("val",mKinetisOutputControl);
  rTw.Write(token);
  token.SetEmpty("KinetisInputControl");
  token.InsAttributeString("val",mKinetisInputControl);
  rTw.Write(token);
}


/*
******************************************************************
******************************************************************
******************************************************************

KinetisCodeGenerator implementation --- code organisation

******************************************************************
******************************************************************
******************************************************************
*/


// DoCompile()
void KinetisCodeGenerator::DoCompile(void) {
  FD_DCG("KinetisCodeGenerator(" << this << ")::DoCompile()");
  // call base
  EmbeddedcCodeGenerator::DoCompile();
}

// DoGenerate()
void KinetisCodeGenerator::DoGenerate(void) {
  FD_DCG("KinetisCodeGenerator(" << this << ")::DoGenerate()");
  // cut and paste from base
  mBitarrays.clear();
  // say hello
  Comment("************************************************");
  Comment("CodeGenerator: Target Kinetis K20 uController   ");
  Comment("************************************************");
  LineFeed(1);
  Comment(std::string("CompileDES ") + VersionString());
  Comment(std::string("Configuration: ") + Name());
  LineFeed(2+1);
  // base class for std snippets
  LiteralPrepend();
  DoGenerateDeclarations();
  // cyclic function
  LineFeed(2);
  Output() << "void " << mPrefix <<"cyclic(void) { ";
  LineFeed();
  DoGenerateResetCode();
  DoGenerateCyclicCode();
  Output() << "}; /* end function " << mPrefix <<"cyclic() */";
  LineFeed();
  LineFeed(2+1);
  // extra from ecCodeGenerator: provide timer decrement interface to host application
  DecrementTimers();
  // extra from KinetisCodeGenerator: initialise ports
  InitialisePorts();
  // snippets
  LiteralAppend();
  // done
  Comment("************************************************");
  Comment("CodeGenerator: Generated Code Ends Here         ");
  Comment("************************************************");
}


// support: parse "PORTxn" to "xn"
std::string KinetisCodeGenerator::ParseLiteralPort(const std::string& lport) {
  std::string res;
  if((lport.size()<6) || (lport.size()>7)) return res;
  if(lport.substr(0,4)!="PORT") return res;
  char port = lport[4];
  if((port < 'A') || (port > 'E')) return res;
  int pin = lport[5] - '0';
  if((pin <0) || (pin > 9)) return res;
  if(lport.size()>6) {
    int pin2 = lport[6] - '0';
    if((pin2 <0) || (pin2 > 9)) return res;
    pin=10*pin+pin2;
  }
  if(pin >=32) return res;
  res.append(1,port);
  res.append(1,pin);
  return res;
}


// code blocks: initialise ports
void KinetisCodeGenerator::InitialisePorts(void){
  // figure output pins on ports A,B,C,D,E
  bool outexists= false;
  std::map<char, std::set< int > > outbits;
  ActionAddressIterator ait=ActionAddressesBegin();
  for(;ait!=ActionAddressesEnd();++ait) {
    // strict syntax check for set/clr actions, otherwise we cannot handle bit operations
    if(!ait->second.mSetClr) continue;
    std::string portpin=ParseLiteralPort(ait->second.mAddress);
    if(portpin=="")
      FCG_ERR("KinetisCodeGenerator::InitialisePorts(): unkown output port [" << ait->second.mAddress << "]");
    outbits[portpin[0]].insert(portpin[1]);
    outexists= true;
  }
  // figure input pins on ports A,B,C,D,E
  bool inpexists= false;
  std::map<char, std::set< int > > inpbits;
  LineIterator lit=LinesBegin();
  for(;lit!=LinesEnd();++lit) {
    // weak syntax check for inputs, interpret as boolean expression if its not a port bit
    std::string portpin=ParseLiteralPort(lit->second.mAddress);
    if(portpin=="") continue;
    inpbits[portpin[0]].insert(portpin[1]);
    inpexists= true;
  }
  // skip this section
  if(mKinetisOutputControl=="") outexists=false;
  if(mKinetisInputControl=="") inpexists=false;
  if( !(outexists || inpexists) ) return;
  // configure ports
  Comment("************************************************");
  Comment("* initialise input/output pins                 *");
  Comment("************************************************");
  Output() << "void " << mPrefix <<"initpio(void) { ";
  LineFeed();
  IndentInc();
  if(outexists) {
    std::map<char, std::set< int > >::iterator oit = outbits.begin();
    for(;oit!= outbits.end(); ++oit) {
      std::set< int >::iterator bit= oit->second.begin();
      for(;bit!=oit->second.end();++bit) {
        Output() << "PORT" << oit->first << "_PCR" << *bit << " = " << mKinetisOutputControl << " | PORT_PCR_MUX(1);";
        LineFeed();
      }
      word_t msk=0;
      bit= oit->second.begin();
      for(;bit!=oit->second.end();++bit)
        msk |= (1L << *bit);
      if(msk!=0) {
          Output() << "GPIO" << oit->first << "_PDDR |= " << WordConstant(msk) << ";";
        LineFeed();
      }
    }
  }
  if(inpexists) {
    std::map<char, std::set< int > >::iterator iit = inpbits.begin();
    for(;iit!= inpbits.end(); ++iit) {
      std::set< int >::iterator bit= iit->second.begin();
      for(;bit!=iit->second.end();++bit) {
        Output() << "PORT" << iit->first << "_PCR" << *bit << " = " << mKinetisInputControl << " | PORT_PCR_MUX(1);";
        LineFeed();
      }
    }
  }
  IndentDec();
  Output() << "};";
  LineFeed(1+2);
}

// output actions
void KinetisCodeGenerator::RunActionSet(const std::string& address) {
  std::string portpin=ParseLiteralPort(address);
  if(portpin=="")
    FCG_ERR("KinetisCodeGenerator::RunAction(): unkown output port [" << address << "]");
  Output() << "GPIO" << portpin[0] << "_PSOR = ( 1L << " << int(portpin[1]) << " );";
  LineFeed();
}
void KinetisCodeGenerator::RunActionClr(const std::string& address) {
  std::string portpin=ParseLiteralPort(address);
  if(portpin=="")
    FCG_ERR("KinetisCodeGenerator::RunAction(): unkown output port [" << address << "]");
  Output() << "GPIO" << portpin[0] << "_PCOR = ( 1L << " << int(portpin[1]) << " );";
  LineFeed();
}

// read inputs
KinetisCodeGenerator::AX KinetisCodeGenerator::ReadInputLine(const std::string& address) {
  // if it is a port bit, convert to boolean expression
  std::string portpin=ParseLiteralPort(address);
  if(portpin!="") {
    std::string res = "( GPIO" + std::string(1,portpin[0]) + "_PDIR & ( 1L << " + ToStringInteger(portpin[1]) + " ) )";
    return AX(res);
  }
  // fallback to boolean expression
  std::string res = address;
  return AX(res);
}






