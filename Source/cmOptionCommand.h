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
#ifndef cmOptionCommand_h
#define cmOptionCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmOptionCommand
 * \brief Provide an option to the user
 *
 * cmOptionCommand provides an option for the user to select
 */
class cmOptionCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmOptionCommand;
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
  virtual bool IsInherited() {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "OPTION";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Provides an option that the user can optionally select";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "OPTION(USE_MPI \"help string decribing the option\")\n"
      "Provide an option for the user to select";
    }
  
  cmTypeMacro(cmOptionCommand, cmCommand);
};



#endif
