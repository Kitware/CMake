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
#include "cmCableData.h"
#include "cmCacheManager.h"


/**
 * Free all data that was stored here.
 */
cmCableData::~cmCableData()
{
  for(OutputFiles::iterator i = m_OutputFiles.begin();
      i != m_OutputFiles.end(); ++i)
    {
    delete i->second;
    }
}


/**
 * The constructor attempts to open the file for writing.
 */
cmCableData::OutputFile
::OutputFile(std::string file, const cmCableCommand* command):
  m_FileStream(file.c_str()),
  m_FirstReferencingCommand(command),
  m_LastReferencingCommand(command)
{
  if(!m_FileStream)
    {
    cmSystemTools::Error("Error can not open for write: ", file.c_str());
    }
}


/**
 * Destructor closes the file, if it was open.
 */
cmCableData::OutputFile
::~OutputFile()
{
  if(m_FileStream)
    m_FileStream.close();
}


/**
 * Get the output stream associated with this OutputFile.
 */
std::ostream&
cmCableData::OutputFile
::GetStream()
{
  return m_FileStream;
}


void
cmCableData::OutputFile
::SetLastReferencingCommand(const cmCableCommand* command)
{
  m_LastReferencingCommand = command;
}


bool
cmCableData::OutputFile
::FirstReferencingCommandIs(const cmCableCommand* command) const
{
  return (m_FirstReferencingCommand == command);
}


bool
cmCableData::OutputFile
::LastReferencingCommandIs(const cmCableCommand* command) const
{
  return (m_LastReferencingCommand == command);
}


/**
 * Get the OutputFile for the file with the given name.  Automatically
 * maintains first and last referencing commands.
 */
cmCableData::OutputFile*
cmCableData::GetOutputFile(const std::string& name,
                           const cmCableCommand* command)
{
  OutputFiles::iterator f = m_OutputFiles.find(name);
  // If the file hasn't yet been opened, create an entry for it.
  if(f == m_OutputFiles.end())
    {
    OutputFile* outputFile = new OutputFile(name, command);
    m_OutputFiles[name] = outputFile;
    
    return outputFile;
    }
  
  // The file has already been opened.  Set the command as the last
  // referencing command.
  f->second->SetLastReferencingCommand(command);
  
  return f->second;
}

