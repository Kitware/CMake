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
#ifndef cmTarget_h
#define cmTarget_h

#include "cmStandardIncludes.h"
#include "cmCustomCommand.h"

/** \class cmTarget
 * \brief Represent a library or executable target loaded from a makefile.
 *
 * cmTarget represents a target loaded from 
 * a makefile.
 */
class cmTarget
{
public:
  std::vector<cmCustomCommand> m_CustomCommands;
  bool m_IsALibrary;
  std::vector<std::string> m_SourceLists;
};

typedef std::map<std::string,cmTarget> cmTargets;

#endif
