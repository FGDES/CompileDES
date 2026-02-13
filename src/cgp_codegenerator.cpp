/** @file cgp_codegenerator.cpp @brief Code-generator common base class  */

/*

  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2010, 2016, Thomas Moor

*/

// my includes
#include "cgp_codegenerator.h"
#include <cctype>

/*
******************************************************************
******************************************************************
******************************************************************

CodeGenerator implementation: source data organisation

******************************************************************
******************************************************************
******************************************************************
*/


// CodeGenerator(void)
CodeGenerator::CodeGenerator(void) : Type(), pOutStream(0), pOutBuffer(0) {
  FD_DCG("CodeGenerator()::CodeGenerator()");
  pErrStream=&std::cerr;
  mOutMode="std::cout";
  mVerbLevel=1;
  mMuteComments=false;
  mMuteMode='*';
}

// CodeGenerator(void)
CodeGenerator::~CodeGenerator(void) { 
  FD_DCG("CodeGenerator()::~CodeGenerator()");
  if(pOutBuffer) delete pOutBuffer;
}

// mRegistry
std::map< std::string, CodeGenerator* (*)(void) >* CodeGenerator::mpsRegistry=0;

// Register(type)
void CodeGenerator::Register(const std::string& type, CodeGenerator* (*newcg)(void) ) {
  if(!mpsRegistry) mpsRegistry = new std::map< std::string, CodeGenerator* (*)(void) >;
  (*mpsRegistry)[type]=newcg;
}

// CodeGeneratorTypes(void)
std::vector< std::string > CodeGenerator::Registry(void) {
  std::vector< std::string > res;
  std::map< std::string, CodeGenerator* (*)(void) >::iterator rit;
  for(rit=mpsRegistry->begin(); rit != mpsRegistry->end(); ++rit)
     res.push_back(rit->first);
  return res;
}

// New(type)
CodeGenerator* CodeGenerator::New(const std::string& type) {
  if(mpsRegistry->find(type)==mpsRegistry->end()) return 0;
  return (*mpsRegistry)[type]();
}

// Clear()
void CodeGenerator::Clear(void) {
  FD_DCG("CodeGenerator()::Clear()");
  mGenerators.clear();
  mAlphabet.Clear();
  mUsedEvents.Clear();
  mWordSize=8;
  mIntegerSize=16;
}

// Insert(rFileName)
void CodeGenerator::Insert(const std::string& rFileName) {
  FCG_VERB2("CodeGenerator::Generator(): from file " << rFileName);
  TokenReader tr(rFileName);
  // create executor and read generator
  mGenerators.push_back(TimedGenerator());
  mGeneratorNames.push_back(rFileName);
  // read generator
  mGenerators.back().Read(tr);
  // report
  FCG_VERB2("CodeGenerator::Generator(): found [" << mGenerators.back().Name() << "] " <<
      "with #" << mGenerators.back().TransRelSize() << " transitions");
}

// Insert(rGen)
void CodeGenerator::Insert(const TimedGenerator& rGen) {
  FCG_VERB2("CodeGenerator::Generator(): generator by reference");
  mGenerators.push_back(TimedGenerator(rGen));
  mGeneratorNames.push_back("");
  // report
  FCG_VERB2("CodeGenerator::Generator(): found [" << mGenerators.back().Name() << "] " <<
    "with #" << mGenerators.back().TransRelSize() << " transitions");
}


// Size()
Idx CodeGenerator::Size(void) const {
  return (Idx) mGenerators.size();
}

//DoReadTargetConfiguration(rTr)
void CodeGenerator::DoReadTargetConfiguration(TokenReader& rTr) {
  (void) rTr;
}


//DoReadGenerators(rTr)
void CodeGenerator::DoReadGenerators(TokenReader& rTr) {
  // get relevant directory
  std::string dirname="";
  if(rTr.SourceMode()==TokenReader::File) 
    dirname=ExtractDirectory(rTr.FileName());
  // report
  FD_DCG("CodeGenerator()::DoReadGenerators(tr): dirname " << dirname);
  // read section
  rTr.ReadBegin("Generators");
  Token token;
  while(!rTr.Eos("Generators")) {
    rTr.Peek(token);
    // is it a file name?
    if(token.Type()==Token::String) {
      Insert(PrependPath(dirname,token.StringValue()));
      rTr.Get(token);
      continue;
    }
    // is it a generator?
    if(token.Type()==Token::Begin) 
    if(token.StringValue()=="Generator") {
      TimedGenerator gen;
      gen.Read(rTr);
      Insert(gen);
      continue;
    }
    // else report error
    std::stringstream errstr;
    errstr << "Invalid token" << rTr.FileLine();
    throw Exception("CodeGenerator::DoReadGenerators", errstr.str(), 502);
  }
  rTr.ReadEnd("Generators");
}


//DoRead(rTr)
void CodeGenerator::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) pContext;
  FD_DCG("CodeGenerator::DoRead()");
  std::string label=rLabel;
  if(label=="") label="CodeGenerator";
  // read my section
  Token token;
  rTr.ReadBegin(label,token);
  if(token.ExistsAttributeString("name"))
     Name(token.AttributeStringValue("name"));
  // loop to end
  while(!rTr.Eos(label)) {
    rTr.Peek(token);
    // global conf
    if(token.Type()==Token::Begin) 
    if(token.StringValue()=="TargetConfiguration") {
      rTr.ReadBegin("TargetConfiguration");
      DoReadTargetConfiguration(rTr);
      rTr.ReadEnd("TargetConfiguration");
      continue;
    }
    // generators
    if(token.Type()==Token::Begin) 
    if(token.StringValue()=="Generators") {
      DoReadGenerators(rTr);
      FCG_VERB1("CodeGenerator::Generator(): found #" << Size() << " generators");
      continue;
    }
    // alphabet
    if(token.Type()==Token::Begin) 
    if(token.StringValue()=="EventConfiguration") {
      mAlphabet.Read(rTr,"EventConfiguration");
      FCG_VERB1("CodeGenerator::Events(): found event configuration for #" << mAlphabet.Size() << " events");
      continue;
    }
    // else report error
    std::stringstream errstr;
    errstr << "Invalid token" << rTr.FileLine();
    throw Exception("CodeGenerator::DoRead", errstr.str(), 502);
  }
  // done
  rTr.ReadEnd(label);
}

//DoWriteTargetConfiguration(rTr,rLabel)
void CodeGenerator::DoWriteTargetConfiguration(TokenWriter& rTw) const {
  (void) rTw;
}

//DoWriteGenerators(rTr,rLabel)
void CodeGenerator::DoWriteGenerators(TokenWriter& rTw) const {
  rTw.WriteBegin("Generators");
  for(Idx i=0; i<Size(); i++) {
    // write filename
    if(mGeneratorNames.at(i)!="") {
      rTw.WriteString(mGeneratorNames.at(i));
    }
    // write generator
    if(mGeneratorNames.at(i)=="") {
      mGenerators.at(i).Write(rTw);
    }
  }
  rTw.WriteEnd("Generators");
}



//DoWrite(rTw);
void CodeGenerator::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  std::string label=rLabel;
  if(label=="") label="CodeGenerator";
  rTw.WriteBegin(label);
  // global conf
  rTw.WriteBegin("TargetConfiguration");
  DoWriteTargetConfiguration(rTw);
  rTw.WriteEnd("TargetConfiguration");
  // alphabet
  mAlphabet.Write(rTw,"EventConfiguration");
  // generators
  DoWriteGenerators(rTw);
  // done
  rTw.WriteEnd(label);
}


// report version
std::string CodeGenerator::VersionString(void) {
  std::stringstream res;
  res << COMPILEDES_VERSION << " (using " << faudes::VersionString() << ")";
  return res.str();
}

/*
******************************************************************
******************************************************************
******************************************************************

CodeGenerator implementation: internal data organisation

******************************************************************
******************************************************************
******************************************************************
*/



// base class: prepare main out stream and compile
void CodeGenerator::Compile(void) { 
  FCG_VERB0("CodeGenerator::Compile(): compile to internal representation");
  // default to vector addresses as state indices
  mUsingVectorAddressStates.assign(Size(),true);
  // virtual compile
  DoCompile(); 
  FCG_VERB0("CodeGenerator::Compile(): done");
}


// base class do compile: organize events and transitionrelations
void CodeGenerator::DoCompile(void) {
  FD_DCG("CodeGenerator::DoCompile()");
  // check that all events have specified attributes
  mUsedEvents.Clear();
  for(size_t git=0; git<Size(); ++git) {
    EventSet::Iterator eit=At(git).AlphabetBegin();
    for(; eit!=At(git).AlphabetEnd(); ++eit) {
      if(!mAlphabet.Exists(*eit)) {
        FCG_ERR("CodeGenerator::Compile(): event [" << EventName(*eit) << "] has unspecified execution semantics")
      }
      mUsedEvents.Insert(*eit);
    }
  }
  if(mUsedEvents!=mAlphabet)
    FCG_VERB0("CodeGenerator::Compile(): event configuration mismatch: #" << (mAlphabet-mUsedEvents).Size() << " unused events");
  // compose event sets by type
  mOutputEvents.Clear();
  EventSet::Iterator eit;
  for(eit=mUsedEvents.Begin();eit!=mUsedEvents.End();++eit)
    if(mAlphabet.Attribute(*eit).Output()) mOutputEvents.Insert(*eit);
  mInputEvents.Clear();
  for(eit=mUsedEvents.Begin();eit!=mUsedEvents.End();++eit) {
    if(mAlphabet.Attribute(*eit).Input()) mInputEvents.Insert(*eit);
    if(mAlphabet.Attribute(*eit).Timer()) mInputEvents.Insert(*eit);
  }
  mInternalEvents.Clear();
  for(eit=mUsedEvents.Begin();eit!=mUsedEvents.End();++eit) {
    if(mAlphabet.Attribute(*eit).Input()) continue;
    if(mAlphabet.Attribute(*eit).Timer()) continue;
    if(mAlphabet.Attribute(*eit).Output()) continue;
    mInternalEvents.Insert(*eit);
  }
  FCG_VERB0("CodeGenerator::Compile(): event statistics: overall used events: #" << mUsedEvents.Size());
  FCG_VERB0("CodeGenerator::Compile(): event statistics: input events incl timer: #" << mInputEvents.Size());
  FCG_VERB0("CodeGenerator::Compile(): event statistics: output events: #" << mOutputEvents.Size());
  FCG_VERB0("CodeGenerator::Compile(): event statistics: internal events excl timer: #" << mInternalEvents.Size());
  // sort events by priority (highest first)
  FD_DCG("CodeGenerator::DoCompile(): sorting events");
  std::multimap<int, Idx> sortev;
  for(eit=mUsedEvents.Begin(); eit!=mUsedEvents.End(); eit++) {
    const AttributeCodeGeneratorEvent& attr = mAlphabet.Attribute(*eit);
    int prio = attr.mPriority;
    if((prio<0) || (prio > 10000)) {
      FCG_ERR("CodeGenerator::Compile(): priority out of range [0, 10000] for event [" << EventName(*eit) << "]");
      prio=0;
    }
    // map timer-events to high priority
    if(attr.Timer() || attr.Input()) prio+=10001;
    // effectively sort to: first inputs/timers then others
    sortev.insert(std::pair<int,Idx>(-prio,*eit));
  }  
  // install bit addresses 
  FD_DCG("CodeGenerator::DoCompile(): set up bitaddress");
  mEventBitAddress.clear();
  mEventFaudesIdx.clear();
  mLastInputEvent=-1;
  mLastOutputEvent=-1;
  std::multimap<int, Idx>::iterator sit;
  int bitaddr=0;
  for(sit=sortev.begin(); sit!=sortev.end(); sit++) {
    if(mAlphabet.Attribute(sit->second).Input()) mLastInputEvent=bitaddr;
    if(mAlphabet.Attribute(sit->second).Timer()) mLastInputEvent=bitaddr;
    if(mAlphabet.Attribute(sit->second).Output()) mLastOutputEvent=bitaddr;
    mEventBitAddress[sit->second]=bitaddr;
    mEventFaudesIdx[bitaddr]=sit->second;
    bitaddr++;
  }
  // report
  if(mVerbLevel>=2) {
    for(int i=0; i<EventBitMaskSize(); ++i)
      FCG_VERB2("CodeGenerator::Compile(): bit-address " << i << " event [" << EventName(mEventFaudesIdx[i]) << "]");
  }
  FCG_VERB1("CodeGenerator::Compile(): bitmask size #" << EventBitMaskSize() << " bits");  
  // represent transitionrelation as integer records
  FD_DCG("CodeGenerator::DoCompie(): set up transition vector");
  mTransitionVector.resize(Size());
  //mStateVector.resize(Size());
  mStateVectorAddress.resize(Size());
  mStateFaudesIndex.resize(Size());
  for(Idx gid=0; gid< Size(); gid++) {
    FD_DCG("CodeGenerator::DoCompile(): set up transition vector " << gid);
    std::vector<int>& transvect = mTransitionVector[gid];
    std::map<Idx,int>& vectoraddr = mStateVectorAddress[gid];
    std::map<int,Idx>& faudesindex = mStateFaudesIndex[gid];
    bool usingvectoraddr = mUsingVectorAddressStates[gid];
    transvect.clear();
    vectoraddr.clear();
    faudesindex.clear();
    const Generator gen = At(gid);
    if(gen.Size()==0) {
      FCG_ERR("CodeGenerator::Complie(): generator must not be empty [" << gen.Name() << "]");
      continue;
    }
    // scan all transitions
    IndexSet::Iterator sit= gen.StatesBegin();
    for(;sit!=gen.StatesEnd();++sit) {
      vectoraddr[*sit]= (int) transvect.size();
      faudesindex[(int) transvect.size()]= *sit;
      TransSet::Iterator tit = gen.TransRelBegin(*sit);
      TransSet::Iterator tit_end = gen.TransRelEnd(*sit);
      for(;tit!=tit_end;++tit) {
        transvect.push_back((int) tit->Ev);
        transvect.push_back((int) tit->X2);
      }
      transvect.push_back(0);
    }
    // convert to target index
    for(size_t vit=0; vit< transvect.size(); ++vit) {
      int ev = transvect[vit];
      if(ev==0) continue;
      transvect[vit]=EventTargetIdx(ev);
      int x2 = transvect[++vit];
      if(usingvectoraddr)
        transvect[vit]=vectoraddr[x2];
    }     
    FCG_VERB0("CodeGenerator::Compile(): [" << gen.Name() << "] raw data for transition array #"
      << (mIntegerSize/8)*transvect.size() << " bytes");
    if(!usingvectoraddr) {
      FCG_VERB0("CodeGenerator::Compile(): [" << gen.Name() << "] raw data for state-address array #"
      << (mIntegerSize/8)*gen.MaxStateIndex() << " bytes");
    } else {
      FCG_VERB0("CodeGenerator::Compile(): [" << gen.Name() << "] using vector addresses as state indices");
    }
  }

  // record line levels and input expressions to monitor
  mLines.clear();
  mFlags.clear();
  for(eit=mAlphabet.Begin(); eit!=mAlphabet.End(); ++eit) {
    if(!mUsedEvents.Exists(*eit)) continue;
    if(!mAlphabet.Attribute(*eit).Input()) continue;
    const std::vector<AttributeCodeGeneratorEvent::InputTrigger>& triggers = mAlphabet.Attribute(*eit).mTriggers;
    for(size_t i=0; i<triggers.size(); i++) {
      if(triggers[i].mExe) {
        std::string flagaddr=triggers[i].mAddress;
        mFlags[flagaddr].mEvents.Insert(*eit);
        mFlags[flagaddr].mAddress=flagaddr;
      }
      if((triggers[i].mNeg) || (triggers[i].mPos)) {
        std::string lineaddr=triggers[i].mAddress;
        if(mLines.find(lineaddr)!=mLines.end()) {
           if(mLines[lineaddr].mStatic != triggers[i].mStatic)
             FCG_ERR("CodeGenerator::Compile(): inconsistent static option on input line [" << lineaddr << "]");
        }
        mLines[lineaddr].mAddress=lineaddr;
        if(triggers[i].mNeg) mLines[lineaddr].mNegEvents.Insert(*eit);
        if(triggers[i].mPos) mLines[lineaddr].mPosEvents.Insert(*eit);
        if(triggers[i].mNeg && triggers[i].mStatic) mLines[lineaddr].mNegStatics.Insert(*eit);
        if(triggers[i].mPos && triggers[i].mStatic) mLines[lineaddr].mPosStatics.Insert(*eit);
        mLines[lineaddr].mStatic=triggers[i].mStatic;
      }
    }
  }
  std::map<std::string, LineAddress>::iterator lit= mLines.begin();
  int linecnt=0;
  for(;lit!=mLines.end();++lit) lit->second.mBitAddress=linecnt++;
  FCG_VERB1("CodeGenerator::Compile(): monitoring #" << mLines.size() << " lines for edge detection");
  // record timer definitions
  EventSet evcnt;
  mTimers.clear();
  mTimerActions.clear();
  for(eit=mAlphabet.Begin(); eit!=mAlphabet.End(); ++eit) {
    if(!mUsedEvents.Exists(*eit)) continue;
    const AttributeCodeGeneratorEvent& attr = mAlphabet.Attribute(*eit);
    if(!attr.Timer()) continue;
    std::string timerev=EventName(*eit);
    const EventSet& starts =  attr.mTimeConstraint.mStartEvents;
    const EventSet& stops =  attr.mTimeConstraint.mStopEvents;
    const EventSet& resets =  attr.mTimeConstraint.mResetEvents;
    mTimers[timerev].mAddress= timerev;
    mTimers[timerev].mElapseEvent= timerev;
    mTimers[timerev].mInitialValue= attr.mTimeConstraint.mInitialValue;
    mTimers[timerev].mStartEvents= starts;
    mTimers[timerev].mStopEvents= stops;
    mTimers[timerev].mResetEvents= resets;
    EventSet::Iterator ait;
    ait = starts.Begin();
    for(;ait!=starts.End();++ait)
     mTimerActions[EventName(*ait)].mTimerStarts.insert(timerev);
    ait = stops.Begin();
    for(;ait!=stops.End();++ait)
     mTimerActions[EventName(*ait)].mTimerStops.insert(timerev);
    ait = resets.Begin();
    for(;ait!=resets.End();++ait)
     mTimerActions[EventName(*ait)].mTimerResets.insert(timerev);
  }
  FCG_VERB1("CodeGenerator::Compile(): operating #" << mTimers.size() << " timers controlled by #" << mTimerActions.size() << " events");
  // record actions
  int evcn=0;
  mActionAddresses.clear();
  for(eit=mAlphabet.Begin(); eit!=mAlphabet.End(); ++eit) {
    if(!mUsedEvents.Exists(*eit)) continue;
    const AttributeCodeGeneratorEvent& attr = mAlphabet.Attribute(*eit);
    if(!attr.Output()) continue;
    evcn++;
    const std::vector<AttributeCodeGeneratorEvent::OutputAction>& actions = mAlphabet.Attribute(*eit).mActions;
    for(size_t i=0; i<actions.size(); i++) {
      std::string actaddr=actions[i].mAddress;
      mActionAddresses[actaddr].mAddress= actions[i].mAddress;
      mActionAddresses[actaddr].mSetClr= actions[i].mSet || actions[i].mClr;
      mActionAddresses[actaddr].mExe= actions[i].mExe;
    }
  }
  FCG_VERB1("CodeGenerator::Compile(): operating #" << mActionAddresses.size() << " action primitives controlled by #" << evcn << " events");
  // bitmaks vectors
  mWordAddressVector.clear();
  mBitMaskVector.clear();
  for(int bitaddr=0; bitaddr<EventBitMaskSize(); ++bitaddr) {
    int baddr = bitaddr % mWordSize;
    int waddr = bitaddr / mWordSize;
    mWordAddressVector.push_back(waddr);
    mBitMaskVector.push_back((1ULL << baddr));
  }
  FD_DCG("CodeGenerator::DoCompile(): ok");
}


// access bit address, ranging from 0
int CodeGenerator::EventBitAddress(Idx idx) {
  std::map<Idx,int>::const_iterator sit;
  sit=mEventBitAddress.find(idx);
  if(sit==mEventBitAddress.end()) return -1;
  return sit->second;
}

// access target index, ranging from 1
int CodeGenerator::EventTargetIdx(Idx idx) {
  return EventBitAddress(idx)+1;
}

// access target index, ranging from 1
int CodeGenerator::EventTargetIdx(const std::string& ev){
  return EventTargetIdx(EventIndex(ev));
}

// access faudes index by target index
Idx CodeGenerator::EventFaudesIdx(int idx) {
  std::map<int,Idx>::const_iterator sit;
  sit=mEventFaudesIdx.find(idx-1);
  if(sit==mEventFaudesIdx.end()) return -1;
  return sit->second;
}

// generate bit mask from event index
std::vector<bool> CodeGenerator::EventBitMask(Idx idx) {
  // prepare result
  std::vector<bool> res;
  size_t vsize= mEventBitAddress.size();
  while(res.size()< vsize) res.push_back(false);
  // get bit address
  int baddr=EventBitAddress(idx);
  if(baddr<0) return res;
  // set the bit
  res[baddr] = true;
  return res;
}

// generate bit mask from event set
std::vector<bool> CodeGenerator::EventBitMask(const EventSet& eset) {
  // prepare result
  std::vector<bool> res;
  size_t vsize= mEventBitAddress.size();
  while(res.size()< vsize) res.push_back(false);
  // merge bit addresses for each event
  EventSet::Iterator eit;
  for(eit=eset.Begin(); eit!=eset.End(); eit++) {
    int baddr=EventBitAddress(*eit);
    if(baddr<0) continue;
    res[baddr] = true;
  }
  return res;
}

// access bit-address map
int CodeGenerator::EventBitMaskSize(void) {
  return (int) mEventBitAddress.size();
}

// access word in bit vector
CodeGenerator::word_t CodeGenerator::WordFromBitVector(const std::vector<bool>& vect, int wordindex) {
  word_t res=0;
  size_t bidx= wordindex*mWordSize;
  int shift=0;
  for(;(shift<mWordSize) && (bidx<vect.size());  shift++,bidx++)
    if(vect[bidx]) res |= (1UL<<shift);
  return res;
}

// conver bit vector to word vector
std::vector< CodeGenerator::word_t > CodeGenerator::WordVectorFromBitVector(const std::vector<bool>& vect) {
  std::vector< word_t > res;
  res.resize((vect.size()+mWordSize - 1) / mWordSize, 0);
  for(size_t vidx=0; vidx < vect.size(); ++vidx) {
    if(!vect[vidx]) continue;
    res[vidx/mWordSize] |= (1UL << (vidx % mWordSize) );
  }
  return res;
}


// access transition vector
const std::vector<int>& CodeGenerator::TransitionVector(size_t git) {
  return mTransitionVector[git];
}

// default target state index refers to vector representation
int CodeGenerator::StateTargetIdx(size_t git, Idx idx) {
  if(!mUsingVectorAddressStates[git]) return idx;
  std::map<Idx,int>::const_iterator sit;
  sit=mStateVectorAddress[git].find(idx);
  if(sit==mStateVectorAddress[git].end()) return -1;
  return sit->second;
}

// default faudes state index refers to vector representation
Idx CodeGenerator::StateFaudesIdx(size_t git, int idx) {
  if(!mUsingVectorAddressStates[git]) return idx;
  std::map<int,Idx>::const_iterator sit;
  sit=mStateFaudesIndex[git].find(idx);
  if(sit==mStateFaudesIndex[git].end()) return -1;
  return sit->second;
}

// access compiled data
CodeGenerator::LineIterator CodeGenerator::LinesBegin(void) {
  return mLines.begin();
}

// access compiled data
CodeGenerator::LineIterator CodeGenerator::LinesEnd(void) {
  return mLines.end();
}

// access compiled data
CodeGenerator::FlagIterator CodeGenerator::FlagsBegin(void) {
  return mFlags.begin();
}

// access compiled data
CodeGenerator::FlagIterator CodeGenerator::FlagsEnd(void) {
  return mFlags.end();
}

// access compiled data
CodeGenerator::TimerIterator CodeGenerator::TimersBegin(void) {
  return mTimers.begin();
}

// access compiled data
CodeGenerator::TimerIterator CodeGenerator::TimersEnd(void) {
  return mTimers.end();
}

// access compiled data
CodeGenerator::ActionAddressIterator CodeGenerator::ActionAddressesBegin(void) {
  return mActionAddresses.begin();
}

// access compiled data
CodeGenerator::ActionAddressIterator CodeGenerator::ActionAddressesEnd(void) {
  return mActionAddresses.end();
}

// access compiled data
CodeGenerator::TimerActionIterator CodeGenerator::TimerActionsBegin(void) {
  return mTimerActions.begin();
}

// access compiled data
CodeGenerator::TimerActionIterator CodeGenerator::TimerActionsEnd(void) {
  return mTimerActions.end();
}


// control error out
void CodeGenerator::Verbose(int level, std::ostream* altout) {
  mVerbLevel=level;
  if(altout!=0) pErrStream=altout;
}

/*
******************************************************************
******************************************************************
******************************************************************

CodeGenerator implementation: generate / outstream

******************************************************************
******************************************************************
******************************************************************
*/


// base class: prepare main out stream and call virtual hook
// to let derived classes generate code
void CodeGenerator::Generate(void) {
  // report on output
  FCG_VERB0("CodeGenerator::Generate(): generating code to \"[" << mOutMode <<"]\"");
  // have stream
  bool tmpstream=false;
  if(pOutStream==0) {
    tmpstream=true;
    if(pOutBuffer!=0) delete pOutBuffer;
    pOutBuffer = new cgp_streambuf(mOutMode);
    pOutStream = new std::ostream(pOutBuffer);
  }
  // set default output mode to "no mute"
  mMuteMode='*';
  // generate code
  DoGenerate();
  // report
  FCG_VERB0("CodeGenerator::Generate(): generated " << LineCount() << " lines of code.");
  // delete/flush managed stream
  if(tmpstream) {
    delete pOutStream;
    pOutStream=0;
  }
}


// generate output
std::ostream& CodeGenerator::Output(void) {
  return *pOutStream;
}

const std::string& CodeGenerator::OutputString(void) {
  static std::string ems("");
  if(!pOutBuffer) return ems;
  return pOutBuffer->Buffer();
}

void CodeGenerator::OutputString(const std::string& strbuf){
  if(!pOutBuffer) return;
  pOutBuffer->Clear();
  if(!strbuf.empty()) FCG_VERB1("CodeGenerator::OutputString(): setting non-trivial not implemented")
}

// generate output
void CodeGenerator::MuteMode(char mode) {
  mMuteMode=mode;
  if(!pOutBuffer) return;
  pOutBuffer->Mute(mode!='*');
};


// generate output
void CodeGenerator::MuteCond(char mode) {
  if(!pOutBuffer) return;
  bool mute = (mode!=mMuteMode) && (mode!='*') && (mMuteMode!='*');
  pOutBuffer->Mute(mute);
  if((!mMuteComments) && (!mute) && (mRecentMutedComment!="")) {
    this->Comment(mRecentMutedComment);
    mRecentMutedComment="";
  }
}

// generate output
void CodeGenerator::LineFeed(int lines) {
  if(!Output()) return;
  while(lines>0) {
     Output() << std::endl;
     --lines;
  }
} 

// generate output
void CodeGenerator::IndentInc(void) {
  if(!pOutBuffer) return;
  pOutBuffer->IndentInc();
}

// generate output
void CodeGenerator::IndentDec(void) {
  if(!pOutBuffer) return;
  pOutBuffer->IndentDec();
}

// generate output
void CodeGenerator::XmlTextEscape(bool on) {
  if(!pOutBuffer) return;
  pOutBuffer->XmlTextEscape(on);
}

// generate output
void CodeGenerator::XmlCdataEscape(bool on) {
  if(!pOutBuffer) return;
  pOutBuffer->XmlCdataEscape(on);
}

// comment (need to reimplement and pass on)
void CodeGenerator::Comment(const std::string& text) {
  if(mMuteComments)
    mRecentMutedComment=text;
  if(pOutBuffer)
    if(pOutBuffer->Mute())
      mRecentMutedComment=text;
}

// generate output
void CodeGenerator::MuteComments(bool on) {
  mMuteComments=on;
}

// generate output
void CodeGenerator::MuteVspace(bool on) {
  if(!pOutBuffer) return;
  pOutBuffer->MuteVspace(on);
}


// generate output
std::string CodeGenerator::LineCount() {
  if(!pOutBuffer) return std::string();
  std::stringstream res;
  res << "#" << pOutBuffer->LineCount();
  return std::string(res.str());
}


// implementation of output buffer
CodeGenerator::cgp_streambuf::cgp_streambuf(std::string mode) :
  std::streambuf(),
  mpFOutStream(0),
  mLines(0),
  mBeginOfLine(true),
  mIndent(0),
  mMute(false),
  mMuteVspace(false),
  mTextEscape(false),
  mCdataEscape(false)
{
  // case a) using string buffer only
  if(mode == "std::string") {
    mMode = "std::string";
    pOutStream=0;
  }
  // case b) using cout
  else if(mode=="std::cout") {
    mMode="std::cout";
    pOutStream=&std::cout;
  } else
  // case c) flush to specified file
  {
    mMode=mode;
    mpFOutStream = new std::ofstream();
    mpFOutStream->exceptions(std::ios::badbit|std::ios::failbit);
    try{
      mpFOutStream->open(mode.c_str(), std::ios::out|std::ios::trunc);
    }
    catch (std::ios::failure&) {
      std::stringstream errstr;
      errstr << "Exception opening/writing file \""<< mode << "\"";
      throw Exception("CodeGenerator::Generate()", errstr.str(), 1);
    }
    pOutStream = mpFOutStream;
  }
};

// implementation of output buffer
CodeGenerator::cgp_streambuf::~cgp_streambuf() {
  Flush();
  if(mpFOutStream) {
    mpFOutStream->close();
    delete mpFOutStream;
  }
}

// implementation of output buffer (hook on virtual function)
std::streambuf::int_type CodeGenerator::cgp_streambuf::overflow (int_type c) {
  //FD_WARN("cgp_steambuf::overflow(): " << c);
  // pass on EOF
  if(c == EOF) return EOF;
  // ignore on mute
  if(mMute) return c;
  // ignore vspace
  if(mMuteVspace && (c=='\n') && (mBuffer.size()==0)) return c;
  // sense eol
  if(c=='\n') {
    mBuffer.push_back(c);
    Flush();
    mLines++;
    mBeginOfLine=true;
    return c;
  }
  // prepend indent
  if(mBeginOfLine) {
    int scnt=mIndent;
    for(;scnt>0;--scnt) mBuffer.push_back(' ');
    mBeginOfLine=false;
  }
  // escape xml (substitute indicators by enteties)
  if(mTextEscape) {
    if(c=='<')
      { mBuffer += "&lt;"; return c;}
    if(c=='>')
      { mBuffer += "&gt;"; return c;}
    if(c=='&')
      { mBuffer +=  "&amp;"; return c;}
    if(c=='\'')
      { mBuffer +=  "&apos;"; return c;}
    if(c=='"')
      { mBuffer +=  "&quot;"; return c;}
  }
  // escape xml (insert "]]><![CDATA[" betwwen "]]" and ">")
  if(mCdataEscape) {
    if((c=='>') && (mBuffer.size()>=2))
      if(mBuffer.at(mBuffer.size()-1) == ']')
        if(mBuffer.at(mBuffer.size()-2) == ']')
          mBuffer += "]]><![CDATA[";
  }
  // plain append
  mBuffer.push_back(c);
  return c;
}


// implementation of output buffer
void CodeGenerator::cgp_streambuf::Flush() {
  // keep in buffer on std::string-mode
  if(!pOutStream) return;
  // try to flush
  try {
    (*pOutStream) << mBuffer;
    (*pOutStream) << std::flush;
  } catch(std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception writing to file \""<< mMode << "\"";
    throw Exception("CodeGenerator::Generate()", errstr.str(), 1);
  }
  // clear buffer
  mBuffer.clear();
}

// implement output buffer
const std::string& CodeGenerator::cgp_streambuf::Buffer(){
  return mBuffer;
}

// implement output buffer
void CodeGenerator::cgp_streambuf::Clear(void){
  Flush();
  mLines=0;
  mBeginOfLine=true;
  mBuffer.clear();
}

 // implementation of output buffer
int CodeGenerator::cgp_streambuf::LineCount() {
  Flush();
  return mLines;
};


// implementation of output buffer
void CodeGenerator::cgp_streambuf::IndentInc() {
  mIndent+=2;
};

// implementation of output buffer
void CodeGenerator::cgp_streambuf::IndentDec() {
  mIndent-=2;
};

// implementation of output buffer
void CodeGenerator::cgp_streambuf::XmlTextEscape(bool on) {
  Flush();
  mCdataEscape=false;
  mTextEscape=on;
};

// implementation of output buffer
void CodeGenerator::cgp_streambuf::XmlCdataEscape(bool on) {
  Flush();
  mTextEscape=false;
  mCdataEscape=on;
};

// implementation of output buffer
void CodeGenerator::cgp_streambuf::Mute(bool on) {
  Flush();
  mMute=on;
};

// implementation of output buffer
bool CodeGenerator::cgp_streambuf::Mute() {
  return mMute;
};

// implementation of output buffer
void CodeGenerator::cgp_streambuf::MuteVspace(bool on) {
  Flush();
  mMuteVspace=on;
};

