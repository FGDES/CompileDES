/** @file cgp_plcoxml.cpp @brief Code-generator PlcOpen XML (IEC 61131-3 ST) */

/*
  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2010, 2016, 2017 Thomas Moor

*/

// my includes
#include "cgp_plcoxml.h"
#include <ctime>


/*
******************************************************************
******************************************************************
******************************************************************

PlcoxmlCodeGenerator implementation --- class

******************************************************************
******************************************************************
******************************************************************
*/


// Register derived class
FAUDES_REGISTERCODEGENERATOR("pox",PlcoxmlCodeGenerator);


// PlcoxmlCodeGenerator(void)
PlcoxmlCodeGenerator::PlcoxmlCodeGenerator(void) : Iec61131stCodeGenerator() {
  FD_DCG("PlcoxmlCodeGenerator(" << this << ")::PlcoxmlCodeGenerator()");
}


// PlcoxmlCodeGenerator(void)
PlcoxmlCodeGenerator::~PlcoxmlCodeGenerator(void) {
  FD_DCG("PlcoxmlCodeGenerator(" << this << ")::~PlcoxmlCodeGenerator()");
}


// clear
void PlcoxmlCodeGenerator::Clear(void) {
  FD_DCG("PlcoxmlCodeGenerator::Clear()");
  // call base
  Iec61131stCodeGenerator::Clear();
  // my output format defaults
  mIecSchemeVersion="v2.01";
  mIecToolVendor="FGDES/LRT";
  mIecContentAuthor="";
  mIecContentOrganization="";
  mIecContentVersion="";
  mIecTextElement="cdata";
  // my code defaults
  mHasIecTimeOperators=false;
}

//DoReadTargetConfiguration(rTr)
void PlcoxmlCodeGenerator::DoReadTargetConfiguration(TokenReader& rTr) {
  FD_DCG("PlcoxmlCodeGenerator::DoReadTargetConfiguration()");
  // base
  Iec61131stCodeGenerator::DoReadTargetConfiguration(rTr);
  // Plcoxml option
  Token token;
  if(rTr.ExistsBegin("IecSchemeVersion")) {
    rTr.ReadBegin("IecSchemeVersion",token);
    mIecSchemeVersion= token.AttributeStringValue("val");
    rTr.ReadEnd("IecSchemeVersion");
  }
  if(rTr.ExistsBegin("IecToolVendor")) {
    rTr.ReadBegin("IecToolVendor",token);
    mIecToolVendor= token.AttributeStringValue("val");
    rTr.ReadEnd("IecToolVendor");
  }
  if(rTr.ExistsBegin("IecContentAuthor")) {
    rTr.ReadBegin("IecContentAuthor",token);
    mIecContentAuthor= token.AttributeStringValue("val");
    rTr.ReadEnd("IecContentAuthor");
  }
  if(rTr.ExistsBegin("IecContentOrganization")) {
    rTr.ReadBegin("IecContenetOrganization",token);
    mIecContentOrganization= token.AttributeStringValue("val");
    rTr.ReadEnd("IecContentOrganization");
  }
  if(rTr.ExistsBegin("IecContentVersion")) {
    rTr.ReadBegin("IecContentVersion",token);
    mIecContentVersion= token.AttributeStringValue("val");
    rTr.ReadEnd("IecContentVersion");
  }
  if(rTr.ExistsBegin("IecTextElement")) {
    rTr.ReadBegin("IecTextElement",token);
    mIecTextElement= token.AttributeStringValue("val");
    rTr.ReadEnd("IecTextElement");
  }
}

//DoWriteTargetConfiguration(rTw)
void PlcoxmlCodeGenerator::DoWriteTargetConfiguration(TokenWriter& rTw) const{
  FD_DCG("PlcoxmlCodeGenerator::DoWriteTargetConfiguration()");
  // base
  Iec61131stCodeGenerator::DoWriteTargetConfiguration(rTw);
  // iec code option
  Token token;
  token.SetEmpty("IecSchemeVersion");
  token.InsAttributeString("val",mIecSchemeVersion);
  rTw.Write(token);
  token.SetEmpty("IecToolVendor");
  token.InsAttributeString("val",mIecToolVendor);
  rTw.Write(token);
  if(mIecContentAuthor.size()>0) {
    token.SetEmpty("IecContentAuthor");
    token.InsAttributeString("val",mIecContentAuthor);
    rTw.Write(token);
  }
  if(mIecContentOrganization.size()>0) {
    token.SetEmpty("IecContentCompany");
    token.InsAttributeString("val",mIecContentOrganization);
  }
  if(mIecContentVersion.size()>0) {
    rTw.Write(token);
    token.SetEmpty("IecContentVersion");
    token.InsAttributeString("val",mIecContentVersion);
  }
  if(mIecTextElement.size()>0) {
    rTw.Write(token);
    token.SetEmpty("IecTextElement");
    token.InsAttributeString("val",mIecTextElement);
    rTw.Write(token);
  }
}



/*
******************************************************************
******************************************************************
******************************************************************

PlcoxmlGenerator implementation --- code organisation

******************************************************************
******************************************************************
******************************************************************
*/

// DoGenerate()
void PlcoxmlCodeGenerator::DoGenerate(void) {
  // report on missconfig
  if((mIecSchemeVersion!="v2.01") && (mIecSchemeVersion!="v1.01"))
    FCG_ERR("PlcoxmlCodeGenerator::DoGenerate(): xml scheme \"" << mIecSchemeVersion << "\" not supported");
  if((mIecTextElement !="xhtml") && (mIecTextElement !="cdata") && (mIecTextElement !=""))
    FCG_ERR("PlcoxmlCodeGenerator::DoGenerate(): xml text element \"" << mIecTextElement << "\" not supported");
  // report on missconfig
  if(mName.size()==0)
    FCG_ERR("PlcoxmlCodeGenerator::DoGenerate(): xml format requires a project name");
  // report on missconfig
  if(mLiteralCyclic.size()!=0)
    FCG_ERR("PlcoxmlCodeGenerator::DoGenerate(): literal insert to cyclic function not supported by this target");
  if(mLiteralPrepend.size()!=0)
    FCG_ERR("PlcoxmlCodeGenerator::DoGenerate(): literal prepend not supported by this target");
  if(mLiteralAppend.size()!=0)
    FCG_ERR("PlcoxmlCodeGenerator::DoGenerate(): literal append not supported by this target");
  // cut and paste from base
  mBitarrays.clear();
  // set up time string
  time_t now_time;
  struct tm* now_tm;
  time (&now_time);
  now_tm = localtime(&now_time);
  char now_str[25];
  snprintf(now_str,24,"%4d-%02d-%02dT%02d:%02d:%02d",now_tm->tm_year+1900,now_tm->tm_mon+1,now_tm->tm_mday,now_tm->tm_hour,now_tm->tm_min,now_tm->tm_sec);
  std::string datestr(now_str);
  std::string verstr=COMPILEDES_VERSION;
  // doit: produce XML encoding
  Output() << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  // doit: open XML project element
  if(mIecSchemeVersion=="v1.01") {
    Output() << "<project" << std::endl;
    Output() << "  xmlns=\"http://www.plcopen.org/xml/tc6.xsd\"" << std::endl;
    Output() << "  xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"" << std::endl;
    Output() << "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl;
    Output() << "  xsi:schemaLocation=\"http://www.plcopen.org/xml/tc6.xsd http://www.plcopen.org/xml/tc6.xsd\" >" << std::endl;
  } else if(mIecSchemeVersion=="v2.01") {
    Output() << "<project" << std::endl;
    Output() << "  xmlns=\"http://www.plcopen.org/xml/tc6_0201\"" << std::endl;
    Output() << "  xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"" << std::endl;
    Output() << "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl;
    Output() << "  xsi:schemaLocation=\"http://www.plcopen.org/xml/tc6_0201 http://www.plcopen.org/xml/tc6.xsd\" >" << std::endl;
  } else {
    Output() << "<project>" << std::endl;
  }
  // doit: empty XML fileHeader element
  Output() << "<fileHeader" << std::endl;
  Output() << "  companyName=\"" << mIecToolVendor << "\"" << std::endl;
  Output() << "  creationDateTime=\"" << datestr << "\"" << std::endl;
  Output() << "  productName=\"CompileDES\"" << std::endl;
  Output() << "  productVersion=\"" << verstr << "\" />" << std::endl;
  // doit: open XML contentHeader element
  Output() << "<contentHeader" << std::endl;
  Output() << "  name=\"" << mName << "\"";
  if(mIecContentAuthor.size()>0) Output() << std::endl << "  author=\"" << mIecContentAuthor << "\"";
  if(mIecContentOrganization.size()>0) Output() << std::endl << "  organization=\"" << mIecContentOrganization << "\"";
  if(mIecContentVersion.size()>0) Output() << std::endl << "  version=\"" << mIecContentVersion << "\"";
  Output() << " >" << std::endl;
  // doit: complete XML coordinateInfo element
  Output() << "  <coordinateInfo>" << std::endl;
  Output() << "    <fbd><scaling x=\"1\" y=\"1\"/></fbd>" << std::endl;
  Output() << "    <ld><scaling x=\"1\" y=\"1\"/></ld>" << std::endl;
  Output() << "    <sfc><scaling x=\"1\" y=\"1\"/></sfc>" << std::endl;
  Output() << "  </coordinateInfo>" << std::endl;
  // doit: close XML header element
  Output() << "</contentHeader>" << std::endl;
  // doit: open XML types element
  Output() << "<types>" << std::endl;
  // doit: empty XML dataTypes element
  Output() << "<dataTypes />" << std::endl;
  // doit: open XML pous element
  Output() << "<pous>" << std::endl;
  DoGenerateFunction();
  DoGenerateLookups();
  Output() << "</pous>" << std::endl;
  Output() << "</types>" << std::endl;
  // doit: void XML instances element
  Output() << "<instances>" << std::endl;
  Output() << "  <configurations />" << std::endl;
  Output() << "</instances>" << std::endl;
  // doit: close XML open elements
  Output() << "</project>" << std::endl;
}

void PlcoxmlCodeGenerator::DoGenerateFunction() {
  // open XML my function block element
  Output() << "<pou name=\"" << mPrefix << "cyclic_fb" << "\" pouType=\"functionBlock\">" << std::endl;
  // open XML my interface element
  Output() << "<interface>" << std::endl;
  IndentInc();
  DoGenerateInterface();
  IndentDec();
  // close XML interface element
  Output() << "</interface>" << std::endl;
  // open XML my body element
  Output() << "<body>" << std::endl;
  Output() << "<ST>" << std::endl;
  XmlBeginPlainText();
  Output() << std::endl;
  DoGenerateBody();
  XmlEndPlainText();
  Output() << std::endl;
  // close XML open elements
  Output() << "</ST>" << std::endl;
  Output() << "</body>" << std::endl;
  Output() << "</pou>" << std::endl;
}


// code blocks: cyclic function block (interface)
void PlcoxmlCodeGenerator::DoGenerateInterface(void){
  // have counts
  int pio = CountImportPhysicalIo();
  int sio = CountImportSymbolicIo();
  // interface: arguments
  MuteComments(true);
  MuteVspace(true);
  Output() << "<inputVars retain=\"false\">" << std::endl;
  IndentInc();
  DeclareReset();
  IndentDec();
  Output() << "</inputVars>" << std::endl;
  MuteComments(false);
  MuteVspace(false);
  // interface: return value
  MuteComments(true);
  MuteVspace(true);
  Output() << "<outputVars retain=\"false\">" << std::endl;
  IndentInc();
  DeclareStatus();
  if(mEventNameLookup) DeclareRecentEvent();
  if(mEventNameLookup) DeclarePendingEvents();
  if(mStateNameLookup) DeclareParallelState();
  IndentDec();
  Output() << "</outputVars>" << std::endl;
  MuteComments(false);
  MuteVspace(false);
  // interface: local variables
  MuteComments(true);
  MuteVspace(true);
  Output() << "<localVars retain=\"false\">" << std::endl;
  IndentInc();
  if(!mEventNameLookup) DeclareRecentEvent();
  if(!mEventNameLookup) DeclarePendingEvents();
  if(!mStateNameLookup) DeclareParallelState();
  DeclareLoopState();
  DeclareTimers();
  DeclareAux();
  DeclareSmallCarray();
  DeclareLargeCarray();
  if((mIecDeclarePhysical=="VAR") && (pio>0))
    DeclareImportPhysicalIo();
  DeclareSystime();
  IndentDec();
  Output() << "</localVars>" << std::endl;
  MuteVspace(false);
  MuteComments(false);
  // interface: extern variables
  MuteComments(true);
  MuteVspace(true);
  if(mIecDeclarePhysical=="VAR_EXTERNAL") {
    if(pio+sio>0) {
      Output() << "<externalVars>" << std::endl;
      IndentInc();
      if(sio>0) DeclareImportSymbolicIo();
      if(pio>0) DeclareImportPhysicalIo();
      IndentDec();
      Output() << "</externalVars>" << std::endl;
    }
  } else {
    if(sio>0) {
      Output() << "<externalVars>" << std::endl;
      IndentInc();
      DeclareImportSymbolicIo();
      IndentDec();
      Output() << "</externalVars>" << std::endl;
    }
  }
  MuteVspace(false);
  MuteComments(false);
}


// code blocks: cyclic function block (body)
void PlcoxmlCodeGenerator::DoGenerateBody(void){
  Comment("************************************************");
  Comment("CodeGenerator: Target IEC 61131 Structured Text ");
  Comment("************************************************");
  LineFeed(2);
  Comment(std::string("CompileDES ") + VersionString());
  Comment(std::string("Configuration: ") + Name());
  LineFeed(2+1);
  LiteralCyclic();
  DoGenerateResetCode();
  DecrementTimers();
  DoGenerateCyclicCode();
  LineFeed(1+2);
  Comment("************************************************");
  Comment("CodeGenerator: Generated Code Ends Here         ");
  Comment("************************************************");
}


void PlcoxmlCodeGenerator::DeclareImportPhysicalIo(void){
  LineIterator lit=LinesBegin();
  for(;lit!=LinesEnd();++lit) {
    std::string lineaddr= lit->second.mAddress;
    if(lineaddr.size()<1) continue; // paranoia
    if(lineaddr.at(0)=='%') {
      Output() << "<variable name=\"\" address=\"" << lineaddr << "\">";
      LineFeed();
      IndentInc();
      Output() << "<type> <BOOL /> </type>";
      LineFeed();
      XmlDocumentation("physical input");
      LineFeed();
      IndentDec();
      Output() << "</variable>";
      LineFeed();
    }
  }
  ActionAddressIterator ait=ActionAddressesBegin();
  for(;ait!=ActionAddressesEnd();++ait) {
    if(!ait->second.mSetClr) continue;
    std::string actaddr= ait->second.mAddress;
    if(actaddr.size()<1) continue; // paranoia
    if(actaddr.at(0)=='%') {
      Output() << "<variable name=\"\" address=\"" << actaddr << "\">";
      LineFeed();
      IndentInc();
      Output() << "<type> <BOOL /> </type>";
      LineFeed();
      XmlDocumentation("physical output");
      LineFeed();
      IndentDec();
      Output() << "</variable>";
      LineFeed();
    }
  }
}


// code blocks: functions to host lookup tables
void PlcoxmlCodeGenerator::DoGenerateLookups(void){
  if(mEventNameLookup) {
    // open XML my function block element
    Output() << "<pou name=\"" << mPrefix << "event_lookup_f" << "\" pouType=\"function\">" << std::endl;
    // open XML my interface element
    Output() << "<interface>" << std::endl;
    IndentInc();
    // interface: return type
    Output() << "<returnType> <string /> </returnType>" << std::endl;
    // interface: argument
    Output() << "<inputVars retain=\"false\">" << std::endl;
    Output() << "  <variable name=\"IDX\">" << std::endl;
    Output() << "    <type> <" << mIntegerType << " /> </type>" << std::endl;
    Output() << "    ";
    XmlDocumentation("event index");
    Output() << std::endl;
    Output() << "  </variable>" << std::endl;
    Output() << "</inputVars>" << std::endl;
    // interface: local variables
    MuteComments(true);
    MuteVspace(true);
    Output() << "<localVars retain=\"false\" constant=\"true\">" << std::endl;
    IndentInc();
    DeclareEventNameLookup();
    IndentDec();
    Output() << "</localVars>" << std::endl;
    MuteVspace(false);
    MuteComments(false);
    IndentDec();
    // close XML interface element
    Output() << "</interface>" << std::endl;
    // open XML my body element
    Output() << "<body>" << std::endl;
    Output() << "<ST>" << std::endl;
    XmlBeginPlainText();
    Output() << mPrefix <<"event_lookup_f"  << " := " << mPrefix << "event_lookup[IDX]" << ";";
    Output() << std::endl;
    XmlEndPlainText();
    // close XML open elements
    Output() << "</ST>" << std::endl;
    Output() << "</body>" << std::endl;
    Output() << "</pou>" << std::endl;
  }
  if(mStateNameLookup) {
    // open XML my function block element
    Output() << "<pou name=\"" << mPrefix << "state_lookup_f" << "\" pouType=\"function\">" << std::endl;
    // open XML my interface element
    Output() << "<interface>" << std::endl;
    IndentInc();
    // interface: return type
    Output() << "<returnType> <string /> </returnType>" << std::endl;
    // interface: argument
    Output() << "<inputVars retain=\"false\">" << std::endl;
    Output() << "  <variable name=\"GID\">" << std::endl;
    Output() << "    <type> <" << mIntegerType << " /> </type>" << std::endl;
    Output() << "    ";
    XmlDocumentation("generator id");
    Output() << std::endl;
    Output() << "  </variable>" << std::endl;
    Output() << "  <variable name=\"IDX\">" << std::endl;
    Output() << "    <type> <" << mIntegerType << " /> </type>" << std::endl;
    Output() << "    ";
    XmlDocumentation("state index");
    Output() << std::endl;
    Output() << "  </variable>" << std::endl;
    Output() << "</inputVars>" << std::endl;
    // interface: local variables
    MuteComments(true);
    MuteVspace(true);
    Output() << "<localVars retain=\"false\" constant=\"true\">" << std::endl;
    IndentInc();
    DeclareStateNameLookup();
    IndentDec();
    Output() << "</localVars>" << std::endl;
    MuteVspace(false);
    MuteComments(false);
    IndentDec();
    // close XML interface element
    Output() << "</interface>" << std::endl;
    // open XML my body element
    Output() << "<body>" << std::endl;
    Output() << "<ST>" << std::endl;
    XmlBeginPlainText();
    Output() << mPrefix << "state_lookup_f:='';";
    LineFeed();
    Output() << "CASE GID OF";
    LineFeed();
    for(size_t gid=0; gid<Size(); ++gid) {
      if(!mHasStateNames[gid]) continue;
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
    Output() << std::endl;
    XmlEndPlainText();
    // close XML open elements
    Output() << "</ST>" << std::endl;
    Output() << "</body>" << std::endl;
    Output() << "</pou>" << std::endl;
  }
}


/*
******************************************************************
******************************************************************
******************************************************************

PlcoxmlCodeGenerator implementation --- code primitives

******************************************************************
******************************************************************
******************************************************************
*/

// switch to/from plain text
void PlcoxmlCodeGenerator::XmlBeginPlainText() {
  if(mIecTextElement=="xhtml") {
    Output() << "<xhtml xmlns=\"http://www.w3.org/1999/xhtml\">";
    XmlTextEscape(true);
  } else if(mIecTextElement=="cdata") {
    Output() << "<![CDATA[";
    XmlCdataEscape(true);
  } else {
    XmlTextEscape(true);
  }
};

// switch to/from plain text
void PlcoxmlCodeGenerator::XmlEndPlainText() {
  if(mIecTextElement=="xhtml") {
    XmlTextEscape(false);
    Output() << "</xhtml>";
  } else if(mIecTextElement=="cdata") {
    XmlCdataEscape(false);
    Output() << "]]>";
  } else {
    XmlTextEscape(false);
  }
};


// switch to/from plain text
void PlcoxmlCodeGenerator::XmlDocumentation(const std::string& text) {
  Output() << "<documentation>";
  XmlBeginPlainText();
  Output() << text;
  XmlEndPlainText();
  Output() << "</documentation>";
};

// generate code: declare template
void PlcoxmlCodeGenerator::VariableDeclare(const std::string& laddr, const std::string& ltype){
  Output() << "<variable name=\"" << laddr << "\">";
  LineFeed();
  IndentInc();
  if((ltype == mIntegerType) || (ltype == mWordType) || (ltype == "BOOL") || (ltype == "TIME")) {
    Output() << "<type> <" << ltype << " /> </type>";
  } else if((ltype == "TON") || (ltype == "STRING")) {
    Output() << "<type> <derived name=\"" << ltype << "\" /> </type>";
  } else {
    FCG_ERR("PlcoxmlCodeGenerator::VariableDeclare(): unsupported type [" << ltype << "]");
  }
  LineFeed();
  XmlDocumentation(RecentComment());
  LineFeed();
  IndentDec();
  Output() << "</variable>";
  LineFeed();
}

// generate code: declare template with initialiser
void PlcoxmlCodeGenerator::VariableDeclare(const std::string& laddr, const std::string& ltype, const std::string& lval){
  Output() << "<variable name=\"" << laddr << "\">";
  LineFeed();
  IndentInc();
  if((ltype == mIntegerType) || (ltype == mWordType) || (ltype == "BOOL") || (ltype == "TIME")) {
    Output() << "<type> <" << ltype << " /> </type>";
  } else {
    FCG_ERR("PlcoxmlCodeGenerator::VariableDeclare(): unsupported type [" << ltype << "]");
  }
  LineFeed();
  Output() << "<initialValue> <simpleValue value=\"" << lval << "\" /> </initialValue>";
  LineFeed();
  XmlDocumentation(RecentComment());
  LineFeed();
  IndentDec();
  Output() << "</variable>";
  LineFeed();
}



// generate code: const-int-array
void PlcoxmlCodeGenerator::CintarrayDeclare(const AA& address, int offset, const std::vector<int>& val) {
  if(val.size()==0) {
    FCG_ERR("PlcoxmlCodeGenerator::Cintarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("PlcoxmlCodeGenerator::Cintarray(): const vector exceeds addres range");
    return;
  }
  Output() << "<variable name=\"" << TargetAddress(address) << "\">";
  LineFeed();
  IndentInc();
  Output() << "<type>";
  LineFeed();
  Output() << "  <array>";
  LineFeed();
  Output() << "    <dimension lower=\"" << offset << "\" upper=\"" << val.size()+offset-1 << "\" />";
  LineFeed();
  Output() << "    <baseType> <" << mIntegerType << " /> </baseType>";
  LineFeed();
  Output() << "  </array>";
  LineFeed();
  Output() << "</type>";
  LineFeed();
  Output() << "<initialValue> ";
  LineFeed();
  Output() << "  <arrayValue> ";
  LineFeed();
  for(size_t i=0; i<val.size(); ++i) {
    Output() << "    <value> <simpleValue value=\"" << IntegerConstant(val[i]) << "\" /> </value>";
    LineFeed();
  }
  Output() << "  </arrayValue> ";
  LineFeed();
  Output() << "</initialValue> ";
  LineFeed();
  XmlDocumentation(RecentComment());
  LineFeed();
  IndentDec();
  Output() << "</variable>";
  LineFeed();
}

// generate code: const-word-array
void PlcoxmlCodeGenerator::CwordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val) {
  if(val.size()==0) {
    FCG_ERR("PlcoxmlCodeGenerator::Cwordarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("PlcoxmlCodeGenerator::Cwordarray(): const vector exceeds addres range");
    return;
  }
  Output() << "<variable name=\"" << TargetAddress(address) << "\">";
  LineFeed();
  IndentInc();
  Output() << "<type>";
  LineFeed();
  Output() << "  <array>";
  LineFeed();
  Output() << "    <dimension lower=\"" << offset << "\" upper=\"" << val.size()+offset-1 << "\" />";
  LineFeed();
  Output() << "    <baseType> <" << mWordType << " /> </baseType>";
  LineFeed();
  Output() << "  </array>";
  LineFeed();
  Output() << "</type>";
  LineFeed();
  Output() << "<initialValue> ";
  LineFeed();
  Output() << "  <arrayValue> ";
  LineFeed();
  for(size_t i=0; i<val.size(); ++i) {
    Output() << "    <value> <simpleValue value=\"" << WordConstant(val[i]) << "\" /> </value>";
    LineFeed();
  }
  Output() << "  </arrayValue> ";
  LineFeed();
  Output() << "</initialValue> ";
  LineFeed();
  XmlDocumentation(RecentComment());
  LineFeed();
  IndentDec();
  Output() << "</variable>";
  LineFeed();
}


// generate code: const-str-array
void PlcoxmlCodeGenerator::CstrarrayDeclare(const AA& address, int offset, const std::vector<std::string>& val) {
  if(val.size()==0) {
    FCG_ERR("PlcoxmlCodeGenerator::Cstrarray(): ignoring empty const vector");
    return;
  }
  if(val.size()+offset >= (1ULL << (mIntegerSize-1))) {
    FCG_ERR("PlcoxmlCodeGenerator::Cstrarray(): const vector exceeds address range");
    return;
  }
  // figure string size
  size_t len=0;
  for(size_t i=0; i<val.size(); ++i)
    if(val[i].size()>len) len=val[i].size();
  // generate code
  Output() << "<variable name=\"" << TargetAddress(address) << "\">";
  LineFeed();
  IndentInc();
  Output() << "<type>";
  LineFeed();
  Output() << "  <array>";
  LineFeed();
  Output() << "    <dimension lower=\"" << offset << "\" upper=\"" << val.size()+offset-1 << "\" />";
  LineFeed();
  Output() << "    <baseType> <string length=\"" << ToStringInteger(len) << "\" /> </baseType>";
  LineFeed();
  Output() << "  </array>";
  LineFeed();
  Output() << "</type>";
  LineFeed();
  Output() << "<initialValue> ";
  LineFeed();
  Output() << "  <arrayValue> ";
  LineFeed();
  for(size_t i=0; i<val.size(); ++i) {
    Output() << "    <value> <simpleValue value=\"";
    XmlTextEscape(true);                    // use Xml escape
    std::string dat=StringConstant(val[i]); // use iec string escape
    // dat = dat.substr(1,dat.size()-2);       // strip delimiting single quotes [codesys does not like this]
    Output() << dat;
    XmlTextEscape(false);
    Output()<< "\" /> </value>";
    LineFeed();
  }
  Output() << "  </arrayValue> ";
  LineFeed();
  Output() << "</initialValue> ";
  LineFeed();
  XmlDocumentation(RecentComment());
  LineFeed();
  IndentDec();
  Output() << "</variable>";
  LineFeed();
}

// generate code: int-array
void PlcoxmlCodeGenerator::IntarrayDeclare(const AA& address, int offset, const std::vector<int>& val) {
  CintarrayDeclare(address,offset,val);
}

// generate code: int-array
void PlcoxmlCodeGenerator::IntarrayDeclare(const AA& address, int offset, int len) {
  if(len==0) {
    FCG_ERR("PlcoxmlCodeGenerator::Intarray(): ignoring empty const vector");
    return;
  }
  if(len+offset >= (1LL << (mIntegerSize-1))) {
    FCG_ERR("PlcoxmlCodeGenerator::Intarray(): const vector exceeds addres range");
    return;
  }
  Output() << "<variable name=\"" << TargetAddress(address) << "\">";
  LineFeed();
  IndentInc();
  Output() << "<type>";
  LineFeed();
  Output() << "  <array>";
  LineFeed();
  Output() << "    <dimension lower=\"" << offset << "\" upper=\"" << len+offset-1 << "\" />";
  LineFeed();
  Output() << "    <baseType> <" << mIntegerType << " /> </baseType>";
  LineFeed();
  Output() << "  </array>";
  LineFeed();
  Output() << "</type>";
  LineFeed();
  XmlDocumentation(RecentComment());
  LineFeed();
  IndentDec();
  Output() << "</variable>";
  LineFeed();
}


// generate code: word-array
void PlcoxmlCodeGenerator::WordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val) {
  CwordarrayDeclare(address,offset,val);
}

// generate code: word-array
void PlcoxmlCodeGenerator::WordarrayDeclare(const AA& address, int offset, int len) {
  if(len==0) {
    FCG_ERR("PlcoxmlCodeGenerator::Wordarray(): ignoring empty const vector");
    return;
  }
  if(len+offset >= (1LL << (mIntegerSize-1))) {
    FCG_ERR("PlcoxmlCodeGenerator::Wordarray(): const vector exceeds addres range");
    return;
  }
  Output() << "<variable name=\"" << TargetAddress(address) << "\">";
  LineFeed();
  IndentInc();
  Output() << "<type>";
  LineFeed();
  Output() << "  <array>";
  LineFeed();
  Output() << "    <dimension lower=\"" << offset << "\" upper=\"" << len+offset-1 << "\" />";
  LineFeed();
  Output() << "    <baseType> <" << mWordType << " /> </baseType>";
  LineFeed();
  Output() << "  </array>";
  LineFeed();
  Output() << "</type>";
  LineFeed();
  XmlDocumentation(RecentComment());
  LineFeed();
  IndentDec();
  Output() << "</variable>";
  LineFeed();
}


