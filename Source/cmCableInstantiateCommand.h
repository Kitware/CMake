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
#include "cmCableCommand.h"

/** \class cmCableInstantiateCommand
 * \brief Define a command that generates a rule for explicit template
 * instantiations.
 *
 * cmCableInstantiateCommand is used to generate a rule in a CABLE
 * configuration file to create explicit template instantiations.
 */
class cmCableInstantiateCommand : public cmCableCommand
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
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);
  
  /**
   * This is called after all input commands have been processed.
   */
  virtual void FinalPass();

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() 
    {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CABLE_INSTANTIATE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Define CABLE InstantiationSet.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABLE_INSTANTIATE(cable_config_file member1 member2 ...)\n"
      "Generates an InstantiationSet in the CABLE configuration.  It is\n"
      "assumed that all members of the set are explicit instantiations of\n"
      "template non-classes (functions, operators, etc).";
    }

  virtual void WriteConfiguration(std::ostream&) const;  

  cmTypeMacro(cmCableInstantiateCommand, cmCableCommand);
protected:
  typedef std::vector<std::string>  Elements;
  
  /**
   * The output file to which to write the configuration.
   */
  cmCableData::OutputFile* m_OutputFile;

  /**
   * The elements describing the set of instantiations.
   */
  Elements m_Elements;
};



#endif
