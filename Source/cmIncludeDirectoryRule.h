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
#ifndef cmIncludeDirectoryRule_h
#define cmIncludeDirectoryRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmIncludeDirectoryRule
 * \brief Add include directories to the build.
 *
 * cmIncludeDirectoryRule is used to specify directory locations
 * to search for included files.
 */
class cmIncludeDirectoryRule : public cmRuleMaker
{
public:
  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmIncludeDirectoryRule;
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
  virtual const char* GetName() { return "INCLUDE_DIRECTORIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* TerseDocumentation() 
    {
    return "Add include directories to the build.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* FullDocumentation()
    {
    return
      "INCLUDE_DIRECTORIES(dir1 dir2 ...)";
    }
};



#endif
