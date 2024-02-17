/** @file cgp_atmega.h @brief Code-generator for ATmega microcontrollers */

/*
  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2016 Thomas Moor

*/



#ifndef FAUDES_ATMEGA_H
#define FAUDES_ATMEGA_H

#include "libfaudes.h"
#include "cgp_embeddedc.h"

using namespace faudes;



/**
 * @brief Target ATmega micro-controller (AVR8)
 *
 * This code generator is derived from EmbeddedcCodeGenerator to further
 * shape the output to fit ATMEL 8bit micro-controllers from the ATmega and ATtiny
 * series -- not the Xmega series, however, adaption should be straight forward.
 * The generated C-code compiles with the <tt>avr-gcc</tt> toolchain and addresses two target specific features.
 *
 *
 * The ATmegaCodeGenerator supports the use of native GPIO pin addresses in Set/Clr output actions and as input triggers;
 * e.g.,  <tt>PB3</tt> for port B pin 3. The generated code includes the additional function <tt>PREFIX_initpio()</tt> 
 * to initialise all referenced output pins by setting the corresponding flag in the DDR register.
 * 
 * The following additional code-options are supported.
 *
 * <strong>ATmegaPullups</strong>. When this option is set, the function <tt>PREFIX_initpio()</tt> will include code 
 * to enable the pullup option of all referenced input pins. Example:
 *
 * @verbatim
   <ATmegaPullups val="true"/>
   @endverbatim
 *
 * <strong>ATmegaProgmem</strong>. When this option is set, constant arrays like those used for precompiled transition
 * relations will reside in the program-memory eeprom as opposed to the RAM area. This introduces some performance
 * penalty, however, RAM is a bottleneck on ATmega devices. Example:
 * @verbatim
   <ATmegaProgmem val="true"/>
   @endverbatim
 *
 *
 * The below example for an Arduino Nano board illustrates a typical overall implementation pattern.
 *
 * @code
 * // device main clock (16MHz for ATmega328)
 * #define F_CPU 16000000UL  // 16MHz for Arduino Nano
 *
 * // std includes
 * #include <avr/io.h>
 * #include <avr/interrupt.h>
 * #include <avr/pgmspace.h>
 *
 * // include generated code
 * #include "blink_atm.c"
 *
 * // system time as 8bit value updated by 100Hz interrupt
 * volatile unsigned char systime;
 *
 * // 100Hz interrupt driven by timer0
 * ISR(TIMER0_COMPA_vect){
 *   ++systime;
 * }
 *
 * // entry point
 * int main(void){
 *   // aux var to figure cycle time
 *   unsigned char  systime_recent;
 *   // initialise system clock (ATmega328 specific)
 *   CLKPR = (1<<CLKPCE);   // enable pre-scaler write access
 *   CLKPR = 0x00;          // no clock pre-scale, i.e. F_CPU 16MHz
 *   // run timer0 to drive 100Hz interrupt (ATmega328)
 *   sei();                             // enable interrupts
 *   TCNT0=0;                           // reset count
 *   OCR0A=160;                         // set top to 160
 *   TIMSK0= (1 << OCIE0A);             // interrupt on match top
 *   TCCR0A= (1 << WGM01);              // clear-on-top mode
 *   TCCR0B= (1 << CS00) | (1 << CS02); // run at clk_IO/1024 i.e. 16kHz
 *   // initialise output pins by generated code
 *   PREFIX_initpio();
 *   // loop forever
 *   while(1) {
 *     // generated code to sense and execute events
 *     PREFIX_cyclic();
 *     // figure cycle time
 *     unsigned char now= systime;
 *     unsigned char dt= now-systime_recent;
 *     systime_recent+= dt;
 *     // generated code to update timers
 *     PREFIX_timerdec(dt);
 *   }
 *   return 0;
 * }
 * @endcode
 *
 *
 * The implementation of system time renders one time unit to correspond to 10ms. Note that the
 * the system time <tt>systime</tt> will overflow every 2.56sec, i.e., the cycle time must not
 * be any longer. Nevertheless, the generated timers can handle arbitrary long durations according
 * to their bit-size. The toy example @ref blink_atm.cgc was used for validation on an Arduino Nano board.
 *
 *
 * @ingroup CGClasses
 *
 */

class ATmegaCodeGenerator : public EmbeddedcCodeGenerator {

public:

  /*****************************************
   *****************************************
   *****************************************
   *****************************************/

  /** @name Basic Class Maintenance */
  /** @{ */
  /**
   * Constructor
   */
  ATmegaCodeGenerator(void);

  /**
   * Explicit destructor.
   */
  virtual ~ATmegaCodeGenerator(void);

  /**
   * Clear all data.
   *
   */
  virtual void Clear(void);


  /** @} */

protected:

  /*! ATmega code options */
  bool mATmegaProgmem;

  /*! ATmega code options */
  bool mATmegaPullups;

  /*! ATmega code options */
  std::string mATmegaPgmReadInteger;

  /*! ATmega code options */
  std::string mATmegaPgmReadWord;

  /*! add my preferences to DoCompile */
  void DoCompile(void);

  /*! protected version of generate */
  void DoGenerate(void);

  /*! reimplemented/additional code blocks */
  virtual void InitialisePorts(void);

  /** @name Re-Implemented Code Primitives */
  /** @{ */
  virtual AX TargetExpression(const AA& address);
  virtual std::string TargetAddress(const AA& address);
  void CintarrayDeclare(const AA& address, int offset, const std::vector<int>& val);
  AA CintarrayAccess(const AA& address, int index);
  AA CintarrayAccess(const AA& address, const AA& indexaddr);
  void CwordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val);
  AA CwordarrayAccess(const AA& address, int index);
  AA CwordarrayAccess(const AA& address, const AA& indexaddr);
  void CstrarrayDeclare(const AA& address, int offset, const std::vector<std::string>& val);
  AA CstrarrayAccess(const AA& address, int index);
  AA CstrarrayAccess(const AA& address, const AA& indexaddr);
  void RunActionSet(const std::string& address);
  void RunActionClr(const std::string& address);
  AX ReadInputLine(const std::string& address);
  /** @} */

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

