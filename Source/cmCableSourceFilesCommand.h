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
#ifndef cmCableSourceFilesCommand_h
#define cmCableSourceFilesCommand_h

#include "cmStandardIncludes.h"
#include "cmCablePackageEntryCommand.h"

/** \class cmCableSourceFilesCommand
 * \brief Define a command that generates a rule for a CABLE Headers block.
 *
 * cmCableSourceFilesCommand is used to generate a rule in a CABLE
 * configuration file to setup a Package's include files.
 */
class cmCableSourceFilesCommand : public cmCablePackageEntryCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmCableSourceFilesCommand;
    }

  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass();  
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CABLE_SOURCE_FILES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Define CABLE header file dependencies in a package.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABLE_SOURCE_FILES(file1 file2 ...)"
      "Generates a Package's Headers block in the CABLE configuration.";
    }

  virtual bool WriteConfiguration();
  bool SourceFileExists(const std::string&) const;
  
  cmTypeMacro(cmCableSourceFilesCommand, cmCableCommand);
protected:
  typedef cmCablePackageEntryCommand::Entries  Entries;
};



#endif
