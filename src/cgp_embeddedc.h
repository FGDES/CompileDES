/** @file cgp_embeddedc.h @brief Code-generator for target C */

/*
  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2010, 2016 Thomas Moor

*/



#ifndef FAUDES_EMBEDDEDC_H
#define FAUDES_EMBEDDEDC_H

#include "libfaudes.h"
#include "cgp_codeprimitives.h"

using namespace faudes;



/**
 * @brief Implementation of code primitives by generic C-code
 *
 * Since the CodePrimitives have been designed with C in mind, the
 * implementation as C-code is straight forward. Regarding top-level code organisation,
 * EmbeddedcCodeGenerator will produce global variable declarations along side
 * with the two functions
 *
 * @code
 * void PREFIX_cyclic(void);
 * void PREFIX_timerdec(int);
 * @endcode
 *
 * where <tt>PREFIX</tt> is specified by the configuration. The prototypical use-case is
 * to include the generated code in a host C-source with an entry-point <tt>main()</tt> to
 * periodically invoke <tt>PREFIX_cycle()</tt> to sense and execute events as well as
 * <tt>PREFIX_timerdec(int)</tt> to pass on the elapsed physical time. Example
 *
 * @code
 * #include <.. time includes ..>
 * #include "generated_code.c"
 *
 * int main(void){
 *
 *   [.. time type ..] recent = [.. get system time ..];
 *
 *   while(1) {
 *     PREFIX_cyclic();
 *     [.. time type ..] now = [.. get system time ..];
 *     int elapsed = [.. time op difference ..](now,recent);
 *     recent = now;
 *     PREFIX_timerdec(elapsed);
 *   }
 *
 *   return 0;
 * }
 * @endcode
 *
 * In particular, time is of an abstract unit in the context of the configuration. The relation to an
 * actual physical unit is established by the conversion of system-time duration to the C-type <tt>int</tt>
 * for the assignment of <tt>elapsed</tt>.
 *
 * For convenience, the configuration options <tt>IncludeBefore</tt> and <tt>IncludeAfter</tt> can be
 * used to generate the entire code in one self contained file. See the provided example @ref blink_atm.cgc for a complete
 * example configuration.
 *
 * @ingroup CGClasses
 */

class EmbeddedcCodeGenerator : public CodePrimitives {

public:

  /*****************************************
   *****************************************
   *****************************************
   *****************************************/

  /** @name Basic Class Maintenance */
  /** @{ */

  /**
   * @brief Constructor
   */
  EmbeddedcCodeGenerator(void);

  /**
   * @brief Explicit destructor.
   */
  virtual ~EmbeddedcCodeGenerator(void);

  /**
   * @brief Clear all data.
   *
   */
  virtual void Clear(void);


  /** @} */

protected:

  /*! add my preferences to DoCompile */
  void DoCompile(void);

  /*! virtual hook for generate */
  void DoGenerate(void);

  /*! re-implemented/additional code blocks */
  virtual void DecrementTimers(void);
  void InsertExecHooks(void);

  /*! abstract address conversion */
  virtual std::string TargetAddress(const AA& address);

  /*! abstract address conversion */
  virtual AX TargetExpression(const AA& address);

  /**
   * @name Re-Implement Primitives
   */
   /** @{ */


  /* re-implement primitives */
  virtual void Comment(const std::string& text);

  /* re-implement primitives */
  virtual void IntegerDeclare(const AA& address);
  virtual void IntegerDeclare(const AA& address, int val);
  virtual void IntegerAssign(const AA& address, int val);
  virtual void IntegerAssign(const AA& address, const AX& expression);
  virtual void IntegerIncrement(const AA& address, int val=1);
  virtual AX IntegerQuotient(const AX& expression, int val);
  virtual AX IntegerRemainder(const AX& expression, int val);
  virtual AX IntegerBitmask(const AX& expression);
  virtual AX IntegerIsEq(const AA& address, int val);
  virtual AX IntegerIsEq(const AA& address, const AX& expression);
  virtual AX IntegerIsNotEq(const AA& address, int val);
  virtual AX IntegerIsNotEq(const AA& address, const AX& expression);
  virtual AX IntegerIsGreater(const AA& address, int val);
  virtual AX IntegerIsLess(const AA& address, int val);
  virtual AX IntegerConstant(int val);

  /* re-implement primitives */
  virtual bool HasIntmaths(void);
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
  virtual void WordNand(const AA& address, const AX& expression);
  virtual AX WordIsBitSet(const AA& address, int idx);
  virtual AX WordIsBitClr(const AA& address, int idx);
  virtual AX WordIsMaskSet(const AA& address, word_t mask);
  virtual AX WordIsEq(const AA& address, word_t val);
  virtual AX WordIsNotEq(const AA& address, word_t val);
  virtual AX WordConstant(word_t val);

  /* re-implement primitives */
  virtual AX StringConstant(const std::string& val);

  /* re-implement primitives */
  virtual void CintarrayDeclare(const AA& address, int offset, const std::vector<int>& val);
  virtual AA CintarrayAccess(const AA& address, int index);
  virtual AA CintarrayAccess(const AA& address, const AA& indexaddr);
  virtual bool HasCintarray(void);
  virtual void CwordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val);
  virtual AA CwordarrayAccess(const AA& address, int index);
  virtual AA CwordarrayAccess(const AA& address, const AA& indexaddr);
  virtual bool HasCwordarray(void);
  virtual void CstrarrayDeclare(const AA& address, int offset, const std::vector<std::string>& val);
  virtual AA CstrarrayAccess(const AA& address, int index);
  virtual AA CstrarrayAccess(const AA& address, const AA& indexaddr);
  virtual bool HasCstrarray(void);
  virtual void IntarrayDeclare(const AA& address, int offset, int len);
  virtual void IntarrayDeclare(const AA& address, int offset, const std::vector<int>& val);
  virtual AA IntarrayAccess(const AA& address, int index);
  virtual AA IntarrayAccess(const AA& address, const AA& indexaddr);
  virtual bool HasIntarray(void);
  virtual void WordarrayDeclare(const AA& address, int offset, int len);
  virtual void WordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val);
  virtual AA WordarrayAccess(const AA& address, int index);
  virtual AA WordarrayAccess(const AA& address, const AA& indexaddr);
  virtual bool HasWordarray(void);


  /* re-implement primitives */
  virtual void IfTrue(const AX& expression);
  virtual void IfFalse(const AX& expression);
  virtual void IfWord(const AX& expression);
  virtual void IfElse(void);
  virtual void IfElseIfTrue(const AX& expression);
  virtual void IfEnd(void);

  /* re-implement primitives */
  virtual void SwitchBegin(const AA& address);
  virtual void SwitchCase(const AA& address, int val);
  virtual void SwitchCases(const AA& address, int from, int to);
  virtual void SwitchCases(const AA& address, const std::set< int>& vals);
  virtual void SwitchBreak(void);
  virtual void SwitchEnd(void);
  virtual bool HasMultiCase(void);

  /* re-implement primitives */
  virtual void LoopBegin(void);
  virtual void LoopBreak(const AX& expression);
  virtual void LoopEnd(void);
  virtual void FunctionReturn(void);

  /* re-implement primitives */
  virtual void RunActionSet(const std::string& address);
  virtual void RunActionClr(const std::string& address);
  virtual void RunActionExe(const AX& expression);

  /* re-implement primitives */
  virtual void TimerDeclare(const AA& address, const std::string& litval);
  virtual void TimerStart(const AA& address);
  virtual void TimerStop(const AA& address);
  virtual void TimerReset(const AA& address, const std::string& litval);
  virtual AX TimerIsElapsed(const AA& address);

  /** @} */

  // code generator primitives, embeddedc-only helper
  AX IntarrayConstant(int offset, const std::vector<int>& val);
  AX WordarrayConstant(int offset, const std::vector<word_t>& val);
  AX StrarrayConstant(int offset, const std::vector<std::string>& val);



  /**
   * @brief File i/o
   *
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
 
  /**
   * @brief File i/o
   *
   * Write global configuration to TokenWriter, excl. label
   *
   * @param rTw
   *   Reference to TokenWriter
   *
   * @exception Exception 
   *   - IO errors (id 2)
   */
  virtual void DoWriteTargetConfiguration(TokenWriter& rTw) const;

};


#endif
