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
#ifndef cmWin32LibrariesCommand_h
#define cmWin32LibrariesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmWin32LibrariesCommand
 * \brief Specify a list of libraries for Win32 platforms.
 *
 * cmWin32LibrariesCommand specifies a list of libraries for Win32 platforms
 * only. Both user and system libraries can be listed.
 */
class cmWin32LibrariesCommand  : public cmCommand
{
public:
  /**
   * Constructor.
   */
  cmWin32LibrariesCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmWin32LibrariesCommand ;
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
  virtual const char* GetName() { return "WIN32_LIBRARIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add libraries that are only used for Win32 programs.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "WIN32_LIBRARIES(library -lm ...)";
    }
  
  cmTypeMacro(cmWin32LibrariesCommand, cmCommand);
};



#endif
