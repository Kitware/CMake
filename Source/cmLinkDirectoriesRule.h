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
#ifndef cmLinkDirectoriesRule_h
#define cmLinkDirectoriesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmLinkDirectoriesRule
 * \brief Define a list of directories containing files to link.
 *
 * cmLinkDirectoriesRule is used to specify a list
 * of directories containing files to link into executable(s). 
 * Note that the rule supports the use of CMake built-in variables 
 * such as CMAKE_BINARY_DIR and CMAKE_SOURCE_DIR.
 */
class cmLinkDirectoriesRule : public cmRuleMaker
{
public:
  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmLinkDirectoriesRule;
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
  virtual bool IsInherited() { return true;  }

  /**
   * The name of the rule as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "LINK_DIRECTORIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* TerseDocumentation() 
    {
    return "Specify link directories.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* FullDocumentation()
    {
    return
      "Specify the paths to the libraries that will be linked in.\n"
      "LINK_DIRECTORIES(directory1 directory2 ...)\n"
      "The directories can use built in definitions like \n"
      "CMAKE_BINARY_DIR and CMAKE_SOURCE_DIR.";
    }
};



#endif
