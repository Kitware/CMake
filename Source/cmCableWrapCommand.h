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
#ifndef cmCableWrapCommand_h
#define cmCableWrapCommand_h

#include "cmStandardIncludes.h"
#include "cmCablePackageEntryCommand.h"

/** \class cmCableWrapCommand
 * \brief Define a command that generates a rule for CABLE-generated wrappers.
 *
 * cmCableWrapCommand is used to generate a rule in a CABLE
 * configuration file to create type wrappers.
 */
class cmCableWrapCommand : public cmCablePackageEntryCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmCableWrapCommand;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CABLE_WRAP";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Define CABLE WrapSet in a package.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABLE_WRAP(member1 member2 ...)\n"
      "Generates a WrapSet in the CABLE configuration.";
    }

  virtual bool WriteConfiguration();
  
  cmTypeMacro(cmCableWrapCommand, cmCablePackageCommand);
};



#endif
