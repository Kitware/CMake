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
#ifndef cmIncludeRegularExpressionCommand_h
#define cmIncludeRegularExpressionCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmIncludeRegularExpressionCommand
 * \brief Set the regular expression for following #includes.
 *
 * cmIncludeRegularExpressionCommand is used to specify the regular expression
 * used by cmMakeDepend to determine whether to follow a #include file in
 * dependency checking.
 */
class cmIncludeRegularExpressionCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmIncludeRegularExpressionCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "INCLUDE_REGULAR_EXPRESSION";}

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() 
    {
    return true;
    }

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Set the regular expression used for dependency checking.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "INCLUDE_REGULAR_EXPRESSION(regex)\n"
      "Sets the regular expression used in dependency checking.  Only\n"
      "include files matching this regular expression will be traced.";
    }
  
  cmTypeMacro(cmIncludeRegularExpressionCommand, cmCommand);
};



#endif
