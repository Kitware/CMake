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
#ifndef cmUnixLibrariesCommand_h
#define cmUnixLibrariesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmUnixLibrariesCommand
 * \brief Specify a list of libraries for Unix platforms.
 *
 * cmUnixLibrariesCommand specifies a list of libraries for Unix platforms
 * only. Both user and system libraries can be listed.
 */
class cmUnixLibrariesCommand : public cmCommand
{
public:
  /**
   * Constructor.
   */
  cmUnixLibrariesCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmUnixLibrariesCommand;
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
  virtual const char* GetName() {return "UNIX_LIBRARIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add libraries that are only used for Unix programs.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "UNIX_LIBRARIES(library -lm ...)";
    }
  
  cmTypeMacro(cmUnixLibrariesCommand, cmCommand);
};



#endif
