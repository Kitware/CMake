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
#ifndef cmUnixLibrariesRule_h
#define cmUnixLibrariesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmUnixLibrariesRule
 * \brief Specify a list of libraries for Unix platforms.
 *
 * cmUnixLibrariesRule specifies a list of libraries for Unix platforms
 * only. Both user and system libraries can be listed.
 */
class cmUnixLibrariesRule : public cmRuleMaker
{
public:
  /**
   * Constructor.
   */
  cmUnixLibrariesRule();

  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmUnixLibrariesRule;
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
  virtual const char* GetName() {return "UNIX_LIBRARIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* TerseDocumentation() 
    {
    return "Add libraries that are only used for Unix programs.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* FullDocumentation()
    {
    return
      "UNIX_LIBRARIES(library -lm ...)";
    }
};



#endif
