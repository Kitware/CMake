/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#ifndef cmRuleMaker_h
#define cmRuleMaker_h

#include "cmStandardIncludes.h"
#include "cmMakefile.h"

/** \class cmRuleMaker
 * \brief Superclass for all rules in CMake.
 *
 * cmRuleMaker is the base class for all rules in CMake.
 * cmRuleMaker defines the API for rules with such features
 * as enable/disable, inheritance, documentation, and construction.
 */
class cmRuleMaker
{
public:
  /**
   * Construct the rule enabled with no makefile.
   * the input file.
   */
  cmRuleMaker()  
    {m_Makefile = 0; m_Enabled = true;}

  /**
   * Specify the makefile.
   */
  void SetMakefile(cmMakefile*m) 
    {m_Makefile = m; }

  /**
   * This is called when the rule is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args) = 0;

  /**
   * This is called at the end after all the information
   * specified by the rules is accumulated.
   */
  virtual void FinalPass() = 0;

  /**
   * This is called to let the rule check the cache.
   */
  virtual void LoadCache() {}
  
  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() = 0;
  
  /**
   * This determines if the rule gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() 
    {
    return false;
    }

  /**
   * The name of the rule as specified in CMakeList.txt.
   */
  virtual const char* GetName() = 0;

  /**
   * Succinct documentation.
   */
  virtual const char* TerseDocumentation() = 0;

  /**
   * More documentation.
   */
  virtual const char* FullDocumentation() = 0;

  /**
   * Enable the rule.
   */
  void EnabledOn() 
    {m_Enabled = true;}

  /**
   * Disable the rule.
   */
  void EnabledOff() 
    {m_Enabled = false;}

  /**
   * Query whether the rule is enabled.
   */
  bool GetEnabled()  
    {return m_Enabled;}

  /**
   * Return the last error string.
   */
  const char* GetError() 
    {return m_Error.c_str();}

protected:
  void SetError(const char* e)
    {
    m_Error = this->GetName();
    m_Error += " ";
    m_Error += e;
    }
  cmMakefile* m_Makefile;

private:
  bool m_Enabled;
  std::string m_Error;
};

#endif
