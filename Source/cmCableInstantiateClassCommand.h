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
#ifndef cmCabilInstantiateClassCommand_h
#define cmCabilInstantiateClassCommand_h

#include "cmStandardIncludes.h"
#include "cmCabilInstantiateCommand.h"

/** \class cmCabilInstantiateClassCommand
 * \brief Define a command that generates a rule for explicit template
 * instantiations of classes.
 *
 * cmCabilInstantiateCommand is used to generate a rule in a CABIL
 * configuration file to create explicit template instantiations of
 * classes.
 */
class cmCabilInstantiateClassCommand : public cmCabilInstantiateCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmCabilInstantiateClassCommand;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CABIL_INSTANTIATE_CLASS";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Define CABIL InstantiationSet of classes.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABIL_INSTANTIATE_CLASS(cabil_config_file member1 member2 ...)\n"
      "Generates an InstantiationSet in the CABIL configuration.  It is\n"
      "assumed that all members of the set are explicit instantiations of\n"
      "template classes (not functions, operators, etc).";
    }

  virtual void WriteConfiguration(std::ostream&) const;  

  cmTypeMacro(cmCabilInstantiateClassCommand, cmCabilInstantiateCommand);
protected:
  typedef cmCabilInstantiateCommand::Elements  Elements;
};



#endif
