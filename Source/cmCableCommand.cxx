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
#include "cmCableCommand.h"
#include "cmCacheManager.h"

// cmCableCommand


/**
 * Constructor initializes to empty m_CableData.
 */
cmCableCommand::cmCableCommand(): m_CableData(0)
{
}


/**
 * Destructor frees the cmCableData only if this command is its owner.
 */
cmCableCommand::~cmCableCommand()
{
  if(m_CableData && m_CableData->OwnerIs(this))
    {
    delete m_CableData;
    }
}


/**
 * Ensure that this cmCableCommand has a valid m_CableData pointer.
 */
void cmCableCommand::SetupCableData()
{
  // Only do something if the pointer is invalid.
  if(m_CableData)
    { return; }
  
  // Look through the vector of commands from the makefile.
  const std::vector<cmCommand*>& usedCommands =
    m_Makefile->GetUsedCommands();  
  for(std::vector<cmCommand*>::const_iterator commandIter =
        usedCommands.begin(); commandIter != usedCommands.end(); ++commandIter)
    {
    // If this command is a cmCableCommand, see if it has a cmCableData
    // instance.
    cmCableCommand* command = cmCableCommand::SafeDownCast(*commandIter);
    if(command)
      { m_CableData = command->m_CableData; }
    
    // If we found an instance of cmCableData, then we are done.
    if(m_CableData)
      { return; }
    }

  // We must make sure the output directory exists so that the CABLE
  // configuration file can be opened by the cmCableData.
  std::string pathName = m_Makefile->GetStartOutputDirectory();
  if(!cmSystemTools::MakeDirectory(pathName.c_str()))
    {
    cmSystemTools::Error("Unable to make directory ", pathName.c_str());
    }

  // We didn't find another cmCableCommand with a valid cmCableData.
  // We must allocate the new cmCableData ourselves, and with this
  // command as its owner.
  pathName += "/cable_config.xml";
  m_CableData = new cmCableData(this, pathName);
  
  // We must add a custom rule to cause the cable_config.xml to be re-built
  // when it is removed.  Rebuilding it means re-running CMake.
  std::string cMakeLists = "\"";
  cMakeLists += m_Makefile->GetStartDirectory();
  cMakeLists += "/";
  cMakeLists += "CMakeLists.txt\"";

  std::string command;
#if defined(_WIN32) && !defined(__CYGWIN__)
  command = "\"";
  command += m_Makefile->GetHomeDirectory();
  command += "/CMake/Source/CMakeSetupCMD\" ";
  command += cMakeLists;
  command += " -DSP";
#else
  command = "\"";
  command += m_Makefile->GetHomeOutputDirectory();  
  command += "/CMake/Source/CMakeBuildTargets\" ";
  command += cMakeLists;
#endif
  command += " -H\"";
  command += m_Makefile->GetHomeDirectory();
  command += "\" -S\"";
  command += m_Makefile->GetStartDirectory();
  command += "\" -O\"";
  command += m_Makefile->GetStartOutputDirectory();
  command += "\" -B\"";
  command += m_Makefile->GetHomeOutputDirectory();
  command += "\"";

  std::vector<std::string> depends;
  m_Makefile->AddCustomCommand(cMakeLists.c_str(), 
                               command.c_str(),
                               depends,
                               "cable_config.xml");
}
