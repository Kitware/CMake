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
#ifndef cmCabilInstantiateCommand_h
#define cmCabilInstantiateCommand_h

#include "cmStandardIncludes.h"
#include "cmCabilCommand.h"

/** \class cmCabilInstantiateCommand
 * \brief Define a command that generates a rule for explicit template
 * instantiations.
 *
 * cmCabilInstantiateCommand is used to generate a rule in a CABIL
 * configuration file to create explicit template instantiations.
 */
class cmCabilInstantiateCommand : public cmCabilCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmCabilInstantiateCommand;
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
  virtual const char* GetName() { return "CABIL_INSTANTIATE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Define a rule for creating explicit template instantiations.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABIL_INSTANTIATE(cabil_config_file member1 member2 ...)";
    }

  virtual void WriteConfiguration(std::ostream&) const;  

  cmTypeMacro(cmCabilInstantiateCommand, cmCabilCommand);
private:
  typedef std::vector<std::string>  Elements;
  
  /**
   * The output file to which to write the configuration.
   */
  cmCabilData::OutputFile* m_OutputFile;

  /**
   * The elements describing the set of instantiations.
   */
  Elements m_Elements;
};



#endif
