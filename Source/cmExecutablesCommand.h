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
#ifndef cmExecutablesCommand_h
#define cmExecutablesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmExecutablesCommand
 * \brief Defines a list of executables to build.
 *
 * cmExecutablesCommand defines a list of executable (i.e., test)
 * programs to create.
 */
class cmExecutablesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmExecutablesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "EXECUTABLES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a list of executables files.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "EXECUTABLES(file1 file2 ...)";
    }
  
  cmTypeMacro(cmExecutablesCommand, cmCommand);
};


#endif
