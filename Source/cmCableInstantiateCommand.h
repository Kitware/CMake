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
#ifndef cmCableInstantiateCommand_h
#define cmCableInstantiateCommand_h

#include "cmStandardIncludes.h"
#include "cmCablePackageEntryCommand.h"

/** \class cmCableInstantiateCommand
 * \brief Define a command that generates a rule for explicit template
 * instantiations.
 *
 * cmCableInstantiateCommand is used to generate a rule in a CABLE
 * configuration file to create explicit template instantiations.
 */
class cmCableInstantiateCommand : public cmCablePackageEntryCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmCableInstantiateCommand;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CABLE_INSTANTIATE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Define CABLE InstantiationSet in a package.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABLE_INSTANTIATE(member1 member2 ...)\n"
      "Generates an InstantiationSet in the CABLE configuration.  It is\n"
      "assumed that all members of the set are explicit instantiations of\n"
      "template non-classes (functions, operators, etc).";
    }

  virtual bool WriteConfiguration();

  cmTypeMacro(cmCableInstantiateCommand, cmCablePackageCommand);
protected:
  typedef cmCablePackageEntryCommand::Entries  Entries;
};



#endif
