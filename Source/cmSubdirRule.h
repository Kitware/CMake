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
#ifndef cmSubdirRule_h
#define cmSubdirRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmSubdirRule
 * \brief Specify a list of subdirectories to build.
 *
 * cmSubdirRule specifies a list of subdirectories to process
 * by CMake. For each subdirectory listed, CMake will descend
 * into that subdirectory and process any CMakeLists.txt found.
 */
class cmSubdirRule : public cmRuleMaker
{
public:
  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmSubdirRule;
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
  virtual const char* GetName() { return "SUBDIRS";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a list of subdirectories to the build.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "SUBDIRS(dir1 dir2 ...)\n"
      "Add a list of subdirectories to the build.\n"
      "This will cause any CMakeLists.txt files in the sub directories\n"
      "to be processed by CMake.";
    }
};



#endif
