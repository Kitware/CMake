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
#ifndef cmSourceFilesCommand_h
#define cmSourceFilesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmSourceFilesCommand
 * \brief Add source files to the build.
 *
 * cmSourceFilesCommand adds source files to the build. The source
 * files will be added to the current library (if defined by the
 * LIBRARY(library) command. Use this command to add source files not
 * dependent on other packages (use SOURCE_FILES_REQUIRED() to add
 * dependent source files).
 *
 * \sa cmSourceFilesRequireCommand
 */
class cmSourceFilesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmSourceFilesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "SOURCE_FILES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a list of source files.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "SOURCE_FILES(file1 file2 ...)";
    }
  
  cmTypeMacro(cmSourceFilesCommand, cmCommand);
};



#endif
