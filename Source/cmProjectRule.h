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
#ifndef cmProjectRule_h
#define cmProjectRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmProjectRule
 * \brief Specify the name for this build project.
 *
 * cmProjectRule is used to specify a name for this build project.
 * It is defined once per set of CMakeList.txt files (including
 * all subdirectories).
 */
class cmProjectRule : public cmRuleMaker
{
public:
  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmProjectRule;
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
  virtual const char* GetName() {return "PROJECT";}

  /**
   * Succinct documentation.
   */
  virtual const char* TerseDocumentation() 
    {
    return "Set a name for the entire project. One argument.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* FullDocumentation()
    {
    return
      "PROJECT(projectname)\n"
      "Set the name for the entire project.  This takes one argument.";
    }
};



#endif
