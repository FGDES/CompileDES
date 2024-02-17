/** @file cgp_gebtools.h @brief Code-generator GEB toolchain (IEC 61131-3 ST) */

/*
  FAU Discrete Event Systems Library (libFAUDES)

  Copyright (C) 2010, 2016 Thomas Moor

*/



#ifndef FAUDES_GEBTOOLS_H
#define FAUDES_GEBTOOLS_H

#include "libfaudes.h"
#include "cgp_iec61131st.h"

using namespace faudes;



/**
 * @brief Target GEB Automation toolchain (IEC 61131 ST)
 *
 * This code generator is a specialisation of the Iec61131stCodeGenerator tailored for
 * the <a href="http://www.gebautomation.com">GEB Automation IDE</a> development tools
 * (freely available for academic use, generates self-contained C-code, both MS Windows and Linux versions:
 * a great environment to evaluate CompileDES). The only specific adaption provided by GebtoolsCodeGenerator
 * is the use of explicit types for constant array initialisation. The example @ref blink_geb.cgc has been validated
 * within the GEB IDE simulator as well as on an embedded device (an Arduino Nano board, professional license of GEB
 * Automation IDE required, available at an affordable rate).
 *
 * @ingroup CGClasses
 */


class GebtoolsCodeGenerator : public Iec61131stCodeGenerator  {

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
  GebtoolsCodeGenerator(void);

  /**
   * @brief Explicit destructor.
   */
  virtual ~GebtoolsCodeGenerator(void);


  /**
   * @brief Clear all data.
   *
   */
  virtual void Clear(void);


  /** @} */

protected:

  /*! option: formal declaration of array constants */
  bool mIecTypedArrayConstants;

  /*! re-implement program block */
  void DoGenerateFunction(void);

  /*! re-implement program block */
  void DoGenerateLookups(void);

  /*! re-implement primitives (extra pass to generate type declarations) */
  virtual void CintarrayDeclare(const AA& address, int offset, const std::vector<int>& val);

  /*! re-implement primitives (extra pass to generate type declarations) */
  virtual void CwordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val);

  /*! re-implement primitives (extra pass to generate type declarations) */
  virtual void CstrarrayDeclare(const AA& address, int offset, const std::vector<std::string>& val);

  /*! re-implement primitives (extra pass to generate type declarations) */
  virtual void IntarrayDeclare(const AA& address, int offset, const std::vector<int>& val);

  /*! re-implement primitives (extra pass to generate type declarations) */
  virtual void IntarrayDeclare(const AA& address, int offset, int len);

  /*! re-implement primitives (extra pass to generate type declarations) */
  virtual void WordarrayDeclare(const AA& address, int offset, const std::vector<word_t>& val);

  /*! re-implement primitives (extra pass to generate type declarations) */
  virtual void WordarrayDeclare(const AA& address, int offset, int len);

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
