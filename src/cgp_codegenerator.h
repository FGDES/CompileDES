/** @file cgp_codegenerator.h @brief Code-generator common base class */

/*

  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2010, 2016-2019 Thomas Moor

*/



#ifndef FAUDES_CODEGENERATOR_H
#define FAUDES_CODEGENERATOR_H

#include "libfaudes.h"
#include "cgp_eventconfig.h"


using namespace faudes;

// diagnostic macros for CodeGenerator and derived classes
#define FCG_VERB0(msg) { if(mVerbLevel>=0)  *pErrStream << msg << std::endl; }
#define FCG_VERB1(msg) { if(mVerbLevel>=1)  *pErrStream << msg << std::endl; }
#define FCG_VERB2(msg) { if(mVerbLevel>=2)  *pErrStream << msg << std::endl; }
#define FCG_ERR(msg)   { *pErrStream << msg << std::endl; throw Exception("CodeGenerator()","internal error", 500); }

// set version string (overwrite by build system)
#ifndef COMPILEDES_VERSION
#define COMPILEDES_VERSION "3.xx"
#endif


/**
 * @brief Code-generation common base
 *
 * The code-generator base class maintains the configuration data common to libFAUDES
 * code generators. To this end, the configuration consists of
 * - a list of generators
 * - event attributes to specify execution semantics per event
 *
 * The CodeGenerator implements file i/o of its configuration via the FAUDES
 * type interface Read()/Write() and provides the virtual member functions
 * DoReadTargetConfiguration()/DoWriteTargetConfiguration() for
 * derived classes to add additional configuration features.
 *
 * There is also a programatic interface to Insert(Generator&) generators and to access individual
 * generators with At(). Likewise, there are setters and getters to access event attributes;
 * see also AttributeCodeGeneratorEvent.
 *
 * The CodeGenerator provides the method Compile() to convert the configuration data
 * to an alternative representation tailored for code generation. E.g., it extracts a
 * map of line levels to be monitored for edge detection by collecting all lines used
 * by any event. The base class currently implements the below data structures, which
 * can be extended for derived classes my the virtual member-function interface DoCompile().
 * - map configuration events to consecutive target event indices ordered by execution
 *   priority
 * - translate sets of configuration events to boolean vectors or bit-mask word-arrays.
 * - map configuration state indices to consecutive target state indices
 * - collection of all relevant input line addresses with associated trigger configuration
 * - collection of all relevant flags used in exec-triggers
 * - a collection of all addresses relevant for output actions
 *
 * Code generation is triggered by Generate() with the virtual interface DoGenerate().
 * In this base class, DoGenerate() does nothing, i.e., to obtain a functional code generator, 
 * re-implement the method DoGenerate() in a derived class. See CodePrimitives() for the
 * next level of specialisation.
 *
 * Technical Detail: we use faudes Type as base for serialisation only.
 * Assignment and copy constructor are not implemented.
 *
 * File format by example. See also AttributeCodeGeneratorEvent for the configuration
 * of individual events.
 *
 @verbatim
  <CodeGenerator name="blink">

  % 1. specify generators (either explicitly or by filename)
  <Generators>
  <Generator name="blink1">
  [.. libFAUDES generator format ..]
  </Generator>
  [.. more generators ..]
  </Generators>

  % 2. specify event configuration
  <EventConfiguration>
  <Event name="led_on">
  [.. event attribute in AttributeCodeGeneratorEvent format ..]
  </Event>
  [.. more events ..]
  </EventConfiguration>

  % 3. target specific configuration
  <TargetConfiguration>
  [.. as provided by derived classes ..]
  </TargetConfiguration>

  </CodeGenerator>
 @endverbatim
 *
 * @ingroup CGClasses
 *
 */

class CodeGenerator : public Type {


public:

  /*****************************************
   *****************************************
   *****************************************
   *****************************************/

  enum OutSink { CONSOLE, FILE, STRING };


  /**
   * @name Basic Class Maintenance
   *
   * Basic class maintenance methods to construct/destruct CodeGenerator objects and
   * compile the configuration to an alternative representation. Note that the interface
   * for file i/o of configuration data is inherited from faudes::Type.
   *
   */

  /** @{ */

  /*!
   * Constructor
   */
  CodeGenerator(void);

  /*!
   * Destructor.
   */
  virtual ~CodeGenerator(void);

  /*!
   * Clear all data.
   *
   * Derived classes must implement this to reset ALL configuration
   * data to consistent defaults.
   *
   */
  virtual void Clear(void);


  /*!
   * Set objects's name (reimplementing base faudes::Type)
   *
   * @param rName
   *   Name
   */
  virtual void Name(const std::string& rName) { mName=rName;};

  /*!
   * Get objects's name (reimplementing base faudes::Type)
   *
   * @return
   *   Name of generator
   */
  virtual const std::string& Name(void) const { return mName;};

  /*!
   * Compile input data for alternative representation.
   *
   * The designated hook for reimplementation is the also virtual "DoCompile()". Do not
   * reimplement Compile() itself, since is resets internal data structures.
   *
   */
  virtual void Compile(void);

  /*!
   * Version (refers to macro COMPILEDES_VERSION, defined in cgp_codegenerator.h)
   *
   */
  static std::string VersionString(void);


  /** @} */



  /*****************************************
   *****************************************
   *****************************************
   *****************************************/

  /** @name Configuration -- Generators
   *
   * Generators are internally organised as a vector. There is a programatic
   * read-only interface with Size() and At() as well as Begin() and End() iterators.
   * Insert methods amount to a "push_back". Individual removal of generators
   * is currently not supported, use Clear() for a consistent reset.
   *
   */

  /** @{ */

  /*!
   * Number of generators
   *
   */
  Idx Size(void) const;
        
  /*!
   * Add a Generator from file.
   *
   * This method uses the Generator's read to find the first generator in
   * the specified file.
   *
   * @param file
   *   File to read
   *
   * @exception Exception
   *   - non-deterministic generator (id 501)
   *   - token mismatch (id 502)
   *   - IO errors (id 1)
   */
  void Insert(const std::string& file);

  /*!
   * Add a generator by reference.
   * A copy of the references generator will be inserted to the internal data-structures.
   *
   * @param rGen
   *   Generator to add
   *
   * @exception Exception
   *   - non-deterministic generator (id 501)
   */
  void Insert(const TimedGenerator& rGen);

  /*! Direct access for read-only access of generators */
  const TimedGenerator& At(int i)  const { return mGenerators.at(i); };

  /*! Iterator for read-only access of generators */
  typedef std::vector<TimedGenerator>::const_iterator Iterator;

  /*! Begin-iterator for read-only access of generators */
  Iterator Begin(void) const { return mGenerators.begin(); };

  /*! End-iterator for read-only access of generators */
  Iterator End(void) const   { return mGenerators.end(); };


  /** @} */

  /*****************************************
   *****************************************
   *****************************************
   *****************************************/

  /**
   * @name Configuration -- Alphabet
   *
   * The overall alphabet amounts to the union of all provided generator alphabets, augmented
   * by the event attributes for execution semantics; see cgEventSet for details. The external interface
   * provides symbolic name and index look-up, both referring to the faudes-global symbol table and faudes event
   * indices. There are also setters/getters to edit individual attributes.
   * attribute type.
   *
   */
  /** @{ */

  /*!
   * Faudes-event index lookup.
   *
   * 
   * @param rName
   *    Name of event to lookup
   *
   * @return 
   *   Valid index or 0 if non-existent
   */
  Idx EventIndex(const std::string& rName) const { return mAlphabet.Index(rName); };

  /*!
   * Faudes-event name lookup
   *
   * @param index
   *   Index of event to look up
   *
   * @return 
   *   Name or empty std::string if non-existent
   */
  std::string EventName(Idx index) const { return mAlphabet.SymbolicName(index); };
   

  /*!
   * Event configuration attribute lookup
   *
   * @param ev
   *
   * @return 
   *   Reference to respective attribute
   */
  const AttributeCodeGeneratorEvent& EventAttribute(Idx ev) const;

  /*!
   * Set event attribute.
   *
   * @param ev
   *   Event index
   * @param attr
   *   New attribute
   * @exception Exception
   *   Index not found in EventSymbolMap (id 42)
   *
   */
  void EventAttribute(Idx ev, const AttributeCodeGeneratorEvent& attr);

  /*!
   * Set all event attributes. 
   * Any previous attributes are removed.
   * Any events not in rAlphabet will have the default attribute attached (which is priority 0)
   *
   *	@param rAlphabet 
   *	    EventSet with AttributeCodeGeneratorEvent data
   */
  void Alphabet(const cgEventSet& rAlphabet);

  /*!
   * Access alphabet (incl event attributes)
   *
   * @return
   *    Overall alphabet
   */
  const cgEventSet& Alphabet(void) const { return mAlphabet; };

  /** @} */


  /*****************************************
   *****************************************
   *****************************************
   *****************************************/

  /**
   * @name Compiled Data -- Eventsets
   *
   * Faudes-EventSets are alternatively represented as boolean vectors.
   * Each individual event has a bit-address to specify the location of the
   * corresponding boolean value in the vector. By convention, bit-addresses start with 0.
   * To facilitate event scheduling, events are ordered by descending priority.
   * Implicitly, this also associates with each event a consecutive target event index, starting with 1.
   *
   * Example for an overall amount of 9 events and the faudes and the event set {3,6,9}
   *
   *
   @verbatim
    faudes Idx:    1 4 5 6 3 2 8 9 7   (typically this is consecutive but not ordered by priority)
    target Idx:    1 2 3 4 5 6 7 8 9   (consecutive and ordered by priority)
    bit-address:   0 1 2 3 4 5 6 7 8   (corresponding bit address)
    vector value:  0 0 0 1 1 0 0 1 0   (vector for the set {3,6,9})
   @endverbatim
   *
   * Regarding efficiency of the generated code, the target platform typically represent
   * boolean vectors as as word-arrays. Continuing the above example for a word-size of 4bits
   * we obtain the correspondence.
   *
   @verbatim
    bit-address:   0  1  2  3  4  5  6  7  8  -  -  -
    word index:   [  word 0  ][  word 1  ][   word 2  ]   (reading "lsb on left hand side")
   @endverbatim
   * <br>
   */
   /** @{ */

  /*! Get target event Idx from faudes Idx (use bit-address + 1) */
  virtual int EventTargetIdx(Idx idx);
  
  /*! Get target event Idx from faudes name (use bit-address + 1) */
  virtual int EventTargetIdx(const std::string& ev);

  /*! Get event bit-address from faudes Idx (consecutive, starts at 0) */
  int EventBitAddress(Idx idx);

  /*! Get faudes Idx from target Idx (aka from bit-address + 1) */
  Idx EventFaudesIdx(int idx);

  /*! Get vector representation for a single faudes event Idx */
  std::vector<bool> EventBitMask(Idx idx);

  /*! Get vector representation for faudes event set */
  std::vector<bool> EventBitMask(const EventSet& eset);

  /*! Get overall number of events */
  int EventBitMaskSize(void);

  /*! Code-generator internal data type of target words */
  typedef unsigned long word_t;

  /*! Extract individual word from boolean vector */
  word_t WordFromBitVector(const std::vector<bool>& vect, int wordindex);

  /*! Convert boolean vector to word array */
  std::vector< word_t > WordVectorFromBitVector(const std::vector<bool>& vect);

  /*! Look-up table to map a bit-address to the word-index */
  std::vector<int> mWordAddressVector;

  /*! Look-up table to map a bit-address to the word-bitmask */
  std::vector<word_t> mBitMaskVector;

  /** @} */


  /*****************************************
   *****************************************
   *****************************************
   *****************************************/

  /**
   *  @name Compiled Data -- Transition Relation
   *
   *  As an optional form of representation, transition relations are compiled to
   *  integer vectors. Each state corresponds to a particular segment of the vector
   *  to hold pairs or events and successor states. The end of each Segment is flagged by
   *  the invalid event index zero. 
   *
   *  Example for {q1-(1)->q1; q1-(3)->q2; q2-(1)->q3; q3-(4)->q3; q3-(2)->q1; q3-(1)->q2};
   *
   @verbatim
   Vector index:      0  1  2  3  4  5  6  7  8  9  10 11 12 13 14
   Current state:     q1             q2       q3
   Successor/Event:   ev q1 ev q2 || ev q3 || ev q3 ev q1 ev q2 ||
   Vector value:      1  0  3  5  0  1  8  0  4  8  2  0  1  5  0
   @endverbatim
   *
   *  Note that the event indices are target indices (priority sorted, starting from 1, 
   *  consecutive). When this scheme is used, it implicitly defines target state indices
   *  to be transition-vector indices (starting by 0, however non-consecutive).
   *
   *  As a variation, one may maintaine the original state indices by using an indirect andress scheme.
   *  This option is useful when the original state indices are relevant fo state-update hooks an
   *  symbolic state names. Example:
   *
   @verbatim
   Vector index:      0  1  2  3  4  5  6  7  8  9  10 11 12 13 14
   Current state:     q1             q2       q3
   Successor/Event:   ev q1 ev q2 || ev q3 || ev q3 ev q1 ev q2 ||
   Transition vector: 1  1  3  2  0  1  3  0  4  3  2  1  1  2  0
   Address vector:    0  0  5  8
   @endverbatim
   */

  /** @{ */
  const std::vector<int>& TransitionVector(size_t git);

  /*! Get target state index (refer to vector representation as default, overload in CodePrimitives) */
  virtual int StateTargetIdx(size_t git, Idx idx);

  /*! Get faudes state index (refer to vector representation as default, overload in CodePrimitives) */
  virtual Idx StateFaudesIdx(size_t git, int idx);

  /** @} */

  /**
   * @name Compiled Data -- Generation of Events by Input Lines, Flags and Timers
   *
   * For each line-level that is relevant for edge detection, a record is kept that
   * accumulates all events that an edge of the respective line must generate. The data
   * set is formally implemented as map with the line address as the key. Similarly, a
   * record for event triggering flag-expressions and timers is kept. For the actual data structures,
   * see LineAddress, FlagExpression and TimerConfiguration, respectively.
   */  

  /** @brief Compiled record per input line */
  struct LineAddress {
    std::string mAddress;    /*! target expression to represent line value */
    int mBitAddress;         /*! bit-address of buffer for edge detection (consecutive) */
    EventSet mPosEvents;     /*! events to trigger on positive edge */
    EventSet mNegEvents;     /*! events to trigger on negative edge */
    EventSet mPosStatics;    /*! events to trigger on high power on */
    EventSet mNegStatics;    /*! events to trigger on low power on */
    bool mStatic;            /*! true <> static triggers exists */
  };
  /*! Access to line records by iterator */
  typedef std::map<std::string, LineAddress>::iterator LineIterator ;
  /*! Access to line records by iterator */
  LineIterator LinesBegin();
  /*! Access to line records by iterator */
  LineIterator LinesEnd();

  /*! @brief Compiled record per input flag-expression */
  struct FlagExpression {
    /*! target expression to represent the line value */
    std::string mAddress;
    /*! events to trigger on positive evaluation */
    EventSet mEvents;
  };
  // access to flag data
  /*! Access to flag records by iterator */
  typedef std::map<std::string, FlagExpression>::iterator FlagIterator ;
  /*! Access to flag records by iterator */
  FlagIterator FlagsBegin();
  /*! Access to flag records by iterator */
  FlagIterator FlagsEnd();

  /*! @brief Compiled record per timer acting */
  struct TimerConfiguration {
    /*! target address prefix to maintain timer state */
    std::string mAddress;
    /*! event to trigger on timer elapse  */
    std::string mElapseEvent;
    /*! target representation of initial value */
    std::string mInitialValue;
    /*! start events as in AttributeCodeGeneratorEvent */
    EventSet mStartEvents;
     /*! stop events as in AttributeCodeGeneratorEvent */
    EventSet mStopEvents;
    /*! reset events as in AttributeCodeGeneratorEvent */
    EventSet mResetEvents;
  };
  /*! Access to timer records by iterator */
  typedef std::map<std::string, TimerConfiguration>::iterator TimerIterator ;
  /*! Access to timer records by iterator */
  TimerIterator TimersBegin();
  /*! Access to timer records by iterator */
  TimerIterator TimersEnd();

  /** @} */

  /**
   * @name Compiled Data -- Effects of Events to Output Lines, Flags and Timers
   *
   * For each abstract target address listed in an action we maintain a record whether it is
   * set or cleared (i.e. interpreted as an address of a boolean type) or executed (i.e. whether is
   * is an an address of a function or an assignment expression, depending on what the target supports).
   * Similarly, we record on a per event basis, which timers are to be started/stoped/reset.
   */
  /** @{ */

  /** @brief Compiled record per action-address */
  struct ActionAddress {
    /*! target address as specified in configuration */
    std::string mAddress;
    /*! is a set/clr type of address */
    bool mSetClr;
    /*! is an executable type of address */
    bool mExe;
  };
  /*! Access to action record by iterator */
  typedef std::map<std::string, ActionAddress>::iterator ActionAddressIterator ;
  /*! Access to action addresses by iterator */
  ActionAddressIterator ActionAddressesBegin();
  /*! Access to action addresses by iterator */
  ActionAddressIterator ActionAddressesEnd();

  /*! @brief Compiled record per event on how it affects timers */
  class TimerAction {
  public:
    /*! timers to start  */
    std::set< std::string > mTimerStarts;
    /*! timers to stop  */
    std::set< std::string > mTimerStops;
    /*! timers to reset  */
    std::set< std::string > mTimerResets;
    /*! need equality operator for STL */
    bool operator==(const TimerAction& other) {
      if(!(mTimerStarts==other.mTimerStarts)) return false;
      if(!(mTimerStops==other.mTimerStops)) return false;
      if(!(mTimerResets==other.mTimerResets)) return false;
      return true;
    }
    /*! need compare operator for STL (should perhaps use hashes for performance) */
    bool operator<(const TimerAction& other) const {
      if(mTimerStarts<other.mTimerStarts) return true;
      if(mTimerStarts>other.mTimerStarts) return false;
      if(mTimerResets<other.mTimerResets) return true;
      if(mTimerResets>other.mTimerResets) return false;
      if(mTimerStops<other.mTimerStops) return true;
      return false;
    }
  };
  /*! Access to timer records by iterator */
  typedef std::map<std::string, TimerAction>::iterator TimerActionIterator ;
  /*! Access to timer records by iterator */
  TimerActionIterator TimerActionsBegin();
  /*! Access to timer records by iterator */
  TimerActionIterator TimerActionsEnd();


  /** @} */




  /*****************************************
   *****************************************
   *****************************************
   *****************************************/

  /**
   * @name Code Generation Interface
   *
   * Although the base class does not generate any code, it provides a
   * common external interface, i.e., a GodeGenerator application may
   * uniformly generate code regardless of the particular target; see also
   * compiledes.cpp. In support of derived classes, CodeGenerator also
   * provides an interface for the output of the actual code either to
   * console, a string or a file.
   */

  /** @{ doxygen group */

  /*!
   * Generate code
   *
   * This method sets up the output stream and triggers actual code generation by
   * invoking the virtual method DoGenerate(). To adapt output facilities, reimplement
   * Generate() to set up the protected variable pOutStream appropriately, then invoke
   * the base class method. To adapt code-generation, reimplement DoGenerate() or,
   * preferably, its building blocks; see also @ref CGClasses.
   *
   * Must invoke Compile() before Generate()
   */
   virtual void Generate(void);

  /*!
   * Set verbosity level.
   *
   * Progress report and compilation details are forwarded to std::cerr
   * by default. Supported levels are "0<>warnings only", "1<>verbose", "2<>very verbose".
   * Diagnostic outpost is conveniently organised by the macros
   * FCG_VERB0,FCG_VERB1 and FCG_VERB2 defined at the top of this file.
   *
   */
   void Verbose(int level, std::ostream* altout=0);

  /*!
   * Set code output mode.
   *
   * Code can be output to the console (mode="std::cout"), a string buffer
   * (mode=="std::string") or a file (mode amounts to the filename).
   * The default behaviour is console output. Code generators that produce multiple output
   * files will interprete a provided file name as base and append appropriate
   * suffixes.
   *
   * Code generators that only support specific output modes should
   * reimplement this virtual function to enforce the respective constrains and/or
   * to issue warning messages,
   *
   *
   * @param filebase
   *   (Base)name of output file(s)
   */
   virtual void OutputMode(const std::string& mode) {mOutMode=mode;};


  /*!
   * Report code output mode.
   *
   * See the corresponding setter OutputFile(const std::string&)
   *
   */
   std::string OutputMode(void) { return mOutMode; }


   /*!
    * Output stream.
    *
    * This stream is setup by Generate() to operate in the specified mode. To this end, CodeGenerator
    * uses a filtered stream that provides various features like line counting, XML escape, and mute
    * modes to support multi-pass code generation. The interface to these extra features is via
    * the CodeGenerator class. For the standard C++ formating features, Output() is a plain std::ostream
    * reference.
    *
    * Derived classes may use more advanced output facilities, however, they are meant to reimplement
    * at least line count and mute by the interface via the class CodeGenerator.
    *
    */
   virtual std::ostream& Output(void);


   /*!
    * Get accumulated output as string
    *
    * Returns the output generatedd so far as string. This is only functional when
    * initialised with output mode "std::string".
    *
    * Technical Note: the return value is a reference to an object owned by the internal
    * output  buffer; the reference becomes invalid at the next invokation of Generate()
    * or at the destruction of the base class object.
    *
    */
   const std::string& OutputString(void);


   /*!
    * Set output to string
    *
    * Overwrites any output generated so far. This is only functional when
    * initialised with output mode "std::string".
    *
    */
   void OutputString(const std::string& strbuf);


   /*!
    * Set current mute mode.
    *
    * Subsequent output is muted if the current mute condition does not match the mute mode.
    * The default mute mode is '*' and matches any mute condition; see below MuteCond(char).
    * Setting a mode other than '*' mutes the output until the condition matches the mode;
    * see also MuteCond(char).
    *
    * By convention, low-level output-generating functions use MuteCond(char) to insist in
    * a particular global mute state which in turn is set via MuteMode(char) by some higher
    * level function that controls a larger code fragment. This mechanism is meant to faciltate
    * simple multi-pass code generation; see GebtoolsCodeGenerator for an example.
    */
   virtual void MuteMode(char mode);

   /*! Set mode condition.
    *
    * Subsequent output is muted if the current mute mode does not match the condition.
    * The default behaviour is the condition '*' which matches any mode, so no muting
    * takes place. See also MuteMode(char).
    *
    */
   virtual void MuteCond(char mode);

   /*! LineFeed (convenience support for derived classes) */
   virtual void LineFeed(int lines=1);

   /*! LineFeed (convenience support for derived classes) */
   virtual std::string LineCount(void);

   /*! Indentation (convenience support for derived classes) */
   virtual void IndentInc();

   /*! Indentation (convenience support for derived classes) */
   virtual void IndentDec();

   /*! Write a comment (reimplement in derived classes, call base) */
   virtual void Comment(const std::string& text);

   /*! Recent muted comment (convenience support for derived classes) */
   std::string RecentComment(void) {return mRecentMutedComment;};

   /*! XmlTextEscape (escape "<", ">", "&", "\"" and "'") */
   virtual void XmlTextEscape(bool on);

   /*! XmlCdataEscape (escape "]]>") */
   virtual void XmlCdataEscape(bool on);

   /*! Mute comments (convenience support for derived classes) */
   virtual void MuteComments(bool on);

   /*! Mute empty lines (convenience support for derived classes) */
   virtual void MuteVspace(bool on);

   /** @} doxygen group */

   /*****************************************
    *****************************************
    *****************************************
    *****************************************/

   /**
    * @name Code-Generator Registry
    *
    * @nosubgrouping
    *
    * Derived classes register via a static registration mechanism
    * to provide an application-invokable constructor. The static registry records
    * the constructor with a std::string typename as identifier.
    * The purpose of this mechanism is to instantiate derived classes by their identifier
    * at runtime.E.g.
    * @code
    * CodeGenerator* cg = CodeGenerator::New("iec");
    * @endcode
    * instantiates a CodeGenerator object of the type registered by "iec".
    * See the command-line tool compiledes.cpp for a relevant use case.
    *
    * Registration can be conveniently triggered by instantiation of a CodeGenerator::Registration
    * object via the provided macro. E.g. in the source that defines the class IecCodeGenerator
    * @code
    * FAUDES_REGISTERCODEGENERATOR("iec",IecCodeGenerator);
    * @endcode
    * is used to register the respective class with the identifier "iec".
    */

   /** @{ */

   /*!
    * Insert derived class in the registry.
    *
    * @param type
    *   Identifier of class to register
    * @param newcg
    *   Function to instantiate object of derived class
    */
    static void Register(const std::string& type, CodeGenerator* (*newcg)(void));

   /*!
    * Access registry contents.
    *
    * @return
    *   Registered CodeGenerator identifiers.
    */
   static std::vector< std::string > Registry(void);

   /*!
    * Instantiate by identifier (returns 0 on unknown class)
    *
    * @param type
    *   Identifier of class to instantiate
    * @return
    *   New CodeGenerator object (NULL if unknown identifier)
    */
   static CodeGenerator* New(const std::string& type);

   /*!
    * @brief Registration class.
    *
    */
   template< class T > class Registration {
   public:
     Registration(const std::string& type) { CodeGenerator::Register(type,&New);};
     static CodeGenerator* New(void) { return new T();};
   };

   /*!
    * Class registration macro
    */
   #define FAUDES_REGISTERCODEGENERATOR(ftype,ctype) static CodeGenerator::Registration<ctype> gRegister(ftype)

   /** @} */


  /*****************************************
   *****************************************
   *****************************************
   *****************************************/

protected:

  /*! faudes object name (aka project name) */
  std::string mName;

  /*! list of executors */
  std::vector<TimedGenerator> mGenerators;

  /*! list of filenames when generator are read from file */
  std::vector<std::string> mGeneratorNames;

  /*! event configuration by attributes */
  cgEventSet mAlphabet;   

  /*! compressed boolean capacity of target type word */
  int mWordSize;

  /*! compressed boolean capacity of target type integer */
  int mIntegerSize;

  /*! mapping from faudes event idx to bit address (descending priority, range 0 .. #-1) */
  std::map<Idx,int> mEventBitAddress;

  /*! mapping from bit address to faudes event idx  */
  std::map<int,Idx> mEventFaudesIdx;

  /*! highest bit-address with input (or timer) event  (-1 for none) */
  int mLastInputEvent;

  /*! highest bit-address with output event  (-1 for none) */
  int mLastOutputEvent;

  /*! mapping from faudes state idx to vector index */
  std::vector< std::map<Idx,int> > mStateVectorAddress;

  /*! mapping from vector state idx to faudes index */
  std::vector< std::map<int,Idx> > mStateFaudesIndex;

  /*! configuration of state indexing per generator*/
  std::vector< bool > mUsingVectorAddressStates;

  /*! compiled transition-sets, represented as vectors of integers with 0 as separator */
  std::vector< std::vector<int> > mTransitionVector;

  /*! configured events that are referred to by some generator */
  EventSet mUsedEvents;

  /*! used events that are configured as outputs */
  EventSet mOutputEvents;

  /*! used events that are configured as inputs (incl timer) */
  EventSet mInputEvents;

  /*! used events that are configured as internal events (excl. timers) */
  EventSet mInternalEvents;

  /*! input event generation */
  std::map<std::string, LineAddress> mLines;

  /*! input event generation */
  std::map<std::string, FlagExpression> mFlags;

  /*! timer definitions */
  std::map<std::string, TimerConfiguration> mTimers;

  /*! action addresses */
  std::map<std::string, ActionAddress> mActionAddresses;

  /*! timer actions by event name */
  std::map<std::string, TimerAction> mTimerActions;

  /*! diagnpstic-output level  */
  int mVerbLevel;

  /*! output file name (base) */
  std::string mOutMode;

  /*! current output mode */
  char mMuteMode;

  /*! mute comments */
  bool mMuteComments;

  /*! recent muted comment  */
  std::string mRecentMutedComment;

  /*! output stream */
  std::ostream* pOutStream;

  /*! error stream */
  std::ostream* pErrStream;

  /*! virtual hook to input parameter compilation */
  virtual void DoCompile(void);

  /*! pure virtual interface to code generation */
  virtual void DoGenerate(void) = 0;

  /*!
   * Read the configuration from TokenReader, see faudes Type for public wrappers.
   *
   * The label argument is ignored, we use the
   * hardcoded section "CodeGenerator". The context argument is ignored.
   * This method calls the virtual hook DoReadTargetConfiguration() to be reimplemented
   * by derived classes.
   *
   * @param rTr
   *   TokenReader to read from
   * @param rLabel
   *   Section to read
   * @param pContext
   *   Read context to provide contextual information
   *
   * @exception Exception
   *   - IO error (id 1)
   */
  virtual void DoRead(TokenReader& rTr,const std::string& rLabel="", const Type* pContext=0);


  /*!
   * Writes the configuration to TokenWriter, see faudes Type for public wrappers.
   *
   * The label argument is ignored, we use the
   * hardcoded section "CodeGenerator". The context argument is ignored.
   * This method calls the virtual hook DoWriteTargetConfiguration() to be reimplemented
   * by derived classes.
   *
   * @param rTw
   *   TokenWriter to write to
   * @param rLabel
   *   Section to write
   * @param pContext
   *   Read context to provide contextual information
   *
   * @exception Exception
   *   - IO error (id 2)
   */

  virtual void DoWrite(TokenWriter& rTw,const std::string& rLabel="", const Type* pContext=0) const;

  /*!
   * Reads global configuration from TokenReader, excl. label
   *
   * @param rTr
   *   TokenReader to read from
   *
   * @exception Exception
   *   - token mismatch (id 502)
   *   - IO error (id 1)
   */
  virtual void DoReadTargetConfiguration(TokenReader& rTr);
 
  /*!
   * Write global configuration to TokenWriter, excl. label
   *
   * @param rTw
   *   Reference to TokenWriter
   *
   * @exception Exception 
   *   - IO errors (id 2)
   */
  virtual void DoWriteTargetConfiguration(TokenWriter& rTw) const;

private:

  // private data: code generator type registry
  static std::map< std::string, CodeGenerator* (*)(void) >* mpsRegistry;

  // private method: code organisation: read generators
  void DoReadGenerators(TokenReader& rTr);

  // private method: code organisation: write generators
  void DoWriteGenerators(TokenWriter& rTw) const;


  // cheap filtering streambuf for line-count/indentation/xml-escape
  class cgp_streambuf : public std::streambuf {
  public:
    // construct by dst filename (alt. "std::string" for string-buffer only, "std::cout" for console),
    cgp_streambuf(std::string mode);
    // destruct
    ~cgp_streambuf();
    // report
    int LineCount();
    // Indentation
    void IndentInc();
    void IndentDec();
    // control
    void Mute(bool on);
    bool Mute(void);
    void MuteVspace(bool on);
    void XmlTextEscape(bool on);
    void XmlCdataEscape(bool on);
    void Flush();
    // access
    const std::string& Buffer();
    void Clear(void);

  protected:
    // use overflow hook to see individual characters one by one
    // this is a perhaps inefficient but cheap way to build a filter
    // --- implementation assumes "char-8bit-ascii" output
    virtual int_type overflow (int_type c);
    // my config data
    std::string mBuffer;
    std::ostream* pOutStream;
    std::ofstream* mpFOutStream;
    std::stringstream* mpSOutStream;
    std::string mMode;
    int mLines;
    bool mBeginOfLine;
    int mIndent;
    bool mMute;
    bool mMuteVspace;
    bool mTextEscape;
    bool mCdataEscape;
    
  };

  // private instance
  cgp_streambuf* pOutBuffer;


};


#endif
