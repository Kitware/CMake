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
#ifndef cmSourceFilesRequireCommand_h
#define cmSourceFilesRequireCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmSourceFilesRequireCommand
 * \brief Add additional sources to the build if certain required files
 *        or CMake variables are defined.
 *
 * cmSourceFilesRequireCommand conditionally adds source files to the
 * build if the specified files of CMake variables are defined.
 * This command can be used to add source files that depend on external
 * packages or operating system features.
*/
class cmSourceFilesRequireCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmSourceFilesRequireCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "SOURCE_FILES_REQUIRE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a list of source files to the source file list NAME\n"
      "if the required variables are set.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "SOURCE_FILES_REQUIRE(var1 var2 ... SOURCES_BEGIN NAME file1 file2 ...)";
    }
  
  cmTypeMacro(cmSourceFilesRequireCommand, cmCommand);
};


#endif
