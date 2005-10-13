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

#include "cmCTestBuildHandler.h"

#include "cmCTest.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmGeneratedFileStream.h"

//#include <cmsys/RegularExpression.hxx>
#include <cmsys/Process.h>

// used for sleep
#ifdef _WIN32
#include "windows.h"
#endif

#include <stdlib.h> 
#include <time.h>
#include <math.h>
#include <float.h>


static const char* cmCTestErrorMatches[] = {
  "^[Bb]us [Ee]rror",
  "^[Ss]egmentation [Vv]iolation",
  "^[Ss]egmentation [Ff]ault",
  "([^ :]+):([0-9]+): ([^ \\t])",
  "([^:]+): error[ \\t]*[0-9]+[ \\t]*:",
  "^Error ([0-9]+):",
  "^Fatal",
  "^Error: ",
  "^Error ",
  "[0-9] ERROR: ",
  "^\"[^\"]+\", line [0-9]+: [^Ww]",
  "^cc[^C]*CC: ERROR File = ([^,]+), Line = ([0-9]+)",
  "^ld([^:])*:([ \\t])*ERROR([^:])*:",
  "^ild:([ \\t])*\\(undefined symbol\\)",
  "([^ :]+) : (error|fatal error|catastrophic error)",
  "([^:]+): (Error:|error|undefined reference|multiply defined)",
  "([^:]+)\\(([^\\)]+)\\) : (error|fatal error|catastrophic error)",
  "^fatal error C[0-9]+:",
  ": syntax error ",
  "^collect2: ld returned 1 exit status",
  "Unsatisfied symbols:",
  "^Unresolved:",
  "Undefined symbols:",
  "^Undefined[ \\t]+first referenced",
  "^CMake Error:",
  ":[ \\t]cannot find",
  ":[ \\t]can't find",
  ": \\*\\*\\* No rule to make target \\`.*\\'.  Stop",
  ": Invalid loader fixup for symbol",
  ": Invalid fixups exist",
  ": Can't find library for",
  ": internal link edit command failed",
  ": Unrecognized option \\`.*\\'",
  "\", line [0-9]+\\.[0-9]+: [0-9]+-[0-9]+ \\([^W]\\)",
  "ld: 0706-006 Cannot find or open library file: -l ",
  "ild: \\(argument error\\) can't find library argument ::",
  "^could not be found and will not be loaded.",
  "s:616 string too big",
  "make: Fatal error: ",
  "ld: 0711-993 Error occurred while writing to the output file:",
  "ld: fatal: ",
  "final link failed:",
  "make: \\*\\*\\*.*Error",
  "\\*\\*\\* Error code",
  "nternal error:",
  "Makefile:[0-9]+: \\*\\*\\* .*  Stop\\.",
  0
};

static const char* cmCTestErrorExceptions[] = {
  "instantiated from ",
  "candidates are:",
  ": warning",
  ": \\(Warning\\)",
  "makefile:",
  "Makefile:",
  ":[ \\t]+Where:",
  "([^ :]+):([0-9]+): Warning",
  "------ Build started: .* ------",
  0
};

static const char* cmCTestWarningMatches[] = {
  "([^ :]+):([0-9]+): warning:",
  "^cc[^C]*CC: WARNING File = ([^,]+), Line = ([0-9]+)",
  "^ld([^:])*:([ \\t])*WARNING([^:])*:",
  "([^:]+): warning ([0-9]+):",
  "^\"[^\"]+\", line [0-9]+: [Ww]arning",
  "([^:]+): warning[ \\t]*[0-9]+[ \\t]*:",
  "^Warning ([0-9]+):",
  "^Warning ",
  "WARNING: ",
  "([^ :]+) : warning",
  "([^:]+): warning",
  "\", line [0-9]+\\.[0-9]+: [0-9]+-[0-9]+ \\(W\\)",
  "^cxx: Warning:",
  ".*file: .* has no symbols",
  "([^ :]+):([0-9]+): Warning",
  "\\([0-9]*\\): remark #[0-9]*",
  "\".*\", line [0-9]+: remark\\([0-9]*\\):",
  "cc-[0-9]* CC: REMARK File = .*, Line = [0-9]*",
  0
};

static const char* cmCTestWarningExceptions[] = {
  "/usr/openwin/include/X11/Xlib\\.h:[0-9]+: warning: ANSI C\\+\\+ forbids declaration",
  "/usr/openwin/include/X11/Xutil\\.h:[0-9]+: warning: ANSI C\\+\\+ forbids declaration",
  "/usr/openwin/include/X11/XResource\\.h:[0-9]+: warning: ANSI C\\+\\+ forbids declaration",
  "WARNING 84 :",
  "WARNING 47 :",
  "makefile:",
  "Makefile:",
  "warning:  Clock skew detected.  Your build may be incomplete.",
  "/usr/openwin/include/GL/[^:]+:",
  "bind_at_load",
  "XrmQGetResource",
  "IceFlush",
  "warning LNK4089: all references to [^ \\t]+ discarded by .OPT:REF",
  "ld32: WARNING 85: definition of dataKey in",
  "cc: warning 422: Unknown option \"\\+b",
  "_with_warning_C",
  0
};

struct cmCTestBuildCompileErrorWarningRex
{
  const char* m_RegularExpressionString;
  int m_FileIndex;
  int m_LineIndex;
};

static cmCTestBuildCompileErrorWarningRex
cmCTestWarningErrorFileLine[] = {
    { "^Warning W[0-9]+ ([a-zA-Z.\\:/0-9_+ ~-]+) ([0-9]+):", 1, 2 },
    { "^([a-zA-Z./0-9_+ ~-]+):([0-9]+):", 1, 2 },
    { "^([a-zA-Z.\\:/0-9_+ ~-]+)\\(([0-9]+)\\)", 1, 2 },
    { "^([a-zA-Z./0-9_+ ~-]+)\\(([0-9]+)\\)", 1, 2 },
    { "\"([a-zA-Z./0-9_+ ~-]+)\", line ([0-9]+)", 1, 2 },
    { "File = ([a-zA-Z./0-9_+ ~-]+), Line = ([0-9]+)", 1, 2 },
    { 0, 0, 0 }
};

//----------------------------------------------------------------------
cmCTestBuildHandler::cmCTestBuildHandler()
{
  m_MaxPreContext = 6;
  m_MaxPostContext = 6;

  m_MaxErrors = 50;
  m_MaxWarnings = 50;

  m_LastErrorOrWarning = m_ErrorsAndWarnings.end();

}

//----------------------------------------------------------------------
void cmCTestBuildHandler::Initialize()
{
  this->Superclass::Initialize();
  m_StartBuild = "";
  m_EndBuild = "";
  m_CustomErrorMatches.clear();
  m_CustomErrorExceptions.clear();
  m_CustomWarningMatches.clear();
  m_CustomWarningExceptions.clear();
  m_ErrorWarningFileLineRegex.clear();

  m_ErrorMatchRegex.clear();
  m_ErrorExceptionRegex.clear();
  m_WarningMatchRegex.clear();
  m_WarningExceptionRegex.clear();
  m_BuildProcessingQueue.clear();
  m_BuildProcessingQueueLocation = m_BuildProcessingQueue.end();
  m_BuildOutputLogSize = 0;
  m_CurrentProcessingLine.clear();

  m_SimplifySourceDir = "";
  m_SimplifyBuildDir = "";
  m_OutputLineCounter = 0;
  m_ErrorsAndWarnings.clear();
  m_LastErrorOrWarning = m_ErrorsAndWarnings.end();
  m_PostContextCount = 0;
  m_MaxPreContext = 6;
  m_MaxPostContext = 6;
  m_PreContext.clear();

  m_TotalErrors = 0;
  m_TotalWarnings = 0;
  m_LastTickChar = 0;

  m_ErrorQuotaReached = false;
  m_WarningQuotaReached = false;

  m_MaxErrors = 50;
  m_MaxWarnings = 50;
}

//----------------------------------------------------------------------
void cmCTestBuildHandler::PopulateCustomVectors(cmMakefile *mf)
{
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_ERROR_MATCH", 
                                m_CustomErrorMatches);
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_ERROR_EXCEPTION", 
                                m_CustomErrorExceptions);
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_WARNING_MATCH", 
                                m_CustomWarningMatches);
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_WARNING_EXCEPTION", 
                                m_CustomWarningExceptions);
  cmCTest::PopulateCustomInteger(mf, 
                             "CTEST_CUSTOM_MAXIMUM_NUMBER_OF_ERRORS", 
                             m_MaxErrors);
  cmCTest::PopulateCustomInteger(mf, 
                             "CTEST_CUSTOM_MAXIMUM_NUMBER_OF_WARNINGS", 
                             m_MaxWarnings);
}

//----------------------------------------------------------------------
//clearly it would be nice if this were broken up into a few smaller
//functions and commented...
int cmCTestBuildHandler::ProcessHandler()
{
  cmCTestLog(m_CTest, HANDLER_OUTPUT, "Build project" << std::endl);

  int entry;
  for ( entry = 0; cmCTestWarningErrorFileLine[entry].m_RegularExpressionString; ++ entry )
    {
    cmCTestBuildHandler::cmCTestCompileErrorWarningRex r;
    if ( r.m_RegularExpression.compile(
        cmCTestWarningErrorFileLine[entry].m_RegularExpressionString) )
      {
      r.m_FileIndex = cmCTestWarningErrorFileLine[entry].m_FileIndex;
      r.m_LineIndex = cmCTestWarningErrorFileLine[entry].m_LineIndex;
      m_ErrorWarningFileLineRegex.push_back(r);
      }
    else
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Problem Compiling regular expression: "
       << cmCTestWarningErrorFileLine[entry].m_RegularExpressionString << std::endl);
      }
    }

  // Determine build command and build directory
  const std::string &makeCommand = m_CTest->GetCTestConfiguration("MakeCommand");
  if ( makeCommand.size() == 0 )
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot find MakeCommand key in the DartConfiguration.tcl" << std::endl);
    return -1;
    }
  const std::string &buildDirectory = m_CTest->GetCTestConfiguration("BuildDirectory");
  if ( buildDirectory.size() == 0 )
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot find BuildDirectory  key in the DartConfiguration.tcl" << std::endl);
    return -1;
    }

  // Create a last build log
  cmGeneratedFileStream ofs;
  double elapsed_time_start = cmSystemTools::GetTime();
  if ( !this->StartLogFile("Build", ofs) )
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot create build log file" << std::endl);
    }

  // Create lists of regular expression strings for errors, error exceptions,
  // warnings and warning exceptions.
  std::vector<cmStdString>::size_type cc;
  for ( cc = 0; cmCTestErrorMatches[cc]; cc ++ )
    {
    m_CustomErrorMatches.push_back(cmCTestErrorMatches[cc]);
    }
  for ( cc = 0; cmCTestErrorExceptions[cc]; cc ++ )
    {
    m_CustomErrorExceptions.push_back(cmCTestErrorExceptions[cc]);
    }
  for ( cc = 0; cmCTestWarningMatches[cc]; cc ++ )
    {
    m_CustomWarningMatches.push_back(cmCTestWarningMatches[cc]);
    }
  for ( cc = 0; cmCTestWarningExceptions[cc]; cc ++ )
    {
    m_CustomWarningExceptions.push_back(cmCTestWarningExceptions[cc]);
    }

  // Pre-compile regular expressions objects for all regular expressions
  std::vector<cmStdString>::iterator it;

#define cmCTestBuildHandlerPopulateRegexVector(strings, regexes) \
  regexes.clear(); \
  for ( it = strings.begin(); it != strings.end(); ++it ) \
    { \
    regexes.push_back(it->c_str()); \
    }
  cmCTestBuildHandlerPopulateRegexVector(m_CustomErrorMatches, m_ErrorMatchRegex);
  cmCTestBuildHandlerPopulateRegexVector(m_CustomErrorExceptions, m_ErrorExceptionRegex);
  cmCTestBuildHandlerPopulateRegexVector(m_CustomWarningMatches, m_WarningMatchRegex);
  cmCTestBuildHandlerPopulateRegexVector(m_CustomWarningExceptions, m_WarningExceptionRegex);


  // Determine source and binary tree substitutions to simplify the output.
  m_SimplifySourceDir = "";
  m_SimplifyBuildDir = "";
  if ( m_CTest->GetCTestConfiguration("SourceDirectory").size() > 20 )
    {
    std::string srcdir = m_CTest->GetCTestConfiguration("SourceDirectory") + "/";
    std::string srcdirrep;
    for ( cc = srcdir.size()-2; cc > 0; cc -- )
      {
      if ( srcdir[cc] == '/' )
        {
        srcdirrep = srcdir.c_str() + cc;
        srcdirrep = "/..." + srcdirrep;
        srcdir = srcdir.substr(0, cc+1);
        break;
        }
      }
    m_SimplifySourceDir = srcdir;
    }
  if ( m_CTest->GetCTestConfiguration("BuildDirectory").size() > 20 )
    {
    std::string bindir = m_CTest->GetCTestConfiguration("BuildDirectory") + "/";
    std::string bindirrep;
    for ( cc = bindir.size()-2; cc > 0; cc -- )
      {
      if ( bindir[cc] == '/' )
        {
        bindirrep = bindir.c_str() + cc;
        bindirrep = "/..." + bindirrep;
        bindir = bindir.substr(0, cc+1);
        break;
        }
      }
    m_SimplifyBuildDir = bindir;
    }


  // Ok, let's do the build
  
  // Remember start build time
  m_StartBuild = m_CTest->CurrentTime();
  int retVal = 0;
  int res = cmsysProcess_State_Exited;
  if ( !m_CTest->GetShowOnly() )
    {
    res = this->RunMakeCommand(makeCommand.c_str(), &retVal, buildDirectory.c_str(), 0, ofs);
    }
  else
    {
    cmCTestLog(m_CTest, DEBUG, "Build with command: " << makeCommand << std::endl);
    }

  // Remember end build time and calculate elapsed time
  m_EndBuild = m_CTest->CurrentTime();
  double elapsed_build_time = cmSystemTools::GetTime() - elapsed_time_start;
  if (res != cmsysProcess_State_Exited || retVal )
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Error(s) when building project" << std::endl);
    }

  // Cleanups strings in the errors and warnings list.
  t_ErrorsAndWarningsVector::iterator evit;
  if ( !m_SimplifySourceDir.empty() )
    {
    for ( evit = m_ErrorsAndWarnings.begin(); evit != m_ErrorsAndWarnings.end(); ++ evit )
      {
      cmSystemTools::ReplaceString(evit->m_Text, m_SimplifySourceDir.c_str(), "/.../");
      cmSystemTools::ReplaceString(evit->m_PreContext, m_SimplifySourceDir.c_str(), "/.../");
      cmSystemTools::ReplaceString(evit->m_PostContext, m_SimplifySourceDir.c_str(), "/.../");
      }
    }

  if ( !m_SimplifyBuildDir.empty() )
    {
    for ( evit = m_ErrorsAndWarnings.begin(); evit != m_ErrorsAndWarnings.end(); ++ evit )
      {
      cmSystemTools::ReplaceString(evit->m_Text, m_SimplifyBuildDir.c_str(), "/.../");
      cmSystemTools::ReplaceString(evit->m_PreContext, m_SimplifyBuildDir.c_str(), "/.../");
      cmSystemTools::ReplaceString(evit->m_PostContext, m_SimplifyBuildDir.c_str(), "/.../");
      }
    }

  // Display message about number of errors and warnings
  cmCTestLog(m_CTest, HANDLER_OUTPUT, "   " << m_TotalErrors
    << (m_TotalErrors >= m_MaxErrors ? " or more" : "")
    << " Compiler errors" << std::endl);
  cmCTestLog(m_CTest, HANDLER_OUTPUT, "   " << m_TotalWarnings
    << (m_TotalWarnings >= m_MaxWarnings ? " or more" : "")
    << " Compiler warnings" << std::endl);

  // Generate XML output
  cmGeneratedFileStream xofs;
  if( !this->StartResultingXML("Build", xofs))
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot create build XML file" << std::endl);
    return -1;
    }
  this->GenerateDartBuildOutput(xofs, m_ErrorsAndWarnings, elapsed_build_time);
  return 0;
}

//----------------------------------------------------------------------
void cmCTestBuildHandler::GenerateDartBuildOutput(
  std::ostream& os, 
  std::vector<cmCTestBuildErrorWarning> ew,
  double elapsed_build_time)
{
  m_CTest->StartXML(os);
  os << "<Build>\n"
     << "\t<StartDateTime>" << m_StartBuild << "</StartDateTime>\n"
     << "<BuildCommand>" 
     << m_CTest->MakeXMLSafe(m_CTest->GetCTestConfiguration("MakeCommand"))
     << "</BuildCommand>" << std::endl;
    
  std::vector<cmCTestBuildErrorWarning>::iterator it;
  
  // only report the first 50 warnings and first 50 errors
  unsigned short numErrorsAllowed = m_MaxErrors;
  unsigned short numWarningsAllowed = m_MaxWarnings;
  std::string srcdir = m_CTest->GetCTestConfiguration("SourceDirectory");
  // make sure the source dir is in the correct case on windows
  // via a call to collapse full path.
  srcdir = cmSystemTools::CollapseFullPath(srcdir.c_str());
  srcdir += "/";
  for ( it = ew.begin(); 
        it != ew.end() && (numErrorsAllowed || numWarningsAllowed); it++ )
    {
    cmCTestBuildErrorWarning *cm = &(*it);
    if (cm->m_Error && numErrorsAllowed ||
        !cm->m_Error && numWarningsAllowed)
      {
      if (cm->m_Error)
        {
        numErrorsAllowed--;
        }
      else
        {
        numWarningsAllowed--;
        }
      os << "\t<" << (cm->m_Error ? "Error" : "Warning") << ">\n"
         << "\t\t<BuildLogLine>" << cm->m_LogLine << "</BuildLogLine>\n"
         << "\t\t<Text>" << m_CTest->MakeXMLSafe(cm->m_Text) 
         << "\n</Text>" << std::endl;
      std::vector<cmCTestCompileErrorWarningRex>::iterator rit;
      for ( rit = m_ErrorWarningFileLineRegex.begin();
            rit != m_ErrorWarningFileLineRegex.end(); ++ rit )
        {
        cmsys::RegularExpression* re = &rit->m_RegularExpression;
        if ( re->find(cm->m_Text.c_str() ) )
          {
          cm->m_SourceFile = re->match(rit->m_FileIndex);
          // At this point we need to make m_SourceFile relative to 
          // the source root of the project, so cvs links will work
          cmSystemTools::ConvertToUnixSlashes(cm->m_SourceFile);
          if(cm->m_SourceFile.find("/.../") != cm->m_SourceFile.npos)
            {
            cmSystemTools::ReplaceString(cm->m_SourceFile, "/.../", "");
            std::string::size_type p = cm->m_SourceFile.find("/");
            if(p != cm->m_SourceFile.npos)
              {
              cm->m_SourceFile = cm->m_SourceFile.substr(p+1, cm->m_SourceFile.size()-p);
              }
            }
          else
            {
            // make sure it is a full path with the correct case
            cm->m_SourceFile = cmSystemTools::CollapseFullPath(cm->m_SourceFile.c_str());
            cmSystemTools::ReplaceString(cm->m_SourceFile, srcdir.c_str(), "");
            }
          cm->m_LineNumber = atoi(re->match(rit->m_LineIndex).c_str());
          break;
          }
        }
      if ( cm->m_SourceFile.size() > 0 )
        {
        os << "\t\t<SourceFile>" << cm->m_SourceFile << "</SourceFile>" 
           << std::endl;
        }
      if ( cm->m_SourceFileTail.size() > 0 )
        {
        os << "\t\t<SourceFileTail>" << cm->m_SourceFileTail 
           << "</SourceFileTail>" << std::endl;
        }
      if ( cm->m_LineNumber >= 0 )
        {
        os << "\t\t<SourceLineNumber>" << cm->m_LineNumber 
           << "</SourceLineNumber>" << std::endl;
        }
      os << "\t\t<PreContext>" << m_CTest->MakeXMLSafe(cm->m_PreContext) 
         << "</PreContext>\n"
         << "\t\t<PostContext>" << m_CTest->MakeXMLSafe(cm->m_PostContext);
      // is this the last warning or error, if so notify
      if (cm->m_Error && !numErrorsAllowed ||
          !cm->m_Error && !numWarningsAllowed)
        {
        os << "\nThe maximum number of reported warnings or errors has been reached!!!\n";
        }
      os << "</PostContext>\n"
         << "\t\t<RepeatCount>0</RepeatCount>\n"
         << "</" << (cm->m_Error ? "Error" : "Warning") << ">\n\n" 
         << std::endl;
      }
    }
  os << "\t<Log Encoding=\"base64\" Compression=\"/bin/gzip\">\n\t</Log>\n"
     << "\t<EndDateTime>" << m_EndBuild << "</EndDateTime>\n"
     << "<ElapsedMinutes>" << static_cast<int>(elapsed_build_time/6)/10.0
     << "</ElapsedMinutes>"
     << "</Build>" << std::endl;
  m_CTest->EndXML(os);
}

//######################################################################
//######################################################################
//######################################################################
//######################################################################

//----------------------------------------------------------------------
int cmCTestBuildHandler::RunMakeCommand(const char* command,
  int* retVal, const char* dir, int timeout, std::ofstream& ofs)
{
  // First generate the command and arguments
  std::vector<cmStdString> args = cmSystemTools::ParseArguments(command);

  if(args.size() < 1)
    {
    return false;
    }

  std::vector<const char*> argv;
  for(std::vector<cmStdString>::const_iterator a = args.begin();
    a != args.end(); ++a)
    {
    argv.push_back(a->c_str());
    }
  argv.push_back(0);

  cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Run command:");
  std::vector<const char*>::iterator ait;
  for ( ait = argv.begin(); ait != argv.end() && *ait; ++ ait )
    {
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, " \"" << *ait << "\"");
    }
  cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, std::endl);
  
  // Now create process object
  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*argv.begin());
  cmsysProcess_SetWorkingDirectory(cp, dir);
  cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
  cmsysProcess_SetTimeout(cp, timeout);
  cmsysProcess_Execute(cp);

  // Initialize tick's
  std::string::size_type tick = 0;
  const std::string::size_type tick_len = 1024;

  char* data;
  int length;
  cmCTestLog(m_CTest, HANDLER_OUTPUT,
    "   Each symbol represents " << tick_len << " bytes of output." << std::endl
    << "   '!' represents an error and '*' a warning." << std::endl
    << "    " << std::flush);

  // Initialize building structures
  m_BuildProcessingQueue.clear();
  m_OutputLineCounter = 0;
  m_ErrorsAndWarnings.clear();
  m_TotalErrors = 0;
  m_TotalWarnings = 0;
  m_BuildOutputLogSize = 0;
  m_LastTickChar = '.';
  m_WarningQuotaReached = false;
  m_ErrorQuotaReached = false;

  // For every chunk of data
  while(cmsysProcess_WaitForData(cp, &data, &length, 0))
    {
    // Replace '\0' with '\n', since '\0' does not really make sense. This is
    // for Visual Studio output
    for(int cc =0; cc < length; ++cc)
      {
      if(data[cc] == 0)
        {
        data[cc] = '\n';
        }
      }

    // Process the chunk of data
    this->ProcessBuffer(data, length, tick, tick_len, ofs);
    }

  this->ProcessBuffer(0, 0, tick, tick_len, ofs);
  cmCTestLog(m_CTest, OUTPUT, " Size of output: "
    << int(m_BuildOutputLogSize / 1024.0) << "K" << std::endl);

  // Properly handle output of the build command
  cmsysProcess_WaitForExit(cp, 0);
  int result = cmsysProcess_GetState(cp);

  if(result == cmsysProcess_State_Exited)
    {
    *retVal = cmsysProcess_GetExitValue(cp);
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Command exited with the value: " << *retVal << std::endl);
    }
  else if(result == cmsysProcess_State_Exception)
    {
    *retVal = cmsysProcess_GetExitException(cp);
    cmCTestLog(m_CTest, WARNING, "There was an exception: " << *retVal << std::endl);
    }
  else if(result == cmsysProcess_State_Expired)
    {
    cmCTestLog(m_CTest, WARNING, "There was a timeout" << std::endl);
    } 
  else if(result == cmsysProcess_State_Error)
    {
    // If there was an error running command, report that on the dashboard.
    cmCTestBuildErrorWarning errorwarning;
    errorwarning.m_LogLine     = 1;
    errorwarning.m_Text        = "*** ERROR executing: "; 
    errorwarning.m_Text        += cmsysProcess_GetErrorString(cp);
    errorwarning.m_PreContext  = "";
    errorwarning.m_PostContext = "";
    errorwarning.m_Error       = true;
    m_ErrorsAndWarnings.push_back(errorwarning);
    m_TotalErrors ++;
    cmCTestLog(m_CTest, ERROR_MESSAGE, "There was an error: " << cmsysProcess_GetErrorString(cp) << std::endl);
    }

  cmsysProcess_Delete(cp);

  return result;
}

//######################################################################
//######################################################################
//######################################################################
//######################################################################

//----------------------------------------------------------------------
void cmCTestBuildHandler::ProcessBuffer(const char* data, int length, size_t& tick, size_t tick_len, 
    std::ofstream& ofs)
{
#undef cerr
  const std::string::size_type tick_line_len = 50;
  const char* ptr;
  for ( ptr = data; ptr < data+length; ptr ++ )
    {
    m_BuildProcessingQueue.push_back(*ptr);
    }
  m_BuildOutputLogSize += length;

  // until there are any lines left in the buffer
  while ( 1 )
    {
    // Find the end of line
    t_BuildProcessingQueueType::iterator it;
    for ( it = m_BuildProcessingQueue.begin();
      it != m_BuildProcessingQueue.end();
      ++ it )
      {
      if ( *it == '\n' )
        {
        break;
        }
      }

    // Once certain number of errors or warnings reached, ignore future errors or warnings.
    if ( m_TotalWarnings >= m_MaxWarnings )
      {
      m_WarningQuotaReached = true;
      }
    if ( m_TotalErrors >= m_MaxErrors )
      {
      m_ErrorQuotaReached = true;
      }

    // If the end of line was found
    if ( it != m_BuildProcessingQueue.end() )
      {
      // Create a contiguous array for the line
      m_CurrentProcessingLine.clear();
      t_BuildProcessingQueueType::iterator cit;
      for ( cit = m_BuildProcessingQueue.begin(); cit != it; ++cit )
        {
        m_CurrentProcessingLine.push_back(*cit);
        }
      m_CurrentProcessingLine.push_back(0);
      const char* line = &*m_CurrentProcessingLine.begin();

      // Process the line
      int lineType = this->ProcessSingleLine(line);

      // Erase the line from the queue
      m_BuildProcessingQueue.erase(m_BuildProcessingQueue.begin(), it+1);

      // Depending on the line type, produce error or warning, or nothing
      cmCTestBuildErrorWarning errorwarning;
      bool found = false;
      switch ( lineType )
        {
      case b_WARNING_LINE:
        m_LastTickChar = '*';
        errorwarning.m_Error = false;
        found = true;
        m_TotalWarnings ++;
        break;
      case b_ERROR_LINE:
        m_LastTickChar = '!';
        errorwarning.m_Error = true;
        found = true;
        m_TotalErrors ++;
        break;
        }
      if ( found )
        {
        // This is an error or warning, so generate report
        errorwarning.m_LogLine     = static_cast<int>(m_OutputLineCounter+1);
        errorwarning.m_Text        = line;
        errorwarning.m_PreContext  = "";
        errorwarning.m_PostContext = "";

        // Copy pre-context to report
        std::deque<cmStdString>::iterator pcit;
        for ( pcit = m_PreContext.begin(); pcit != m_PreContext.end(); ++pcit )
          {
          errorwarning.m_PreContext += *pcit + "\n";
          }
        m_PreContext.clear();

        // Store report
        m_ErrorsAndWarnings.push_back(errorwarning);
        m_LastErrorOrWarning = m_ErrorsAndWarnings.end()-1;
        m_PostContextCount = 0;
        }
      else
        {
        // This is not an error or warning.
        // So, figure out if this is a post-context line
        if ( m_LastErrorOrWarning != m_ErrorsAndWarnings.end() && m_PostContextCount < m_MaxPostContext )
          {
          m_PostContextCount ++;
          m_LastErrorOrWarning->m_PostContext += line;
          if ( m_PostContextCount < m_MaxPostContext )
            {
            m_LastErrorOrWarning->m_PostContext += "\n";
            }
          }
        else
          {
          // Otherwise store pre-context for the next error
          m_PreContext.push_back(line);
          if ( m_PreContext.size() > m_MaxPreContext )
            {
            m_PreContext.erase(m_PreContext.begin(), m_PreContext.end()-m_MaxPreContext);
            }
          }
        }
      m_OutputLineCounter ++;
      }
    else
      {
      break;
      }
    }

  // Now that the buffer is processed, display missing ticks
  int tickDisplayed = false;
  while ( m_BuildOutputLogSize > (tick * tick_len) )
    {
    tick ++;
    cmCTestLog(m_CTest, HANDLER_OUTPUT, m_LastTickChar);
    tickDisplayed = true;
    if ( tick % tick_line_len == 0 && tick > 0 )
      {
      cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Size: "
        << int((m_BuildOutputLogSize / 1024.0) + 1) << "K" << std::endl
        << "    ");
      }
    }
  if ( tickDisplayed )
    {
    m_LastTickChar = '.';
    }

  // And if this is verbose output, display the content of the chunk
  cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, cmCTestLogWrite(data, length));

  // Always store the chunk to the file
  ofs << cmCTestLogWrite(data, length);
}

//----------------------------------------------------------------------
int cmCTestBuildHandler::ProcessSingleLine(const char* data)
{
  cmCTestLog(m_CTest, DEBUG, "Line: [" << data << "]" << std::endl);

  std::vector<cmsys::RegularExpression>::iterator it;

  int warningLine = 0;
  int errorLine = 0;

  // Check for regular expressions
  
  if ( !m_ErrorQuotaReached )
    {
    // Errors 
    for ( it = m_ErrorMatchRegex.begin(); it != m_ErrorMatchRegex.end(); ++ it )
      {
      if ( it->find(data) )
        {
        errorLine = 1;
        cmCTestLog(m_CTest, DEBUG, "  Error Line: " << data << std::endl);
        break;
        }
      }
    // Error exceptions 
    for ( it = m_ErrorExceptionRegex.begin(); it != m_ErrorExceptionRegex.end(); ++ it )
      {
      if ( it->find(data) )
        {
        errorLine = 0;
        cmCTestLog(m_CTest, DEBUG, "  Not an error Line: " << data << std::endl);
        break;
        }
      }
    }
  if ( !m_WarningQuotaReached )
    {
    // Warnings
    for ( it = m_WarningMatchRegex.begin(); it != m_WarningMatchRegex.end(); ++ it )
      {
      if ( it->find(data) )
        {
        warningLine = 1;
        cmCTestLog(m_CTest, DEBUG, "  Warning Line: " << data << std::endl);
        break;
        }    
      }

    // Warning exceptions
    for ( it = m_WarningExceptionRegex.begin(); it != m_WarningExceptionRegex.end(); ++ it )
      {
      if ( it->find(data) )
        {
        warningLine = 0;
        cmCTestLog(m_CTest, DEBUG, "  Not a warning Line: " << data << std::endl);
        break;
        }    
      }
    }
  if ( errorLine )
    {
    return b_ERROR_LINE;
    }
  if ( warningLine )
    {
    return b_WARNING_LINE;
    }
  return b_REGULAR_LINE;
}

