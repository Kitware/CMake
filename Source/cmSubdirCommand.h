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
#ifndef cmSubdirCommand_h
#define cmSubdirCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmSubdirCommand
 * \brief Specify a list of subdirectories to build.
 *
 * cmSubdirCommand specifies a list of subdirectories to process
 * by CMake. For each subdirectory listed, CMake will descend
 * into that subdirectory and process any CMakeLists.txt found.
 */
class cmSubdirCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmSubdirCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "SUBDIRS";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a list of subdirectories to the build.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "SUBDIRS(dir1 dir2 ...)\n"
      "Add a list of subdirectories to the build.\n"
      "This will cause any CMakeLists.txt files in the sub directories\n"
      "to be processed by CMake.";
    }
  
  cmTypeMacro(cmSubdirCommand, cmCommand);
};



#endif
