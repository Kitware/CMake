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
#ifndef cmWin32IncludeDirectoryCommand_h
#define cmWin32IncludeDirectoryCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmWin32IncludeDirectoryCommand
 * \brief Add Win32 include directories to the build.
 *
 * cmWin32IncludeDirectoryCommand is used to specify directory locations
 * to search for included files under a Windows system.
 */
class cmWin32IncludeDirectoryCommand : public cmCommand
{
public:
  /**
   * Constructor
   */
  cmWin32IncludeDirectoryCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmWin32IncludeDirectoryCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "WIN32_INCLUDE_DIRECTORIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add Win32 include directories to the build.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "WIN32_INCLUDE_DIRECTORIES(dir1 dir2 ...)";
    }
  
  cmTypeMacro(cmWin32IncludeDirectoryCommand, cmCommand);
};



#endif
