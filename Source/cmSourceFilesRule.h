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
#ifndef cmSourceFilesRule_h
#define cmSourceFilesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmSourceFilesRule
 * \brief Add source files to the build.
 *
 * cmSourceFilesRule adds source files to the build. The source
 * files will be added to the current library (if defined by the
 * LIBRARY(library) rule. Use this rule to add source files not
 * dependent on other packages (use SOURCE_FILES_REQUIRED() to add
 * dependent source files).
 *
 * \sa cmSourceFilesRequireRule
 */
class cmSourceFilesRule : public cmRuleMaker
{
public:
  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmSourceFilesRule;
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
  virtual const char* GetName() { return "SOURCE_FILES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a list of source files.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "SOURCE_FILES(file1 file2 ...)";
    }
};



#endif
