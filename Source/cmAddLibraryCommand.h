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
#ifndef cmLibrarysCommand_h
#define cmLibrarysCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmLibrarysCommand
 * \brief Defines a list of executables to build.
 *
 * cmLibrarysCommand defines a list of executable (i.e., test)
 * programs to create.
 */
class cmAddLibraryCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddLibraryCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ADD_LIBRARY";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add an library to the project that uses the specified srclists";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "ADD_LIBRARY(libname srclist srclist srclist ...)";
    }
  
  cmTypeMacro(cmAddLibraryCommand, cmCommand);
};


#endif
