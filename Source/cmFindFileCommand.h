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
#ifndef cmFindFileCommand_h
#define cmFindFileCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmFindFileCommand
 * \brief Define a command to search for an executable program.
 *
 * cmFindFileCommand is used to define a CMake variable
 * that specifies an executable program. The command searches 
 * in the current path (e.g., PATH environment variable) for
 * an executable that matches one of the supplied names.
 */
class cmFindFileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmFindFileCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() { return true;  }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "FIND_FILE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Find a file.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "FIND_FILE(NAME file extrapath extrapath ...)";
    }
  
  cmTypeMacro(cmFindFileCommand, cmCommand);
};



#endif
