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
  /**
   * is this target a library?
   */
  bool IsALibrary() const { return m_IsALibrary; }
  bool GetIsALibrary() const { return m_IsALibrary; }
  void SetIsALibrary(bool f) { m_IsALibrary = f; }
  
  /**
   * Get the list of the custom commands for this target
   */
  const std::vector<cmCustomCommand> &GetCustomCommands() const {return m_CustomCommands;}
  std::vector<cmCustomCommand> &GetCustomCommands() {return m_CustomCommands;}

  /**
   * Get the list of the source lists used by this target
   */
  const std::vector<std::string> &GetSourceLists() const {return m_SourceLists;}
  std::vector<std::string> &GetSourceLists() {return m_SourceLists;}
  
private:
  std::vector<cmCustomCommand> m_CustomCommands;
  std::vector<std::string> m_SourceLists;
  bool m_IsALibrary;
};

typedef std::map<std::string,cmTarget> cmTargets;

#endif
