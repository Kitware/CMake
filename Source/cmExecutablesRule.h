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
#ifndef cmExecutablesRule_h
#define cmExecutablesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmExecutablesRule
 * \brief Defines a list of executables to build.
 *
 * cmExecutablesRule defines a list of executable (i.e., test)
 * programs to create.
 */
class cmExecutablesRule : public cmRuleMaker
{
public:
  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmExecutablesRule;
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
  virtual const char* GetName() { return "EXECUTABLES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a list of executables files.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "EXECUTABLES(file1 file2 ...)";
    }
};


#endif
