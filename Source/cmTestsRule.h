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
#ifndef cmTestsRule_h
#define cmTestsRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmTestsRule
 * \brief Specify a list of executables to build and which are 
 *        identified as tests.
 *
 * cmTestsRule specifies a list of executables to be built by CMake.
 * These executables are identified as tests. This rule is similar to
 * the EXECUTABLES() rule.
 *
 * \sa cmExecutablesRule
 */
class cmTestsRule : public cmRuleMaker
{
public:
  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmTestsRule;
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
  virtual const char* GetName() {return "TESTS";}

  /**
   * Succinct documentation.
   */
  virtual const char* TerseDocumentation() 
    {
    return "Add a list of executables files that are run as tests.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* FullDocumentation()
    {
    return
      "TESTS(file1 file2 ...)";
    }
};



#endif
