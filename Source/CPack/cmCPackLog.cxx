/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cmCPackLog.h"

#include "cmGeneratedFileStream.h"

//----------------------------------------------------------------------
cmCPackLog::cmCPackLog()
{
  m_Verbose = false;
  m_Debug = false;
  m_Quiet = false;
  m_NewLine = true;

  m_LastTag = cmCPackLog::NOTAG;
#undef cerr
#undef cout
  m_DefaultOutput = &std::cout;
  m_DefaultError = &std::cerr;
  
  m_LogOutput = 0;
  m_LogOutputCleanup = false;
}

//----------------------------------------------------------------------
cmCPackLog::~cmCPackLog()
{
  this->SetLogOutputStream(0);
}

//----------------------------------------------------------------------
void cmCPackLog::SetLogOutputStream(std::ostream* os)
{
  if ( m_LogOutputCleanup && m_LogOutput )
    {
    delete m_LogOutput;
    }
  m_LogOutputCleanup = false;
  m_LogOutput = os;
}

//----------------------------------------------------------------------
bool cmCPackLog::SetLogOutputFile(const char* fname)
{
  cmGeneratedFileStream *cg = 0;
  if ( fname )
    {
    cg = new cmGeneratedFileStream(fname);
    }
  if ( cg && !*cg )
    {
    delete cg;
    cg = 0;
    }
  this->SetLogOutputStream(cg);
  if ( !cg )
    {
    return false;
    }
  m_LogOutputCleanup = true;
  return true;
}

//----------------------------------------------------------------------
void cmCPackLog::Log(int tag, const char* file, int line, const char* msg, size_t length)
{
  // By default no logging
  bool display = false;

  // Display file and line number if debug
  bool useFileAndLine = m_Debug;

  bool output  = false;
  bool debug   = false;
  bool warning = false;
  bool error   = false;
  bool verbose = false;

  // When writing in file, add list of tags whenever tag changes.
  std::string tagString;
  bool needTagString = false;
  if ( m_LogOutput && m_LastTag != tag )
    {
    needTagString = true;
    }

  if ( tag & LOG_OUTPUT )
    {
    output = true;
    display = true;
    if ( needTagString )
      {
      if ( tagString.size() > 0 ) { tagString += ","; }
      tagString = "VERBOSE";
      }
    }
  if ( tag & LOG_WARNING )
    {
    warning = true;
    display = true;
    if ( needTagString )
      {
      if ( tagString.size() > 0 ) { tagString += ","; }
      tagString = "WARNING";
      }
    }
  if ( tag & LOG_ERROR )
    {
    error = true;
    display = true;
    if ( needTagString )
      {
      if ( tagString.size() > 0 ) { tagString += ","; }
      tagString = "ERROR";
      }
    }
  if ( tag & LOG_DEBUG && m_Debug )
    {
    debug = true;
    display = true;
    if ( needTagString )
      {
      if ( tagString.size() > 0 ) { tagString += ","; }
      tagString = "DEBUG";
      }
    useFileAndLine = true;
    }
  if ( tag & LOG_VERBOSE && m_Verbose )
    {
    verbose = true;
    display = true;
    if ( needTagString )
      {
      if ( tagString.size() > 0 ) { tagString += ","; }
      tagString = "VERBOSE";
      }
    }
  if ( m_Quiet )
    {
    display = false;
    }
  if ( m_LogOutput )
    {
    if ( needTagString )
      {
      *m_LogOutput << "[" << file << ":" << line << " " << tagString << "] ";
      }
    m_LogOutput->write(msg, length);
    }
  m_LastTag = tag;
  if ( !display )
    {
    return;
    }
  if ( m_NewLine )
    {
    if ( error && !m_ErrorPrefix.empty() )
      {
      *m_DefaultError << m_ErrorPrefix.c_str();
      }
    else if ( warning && !m_WarningPrefix.empty() )
      {
      *m_DefaultError << m_WarningPrefix.c_str();
      }
    else if ( output && !m_OutputPrefix.empty() )
      {
      *m_DefaultOutput << m_OutputPrefix.c_str();
      }
    else if ( verbose && !m_VerbosePrefix.empty() )
      {
      *m_DefaultOutput << m_VerbosePrefix.c_str();
      }
    else if ( debug && !m_DebugPrefix.empty() )
      {
      *m_DefaultOutput << m_DebugPrefix.c_str();
      }
    else if ( !m_Prefix.empty() )
      {
      *m_DefaultOutput << m_Prefix.c_str();
      }
    if ( useFileAndLine )
      {
      *m_DefaultOutput << __FILE__ << ":" << __LINE__ << " ";
      }
    }
  if ( error || warning )
    {
    m_DefaultError->write(msg, length);
    }
  else
    {
    m_DefaultOutput->write(msg, length);
    }
  if ( msg[length-1] == '\n' || length > 2 )
    {
    m_NewLine = true;;
    }
}
