/** @file cgp_kinetis.h @brief Code-generator for Freescale Kinetis microcontrollers */

/*
  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2016 Thomas Moor

*/



#ifndef FAUDES_KINETIS_H
#define FAUDES_KINETIS_H

#include "libfaudes.h"
#include "cgp_embeddedc.h"

using namespace faudes;



/**
 * @brief Target Freescale Kinetis micro-controllers (K20)
 *
 * This code generator is derived from EmbeddedcCodeGenerator to further
 * shape the output to fit Freescale Kinetis K20 microcontrollers.
 * The generated C-code compiles with the <tt>arm-none-eabi-gcc</tt> toolchain
 * and requires appropriate headers and/or additional support code for device initialisation
 * and symbolic register access.
 *
 * The KinetisCodeGenerator requires the word data type to be a 32bit integer and sets the default integer data type
 * to a signed 16 bit integer. Regarding digital i/o, the configuration supports the use of Kinetis K20 native 
 * GPIO pin addresses in Set/Clr output actions and as input triggers. If either of the below control parameters are
 * set, the generated code includes the additional function <tt>PREFIX_initpio()</tt> to initialise all referenced
 * input and output pins accordingly.
 * 
 * The following additional code-options are supported.
 *
 * <strong>KinetisOutputControl</strong>. When this option is set to a non-empty string, the function
 * <tt>PREFIX_initpio()</tt> will include code to set all GPIO port-control-registers (PORTx_PCRn) referenced by
 * Set/Clr output actions accordingly. In addition, the ports will be configured as outputs by setting the
 * respective flags in the register PORTx_PDDR. Example: for slow slew-rate and high drive strength use
 * the following target-configuration option:
 *
 * @verbatim
   <KinetisOutputControl val="PORT_PCR_SRE | PORT_PCR_DSE"/>
   @endverbatim
 *
 * <strong>KinetisInputControl</strong>. When this option is set to a non-empty string, the function
 * <tt>PREFIX_initpio()</tt> will include code to set the GPIO port-configuration-register referenced by
 * input triggers accordingly. E.g., for passive filter and pullups use
 * the following target-configuration option:
 *
 * @verbatim
   <KinetisInputControl val="PORT_PCR_PFE | PORT_PCR_PE | PORT_PCR_PS"/>
   @endverbatim
 *
 *
 *
 * The following example for a Teensy 3.2 board illustrates a typical overall implementation pattern.
 *
 * @code
 * // K20 includes
 * #include "mk20dx256.h">
 *
 * // include generated code
 * #include "blink_k20.c"
 *
 * // entry point 
 * int main(void){
 *   // aux var to figure 100Hz cycle time
 *   unsigned long systime_recent;
 *   // use timer 3 for system clock = 0;
 *   SIM_SCGC6 |= SIM_SCGC6_PIT;// enable timer module
 *   PIT_MCR = 0x00;            // enable timer
 *   PIT_LDVAL3 =  20L * F_BUS; // 20sec, typ. F_BUS=36000000
 *   PIT_TCTRL3 |= 0x01;        // start Timer 3
 *   // initialise input-pins and output-pins by generated code
 *   PREFIX_initpio();
 *   // loop forever
 *   while(1) {
 *     // generated code to sense and execute events
 *     PREFIX_cyclic();
 *     // figure cycle time
 *     unsigned long now = PIT_CVAL3;
 *     unsigned long dec = 0;
 *     if(systime_recent < now) systime_recent += 20L * F_BUS;
 *     if(systime_recent - now >= (F_BUS / 100)) {
 *       dec=(systime_recent-now)/(F_BUS/100);
 *       systime_recent -= dec * (F_BUS / 100);
 *     }
 *     // generated code to update timers
 *     PREFIX_timerdec(dec);
 *   }
 *   return 0;
 * }
 * @endcode
 *
 *
 * The implementation of system time renders one time unit to correspond to 10ms. The toy example
 * @ref blink_k20.cgc was used for validation on an Teensy 3.2 board.
 *
 * @ingroup CGClasses
 *
 */

class KinetisCodeGenerator : public EmbeddedcCodeGenerator {

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
  KinetisCodeGenerator(void);

  /**
   * Explicit destructor.
   */
  virtual ~KinetisCodeGenerator(void);

  /**
   * Clear all data.
   *
   */
  virtual void Clear(void);


  /** @}  */

protected:

  /*! Kinetis code options */
  std::string mKinetisOutputControl;

  /*! Kinetis code options */
  std::string mKinetisInputControl;

  /*! add my preferences to DoCompile */
  void DoCompile(void);

  /*! protected version of generate */
  void DoGenerate(void);

  /*! reimplemented/additional code blocks */
  virtual void InitialisePorts(void);

  /** @name Re-Implemented Code Primitives */
  /** @{ */
  void RunActionSet(const std::string& address);
  void RunActionClr(const std::string& address);
  AX ReadInputLine(const std::string& address);
  std::string ParseLiteralPort(const std::string& port);
  /** @} */

  /*! reimplemented/additional code blocks */

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

