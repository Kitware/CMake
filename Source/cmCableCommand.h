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
#ifndef cmCabilCommand_h
#define cmCabilCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"
#include "cmCabilData.h"

/** \class cmCabilCommand
 * \brief Superclass for all cmCabil command classes.
 *
 * cmCabilCommand is the superclass for all CABIL-related commands.
 * The C++ Automated Bindings for Interpreted Languages (CABIL,
 * pronounced "sawbill") tool is configured using an XML input file.
 * The input format is quite flexible, but XML is hard for humans to
 * write by hand.  The CABIL commands in CMake are designed to simplify
 * the interface with only a small loss in functionality.  These commands
 * can be used to automatically generate CABIL configuration files.
 */
class cmCabilCommand : public cmCommand
{
public:
  cmCabilCommand();
  virtual ~cmCabilCommand();
  
  void WriteConfigurationHeader(std::ostream&) const;
  void WriteConfigurationFooter(std::ostream&) const;
  
  cmTypeMacro(cmCabilCommand, cmCommand);
protected:
  void SetupCabilData();
  
  /**
   * The cmCabilData holding common information for all cmCabilCommand
   * instances.
   */
  cmCabilData* m_CabilData;
};



#endif
