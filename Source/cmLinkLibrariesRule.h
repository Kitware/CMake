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
#ifndef cmLinkLibrariesRule_h
#define cmLinkLibrariesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmLinkLibrariesRule
 * \brief Specify a list of libraries to link into executables.
 *
 * cmLinkLibrariesRule is used to specify a list of libraries to link
 * into executable(s) or shared objects. The names of the libraries
 * should be those defined by the LIBRARY(library) rule(s).  
 */
class cmLinkLibrariesRule : public cmRuleMaker
{
public:
  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmLinkLibrariesRule;
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
  virtual const char* GetName() { return "LINK_LIBRARIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* TerseDocumentation() 
    {
    return 
      "Specify a list of libraries to be linked into executables or \n"
      "shared objects.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* FullDocumentation()
    {
    return
      "Specify a list of libraries to be linked into executables or \n"
      "shared objects.  This rule is passed down to all other rules."
      "LINK_LIBRARIES(library1 library2).\n"
      "The library name should be the same as the name used in the\n"
      "LIBRARY(library) rule.";
    }
};



#endif
