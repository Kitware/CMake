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
#ifndef cmAddTargetRule_h
#define cmAddTargetRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmAddTargetRule
 * \brief Rule that adds a target to the build system.
 *
 * cmAddTargetRule adds an extra target to the build system.
 * This is useful when you would like to add special
 * targets like "install,", "clean," and so on.
 */
class cmAddTargetRule : public cmRuleMaker
{
public:
  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmAddTargetRule;
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
  virtual void FinalPass() {}
  
  /**
   * The name of the rule as specified in CMakeList.txt.
   */
  virtual const char* GetName() 
    {return "ADD_TARGET";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* TerseDocumentation() 
    {
    return "Add an extra target to the build system.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* FullDocumentation()
    {
    return
      "ADD_TARGET(Name \"command to run\");";
    }
};

#endif
