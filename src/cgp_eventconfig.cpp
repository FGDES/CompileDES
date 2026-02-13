/** @file cgp_eventconfig.cpp @brief Event attributes (execution semantics)  */

/*
  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2010-2025 Thomas Moor

*/

// my includes
#include "cgp_eventconfig.h"


/*
******************************************************************
******************************************************************
******************************************************************

AttributeCodeGeneratorEvent implementation

******************************************************************
******************************************************************
******************************************************************
*/

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(AttributeCodeGeneratorEvent,AttributeCodeGeneratorEvent,AttributeVoid)


// reset to default
void AttributeCodeGeneratorEvent::Clear(void) {
  mType = ETInternal;
  mPriority=0;
  mTriggers.clear(); 
  mActions.clear(); 
  mTimeConstraint.mInitialValue="";
  mTimeConstraint.mStartEvents.Clear();
  mTimeConstraint.mStopEvents.Clear();
  mTimeConstraint.mResetEvents.Clear();
};


//DoAssign(Src)
AttributeCodeGeneratorEvent& AttributeCodeGeneratorEvent::DoAssign(const AttributeCodeGeneratorEvent& rSrc) {
  FD_DCG("AttributeCodeGeneratorEvent(" << this << "):DoAssign(): assignment from " <<  &rSrc);
  mType=rSrc.mType;
  mPriority=rSrc.mPriority;
  mTriggers=rSrc.mTriggers;
  mActions=rSrc.mActions;
  mTimeConstraint=rSrc.mTimeConstraint;
  return *this;
}

//DoWrite(rTw);
void AttributeCodeGeneratorEvent::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
  FD_DCG("AttributeCodeGeneratorEvent::DoWrite()");
  std::vector<InputTrigger>::const_iterator tit;
  std::vector<OutputAction>::const_iterator ait;
  Token token;
  switch(mType) {
  // its an input event
  case ETInput:
    rTw.WriteBegin("Input");
    token.SetEmpty("Priority");
    token.InsAttributeFloat("val",mPriority);
    rTw.Write(token);
    rTw.WriteBegin("Triggers");
    for(tit=mTriggers.begin(); tit!=mTriggers.end(); tit++) {
      rTw.WriteString(tit->mAddress);
      if(tit->mPos &&  (!tit->mNeg)) 
        rTw.WriteOption("PosEdge");
      if((!tit->mPos) &&  tit->mNeg) 
        rTw.WriteOption("NegEdge");
      if((tit->mPos) &&  tit->mNeg) 
        rTw.WriteOption("AnyEdge");
      if(tit->mStatic)
        rTw.WriteOption("Static");
      if((tit->mExe))
        rTw.WriteOption("Value");
    }
    rTw.WriteEnd("Triggers");
    rTw.WriteEnd("Input");
    break;
  // its an output event
  case ETOutput:
    rTw.WriteBegin("Output");
    token.SetEmpty("Priority");
    token.InsAttributeFloat("val",mPriority);
    rTw.Write(token);
    rTw.WriteBegin("Actions");
    for(ait=mActions.begin(); ait!=mActions.end(); ait++) {
      rTw.WriteString(ait->mAddress);
      if(ait->mSet) rTw.WriteOption("Set");
      if(ait->mClr) rTw.WriteOption("Clr");
      if(ait->mExe) rTw.WriteOption("Exe");
    }
    rTw.WriteEnd("Actions");
    rTw.WriteEnd("Output"); 
    break;
  // its an internal event
  case ETInternal:
    rTw.WriteBegin("Internal");
    token.SetEmpty("Priority");
    token.InsAttributeFloat("val",mPriority);
    rTw.Write(token);
    if(mTimeConstraint.mInitialValue!="") {
      token.SetBegin("Timer");
      token.InsAttributeString("val",mTimeConstraint.mInitialValue);
      rTw.Write(token);
      mTimeConstraint.mStartEvents.Write(rTw,"StartEvents");
      mTimeConstraint.mStopEvents.Write(rTw,"StopEvents");
      mTimeConstraint.mResetEvents.Write(rTw,"ResetEvents");
      rTw.WriteEnd("Timer");
    }
    rTw.WriteEnd("Internal");
    break;
    // unimplemented others
  default:
    break;
  }
}


//DoRead(rTr)
void AttributeCodeGeneratorEvent::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; (void) pContext;
  std::string label;
  bool err=false;
  FD_DCG("AttributeCodeGeneratorEvent::DoRead()");
  // clear
  Clear();
  // test for type
  Token token;
  rTr.Peek(token);
  // output section
  if(token.Type()==Token::Begin) 
  if(token.StringValue()=="Output") {
    mType=ETOutput;
    label="Output";
  }
  // input section
  if(token.Type()==Token::Begin) 
    if(token.StringValue()=="Input") {
    mType=ETInput;
    label="Input";
  }
  // internal section
  if(token.Type()==Token::Begin) 
  if(token.StringValue()=="Internal") {
    mType=ETInternal;
    label="Internal";
  }
  // none of the above
  if(label=="") return;
  // read my section
  while(!rTr.Eos(label)) {
    rTr.Peek(token);
    // priority
    if(token.Type()==Token::Begin) 
    if(token.StringValue()=="Priority") {
      rTr.ReadBegin("Priority");
      mPriority=(int) token.AttributeFloatValue("val");
      rTr.ReadEnd("Priority");
      continue;
    }     
    // output
    if(mType==ETOutput)
    if(token.Type()==Token::Begin) 
    if(token.StringValue()=="Actions") {
      rTr.ReadBegin("Actions");
      while(!rTr.Eos("Actions")) {
        // action
        OutputAction action;
        action.mClr=false;
        action.mSet=false;
        action.mExe=false;
        // 1: address
        action.mAddress=rTr.ReadString();
        // 2: value
        std::string value=rTr.ReadOption();
        if(value== "Set") {
          action.mSet=true;
        } else if(value == "Clr") {
          action.mClr=true;
        } else if(value == "Execute") {
          action.mExe=true;
        } else {
          err=true;
          break;
        }
        // 3. record
        mActions.push_back(action);
      }  
      // done
      rTr.ReadEnd("Actions");
      continue;
    }
    // input
    if(mType==ETInput)
    if(token.Type()==Token::Begin) 
    if(token.StringValue()=="Triggers") {
      rTr.ReadBegin("Triggers");
      while(!rTr.Eos("Triggers")) {
        // action
        InputTrigger trigger;
        trigger.mPos=false;
        trigger.mNeg=false;
        trigger.mExe=false;
	trigger.mStatic=false;
        // 1: address
        trigger.mAddress=rTr.ReadString();
        // 2: value
        std::string value=rTr.ReadOption();
        if (value == "PosEdge") {
          trigger.mPos=true;
        } else if (value == "NegEdge") {
          trigger.mNeg=true;
        } else if (value == "AnyEdge") {
          trigger.mPos=true;
          trigger.mNeg=true;
        } else if (value == "Value") {
          trigger.mExe=true;
        } else {
          err=true;
          break;
        }
        // 3. static option
        if(trigger.mPos || trigger.mNeg) {
           rTr.Peek(token);
           if(token.IsOption())
           if(token.StringValue()=="Static") {
             rTr.Get(token);
             trigger.mStatic=true;
           }
        }
        // 4. record
        mTriggers.push_back(trigger);
      }  
      // done
      rTr.ReadEnd("Triggers");
      continue;
    }
    // internal
    if(mType==ETInternal)
    if(token.Type()==Token::Begin) 
    if(token.StringValue()=="Timer") {
      rTr.ReadBegin("Timer");
      // 1: initial value
      mTimeConstraint.mInitialValue = token.AttributeStringValue("val");
      // 2,3,4: loop for event sets
      while(!rTr.Eos("Timer")) {
        rTr.Peek(token);
        if(token.Type()==Token::Begin) 
        if(token.StringValue()=="StartEvents") {
          mTimeConstraint.mStartEvents.Read(rTr,"StartEvents");
          continue;
	}
        if(token.Type()==Token::Begin) 
        if(token.StringValue()=="StopEvents") {
          mTimeConstraint.mStopEvents.Read(rTr,"StopEvents");
          continue;
	}
        if(token.Type()==Token::Begin) 
        if(token.StringValue()=="ResetEvents") {
          mTimeConstraint.mResetEvents.Read(rTr,"ResetEvents");
          continue;
	}
        // ignore unknown tokens (should throw)
        rTr.Get(token);
      }
      // done
      rTr.ReadEnd("Timer");
      continue;
    }
    // ignore unknown sections
    rTr.Get(token);
  }
  rTr.ReadEnd(label);
  // report error
  if(err) {
    std::stringstream errstr;
    errstr << "invalid code generator event property" << rTr.FileLine();
    throw Exception("AttributeCodeGeneratorEvent::Read", errstr.str(), 52);  //52 oder 352
  }
}//DoRead

// register type for XML token-io
AutoRegisterType<cgEventSet> gRtiRegisterCodeGeneratorAlphabet("CodeGeneratorAlphabet");
AutoRegisterElementTag<cgEventSet> gRtiRegisterCodeGeneratorAlphabetElementTagAlphabet("CodeGeneratorAlphabet", "Event");


