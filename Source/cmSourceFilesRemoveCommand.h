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
#ifndef cmSourceFilesRemoveCommand_h
#define cmSourceFilesRemoveCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmSourceFilesRemoveCommand
 * \brief Remove source files from the build.
 *
 * cmSourceFilesRemoveCommand removes source files from the build.
 * The sourcefiles will be removed from the current library (if defined by the
 * LIBRARY(library) command. This command is primarily designed to allow
 * customization of builds where (for example) a certain source file is
 * failing to compile and the user wishes to edit it out conditionally.
 *
 * \sa cmSourceFilesRequireCommand
 */
class cmSourceFilesRemoveCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmSourceFilesRemoveCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "SOURCE_FILES_REMOVE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Remove a list of source files - associated with NAME.";
    }

  /**
   * Remove (conditionally if desired) a list of source files from a group.
   * This is most likely when the user has problems compiling certain files
   * in a library and wants to remove them (perhaps in an optional INCLUDE)
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "SOURCE_FILES_REMOVE(NAME file1 file2 ...)";
    }
  
  cmTypeMacro(cmSourceFilesRemoveCommand, cmCommand);
};



#endif
