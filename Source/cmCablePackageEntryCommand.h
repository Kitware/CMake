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
#ifndef cmCablePackageEntryCommand_h
#define cmCablePackageEntryCommand_h

#include "cmStandardIncludes.h"
#include "cmCableCommand.h"

/** \class cmCablePackageEntryCommand
 * \brief Superclass to all CABLE Package entry generation commands.
 *
 * cmCablePackageEntryCommand implements the Invoke method of a cmCommand
 * to save the arguments as a vector of entries to a CABLE Package.  The
 * Invoke then calls the virtual WriteConfiguration() so that the subclass
 * can generate the configuration code for its particular type of Package
 * entry.
 */
class cmCablePackageEntryCommand : public cmCableCommand
{
public:
  cmCablePackageEntryCommand() {}
  virtual ~cmCablePackageEntryCommand() {}
  
  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);  

  cmTypeMacro(cmCablePackageEntryCommand, cmCableCommand);

  virtual bool WriteConfiguration() =0;
protected:
  typedef std::vector<std::string>  Entries;
  
  /**
   * The package entries.
   */
  Entries m_Entries;
};



#endif
