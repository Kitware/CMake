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
#ifndef cmFindLibraryCommand_h
#define cmFindLibraryCommand_h

#include "cmCommand.h"


/** \class cmFindLibraryCommand
 * \brief Define a command to search for a library.
 *
 * cmFindLibraryCommand is used to define a CMake variable
 * that specifies a library. The command searches for a given
 * file in a list of directories.
 */
class cmFindLibraryCommand : public cmCommand
{
public:
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
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return false;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "FIND_LIBRARY";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Find a library.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  FIND_LIBRARY(VAR libraryName\n"
      "               [NAMES name1 name2 ...]\n"
      "               [PATHS path1 path2 ...]\n"
      "               [DOC helpstring])\n"
      "Find the library by looking in paths specified in the command.  "
      "A cache entry named by VAR is created to "
      "store the result.  VAR-NOTFOUND is the value used if the library was "
      "not found.  CMake will continue to look as long as the value "
      "is not found.  If DOC is specified the next argument is the "
      "documentation string for the cache entry VAR.";
    }
  
  cmTypeMacro(cmFindLibraryCommand, cmCommand);
};



#endif
