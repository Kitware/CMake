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
#ifndef cmUnixDefinesRule_h
#define cmUnixDefinesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"

/** \class cmUnixDefinesRule
 * \brief Specify a list of compiler defines for Unix platforms.
 *
 * cmUnixDefinesRule specifies a list of compiler defines for Unix platforms
 * only. This defines will be added to the compile command.
 */
class cmUnixDefinesRule : public cmRuleMaker
{
public:
  /**
   * Constructor.
   */
  cmUnixDefinesRule();

  /**
   * This is a virtual constructor for the rule.
   */
  virtual cmRuleMaker* Clone() 
    {
    return new cmUnixDefinesRule;
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
  virtual const char* GetName() { return "UNIX_DEFINES";}

  /**
   * Succinct documentation.
   */
  virtual const char* TerseDocumentation() 
    {
    return "Add -D flags to the command line for Unix only.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* FullDocumentation()
    {
    return
      "UNIX_DEFINES(-DFOO -DBAR)\n"
      "Add -D flags to the command line for Unix only.";
    }
};



#endif
