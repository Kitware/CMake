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
#ifndef cmWin32DefinesRule_h
#define cmWin32DefinesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmWin32DefinesRule
 * \brief Specify a list of compiler defines for Win32 platforms.
 *
 * cmWin32DefinesRule specifies a list of compiler defines for Win32 platforms
 * only. This defines will be added to the compile command.
 */
class cmWin32DefinesRule : public cmRuleMaker
{
public:
  /**
   * Constructor.
   */
  cmWin32DefinesRule();

  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmWin32DefinesRule;
    }

  /**
   * This is called when the rule is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * This is called at the end after all the information
   * specified by the rules is accumulated.
   */
  virtual void FinalPass() { }

  /**
   * This determines if the rule gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return true;}

  /**
   * The name of the rule as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "WIN32_DEFINES";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* TerseDocumentation() 
    {
    return "Add -D define flags to command line for Win32 environments.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* FullDocumentation()
    {
    return
      "WIN32_DEFINES(-DFOO -DBAR ...)\n"
      "Add -D define flags to command line for Win32 environments.";
    }
};



#endif
