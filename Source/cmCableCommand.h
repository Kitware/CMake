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
 * \brief Superclass for all CABIL_ command classes.
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
