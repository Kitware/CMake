/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmFindLibraryCommand_h
#define cmFindLibraryCommand_h

#include "cmStandardIncludes.h"
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
      "FIND_LIBRARY(DEFINE_PATH libraryName [NAMES] name1 name2 name3 [PATHS path1 path2 path3...] [DOC docstring] )\n"
      "If the library is found, then DEFINE_PATH is set to the full path where it was found.  "
      "If DOC is specified the next argument is the "
      "documentation string for the cache entry NAME.";
    }
  
  cmTypeMacro(cmFindLibraryCommand, cmCommand);
};



#endif
