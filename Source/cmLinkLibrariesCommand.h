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
#ifndef cmLinkLibrariesCommand_h
#define cmLinkLibrariesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmLinkLibrariesCommand
 * \brief Specify a list of libraries to link into executables.
 *
 * cmLinkLibrariesCommand is used to specify a list of libraries to link
 * into executable(s) or shared objects. The names of the libraries
 * should be those defined by the LIBRARY(library) command(s).  
 */
class cmLinkLibrariesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmLinkLibrariesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return true;}
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "LINK_LIBRARIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Link libraries to all targets added later.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  LINK_LIBRARIES(library1 <debug | optimized> library2 ...)\n"
      "Specify a list of libraries to be linked into "
      "any following targets (typically added with the ADD_EXECUTABLE "
      "or ADD_LIBRARY calls).  This command is passed "
      "down to all subdirectories.  "
      "The debug and optimized strings may be used to indicate that "
      "the next library listed is to be used only for that specific "
      "type of build.  Considure using TARGET_LINK_LIBRARIES for more "
      "specific linking control.";
    }
  
  cmTypeMacro(cmLinkLibrariesCommand, cmCommand);
};



#endif
