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
#ifndef cmProjectCommand_h
#define cmProjectCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmProjectCommand
 * \brief Specify the name for this build project.
 *
 * cmProjectCommand is used to specify a name for this build project.
 * It is defined once per set of CMakeList.txt files (including
 * all subdirectories). Currently it just sets the name of the workspace
 * file for Microsoft Visual C++
 */
class cmProjectCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmProjectCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "PROJECT";}

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() 
    {
    return true;
    }

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Set a name for the entire project. One argument.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "PROJECT(projectname) Sets the name of the Microsoft workspace .dsw file. Does nothing on UNIX currently\n";
    }
};



#endif
