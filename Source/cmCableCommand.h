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
#ifndef cmCableCommand_h
#define cmCableCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"
#include "cmCableData.h"

/** \class cmCableCommand
 * \brief Superclass for all cmCable command classes.
 *
 * cmCableCommand is the superclass for all CABLE-related commands.
 * The C++ Automated Bindings for Language Extension (CABLE) tool is
 * configured using an XML input file.  The input format is quite
 * flexible, but XML is hard for humans to write by hand.  The CABLE
 * commands in CMake are designed to simplify the interface with only
 * a small loss in functionality.  These commands can be used to
 * automatically generate CABLE configuration files.
 */
class cmCableCommand : public cmCommand
{
public:
  cmCableCommand();
  virtual ~cmCableCommand();
  
  cmTypeMacro(cmCableCommand, cmCommand);
protected:
  void SetupCableData();
  
  /**
   * The cmCableData holding common information for all cmCableCommand
   * instances.
   */
  cmCableData* m_CableData;
};



#endif
