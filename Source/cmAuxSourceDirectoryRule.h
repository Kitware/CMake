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
#ifndef cmAuxSourceDirectoryRule_h
#define cmAuxSourceDirectoryRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmAuxSourceDirectoryRule
 * \brief Specify auxiliary source code directories.
 *
 * cmAuxSourceDirectoryRule specifies source code directories
 * that must be built as part of this build process. This directories
 * are not recursively processed like the SUBDIR rule (cmSubdirRule).
 * A side effect of this rule is to create a subdirectory in the build
 * directory structure.
 */
class cmAuxSourceDirectoryRule : public cmRuleMaker
{
public:
  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmAuxSourceDirectoryRule;
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
   * The name of the rule as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "AUX_SOURCE_DIRECTORY";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add all the source files found in the specified\n"
           "directory to the build.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "AUX_SOURCE_DIRECTORY(dir)";
    }
};



#endif
