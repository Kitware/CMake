/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFindLibraryCommand_h
#define cmFindLibraryCommand_h

#include "cmFindBase.h"


/** \class cmFindLibraryCommand
 * \brief Define a command to search for a library.
 *
 * cmFindLibraryCommand is used to define a CMake variable
 * that specifies a library. The command searches for a given
 * file in a list of directories.
 */
class cmFindLibraryCommand : public cmFindBase
{
public:
  cmFindLibraryCommand();
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmFindLibraryCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const {return "find_library";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Find a library.";
    }
  cmTypeMacro(cmFindLibraryCommand, cmFindBase);
  
protected:
  void AddArchitecturePaths(const char* suffix);
  void AddArchitecturePath(std::string const& dir,
                           std::string::size_type start_pos,
                           const char* suffix,
                           bool fresh = true);
  std::string FindLibrary();
  virtual void GenerateDocumentation();
private:
  std::string FindNormalLibrary();
  std::string FindFrameworkLibrary();
};



#endif
