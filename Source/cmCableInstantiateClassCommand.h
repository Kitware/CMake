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
#ifndef cmCableInstantiateClassCommand_h
#define cmCableInstantiateClassCommand_h

#include "cmStandardIncludes.h"
#include "cmCablePackageEntryCommand.h"

/** \class cmCableInstantiateClassCommand
 * \brief Define a command that generates a rule for explicit template
 * instantiations of classes.
 *
 * cmCableInstantiateCommand is used to generate a rule in a CABLE
 * configuration file to create explicit template instantiations of
 * classes.
 */
class cmCableInstantiateClassCommand : public cmCablePackageEntryCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmCableInstantiateClassCommand;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CABLE_INSTANTIATE_CLASS";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Define CABLE InstantiationSet of classes in a package.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABLE_INSTANTIATE_CLASS(package_name member1 member2 ...)\n"
      "Generates an InstantiationSet in the CABLE configuration.  It is\n"
      "assumed that all members of the set are explicit instantiations of\n"
      "template classes (not functions, operators, etc).";
    }

  virtual bool WriteConfiguration();

  cmTypeMacro(cmCableInstantiateClassCommand, cmCableInstantiateCommand);
protected:
  typedef cmCablePackageEntryCommand::Entries  Entries;
};



#endif
