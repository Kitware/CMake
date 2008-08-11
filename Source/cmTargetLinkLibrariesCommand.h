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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "target_link_libraries";}

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
      "  target_link_libraries(<target> [INTERFACE]\n"
      "                        [[debug|optimized|general] <lib>] ...)\n"
      "Specify a list of libraries to be linked into the specified target.  "
      "If any library name matches that of a target in the current project "
      "a dependency will automatically be added in the build system to make "
      "sure the library being linked is up-to-date before the target links."
      "\n"
      "A \"debug\", \"optimized\", or \"general\" keyword indicates that "
      "the library immediately following it is to be used only for the "
      "corresponding build configuration.  "
      "The \"debug\" keyword corresponds to the Debug configuration.  "
      "The \"optimized\" keyword corresponds to all other configurations.  "
      "The \"general\" keyword corresponds to all configurations, and is "
      "purely optional (assumed if omitted).  "
      "Higher granularity may be achieved for per-configuration rules "
      "by creating and linking to IMPORTED library targets.  "
      "See the IMPORTED mode of the add_library command for more "
      "information.  "
      "\n"
      "Library dependencies are transitive by default.  "
      "When this target is linked into another target then the libraries "
      "linked to this target will appear on the link line for the other "
      "target too.  "
      "See the LINK_INTERFACE_LIBRARIES target property to override the "
      "set of transitive link dependencies for a target."
      "\n"
      "The INTERFACE option tells the command to append the libraries "
      "to the LINK_INTERFACE_LIBRARIES and LINK_INTERFACE_LIBRARIES_DEBUG "
      "target properties instead of using them for linking.  "
      "Libraries specified as \"debug\" are appended to the "
      "the LINK_INTERFACE_LIBRARIES_DEBUG property.  "
      "Libraries specified as \"optimized\" are appended to the "
      "the LINK_INTERFACE_LIBRARIES property.  "
      "Libraries specified as \"general\" (or without any keyword) are "
      "appended to both properties."
      ;
    }
  
  cmTypeMacro(cmTargetLinkLibrariesCommand, cmCommand);
private:
  void LinkLibraryTypeSpecifierWarning(int left, int right);
  static const char* LinkLibraryTypeNames[3];

  cmTarget* Target;
  bool DoingInterface;

  void HandleLibrary(const char* lib, cmTarget::LinkLibraryType llt);
};



#endif
