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
#ifndef cmWin32LibrariesRule_h
#define cmWin32LibrariesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmWin32LibrariesRule
 * \brief Specify a list of libraries for Win32 platforms.
 *
 * cmWin32LibrariesRule specifies a list of libraries for Win32 platforms
 * only. Both user and system libraries can be listed.
 */
class cmWin32LibrariesRule  : public cmRuleMaker
{
public:
  /**
   * Constructor.
   */
  cmWin32LibrariesRule();

  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmWin32LibrariesRule ;
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
  virtual const char* GetName() { return "WIN32_LIBRARIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add libraries that are only used for Win32 programs.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "WIN32_LIBRARIES(library -lm ...)";
    }
};



#endif
