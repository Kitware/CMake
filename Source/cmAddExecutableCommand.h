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
#ifndef cmExecutablesCommand_h
#define cmExecutablesCommand_h

#include "cmCommand.h"

/** \class cmExecutablesCommand
 * \brief Defines a list of executables to build.
 *
 * cmExecutablesCommand defines a list of executable (i.e., test)
 * programs to create.
 */
class cmAddExecutableCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddExecutableCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ADD_EXECUTABLE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add an executable to the project using the specified source files.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ADD_EXECUTABLE(exename [WIN32] [MACBUNDLE] source1\n"
      "                 source2 ... sourceN)\n"
      "This command adds an executable target to the current directory.  "
      "The executable will be built from the list of source files "
      "specified.\n"
      "After specifying the executable name, WIN32 and/or MACBUNDLE can "
      "be specified. WIN32 indicates that the executable (when compiled on "
      "windows) is a windows app (using WinMain) not a console app (using main). "
      "MACBUNDLE indicates that when build on Mac OSX, executable should be in "
      "the bundle form. The MACBUNDLE also allows several variables to be specified:\n"
      "  MACOSX_BUNDLE_INFO_STRING\n"
      "  MACOSX_BUNDLE_ICON_FILE\n"
      "  MACOSX_BUNDLE_GUI_IDENTIFIER\n"
      "  MACOSX_BUNDLE_LONG_VERSION_STRING\n"
      "  MACOSX_BUNDLE_BUNDLE_NAME\n"
      "  MACOSX_BUNDLE_SHORT_VERSION_STRING\n"
      "  MACOSX_BUNDLE_BUNDLE_VERSION\n"
      "  MACOSX_BUNDLE_COPYRIGHT\n"
      ;
    }
  
  cmTypeMacro(cmAddExecutableCommand, cmCommand);
};


#endif
