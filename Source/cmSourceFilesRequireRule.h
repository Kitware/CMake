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
#ifndef cmSourceFilesRequireRule_h
#define cmSourceFilesRequireRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmSourceFilesRequireRule
 * \brief Add additional sources to the build if certain required files
 *        or CMake variables are defined.
 *
 * cmSourceFilesRequireRule conditionally adds source files to the
 * build if the specified files of CMake variables are defined.
 * This rule can be used to add source files that depend on external
 * packages or operating system features.
*/
class cmSourceFilesRequireRule : public cmRuleMaker
{
public:
  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmSourceFilesRequireRule;
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
  virtual const char* GetName() { return "SOURCE_FILES_REQUIRE";}

  /**
   * Succinct documentation.
   */
  virtual const char* TerseDocumentation() 
    {
    return "Add a list of source files if the required variables are set.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* FullDocumentation()
    {
    return
      "SOURCE_FILES_REQUIRE(var1 var2 ... SOURCES_BEGIN file1 file2 ...)";
    }
};


#endif
