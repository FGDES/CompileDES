/** @file cgp_codeprimitives.h @brief Code-generator with abstract types and operations */

/*
  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2010, 2016, 2017 Thomas Moor

*/



#ifndef FAUDES_CODEPRIMITIVES_H
#define FAUDES_CODEPRIMITIVES_H

#include "libfaudes.h"
#include "cgp_codegenerator.h"

using namespace faudes;



/**
 * @brief Execution semantics in terms of code primitives
 *
 * The CodePrimitives class extends the common base CodeGenerator by the intended
 * run-time behaviour constructed in terms of abstract primitives
 * of target code. By "abstract" we mean the primitives are not associated with
 * actual target code; this is left for derived classes. Expressing the semantics of
 * synchronised execution of automata and event configuration in terms of primitives
 * enables consistent run-time behaviour of code generated by derived classes; see
 * @ref PExSem for a dicussion of the intended run-time behaviour.
 *
 * Referring to compilers in general, CodePrimitives is on the same layer of
 * abstraction as the parse tree commonly generated from source code. However, since
 * our case is much more specific, we can drastically simplify the approach. In CodePrimitives,
 * the to-be-generated code is implicitly represented by a linear sequence of primitives,
 * each technically realised by a virtual member method.
 *
 * Example to iterate over integers ranging from 1 to 10
 *
 @verbatim
  DeclareInteger("i",1);
  LoopBegin();
  [... do whatever with integer i ...]
  IntegerInc("i");
  LoopBreak(IntegerIsEq("i",11));
  LoopEnd();
 @endverbatim
 *
 * Commenting on the example, we refer to a notion of an integer typed variable, including its
 * declaration/definition, incrementing and comparing with a constant, as well as a notion
 * of a control structure for a conditional loop. For high-level languages such as C, one may safely expect
 * each primitive to be expressible one-to-one by a line of code.
 *
 @verbatim
  int i=1;
  while(true) {
    [... do whatever with integer i ...]
    ++i;
    if(i==11) break;
  }
 @endverbatim
 *
 * For low-level languages such as machine-code assembler one will need to keep track of some aspects of
 * the corresponding parse tree. If need be, the parse tree can be constructed by a derived class.
 *
 * Tailored for the specific use case of synchronised automata behaviour CodePrimitives() uses primitives
 * in the above style to assemble code snippets that make up the core of a cyclic function that implements
 * the desired runtime behaviour on the target device. Outline:
 *
 @verbatim
  Declarations();     // current state, line buffers for edge detection etc, event sets
  SenseInputs();      // add to pending events
  SenseTimerElapse(); // add to pending events
  UpdateEnabled();    // figure enabled events
  SelectEvent();      // select from pending&enabled with highest priority
  ExecuteEvent();     // update state w.r.t. selected event
  OperateOutputs();   // operate outputs w.r.t. selected output
 @endverbatim
 *
 * Referring to the above outline there are a number of options to adapt the
 * generated code to capabilities of the intended target and/or to evaluate trade-offs in performance
 * and memory footprint. Code-options are set in the configuration file and read by CodePrimitives using
 * the virtual method hook DoReadTargetConfiguration(). The following options are supported.
 *
 * <strong>IntegerType</strong>/<strong>IntegerSize</strong>. Target type and bitsize of the designated
 * signed integer datatype. The type is typically gigen as target language literal. Integers are used
 * as event and state indices, so a 16bit type should serve most purposes. When not using compiled
 * transition relations (see below ArrayForTransitions), an 8bit type may be sufficient. Example:
 *
 * @verbatim
   <IntegerType val="int16_t"/>
   <IntegerSize val="16"/>
   @endverbatim
 *
 * <strong>WordType</strong>/<strong>WordSize</strong>. Target type and bitsize of the designated
 * unsigned word datatype. The type is typically gigen as target language literal. Words are used
 * to organise bit-arrays. Best performance is expected for natively supported types, e.g. 8bit on
 * AVR8 microcontrollers, 16bit or 32nit for a typical PLC. Some of the below code options assume the size
 * to be a power of 2 and a careful code review is required for other word sizes. Example:
 *
 * @verbatim
   <WordType val="unsigned char"/>
   <WordSize val="8"/>
   @endverbatim
 *
 * <strong>ArrayForTransitions</strong>. The fallback behaviour is to represent transition
 * relations by a switch/case construct. Depending on the specific target, one may expect reduced usage of
 * variable memory at the cost of extensive use of program memory. Alternatively, transition relations
 * can be precompiled and represented as arrays of integers to be interpreted at runtime; see CodeGenerator()
 * for the data format. Example opting for the alternative:
 *
   @verbatim
   <ArrayForTransitions val="true"/>
   @endverbatim
 *
 * <strong>MaintainStateIndices</strong>. By default, the generated code will refer to strategically
 * re-indexd states whenever this is deemned beneficial. Alternatively, one may specify that the provided states
 * indices are maintained. When using the array representation of transition relations, this will require an
 * additional array-address translation table and thus introduces a relevant memory overhead.
 * Example opting for the alternative:
 *
   @verbatim
   <MaintainStateIndices val="true"/>
   @endverbatim
 *
 * <strong>ArrayForBitarray</strong>. EventSets are represented arrays of bits. The default is to use
 * a sufficient number of individual words and to keep track which events are represented by which word.
 * If the target supports arrays of words these can be used as an alternative to individual words.
 * A performance benefit is expected from avoiding a switch/case construct for word selection.
 * Example opting for the alternative:
 *
 * @verbatim
   <ArrayForBitarray val="true"/>
   @endverbatim
 *
 * <strong>ArrayForBitmasks</strong>. When the generated code shall access a particular bit within an array of bits
 * or within a word, the corresponding bit-mask (and word-address) can be computed at runtime by bit shift
 * operations. This is the default. Alternatively, the bit-mask (and word-address) can be computed
 * offline and provided to the runtime environment by a look-up table. While performance trade-offs
 * are expected to be minimal, this addresses situation where the target does not provide bit shift operations.
 * Example opting for the alternative:
 *
 * @verbatim
   <ArrayForBitmasks val="true"/>
   @endverbatim
 *
 * <strong>BitAddressArithmetic</strong>. When not using ArrayForBitmask falling back to
 * runtime computation is the default. Alternatively, avoiding look-up tables and bit shifting altogether,
 * bit access can be resolved by extensive switch/case construct. This option is
 * chosen by setting the BitAddressArithmetic attribute to false. This is not recommended for
 * practical use. Example to opt for runtime computations:
 *
 * @verbatim
   <BitAddressArithmetic val="true"/>
   @endverbatim
 *
 * <strong>BisectionForBitfind</strong>. Technically, event selection amounts to finding the left-most bit set
 * in an bitarray. This can be done by testing all individual bits until a set bit is found.
 * Alternatively, one can organize the search by an bisection approach, gaining 2x or 4x
 * in performance for 8bit words and 16bit words, respectively, provided that the word data type
 * is natively supported by the target platform.
 * Example to use bisection:
 *
 * @verbatim
   <BisectionForBitfind val="true"/>
   @endverbatim
 *
 * <strong>ArrayForState</strong>. Use an array of integers to represent the
 * overall state (aka parallel-state). This option is required for the optional
 * callback hook on state updates. In the current implementation, the generated
 * code will only access the state array with addresses known at compiletime. Thus,
 * there should be no performamce penalty.
 * Example opting for arrays:
 *
 * @verbatim
   <ArrayForState val="true"/>
   @endverbatim
 *
 * <strong>LoopPendingInputs</strong>.
 * There is a relevant history on whether and when multiple transitions can be taken within one scan-cycle.
 * The conservative answer from the semantic perspective is "never", however, performance considerations push
 * for "under certain circumstances we should". Using the option <tt>LoopPendingInputs</tt> the event-execution
 * loops until no more input event is scheduled. Since there can be only finitely many pending input events and
 * since input events are preferred over other events, this option is considered safe.
 * Example:
 *
 * @verbatim
   <LoopPendingInputs val="true"/>
   @endverbatim
 *
 * <strong>LoopEnabledOutputs</strong>.
 * With this option, event-execution loops until an event is scheduled with priority lower than the lowest
 * output event priority. To prevent high-level activity-loops, this option requires all internal events to
 * be of lower priority than any non-internal event. In order to ensure that the loop does not get stuck
 * one can conservatively test that neither automaton has a strictly connected component with output
 * events only. This option targets "react within one scan cycle" performance requirements --- use with care.
 *
 * @verbatim
   <LoopEnabledOutputs val="true"/>
   @endverbatim
 *
 *
 * <strong>StrictEventSynchronisation</strong>.
 * It is considered an error if no event can be scheduled while pending input events are available.
 * However, when this error is caused by lazy modelling it can be resolved by discarding pending events.
 * Since this is the default behaviour of <tt>simfaudes</tt>, some of the older FGDES lab experiments
 * require this option to be set. For new developments, it is recommended to set this option
 * to <tt>false</tt>.
 *
 * @verbatim
   <StrictEventSynchronisation val="false"/>
   @endverbatim
 *
 *
 * <strong>EventNameLookup</strong>,  <strong>StateNameLookup</strong>.
 * Events and states are internaly represented by integers in a particular layout depending on various code generation
 * options. For diagnostic puposes, a lookup table can be generated to recover the original event names and state
 * names, respectively. For memory efficient name-lookup arrays, index ranges should be consecutive. This is naturaly
 * the case for events. Regarding state indices, the code generator will automatically apply strategic re-indexing
 * for a consecutive representation. For the array representation of transitions this requires a second array to map
 * consecutive indices to array addresses in the same way as if the option <tt>MaintainStateIndices</tt> was activated.
 * Further implementation details are target dependent.
 * Example:
 *
 * @verbatim
   <EventNameLookup val="true"/>
   <StateNameLookup val="true"/>
   @endverbatim
 *
 *
 * <strong>EventExecutionHook</strong>, <strong>StateUpdateHook</strong>.
 * The generated code can invoke hook functions whenever an event is executed
 * and whenever a state changes. This mechanism is meant for diagnosis purposes only, it should not be
 * used to implement relevant dynamics like state based execution code.
 * Implementation details are target dependent.
 * Example:
 *
 * @verbatim
   <EventExecutionHook val="report_event"/>
   <StateUpdateHook val="report_state"/>
   @endverbatim
 *
 * @ingroup CGClasses
 *
 */

class CodePrimitives : public CodeGenerator {

public:

  /*****************************************
   *****************************************
   *****************************************
   *****************************************/

  /**
   * @name Basic Class Maintenance
   *
   * As usual, we have constructor and destructor. Note the necessity to reimplement
   * the virtual member Clear() since we introduce additional configuration data.
   *
   */
  /** @{ */
  /*!
   * Constructor
   */
  CodePrimitives(void);

  /*!
   * Explicit destructor.
   */
  virtual ~CodePrimitives(void);

  /*!
   * Clear all data.
   *
   */
  virtual void Clear(void);

  /** @} */

protected:

  /**
   *  @name Code Snippets
   *
   * Code modules to assemble standard semantics as known from simfaudes. The individual blocks
   * interact via common global variables, i.e., relevant re-implementations require
   * the examination of all modules from scratch.
   */

  /** @{ */

  /** @brief Cosmetic: prepend literally from configuration data */
  virtual void LiteralPrepend(void);

  /** @brief Declare "status" */
  virtual void DeclareStatus(void);

  /** @brief Declare "reset" */
  virtual void DeclareReset(void);

  /** @brief Declare "recent_event" */
  virtual void DeclareRecentEvent(void);

  /** @brief Declare "parallel_state" */
  virtual void DeclareParallelState(void);

  /** @brief Declare "pending_events" and "enabled_events" */
  virtual void DeclarePendingEvents(void);

  /** @brief Declare loop state, i.e. line levels, loop flag */
  virtual void DeclareLoopState(void);

  /** @brief Use target implementation to declare timers, typically "timer_run_*" and "timer_cnt_*" */
  virtual void DeclareTimers(void);

  /** @brief Declare variables local to the provided snippets, e.g. helpers for bit-mask computation */
  virtual void DeclareAux(void);

  /** @brief Declare compiled transition relations */
  virtual void DeclareLargeCarray(void);

  /** @brief Declare bit-mask loop-ups */
  virtual void DeclareSmallCarray(void);

  /** @brief Declare symbolic name lookup tables */
  virtual void DeclareEventNameLookup(void);

  /** @brief Declare symbolic name lookup tables */
  virtual void DeclareStateNameLookup(void);

  /** @brief Reset state */
  virtual void ResetState(void);

  /** @brief Reset bail out */
  virtual void ResetReturn(void);

  /** @brief Sense input events and add to "pending_events" */
  virtual void SenseInputs(void);

  /** @brief Sense timer elapse vents and add to "pending_events" */
  virtual void SenseTimerElapse(void);

  /** @brief Execution Loop, begin */
  virtual void BeginExecutionLoop(void);

  /** @brief Update "enabled_events" from "parallel_state" if "exec_event" was set */
  virtual void UpdateEnabled(void);

  /** @brief Select event to execute from "pending_and_enabled_events" or "enabled_events" */
  virtual void ScheduleEvent(void);

  /** @brief Execution Loop, break */
  virtual void BreakExecutionLoop(void);

  /** @brief Take transition and figure new state */
  virtual void ExecuteEvent(void);

  /** @brief Start/stop/reset timers w.r.t. "exec_event" */
  virtual void OperateTimers(void);

  /** @brief Operate output lines w.r.t. "exec_event" */
  virtual void OperateOutputs(void);

  /** @brief Loop end */
  virtual void EndExecutionLoop(void);

  /** @brief Cosmetic: append literally from configuration */
  virtual void LiteralAppend(void);

  /** @brief Helper to insert target code for execution hooks */
  virtual void InsertExecHooks(void);

  /** @brief Alternative implementation of ExecuteEvent() */
  virtual void ExecuteEventBySwitching(void);

  /** @brief Alternative implementation of ExecuteEventBy() */
  virtual void ExecuteEventByInterpreter(void);

  /** @brief Alternative implementations UpdateEnabled() */
  virtual void UpdateEnabledBySwitching(void);

  /** @brief Alternative implementations UpdateEnabled() */
  virtual void UpdateEnabledByInterpreter(void);

  /** @} */

  /**
   *  @anchor Absstract_Addresses
   *  @name Abstract Addresses and Expressions
   *
   * We use strings as abstract addresses that correspond to symbolic addresses
   * on the target platform. Similarly, we use strings as abstract expressions that
   * evaluate to a value on the target platform. To differentiate the two types, we formally
   * derive AA (for address) and AX (for expression) from std::string.
   *
   * TargetAddress() resolves the abstract address argument to an actual
   * address on the target platform. In order to invoke primitives that expect
   * an expression type argument with an address, TargetExpression() converts
   * an abstract address to an expression that represents the associated value.
   * There is also a convenience method TargetSymbol() to mangle any string to
   * a valid target symbol.
   *
   * Note: a more advanced approach could use different classes to distinguish the
   * target type of expressions and one would perhaps maintain the parse tree of an expression.
   *
   */

  /** @{ */

  /** @brief Abstract address; see also @ref Absstract_Addresses */
  class AA : public std::string {
  public:
    explicit AA(void) : std::string() {};
    explicit AA(std::string src) : std::string(src) {};
    AA Sub(int subscript) const { return AA(std::string(*this) + "_" + ToStringInteger(subscript));};
  };
  /** @brief Abstract expression; see also @ref Absstract_Addresses */
  class AX : public std::string {
  public:
    explicit AX(void) : std::string() {};
    explicit AX(std::string src) : std::string(src) {};
  };
  /*!  Convert abstract address to target symbolic address */
  virtual std::string TargetAddress(const AA& address)=0;
  /*!  Convert abstract address to target expression of the respective value */
  virtual AX TargetExpression(const AA& address)=0;
  /*! Mangle string to valid target symbol. */
  virtual std::string TargetSymbol(const std::string& str);

  /** @} */



  /**
   * @name Model of Integer Type
   *
   * The elementary integer type is meant to represent target event/state indices.
   * Thus, it typically should be 16bit, and, to this end, signed.
   *
   * We discuss some conventions used for this and the subsequent data types.
   * - declaration is provided with and without explicit initialisation; 
   * - "address" is an abstract address, e.g, "current_state" and will be transformed
   *   by TargetAddress() just before writing to the output file, e.g. "FGDES_current_state"
   *   for a high-level target with prefix to emulate a namespace or, in contrast, "0x0024"
   *   on a low-level target that maintains a symbol table and organises memory allocation.
   * - "val" is a constant value and will be transformed by IntegerConstant() to
   *   the appropriate textual representation on the target, e.g., "BYTE#16#40" for
   *   a bit-mask in IEC 61131-3 syntax.
   * - "expression" is an expression of appropriate type on the target; for targets that support
   *   the concept of an expression this can be a literal representation, e.g., "a+3"; low-level
   *   targets record a suitably formatted representation to be resolved when actually used, e.g., in an assignment.
   * - return values are target expressions.
   *
   *
   * Specifically for the integer type, declaration, assignment and increment is mandatory to
   * reimplement in derived classes, and, hence, declared pure virtual. Other integer primitives
   * by default trigger a run-time error when needed by the particular code options. E.g.,
   * when generating code that uses IntegerIsGreater(), the implementation of CodePrimitives()
   * will trigger an error if not reimplemented by the respective derived class. For arithmetics
   * (reading quotient and remainder), there is a query-function to allow the derived class
   * to seek for alternatives if the respective functions are not implemented.
   *
   */

  /** @{ */
  virtual void IntegerDeclare(const AA& address) =0;
  virtual void IntegerDeclare(const AA& address, int val) =0;
  virtual void IntegerAssign(const AA& address, int val) =0;
  virtual void IntegerAssign(const AA& address, const AX& expression)=0;
  virtual void IntegerIncrement(const AA& address, int val=1)=0;
  virtual void IntegerDecrement(const AA& address, int val=1);
  virtual AX IntegerQuotient(const AX& expression, int val);
  virtual AX IntegerRemainder(const AX& expression, int val);
  virtual AX IntegerBitmask(const AX& expression);
  virtual AX IntegerIsEq(const AA& address, int val);
  virtual AX IntegerIsEq(const AA& address, const AX& expression);
  virtual AX IntegerIsNotEq(const AA& address, int val);
  virtual AX IntegerIsNotEq(const AA& address, const AX& expression);
  virtual AX IntegerIsGreater(const AA& address, int val);
  virtual AX IntegerIsLess(const AA& address, int val);
  virtual AX IntegerConstant(int val)=0;
  virtual bool HasIntmaths(void);
  /** @} */


  /**
   * @name Model of Word Type.
   *
   * Elementary type to represent a word-of-bits.
   *
   * This may be anything from 8bit to 64bit, with different tradeoffs in memory footprint and
   * performance. This data type is exclusively used as a default basis for more general bit-arrays
   * with uniform but arbitrary size. You may opt to reimplement Bitarray directly. In this case, Word
   * is not required.
   */

  /** @{ */
  virtual void WordDeclare(const AA& address);
  virtual void WordDeclare(const AA& address, word_t val);
  virtual void WordAssign(const AA& address, word_t val);
  virtual void WordAssign(const AA& address, const AX& expression);
  virtual void WordOr(const AA& address, word_t val);
  virtual void WordOr(const AA& address, const AX& expression);
  virtual void WordOr(const AA& address, const AA& op1, const AA& op2);
  virtual void WordOr(const AA& address, const AA& op1, word_t op2);
  virtual void WordAnd(const AA& address, word_t val);
  virtual void WordAnd(const AA& address, const AX& expression);
  virtual void WordAnd(const AA& address, const AA& op1, const AA& op2);
  virtual void WordAnd(const AA& address, const AA& op1, word_t op2);
  virtual void WordNand(const AA& address, const AX& expresson);
  virtual AX WordIsBitSet(const AA& address, int idx);
  virtual AX WordIsBitClr(const AA& address, int idx);
  virtual AX WordIsMaskSet(const AA& address, word_t mask);
  virtual AX WordIsEq(const AA& address, word_t val);
  virtual AX WordIsNotEq(const AA& address, word_t val);
  virtual AX WordConstant(word_t val);
  /** @} */


  /**
   *  @name Model of Boolean Type.
   *
   * Elementary type to represent single bits.
   *
   * The base implementation defaults to Integer
   * and may be overloaded for a more compact representation. It is currently used to
   * manage conditionals and edge detection on line level. Performance of bit access
   * is regarded more relevant than memory footprint.
   */

  /** @{ */
  virtual void BooleanDeclare(const AA& address);
  virtual void BooleanDeclare(const AA& address, int val);
  virtual void BooleanAssign(const AA& address, int val);
  virtual void BooleanAssign(const AA& address, const AX& expression);
  virtual AX BooleanIsEq(const AA& op1, const AA& op2);
  virtual AX BooleanIsNotEq(const AA& op1, const AA& op2);
  /** @} */


  /**
   *  @name Model of Constant Array of Integers
   *
   * Type to represent a constant vector of integers.
   *
   * This is used to represent
   * precompiled transition relations; i.e., implementation is optional. When implemented,
   * the target type must be identical to the type used for the implementation of Integer.
   */

  /** @{ */
  virtual void CintarrayDeclare(const AA& address, int offset, const std::vector<int>& val);
  virtual AA CintarrayAccess(const AA& address, int index);
  virtual AA CintarrayAccess(const AA& address, const AA& indexaddr);
  virtual bool HasCintarray(void);
  /** @} */


  /**
   *  @name Model of Constant Array of Words
   *
   * Representation of a constant vector of words.
   *
   * This is used to represent precompiled bit-masks; i.e., implementation is optional.
   * When implemented, the target type must be identical to the type used for the
   * implementation of Word.
   */

  /** @{ */
  virtual void CwordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val);
  virtual AA CwordarrayAccess(const AA& address, int index);
  virtual AA CwordarrayAccess(const AA& address, const AA& indexaddr);
  virtual bool HasCwordarray(void);
  /** @} */


  /**
   *  @name Model of Constant Array of Strings
   *
   * Type to represent a constant vector of strings.
   *
   * This is used to represent symbol tables for e.g. event names. Implementation
   * is optional. 
   */

  /** @{ */
  virtual AX StringConstant(const std::string &val);
  virtual void CstrarrayDeclare(const AA& address, int offset, const std::vector<std::string>& val);
  virtual AA CstrarrayAccess(const AA& address, int index);
  virtual AA CstrarrayAccess(const AA& address, const AA& indexaddr);
  virtual bool HasCstrarray(void);
  /** @} */


  /**
   *  @name Model of Array of Integers
   *
   * Type to represent a variable vector of integers.
   *
   * This is optionally used to represent the parallel state.
   * When implemented, the target type must be identical
   * to the type used for the implementation of Integer.
   */

  /** @{ */
  virtual void IntarrayDeclare(const AA& address, int offset, int len);
  virtual void IntarrayDeclare(const AA& address, int offset, const std::vector<int>& val);
  virtual AA IntarrayAccess(const AA& address, int index);
  virtual AA IntarrayAccess(const AA& address, const AA& indexaddr);
  virtual bool HasIntarray(void);
  /** @} */


  /**
   *  @name Model of Array of Words
   *
   * Type to represent a variable vector of words.
   *
   * This is optionally used to represent
   * bit-arrays, i.e., implementation is not mandatory. When implemented, the target type must be identical
   * to the type used for the implementation of Word.
   */

  /** @{ */
  virtual void WordarrayDeclare(const AA& address, int offset, int len);
  virtual void WordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val);
  virtual AA WordarrayAccess(const AA& address, int index);
  virtual AA WordarrayAccess(const AA& address, const AA& indexaddr);
  virtual bool HasWordarray(void); 
  /** @} */


  /**
   * @name Model of Array of Bits
   *
   * Representation of arrays-of-bits.
   *
   * The size can be arbitrary but must be known at compile
   * time. This data type is used to model event sets and is used for the default implementation
   * of EventSet. If EventSet is reimplemented from scratch, neither arrays-of-bits nor arrays-of-words
   * are not required.
   *
   * For indexed bit access via integer expressions, the size must not exceed the
   * number of events. For the default implementation, bit-arrays are represented by an
   * appropriate number of words-of-bits. An alternative implementations uses target arrays-of-words.
   * The main purpose of the data type is the representation of event sets. Here, indexed
   * bit access uses the parameter offset to account for target event indices to begin with
   * 1 as opposed to native bit-addresses that begin with 0.
   *
   * Without the options ArrayForBitmasks or BitaddressArithmatic, indexed access performs
   * poorly. With the option ArrayForBitarrays indexed access performs best. These considerations apply
   * to the indexed version of BitarraySetBit() and BitarrayClrBit() as well as BitarrayFindFirst().
   * Check the implementation for details.
   */

  /** @{ */
  virtual void BitarrayDeclare(const AA& address, int blen);
  virtual void BitarrayDeclare(const AA& address, const std::vector<bool>& val);
  virtual void BitarrayAssign(const AA& address, const std::vector<bool>& val);
  virtual void BitarrayAssign(const AA& address, const AA& otherarray);
  virtual void BitarrayClear(const AA& address);
  virtual void BitarrayFull(const AA& address);
  virtual void BitarraySetBit(const AA& address, int bitaddr);
  virtual void BitarraySetBit(const AA& address, const AA& indexaddr, int offset=0, const std::vector<bool>& hint=std::vector<bool>() );
  virtual void BitarrayClrBit(const AA& address, int bitaddr);
  virtual void BitarrayClrBit(const AA& address, const AA& indexaddr, int offset=0, const std::vector<bool>& hint=std::vector<bool>() );
  virtual void BitarrayIsBitSet(const AA& address, const AA& indexaddr, const AA& result,int offset=0, const std::vector<bool>& hint=std::vector<bool>() );
  virtual void BitarrayOr(const AA& address, const std::vector<bool>& val);
  virtual void BitarrayOr(const AA& address, const AA& op1, const std::vector<bool>& op2);
  virtual void BitarrayOrAllWords(const AA& address, const AA& result);
  virtual void BitarrayAnd(const AA& address, const std::vector<bool>& val);
  virtual void BitarrayAnd(const AA& address, const AA& otherarray);
  virtual void BitarrayAnd(const AA& address, const AA& op1, const AA& op2);
  virtual void BitarrayAnd(const AA& address, const AA& op1, const std::vector<bool>& op2);
  virtual void BitarrayFindFirst(const AA& address, const AA& result, int offset=0);
  /** @} */



  /**
   *  @name Model of Eventsets
   *
   * Representation of EventSets. This is a wrapper for Bitarray, however, derived classes
   * may reimplement it altogether. This may be worthwhile on C++ targets with STL available.
   * In the default implementation, the bit-addresses assigned by the CodeGenerator are used,
   * i.e., ordered by execution priority.
   *
   */

  /** @{ */
  virtual void EventSetDeclare(const AA& address);
  virtual void EventSetDeclare(const AA& address, const EventSet& evset);
  virtual void EventSetAssign(const AA& address, const EventSet& evset);
  virtual void EventSetInsert(const AA& address, const EventSet& evset);
  virtual void EventSetInsert(const AA& address, Idx ev);
  virtual void EventSetInsert(const AA& address, const AA& evaddr);
  virtual void EventSetInsert(const AA& address, const AA& evaddr, const EventSet& hint);
  virtual void EventSetErase(const AA& address, const EventSet& evset);
  virtual void EventSetErase(const AA& address, Idx ev);
  virtual void EventSetErase(const AA& address, const AA& evaddr);
  virtual void EventSetErase(const AA& address, const AA& evaddr, const EventSet& hint);
  virtual void EventSetExists(const AA& address, const AA& evaddr, const AA& result, const EventSet& hint);
  virtual void EventSetRestrict(const AA& address, const AA& otherset);
  virtual void EventSetUnion(const AA& address, const AA& op1, const EventSet& op2);
  virtual void EventSetIntersection(const AA& address, const AA& op1, const EventSet& op2);
  virtual void EventSetClear(const AA& address);
  virtual void EventSetFull(const AA& address);
  virtual void EventSetIsNotEmpty(const AA& address, const AA& result);
  virtual void EventSetFindHighestPriority(const AA& address, const AA& result);
  /** @} */


  /**
   * @name Controls
   *
   * The common if/then/else construct is mandatory, while switch/case and loop
   * depend on code options. Without array-of-words, switch/case is required. With array-of-words
   * or precompiled vectors for transition relations, loops are required.
   * Loops are also required for loop-all-pending-events.
   *
   * The current code generators do not collapse switch-cases, this may change in a later version.
   */

  /** @{ */
  virtual void IfTrue(const AX& expression);
  virtual void IfFalse(const AX& expression);
  virtual void IfWord(const AX& expression);
  virtual void IfElseIfTrue(const AX& expression);
  virtual void IfElse(void);
  virtual void IfEnd(void);

  virtual void SwitchBegin(const AA& address);
  virtual void SwitchCase(const AA& address, int val);
  virtual void SwitchCases(const AA& address, int from, int to);
  virtual void SwitchCases(const AA& address, const std::set< int>& vals);
  virtual void SwitchBreak(void);
  virtual void SwitchEnd(void);
  virtual bool HasMultiCase(void);

  virtual void LoopBegin(void);
  virtual void LoopBreak(const AX& expression);
  virtual void LoopEnd(void);

  virtual void FunctionReturn(void);
  /** @} */


  /**
   *  @name Primitives for Triggers andAactions
   *
   *  The default implementation passes through as in value access and assignment, respectively.
  */
  /** @{ */
  virtual void RunActionSet(const std::string& address);
  virtual void RunActionClr(const std::string& address);
  virtual void RunActionExe(const AX& expression);

  virtual AX ReadInputLine(const std::string& address);
  virtual AX InputExpression(const std::string& expression);
  /** @} */


  /**
   *  @name Primitives to Implement Timers
   */
  /** @{ */
  virtual void TimerDeclare(const AA& address, const std::string& litval);
  virtual void TimerStart(const AA& address);
  virtual void TimerStop(const AA& address);
  virtual void TimerReset(const AA& address, const std::string& litval);
  virtual AX TimerIsElapsed(const AA& address);
  /** @} */


  /** @name Misc  */
  /** @{ */

  /*! Overload base class to use the vector address only if the respective code option is active) */
  virtual int StateTargetIdx(size_t git, Idx idx);
  virtual Idx StateFaudesIdx(size_t git, int idx);

  /*! Target comments  (see EmbeddedcCodeGenerator for consistent reimplementation pattern) */
  virtual void Comment(const std::string& text);

  /*! declaration template (optional to facilitate declaration constructs) */
  virtual void VariableDeclare(const std::string& laddr, const std::string& ltype);
  virtual void VariableDeclare(const std::string& laddr, const std::string& ltype, const std::string& lval);


  /** @} */

  /*****************************************
   *****************************************
   *****************************************
   *****************************************/

 protected:

  /*! target data type for word  */
  std::string mWordType;

  /*! target data type for integer  */
  std::string mIntegerType;

  /*! universal prefix (pseudo name space) */
  std::string mPrefix;  

  /*! code option: use const array to represent transitions */
  bool mArrayForTransitions;

  /*! code option: use state indices as provided */
  bool mMaintainStateIndices;

  /*! code option: compute bit and word address on target*/
  bool mBitAddressArithmetic;
  
  /*! code option: use const array to represent bit-masks */
  bool mArrayForBitmasks;

  /*! code option: use const array to represent bit-masks */
  bool mArrayForBitarray;

  /*! code option: use bisection to fing lowest set bit */
  bool mBisectionForBitfind;

  /*! code option: use int arrays to represent that overall state */
  bool mArrayForState;

  /*! code option: eventsets for performance */
  bool mEventsetsForPerformance;

  /*! code option: loop until all inputs are resolved */
  bool mLoopPendingInputs;

  /*! code option: loop until all enabled outputs are executed */
  bool mLoopEnabledOutputs;

  /*! code option: strict event synchronisation */
  bool mStrictEventSynchronisation;

  /*! code option: event name lookup */
  bool mEventNameLookup;

  /*! code option: state name lookup */
  bool mStateNameLookup;

  /*! record per generator whether there is a lookup table */
  std::vector<bool> mHasStateNames;

  /*! record whether there exist statenames at all */
  bool mExistStateNames;

  /*! code option: event exec hook */
  std::string mEventExecutionHook;

  /*! code option: state change hook */
  std::string mStateUpdateHook;

  /*! extra code to prepend */
  std::string mLiteralPrepend;

  /*! extra code to prepend */
  std::string mLiteralAppend;

  /**
   * @brief Record declared bit-arrays
   *
   * Record in support of the CodePrimitives implementation of arrays-of-bits
   */
  struct bitarray_rec {
    /*! length in bits */
    int blen;
    /*! initialisation value */
    std::vector<bool> value;
  };
  /*! Record of all declared bit-arrays */
  std::map<std::string,bitarray_rec> mBitarrays;

  /*! virtual hook to extend compiled data  */
  virtual void DoCompile(void);

  /*! cut-and-paste template for code snippet assembly  */
  virtual void DoGenerate(void);

  /*! cut-and-paste template for code snippet assembly  */
  virtual void DoGenerateDeclarations(void);

  /*! cut-and-paste template for code snippet assembly  */
  virtual void DoGenerateResetCode(void);

  /*! cut-and-paste template for code snippet assembly  */
  virtual void DoGenerateCyclicCode(void);

  /*! re-implement token i/o for extra configuration */
  virtual void DoReadTargetConfiguration(TokenReader& rTr);

  /*! re-implement token i/o for extra configuration */
  virtual void DoWriteTargetConfiguration(TokenWriter& rTw) const;

  private:

  /**
   * Templated function to consolidate consecutive switch-cases.
   *
   * The template parameter Data is meant to parameterise the code that will be
   * generated if the case condition is satisfied. The procedures inspects caseData to
   * figure case consitions that effectively will generate the same code. Therefore,
   * the Data class must support the boolean operator==. The result is returned by
   * the map of ranges caseFromTo; i.e., each entry represents a consecutive range of
   * case consitions that share the same to-be-generated execution code.
   *
   * See also the below varuation ConsolidateCaseSets.
   */
  template<class Data> void ConsolidateCaseRanges(std::map< int, Data>& caseData, std::map< int, int>& caseFromTo) {
    // prepare result
    caseFromTo.clear();
    // bail out
    if(caseData.empty()) return;
    // inititialise from-to-range to first case data record
    int fcase=caseData.begin()->first;
    int tcase=caseData.begin()->first;
    caseFromTo[fcase]=tcase;
    // loop over all other cases
    typename std::map< int, Data>::iterator cit=caseData.begin();
    for(++cit; cit!=caseData.end(); ++cit) {
      // current case condition
      int ccase = cit->first;
      // if we are meant to merge; and there is no gap; and the data matches ...
      if(HasMultiCase())
        if(tcase+1 == ccase)
          if(caseData[tcase]==caseData[ccase]) {
              // ... extend the range
              tcase=ccase;
              caseFromTo[fcase]=tcase;
              continue;
          }
      // so we did not want extend the range ... start a new range to extend
      fcase=ccase;
      tcase=ccase;
      caseFromTo[fcase]=tcase;
    }
  }

  /**
   * Templated function to consolidate switch-cases
   *
   * This is a variant of the above procedure ConsolidateCaseRanges. It takes the same
   * template parameter and the same argument caseData, however, it returns sets of case conditions
   * as oposed to ranges of case conditions. Here, it is left to the target compiler to optimize
   * space versus time if consecutive ranges are detected in the respective sets.
   *
   * This method is currently used in OperateTimers() and EcecuteEventBySwitching().
   */
  template<class Data> void ConsolidateCaseSets(std::map< int, Data>& caseData, std::set< std::set< int > >& caseSets) {
    // prepare result
    caseSets.clear();
    // bail out
    if(caseData.empty()) return;
    // if we are not meant to consolidate, produce trivial sets
    if(!HasMultiCase()) {
      typename std::map< int , Data >::const_iterator dit=caseData.begin();
      for(; dit!=caseData.end(); ++dit) {
        std::set< int > sngl;
        sngl.insert(dit->first);
        caseSets.insert(sngl);
      }
      return;
    }
    // setup reverse map
    std::map< Data , std::set<int> > rmap;
    typename std::map< int , Data >::const_iterator dit=caseData.begin();
    for(; dit!=caseData.end(); ++dit)
      rmap[dit->second].insert(dit->first);
    // strip result
    typename std::map< Data , std::set<int> >::iterator rit=rmap.begin();
    for(; rit!=rmap.end(); ++rit)
      caseSets.insert(rit->second);
  }

 };


#endif
