/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLinkDirectoriesCommand_h
#define cmLinkDirectoriesCommand_h

#include "cmCommand.h"

/** \class cmLinkDirectoriesCommand
 * \brief Define a list of directories containing files to link.
 *
 * cmLinkDirectoriesCommand is used to specify a list
 * of directories containing files to link into executable(s). 
 * Note that the command supports the use of CMake built-in variables 
 * such as CMAKE_BINARY_DIR and CMAKE_SOURCE_DIR.
 */
class cmLinkDirectoriesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmLinkDirectoriesCommand;
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
  virtual const char* GetName() { return "link_directories";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Specify directories in which the linker will look for libraries.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  link_directories(directory1 directory2 ...)\n"
      "Specify the paths in which the linker should search for libraries. "
      "The command will apply only to targets created after it is called. "
      "For historical reasons, relative paths given to this command are "
      "passed to the linker unchanged "
      "(unlike many CMake commands which interpret them relative to the "
      "current source directory).\n"
      "Note that this command is rarely necessary.  Library locations "
      "returned by find_package() and find_library() are absolute paths.  "
      "Pass these absolute library file paths directly to the "
      "target_link_libraries() command.  CMake will ensure the linker finds "
      "them."
      ;
    }
  
  cmTypeMacro(cmLinkDirectoriesCommand, cmCommand);
private:
  void AddLinkDir(std::string const& dir);
};



#endif
