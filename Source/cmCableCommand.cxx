/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
}

