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
  "make: \\*\\*\\*.*Error",
  0
};

static const char* cmCTestErrorExceptions[] = {
  "instantiated from ",
  "candidates are:",
  ": warning",
  "makefile:",
  "Makefile:",
  ":[ \\t]+Where:",
  "([^ :]+):([0-9]+): Warning",
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
  m_Verbose = false; 
  m_CTest = 0;
  int cc;
  for ( cc = 0; cmCTestWarningErrorFileLine[cc].m_RegularExpressionString; ++ cc )
    {
    cmCTestBuildHandler::cmCTestCompileErrorWarningRex r;
    if ( r.m_RegularExpression.compile(
        cmCTestWarningErrorFileLine[cc].m_RegularExpressionString) )
      {
      r.m_FileIndex = cmCTestWarningErrorFileLine[cc].m_FileIndex;
      r.m_LineIndex = cmCTestWarningErrorFileLine[cc].m_LineIndex;
      m_ErrorWarningFileLineRegex.push_back(r);
      }
    else
      {
      std::cout << "Problem Compiling regular expression: "
       << cmCTestWarningErrorFileLine[cc].m_RegularExpressionString << std::endl;
      }
    }
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
}

//----------------------------------------------------------------------
//clearly it would be nice if this were broken up into a few smaller
//functions and commented...
int cmCTestBuildHandler::BuildDirectory(cmCTest *ctest_inst)
{
  m_CTest = ctest_inst;
  
  std::cout << "Build project" << std::endl;
  std::string makeCommand = m_CTest->GetDartConfiguration("MakeCommand");
  if ( makeCommand.size() == 0 )
    {
    std::cerr << "Cannot find MakeCommand key in the DartConfiguration.tcl" << std::endl;
    return 1;
    }
  std::string buildDirectory = m_CTest->GetDartConfiguration("BuildDirectory");
  if ( buildDirectory.size() == 0 )
    {
    std::cerr << "Cannot find BuildDirectory  key in the DartConfiguration.tcl" << std::endl;
    return 1;
    }

  cmGeneratedFileStream ofs;
  double elapsed_time_start = cmSystemTools::GetTime();
  if ( !m_CTest->OpenOutputFile("Temporary", "LastBuild.log", ofs) )
    {
    std::cerr << "Cannot create LastBuild.log file" << std::endl;    
    }

  m_StartBuild = m_CTest->CurrentTime();
  std::string output;
  int retVal = 0;
  int res = cmsysProcess_State_Exited;
  if ( !m_CTest->GetShowOnly() )
    {
    res = m_CTest->RunMakeCommand(makeCommand.c_str(), &output, 
      &retVal, buildDirectory.c_str(), 
      m_Verbose, 0, ofs);
    }
  else
    {
    std::cout << "Build with command: " << makeCommand << std::endl;
    }
  m_EndBuild = m_CTest->CurrentTime();
  double elapsed_build_time = cmSystemTools::GetTime() - elapsed_time_start;
  if (res != cmsysProcess_State_Exited || retVal )
    {
    std::cerr << "Error(s) when building project" << std::endl;
    }

  std::vector<cmStdString>::size_type cc;
  if ( m_CTest->GetDartConfiguration("SourceDirectory").size() > 20 ||
    m_CTest->GetDartConfiguration("BuildDirectory").size() > 20 )
    {
    std::string srcdir = m_CTest->GetDartConfiguration("SourceDirectory") + "/";
    std::string bindir = m_CTest->GetDartConfiguration("BuildDirectory") + "/";
    std::string srcdirrep;
    std::string bindirrep;
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

    cmSystemTools::ReplaceString(output, srcdir.c_str(), "/.../"); //srcdirrep.c_str());
    cmSystemTools::ReplaceString(output, bindir.c_str(), "/.../"); //bindirrep.c_str());
    }

  // Parsing of output for errors and warnings.

  std::vector<cmStdString> lines;
  cmSystemTools::Split(output.c_str(), lines);

  
  // Lines are marked: 
  // 0 - nothing
  // 1 - error
  // > 1 - warning
  std::vector<int> markedLines(lines.size(), 0);
  
  // Errors
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

  for ( cc = 0; cc < m_CustomErrorMatches.size(); cc ++ )
    {
    cmsys::RegularExpression re(m_CustomErrorMatches[cc].c_str());
    std::vector<cmStdString>::size_type kk;
    //cout << "error Line: " << m_CustomErrorMatches[cc] << endl;
    for ( kk = 0; kk < lines.size(); kk ++ )
      {
      //cout << "  Line: " << lines[kk] << endl;
      if ( re.find(lines[kk]) )
        {
        //cout << "******************************" << endl;
        markedLines[kk] = 1;
        }
      }    
    }
  // Warnings
  for ( cc = 0; cc < m_CustomWarningMatches.size(); cc ++ )
    {
    cmsys::RegularExpression re(m_CustomWarningMatches[cc].c_str());
    std::vector<cmStdString>::size_type kk;
    //cout << "warning Line: " << m_CustomWarningMatches[cc] << endl;
    for ( kk = 0; kk < lines.size(); kk ++ )
      {
      //cout << "  Line: " << lines[kk] << endl;
      if ( re.find(lines[kk]) )
        {
        //cout << "******************************" << endl;
        markedLines[kk] += 2;
        }
      }    
    }
  
  // the two follwing blocks of code appear wrong to me - Ken
  // I belive that the middle if tests should be %2 type tests
  // need to close the loop with Andy
  
  // Errors exceptions
  for ( cc = 0; cc < m_CustomErrorExceptions.size(); cc ++ )
    {
    cmsys::RegularExpression re(m_CustomErrorExceptions[cc].c_str());
    std::vector<int>::size_type kk;
    for ( kk =0; kk < markedLines.size(); kk ++ )
      {
      if ( markedLines[kk] == 1 )
        {
        if ( re.find(lines[kk]) )
          {
          markedLines[kk] = 0;
          }
        }
      }
    }
  // Warning exceptions
  for ( cc = 0; cc < m_CustomWarningExceptions.size(); cc ++ )
    {
    cmsys::RegularExpression re(m_CustomWarningExceptions[cc].c_str());
    std::vector<int>::size_type kk;
    for ( kk =0; kk < markedLines.size(); kk ++ )
      {
      if ( markedLines[kk] > 1 )
        {
        if ( re.find(lines[kk]) )
          {
          markedLines[kk] = 0;
          }
        }
      }
    }
  std::vector<cmCTestBuildErrorWarning> errorsWarnings;

  int errors = 0;
  int warnings = 0;

  std::vector<int>::size_type kk;
  cmCTestBuildErrorWarning errorwarning;
  for ( kk =0; kk < markedLines.size(); kk ++ )
    {
    errorwarning.m_LineNumber = -1;
    bool found = false;
    if ( markedLines[kk] == 1 )
      {
      //std::cout << "Error: " << lines[kk] << std::endl;
      errorwarning.m_Error = true;
      found = true;
      }
    else if ( markedLines[kk] > 1 )
      {
      //std::cout << "Warning: " << lines[kk] << std::endl;
      errorwarning.m_Error = false;
      found = true;
      }
    if ( found )
      {
      errorwarning.m_LogLine     = static_cast<int>(kk+1);
      errorwarning.m_Text        = lines[kk];
      errorwarning.m_PreContext  = "";
      errorwarning.m_PostContext = "";
      std::vector<int>::size_type jj;
      std::vector<int>::size_type ll = 0;
      if ( kk > 6 )
        {
        ll = kk - 6;
        }
      for ( jj = kk-1; 
            jj > 0 && jj > ll && markedLines[jj] != markedLines[kk]; 
            jj -- );
      while ( markedLines[jj] == markedLines[kk] && jj < kk )
        {
        jj ++;
        }
      for (; jj < kk; jj ++ )
        {
        errorwarning.m_PreContext += lines[jj] + "\n";
        }
      for ( jj = kk+1; 
            jj < lines.size() && jj < kk + 7 && markedLines[jj] != markedLines[kk]; 
            jj ++ )
        {
        errorwarning.m_PostContext += lines[jj] + "\n";
        }
      errorsWarnings.push_back(errorwarning);
      if ( errorwarning.m_Error )
        {
        errors ++;
        }
      else
        {
        warnings ++;
        }
      }
    }

  std::cout << "   " << errors << " Compiler errors" << std::endl;
  std::cout << "   " << warnings << " Compiler warnings" << std::endl;

  cmGeneratedFileStream xofs;
  if( !m_CTest->OpenOutputFile(m_CTest->GetCurrentTag(), "Build.xml", xofs, true) )
    {
    std::cerr << "Cannot create build XML file" << std::endl;
    return 1;
    }
  this->GenerateDartBuildOutput(xofs, errorsWarnings, elapsed_build_time);
  return 0;
}


void cmCTestBuildHandler::GenerateDartBuildOutput(
  std::ostream& os, 
  std::vector<cmCTestBuildErrorWarning> ew,
  double elapsed_build_time)
{
  m_CTest->StartXML(os);
  os << "<Build>\n"
     << "\t<StartDateTime>" << m_StartBuild << "</StartDateTime>\n"
     << "<BuildCommand>" 
     << m_CTest->MakeXMLSafe(m_CTest->GetDartConfiguration("MakeCommand"))
     << "</BuildCommand>" << std::endl;
    
  std::vector<cmCTestBuildErrorWarning>::iterator it;
  
  // only report the first 50 warnings and first 50 errors
  unsigned short numErrorsAllowed = 50;
  unsigned short numWarningsAllowed = 50;

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
  
