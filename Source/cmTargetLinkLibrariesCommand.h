/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmTargetLinkLibrariesCommand_h
#define cmTargetLinkLibrariesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmTargetLinkLibrariesCommand
 * \brief Specify a list of libraries to link into executables.
 *
 * cmTargetLinkLibrariesCommand is used to specify a list of libraries to link
 * into executable(s) or shared objects. The names of the libraries
 * should be those defined by the LIBRARY(library) command(s).  
 */
class cmTargetLinkLibrariesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmTargetLinkLibrariesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "TARGET_LINK_LIBRARIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return 
      "Link a target to given libraries.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  TARGET_LINK_LIBRARIES(target library1\n"
      "                        <debug | optimized> library2\n"
      "                        ...)\n"
      "Specify a list of libraries to be linked into the specified target "
      "The debug and optimized strings may be used to indicate that "
      "the next library listed is to be used only for that specific "
      "type of build";
    }
  
  cmTypeMacro(cmTargetLinkLibrariesCommand, cmCommand);
};



#endif
