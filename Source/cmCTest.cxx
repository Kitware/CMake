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
#include "cmCTest.h"
#include "cmSystemTools.h"
#include "cmListFileCache.h"

#ifdef HAVE_CURL
# include "cmCTestSubmit.h"
# include "curl/curl.h"
#endif

#include <cmsys/RegularExpression.hxx>
#include <cmsys/Process.h>
#include <cmsys/Base64.h>

#include <stdlib.h> // required for atoi
#include <time.h>
#include <math.h>
#include <float.h>

#define SAFEDIV(x,y) (((y)!=0)?((x)/(y)):(0))

#ifdef HAVE_CURL
static struct tm* GetNightlyTime(std::string str)
{
  struct tm* lctime;
  time_t tctime = time(0);
  //Convert the nightly start time to seconds. Since we are
  //providing only a time and a timezone, the current date of
  //the local machine is assumed. Consequently, nightlySeconds
  //is the time at which the nightly dashboard was opened or
  //will be opened on the date of the current client machine.
  //As such, this time may be in the past or in the future.
  time_t ntime = curl_getdate(str.c_str(), &tctime);
  tctime = time(0);
  //std::cout << "Seconds: " << tctime << std::endl;
  if ( ntime > tctime )
    {
    // If nightlySeconds is in the past, this is the current
    // open dashboard, then return nightlySeconds.  If
    // nightlySeconds is in the future, this is the next
    // dashboard to be opened, so subtract 24 hours to get the
    // time of the current open dashboard
    ntime -= (24 * 60 * 60 );
    //std::cout << "Pick yesterday" << std::endl;
    }
  //std::cout << "nightlySeconds: " << ntime << std::endl;
  lctime = gmtime(&ntime);
  return lctime;
}
#endif

static std::string CleanString(std::string str)
{
  std::string::size_type spos = str.find_first_not_of(" \n\t");
  std::string::size_type epos = str.find_last_not_of(" \n\t");
  if ( spos == str.npos )
    {
    return std::string();
    }
  if ( epos != str.npos )
    {
    epos = epos - spos + 1;
    }
  return str.substr(spos, epos);
}

static std::string CurrentTime()
{
  time_t currenttime = time(0);
  struct tm* t = localtime(&currenttime);
  //return ::CleanString(ctime(&currenttime));
  char current_time[1024];
  strftime(current_time, 1000, "%a %b %d %H:%M:%S %Z %Y", t);
  //std::cout << "Current_Time: " << current_time << std::endl;
  return ::CleanString(current_time);
}

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
  0
};

static const char* cmCTestErrorExceptions[] = {
  "instantiated from ",
  "candidates are:",
  ": warning",
  "makefile:",
  "Makefile:",
  ":[ \\t]+Where:",
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
  "cc: warning 422: Unknown option \"\\+b\\.\" ignored", 
  0
};

std::string cmCTest::MakeXMLSafe(const std::string& str)
{
  cmOStringStream ost;
  char buffer[10];
  for (std::string::size_type  pos = 0; pos < str.size(); pos ++ )
    {
    unsigned char ch = str[pos];
    if ( (ch > 126 || ch < 32) && ch != 9  && ch != 10 && ch != 13 )
      {
      sprintf(buffer, "&gt;%d&lt;", (int)ch);
      //sprintf(buffer, "&#x%0x;", (unsigned int)ch);
      ost << buffer;
      }
    else
      {
      switch ( ch )
        {
        case '&': ost << "&amp;"; break;
        case '<': ost << "&lt;"; break;
        case '>': ost << "&gt;"; break;
        default: ost << ch;
        }
      }
    }
  return ost.str();
}

std::string cmCTest::MakeURLSafe(const std::string& str)
{
  cmOStringStream ost;
  char buffer[10];
  for ( std::string::size_type pos = 0; pos < str.size(); pos ++ )
    {
    unsigned char ch = str[pos];
    if ( ( ch > 126 || ch < 32 ||
           ch == '&' ||
           ch == '%' ||
           ch == '+' ||
           ch == '=' || 
           ch == '@'
          ) && ch != 9 )
      {
      sprintf(buffer, "%02x;", (unsigned int)ch);
      ost << buffer;
      }
    else
      {
      ost << ch;
      }
    }
  return ost.str();
}

bool TryExecutable(const char *dir, const char *file,
                   std::string *fullPath, const char *subdir)
{
  // try current directory
  std::string tryPath;
  if (dir && strcmp(dir,""))
    {
    tryPath = dir;
    tryPath += "/";
    }
  
  if (subdir && strcmp(subdir,""))
    {
    tryPath += subdir;
    tryPath += "/";
    }
  
  tryPath += file;
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    *fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    return true;
    }
  tryPath += cmSystemTools::GetExecutableExtension();
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    *fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    return true;
    }
  return false;
}

cmCTest::cmCTest() 
{ 
  m_UseIncludeRegExp      = false;
  m_UseExcludeRegExp      = false;
  m_UseExcludeRegExpFirst = false;
  m_Verbose               = false;
  m_DartMode              = false;
  m_ShowOnly              = false;
  m_TestModel             = cmCTest::EXPERIMENTAL;
  m_TimeOut               = 0;
  int cc; 
  for ( cc=0; cc < cmCTest::LAST_TEST; cc ++ )
    {
    m_Tests[cc] = 0;
    }
}

void cmCTest::Initialize()
{
  m_ToplevelPath = cmSystemTools::GetCurrentWorkingDirectory();
  // parse the dart test file
  std::ifstream fin("DartConfiguration.tcl");
  if(!fin)
    {
    return;
    }

  char buffer[1024];
  while ( fin )
    {
    buffer[0] = 0;
    fin.getline(buffer, 1023);
    buffer[1023] = 0;
    std::string line = ::CleanString(buffer);
    if(line.size() == 0)
      {
      continue;
      }
    while ( fin && (line[line.size()-1] == '\\') )
      {
      line = line.substr(0, line.size()-1);
      buffer[0] = 0;
      fin.getline(buffer, 1023);
      buffer[1023] = 0;
      line += ::CleanString(buffer);
      }
    if ( line[0] == '#' )
      {
      continue;
      }
    std::string::size_type cpos = line.find_first_of(":");
    if ( cpos == line.npos )
      {
      continue;
      }
    std::string key = line.substr(0, cpos);
    std::string value = ::CleanString(line.substr(cpos+1, line.npos));
    m_DartConfiguration[key] = value;
    }
  fin.close();
  if ( m_DartMode )
    {
    m_TimeOut = atoi(m_DartConfiguration["TimeOut"].c_str());
    std::string testingDir = m_ToplevelPath + "/Testing";
    if ( cmSystemTools::FileExists(testingDir.c_str()) )
      {
      if ( !cmSystemTools::FileIsDirectory(testingDir.c_str()) )
        {
        std::cerr << "File " << testingDir << " is in the place of the testing directory"
                  << std::endl;
        return;
        }
      }
    else
      {
      if ( !cmSystemTools::MakeDirectory(testingDir.c_str()) )
        {
        std::cerr << "Cannot create directory " << testingDir
                  << std::endl;
        return;
        }
      }
    std::string tagfile = testingDir + "/TAG";
    std::ifstream tfin(tagfile.c_str());
    std::string tag;
    time_t tctime = time(0);
    struct tm *lctime = gmtime(&tctime);
    if ( tfin )
      {
      tfin >> tag;
      tfin.close();
      int year = 0;
      int mon = 0;
      int day = 0;
      int hour = 0;
      int min = 0;
      sscanf(tag.c_str(), "%04d%02d%02d-%02d%02d",
             &year, &mon, &day, &hour, &min);
      if ( year != lctime->tm_year + 1900 ||
           mon != lctime->tm_mon+1 ||
           day != lctime->tm_mday )
        {
        tag = "";
        }

      }
    if ( tag.size() == 0 || m_Tests[cmCTest::START_TEST] || m_Tests[ALL_TEST])
      {
#ifdef HAVE_CURL
      //std::cout << "TestModel: " << this->GetTestModelString() << std::endl;
      //std::cout << "TestModel: " << m_TestModel << std::endl;
      if ( m_TestModel == cmCTest::NIGHTLY )
        {
        lctime = ::GetNightlyTime(m_DartConfiguration["NightlyStartTime"]);
        }
#endif
      char datestring[100];
      sprintf(datestring, "%04d%02d%02d-%02d%02d",
              lctime->tm_year + 1900,
              lctime->tm_mon+1,
              lctime->tm_mday,
              lctime->tm_hour,
              lctime->tm_min);
      tag = datestring;
      std::ofstream ofs(tagfile.c_str());
      if ( ofs )
        {
        ofs << tag << std::endl;
        }
      ofs.close();
      std::cout << "Create new tag: " << tag << std::endl;
      }
    m_CurrentTag = tag;
    }
}

bool cmCTest::SetTest(const char* ttype)
{
  if ( cmSystemTools::LowerCase(ttype) == "all" )
    {
    m_Tests[cmCTest::ALL_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "start" )
    {
    m_Tests[cmCTest::START_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "update" )
    {
    m_Tests[cmCTest::UPDATE_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "configure" )
    {
    m_Tests[cmCTest::CONFIGURE_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "build" )
    {
    m_Tests[cmCTest::BUILD_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "test" )
    {
    m_Tests[cmCTest::TEST_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "coverage" )
    {
    m_Tests[cmCTest::COVERAGE_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "purify" )
    {
    m_Tests[cmCTest::PURIFY_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "submit" )
    {
    m_Tests[cmCTest::SUBMIT_TEST] = 1;
    }
  else
    {
    std::cerr << "Don't know about test \"" << ttype << "\" yet..." << std::endl;
    return false;
    }
  return true;
}

void cmCTest::Finalize()
{
}

std::string cmCTest::FindTheExecutable(const char *exe)
{
  std::string fullPath = "";
  std::string dir;
  std::string file;

  cmSystemTools::SplitProgramPath(exe, dir, file);
  if(m_ConfigType != "")
    {
    if(TryExecutable(dir.c_str(), file.c_str(), &fullPath, 
                     m_ConfigType.c_str()))
      {
      return fullPath;
      }
    dir += "/";
    dir += m_ConfigType;
    dir += "/";
    dir += file;
    cmSystemTools::Error("config type specified on the command line, but test executable not found.",
                         dir.c_str());
    return "";
    }
  if (TryExecutable(dir.c_str(),file.c_str(),&fullPath,"."))
    {
    return fullPath;
    }

  if (TryExecutable(dir.c_str(),file.c_str(),&fullPath,""))
    {
    return fullPath;
    }

  if (TryExecutable(dir.c_str(),file.c_str(),&fullPath,"Release"))
    {
    return fullPath;
    }

  if (TryExecutable(dir.c_str(),file.c_str(),&fullPath,"Debug"))
    {
    return fullPath;
    }

  if (TryExecutable(dir.c_str(),file.c_str(),&fullPath,"MinSizeRel"))
    {
    return fullPath;
    }

  if (TryExecutable(dir.c_str(),file.c_str(),&fullPath,"RelWithDebInfo"))
    {
    return fullPath;
    }

  // if everything else failed, check the users path
  if (dir != "")
    {
    std::string path = cmSystemTools::FindProgram(file.c_str());
    if (path != "")
      {
      return path;
      }
    }
  
  return fullPath;
}

int cmCTest::UpdateDirectory()
{
  int count = 0;
  std::string::size_type cc, kk;
  std::string cvsCommand = m_DartConfiguration["CVSCommand"];
  if ( cvsCommand.size() == 0 )
    {
    std::cerr << "Cannot find CVSCommand key in the DartConfiguration.tcl" << std::endl;
    return -1;
    }
  std::string cvsOptions = m_DartConfiguration["CVSUpdateOptions"];
  if ( cvsOptions.size() == 0 )
    {
    std::cerr << "Cannot find CVSUpdateOptions key in the DartConfiguration.tcl" << std::endl;
    return -1;
    }

  std::string sourceDirectory = m_DartConfiguration["SourceDirectory"];
  if ( sourceDirectory.size() == 0 )
    {
    std::cerr << "Cannot find SourceDirectory  key in the DartConfiguration.tcl" << std::endl;
    return -1;
    }

  std::string extra_update_opts;
#ifdef HAVE_CURL
  if ( m_TestModel == cmCTest::NIGHTLY )
    {
    struct tm* t = ::GetNightlyTime(m_DartConfiguration["NightlyStartTime"]);
    char current_time[1024];
    strftime(current_time, 1000, "%Y-%m-%d %H:%M:%S %Z", t);
    std::string today_update_date = current_time;
   
    //std::string today_update_date = current_time + 
    //  m_DartConfiguration["NightlyStartTime"];
    extra_update_opts += "-D \"" + today_update_date +"\"";
    //std::cout << "Update: " << extra_update_opts << std::endl;
    }
#endif

  std::string command = cvsCommand + " -z3 update " + cvsOptions +
    " " + extra_update_opts;
  std::ofstream os; 
  if ( !this->OpenOutputFile(m_CurrentTag, "Update.xml", os) )
    {
    std::cout << "Cannot open log file" << std::endl;
    }
  std::string start_time = ::CurrentTime();
 

  std::string goutput;
  int retVal = 0;
  bool res = true;
  std::ofstream ofs;
  if ( !m_ShowOnly )
    {
    res = cmSystemTools::RunSingleCommand(command.c_str(), &goutput, 
      &retVal, sourceDirectory.c_str(),
      m_Verbose, m_TimeOut);
    if ( this->OpenOutputFile("Temporary", "LastUpdate.log", ofs) )
      {
      ofs << goutput << std::endl;; 
      }
    }
  else
    {
    std::cout << "Update with command: " << command << std::endl;
    }

  os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
     << "<Update mode=\"Client\">\n"
     << "\t<Site>" <<m_DartConfiguration["Site"] << "</Site>\n"
     << "\t<BuildName>" << m_DartConfiguration["BuildName"]
     << "</BuildName>\n"
     << "\t<BuildStamp>" << m_CurrentTag << "-"
     << this->GetTestModelString() << "</BuildStamp>\n"
     << "\t<StartDateTime>" << start_time << "</StartDateTime>\n"
     << "\t<UpdateCommand>" << command << "</UpdateCommand>\n"
     << "\t<UpdateReturnStatus>";
  if ( retVal )
    {
    os << goutput;
    }
  os << "</UpdateReturnStatus>" << std::endl;

  std::vector<cmStdString> lines;
  cmSystemTools::Split(goutput.c_str(), lines);
  std::cout << "Updated; gathering version information" << std::endl;
  cmsys::RegularExpression date_author("^date: +([^;]+); +author: +([^;]+); +state: +[^;]+;");
  cmsys::RegularExpression revision("^revision +([^ ]*) *$");
  cmsys::RegularExpression end_of_file("^=============================================================================$");
  cmsys::RegularExpression end_of_comment("^----------------------------$");
  std::string current_path = "";
  bool first_file = true;

  cmCTest::AuthorsToUpdatesMap authors_files_map;
  int num_updated = 0;
  int num_modified = 0;
  int num_conflicting = 0;
  for ( cc= 0 ; cc < lines.size(); cc ++ )
    {
    const char* line = lines[cc].c_str();
    char mod = line[0];
    if ( line[1] == ' ' && mod != '?' )
      {
      count ++;
      const char* file = line + 2;
      //std::cout << "Line" << cc << ": " << mod << " - " << file << std::endl;
      std::string logcommand = cvsCommand + " -z3 log -N " + file;
      //std::cout << "Do log: " << logcommand << std::endl;
      std::string output;
      res = cmSystemTools::RunSingleCommand(logcommand.c_str(), &output, 
        &retVal, sourceDirectory.c_str(),
        m_Verbose, m_TimeOut);
      if ( ofs )
        {
        ofs << output << std::endl;
        }
      if ( res && retVal == 0)
        {
        //std::cout << output << std::endl;
        std::vector<cmStdString> ulines;
        cmSystemTools::Split(output.c_str(), ulines);
        std::string::size_type sline = 0;
        std::string srevision1 = "Unknown";
        std::string sdate1     = "Unknown";
        std::string sauthor1   = "Unknown";
        std::string semail1    = "Unknown";
        std::string comment1   = "";
        std::string srevision2 = "Unknown";
        std::string sdate2     = "Unknown";
        std::string sauthor2   = "Unknown";
        std::string comment2   = "";
        std::string semail2    = "Unknown";
        bool have_first = false;
        bool have_second = false;
        for ( kk = 0; kk < ulines.size(); kk ++ )
          {
          const char* clp = ulines[kk].c_str();
          if ( !have_second && !sline && revision.find(clp) )
            {
            if ( !have_first )
              {
              srevision1 = revision.match(1);
              }
            else
              {
              srevision2 = revision.match(1);
              }
            }
          else if ( !have_second && !sline && date_author.find(clp) )
            {
            sline = kk + 1;
            if ( !have_first )
              {
              sdate1 = date_author.match(1);
              sauthor1 = date_author.match(2);
              }
            else
              {
              sdate2 = date_author.match(1);
              sauthor2 = date_author.match(2);
              }
            }
          else if ( sline && end_of_comment.find(clp) || end_of_file.find(clp))
            {
            if ( !have_first )
              {
              have_first = true;
              }
            else if ( !have_second )
              {
              have_second = true;
              }
            sline = 0;
            }
          else if ( sline )
            {
            if ( !have_first )
              {
              comment1 += clp;
              comment1 += "\n";
              }
            else
              {
              comment2 += clp;
              comment2 += "\n";
              }
            }
          }
        if ( mod == 'M' )
          {
          comment1 = "Locally modified file\n";
          }
        std::string path = cmSystemTools::GetFilenamePath(file);
        std::string fname = cmSystemTools::GetFilenameName(file);
        if ( path != current_path )
          {
          if ( !first_file )
            {
            os << "\t</Directory>" << std::endl;
            }
          else
            {
            first_file = false;
            }
          os << "\t<Directory>\n"
             << "\t\t<Name>" << path << "</Name>" << std::endl;
          }
        if ( mod == 'C' )
          {
          num_conflicting ++;
          os << "\t<Conflicting>" << std::endl;
          }
        else if ( mod == 'M' )
          {
          num_modified ++;
          os << "\t<Modified>" << std::endl;
          }
        else
          {
          num_updated ++;
          os << "\t<Updated>" << std::endl;
          }
        if ( srevision2 == "Unknown" )
          {
          srevision2 = srevision1;
          }
        os << "\t\t<File Directory=\"" << path << "\">" << fname 
           << "</File>\n"
           << "\t\t<Directory>" << path << "</Directory>\n"
           << "\t\t<FullName>" << file << "</FullName>\n"
           << "\t\t<CheckinDate>" << sdate1 << "</CheckinDate>\n"
           << "\t\t<Author>" << sauthor1 << "</Author>\n"
           << "\t\t<Email>" << semail1 << "</Email>\n"
           << "\t\t<Log>" << this->MakeXMLSafe(comment1) << "</Log>\n"
           << "\t\t<Revision>" << srevision1 << "</Revision>\n"
           << "\t\t<PriorRevision>" << srevision2 << "</PriorRevision>"
           << std::endl;
        if ( srevision2 != srevision1 )
          {
          os
            << "\t\t<Revisions>\n"
            << "\t\t\t<Revision>" << srevision1 << "</Revision>\n"
            << "\t\t\t<PreviousRevision>" << srevision2 << "</PreviousRevision>\n"
            << "\t\t\t<Author>" << sauthor1<< "</Author>\n"
            << "\t\t\t<Date>" << sdate1 << "</Date>\n"
            << "\t\t\t<Comment>" << this->MakeXMLSafe(comment1) << "</Comment>\n"
            << "\t\t\t<Email>" << semail1 << "</Email>\n"
            << "\t\t</Revisions>\n"
            << "\t\t<Revisions>\n"
            << "\t\t\t<Revision>" << srevision2 << "</Revision>\n"
            << "\t\t\t<PreviousRevision>" << srevision2 << "</PreviousRevision>\n"
            << "\t\t\t<Author>" << sauthor2<< "</Author>\n"
            << "\t\t\t<Date>" << sdate2 << "</Date>\n"
            << "\t\t\t<Comment>" << this->MakeXMLSafe(comment2) << "</Comment>\n"
            << "\t\t\t<Email>" << semail2 << "</Email>\n"
            << "\t\t</Revisions>" << std::endl;
          }
        if ( mod == 'C' )
          {
          os << "\t</Conflicting>" << std::endl;
          }
        else if ( mod == 'M' )
          {
          os << "\t</Modified>" << std::endl;
          }
        else
          {
          os << "\t</Updated>" << std::endl;
          }
        cmCTest::UpdateFiles *u = &authors_files_map[sauthor1];
        cmCTest::StringPair p;
        p.first = path;
        p.second = fname;
        u->push_back(p);

        current_path = path;
        }
      }
    }
  if ( num_updated )
    {
    std::cout << "Found " << num_updated << " updated files" << std::endl;
    }
  if ( num_modified )
    {
    std::cout << "Found " << num_modified << " locally modified files" 
              << std::endl;
    }
  if ( num_conflicting )
    {
    std::cout << "Found " << num_conflicting << " conflicting files" 
              << std::endl;
    }
  if ( !first_file )
    {
    os << "\t</Directory>" << std::endl;
    }
  
  cmCTest::AuthorsToUpdatesMap::iterator it;
  for ( it = authors_files_map.begin();
        it != authors_files_map.end();
        it ++ )
    {
    os << "\t<Author>\n"
       << "\t\t<Name>" << it->first << "</Name>" << std::endl;
    cmCTest::UpdateFiles *u = &(it->second);
    for ( cc = 0; cc < u->size(); cc ++ )
      {
      os << "\t\t<File Directory=\"" << (*u)[cc].first << "\">"
         << (*u)[cc].second << "</File>" << std::endl;
      }
    os << "\t</Author>" << std::endl;
    }
  
  //std::cout << "End" << std::endl;
  std::string end_time = ::CurrentTime();
  os << "\t<EndDateTime>" << end_time << "</EndDateTime>\n"
     << "</Update>" << std::endl;

  if ( ofs )
    {
    ofs.close();
    }

  if (! res || retVal )
    {
    std::cerr << "Error(s) when updating the project" << std::endl;
    std::cerr << "Output: " << goutput << std::endl;
    return -1;
    }
  return count;
}

int cmCTest::ConfigureDirectory()
{
  std::cout << "Configure project" << std::endl;
  std::string cCommand = m_DartConfiguration["ConfigureCommand"];
  if ( cCommand.size() == 0 )
    {
    std::cerr << "Cannot find ConfigureCommand key in the DartConfiguration.tcl" 
              << std::endl;
    return 1;
    }

  std::string buildDirectory = m_DartConfiguration["BuildDirectory"];
  if ( buildDirectory.size() == 0 )
    {
    std::cerr << "Cannot find BuildDirectory  key in the DartConfiguration.tcl" << std::endl;
    return 1;
    }

  std::string output;
  int retVal = 0;
  bool res = true;
  if ( !m_ShowOnly )
    {
    std::ofstream os; 
    if ( !this->OpenOutputFile(m_CurrentTag, "Configure.xml", os) )
      {
      std::cout << "Cannot open log file" << std::endl;
      }
    std::string start_time = ::CurrentTime();

    std::ofstream ofs;
    this->OpenOutputFile("Temporary", "LastConfigure.log", ofs);
    res = this->RunMakeCommand(cCommand.c_str(), &output, 
      &retVal, buildDirectory.c_str(),
      m_Verbose, m_TimeOut, ofs);

    if ( ofs )
      {
      ofs.close();
      }
    
    if ( os )
      {
      os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         << "<Site BuildName=\"" << m_DartConfiguration["BuildName"]
         << "\" BuildStamp=\"" << m_CurrentTag << "-"
         << this->GetTestModelString() << "\" Name=\""
         << m_DartConfiguration["Site"] << "\">\n"
         << "<Configure>\n"
         << "\t<StartDateTime>" << start_time << "</StartDateTime>" << std::endl;
      if ( retVal )
        {
        os << retVal;
        }
      os << "<ConfigureCommand>" << cCommand.c_str() << "</ConfigureCommand>" << std::endl;
      //std::cout << "End" << std::endl;
      os << "<Log>" << this->MakeXMLSafe(output) << "</Log>" << std::endl;
      std::string end_time = ::CurrentTime();
      os << "\t<ConfigureStatus>" << retVal << "</ConfigureStatus>\n"
         << "\t<EndDateTime>" << end_time << "</EndDateTime>\n"
         << "</Configure>\n"
         << "</Site>" << std::endl;
      }    
    }
  else
    {
    std::cout << "Configure with command: " << cCommand << std::endl;
    }
  if (! res || retVal )
    {
    std::cerr << "Error(s) when updating the project" << std::endl;
    return 1;
    }
  return 0;
}

int cmCTest::BuildDirectory()
{
  std::cout << "Build project" << std::endl;
  std::string makeCommand = m_DartConfiguration["MakeCommand"];
  if ( makeCommand.size() == 0 )
    {
    std::cerr << "Cannot find MakeCommand key in the DartConfiguration.tcl" << std::endl;
    return 1;
    }
  std::string buildDirectory = m_DartConfiguration["BuildDirectory"];
  if ( buildDirectory.size() == 0 )
    {
    std::cerr << "Cannot find BuildDirectory  key in the DartConfiguration.tcl" << std::endl;
    return 1;
    }

  std::ofstream ofs;
  if ( !this->OpenOutputFile("Temporary", "LastBuild.log", ofs) )
    {
    std::cerr << "Cannot create LastBuild.log file" << std::endl;    
    }
  m_StartBuild = ::CurrentTime();
  std::string output;
  int retVal = 0;
  bool res = true;
  if ( !m_ShowOnly )
    {
    res = this->RunMakeCommand(makeCommand.c_str(), &output, 
      &retVal, buildDirectory.c_str(), 
      m_Verbose, m_TimeOut, ofs);
    }
  else
    {
    std::cout << "Build with command: " << makeCommand << std::endl;
    }
  m_EndBuild = ::CurrentTime();
  if (! res || retVal )
    {
    std::cerr << "Error(s) when building project" << std::endl;
    }
  if ( ofs )
    {
    ofs.close();
    }

  // Parsing of output for errors and warnings.

  std::vector<cmStdString> lines;
  cmSystemTools::Split(output.c_str(), lines);

  
  // Lines are marked: 
  // 0 - nothing
  // 1 - error
  // > 1 - warning
  std::vector<int> markedLines(lines.size(), 0);
  
  int cc;
  // Errors
  for ( cc = 0; cmCTestErrorMatches[cc]; cc ++ )
    {
    cmsys::RegularExpression re(cmCTestErrorMatches[cc]);
    std::vector<std::string>::size_type kk;
    for ( kk = 0; kk < lines.size(); kk ++ )
      {
      if ( re.find(lines[kk]) )
        {
        markedLines[kk] = 1;
        }
      }    
    }
  // Warnings
  for ( cc = 0; cmCTestWarningMatches[cc]; cc ++ )
    {
    cmsys::RegularExpression re(cmCTestWarningMatches[cc]);
    std::vector<std::string>::size_type kk;
    for ( kk = 0; kk < lines.size(); kk ++ )
      {
      if ( re.find(lines[kk]) )
        {
        markedLines[kk] += 2;
        }
      }    
    }
  // Errors exceptions
  for ( cc = 0; cmCTestErrorExceptions[cc]; cc ++ )
    {
    cmsys::RegularExpression re(cmCTestErrorExceptions[cc]);
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
  for ( cc = 0; cmCTestWarningExceptions[cc]; cc ++ )
    {
    cmsys::RegularExpression re(cmCTestWarningExceptions[cc]);
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
      for ( jj = kk; 
            jj > 0 && jj > ll /* && markedLines[jj] == 0 */; 
            jj -- );
      for (; jj < kk; jj ++ )
        {
        errorwarning.m_PreContext += lines[jj] + "\n";
        }
      for ( jj = kk+1; 
            jj < lines.size() && jj < kk + 7 /* && markedLines[jj] == 0*/; 
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

  if( !this->OpenOutputFile(m_CurrentTag, "Build.xml", ofs) )
    {
    std::cerr << "Cannot create build XML file" << std::endl;
    return 1;
    }
  this->GenerateDartBuildOutput(ofs, errorsWarnings);
  return 0;
}
 
int cmCTest::CoverageDirectory()
{
  std::cout << "Performing coverage" << std::endl;
  std::vector<std::string> files;
  std::vector<std::string> cfiles;
  std::vector<std::string> cdirs;
  bool done = false;
  std::string::size_type cc;
  std::string glob;
  std::map<std::string, std::string> allsourcefiles;
  std::map<std::string, std::string> allbinaryfiles;

  std::string start_time = ::CurrentTime();

  // Find all source files.
  std::string sourceDirectory = m_DartConfiguration["SourceDirectory"];
  if ( sourceDirectory.size() == 0 )
    {
    std::cerr << "Cannot find SourceDirectory  key in the DartConfiguration.tcl" << std::endl;
    return 1;
    }
  cdirs.push_back(sourceDirectory);
  while ( !done ) 
    {
    if ( cdirs.size() <= 0 )
      {
      break;
      }
    glob = cdirs[cdirs.size()-1] + "/*";
    //std::cout << "Glob: " << glob << std::endl;
    cdirs.pop_back();
    if ( cmSystemTools::SimpleGlob(glob, cfiles, 1) )
      {
      for ( cc = 0; cc < cfiles.size(); cc ++ )
        {
        allsourcefiles[cmSystemTools::GetFilenameName(cfiles[cc])] = cfiles[cc];
        }
      }
    if ( cmSystemTools::SimpleGlob(glob, cfiles, -1) )
      {
      for ( cc = 0; cc < cfiles.size(); cc ++ )
        {
        if ( cfiles[cc] != "." && cfiles[cc] != ".." )
          {
          cdirs.push_back(cfiles[cc]);
          }
        }
      }
    }

  // find all binary files
  cdirs.push_back(cmSystemTools::GetCurrentWorkingDirectory());
  while ( !done ) 
    {
    if ( cdirs.size() <= 0 )
      {
      break;
      }
    glob = cdirs[cdirs.size()-1] + "/*";
    //std::cout << "Glob: " << glob << std::endl;
    cdirs.pop_back();
    if ( cmSystemTools::SimpleGlob(glob, cfiles, 1) )
      {
      for ( cc = 0; cc < cfiles.size(); cc ++ )
        {
        allbinaryfiles[cmSystemTools::GetFilenameName(cfiles[cc])] = cfiles[cc];
        }
      }
    if ( cmSystemTools::SimpleGlob(glob, cfiles, -1) )
      {
      for ( cc = 0; cc < cfiles.size(); cc ++ )
        {
        if ( cfiles[cc] != "." && cfiles[cc] != ".." )
          {
          cdirs.push_back(cfiles[cc]);
          }
        }
      }
    }

  std::map<std::string, std::string>::iterator sit;
  for ( sit = allbinaryfiles.begin(); sit != allbinaryfiles.end(); sit ++ )
    {
    const std::string& fname = sit->second;
    //std::cout << "File: " << fname << std::endl;
    if ( strcmp(fname.substr(fname.size()-3, 3).c_str(), ".da") == 0 )
      {
      files.push_back(fname);
      }
    }
  
  if ( files.size() == 0 )
    {
    std::cout << "Cannot find any coverage information files (.da)" << std::endl;
    return 1;
    }

  std::ofstream log; 
  if (!this->OpenOutputFile("Temporary", "Coverage.log", log))
    {
    std::cout << "Cannot open log file" << std::endl;
    return 1;
    }
  log.close();
  if (!this->OpenOutputFile(m_CurrentTag, "Coverage.xml", log))
    {
    std::cout << "Cannot open log file" << std::endl;
    return 1;
    }

  std::string opath = m_ToplevelPath + "/Testing/Temporary/Coverage";
  cmSystemTools::MakeDirectory(opath.c_str());
  
  for ( cc = 0; cc < files.size(); cc ++ )
    {
    std::string command = "gcov -l \"" + files[cc] + "\"";
    std::string output;
    int retVal = 0;
    //std::cout << "Run gcov on " << files[cc] << std::flush;
    //std::cout << "   --- Run [" << command << "]" << std::endl;
    bool res = true;
    if ( !m_ShowOnly )
      {
      res = cmSystemTools::RunSingleCommand(command.c_str(), &output, 
        &retVal, opath.c_str(),
        m_Verbose, m_TimeOut);
      }
    if ( res && retVal == 0 )
      {
      //std::cout << " - done" << std::endl;
      }
    else
      {
      //std::cout << " - fail" << std::endl;
      }
    }
  
  files.clear();
  glob = opath + "/*";
  if ( !cmSystemTools::SimpleGlob(glob, cfiles, 1) )
    {
    std::cout << "Cannot found any coverage files" << std::endl;
    return 1;
    }
  std::map<std::string, std::vector<std::string> > sourcefiles;
  for ( cc = 0; cc < cfiles.size(); cc ++ )
    {
    std::string& fname = cfiles[cc];
    //std::cout << "File: " << fname << std::endl;
    if ( strcmp(fname.substr(fname.size()-5, 5).c_str(), ".gcov") == 0 )
      {
      files.push_back(fname);
      std::string::size_type pos = fname.find(".da.");
      if ( pos != fname.npos )
        {
        pos += 4;
        std::string::size_type epos = fname.size() - pos - strlen(".gcov");
        std::string nf = fname.substr(pos, epos);
        //std::cout << "Substring: " << nf << std::endl;
        if ( allsourcefiles.find(nf) != allsourcefiles.end() || 
             allbinaryfiles.find(nf) != allbinaryfiles.end() )
          {
          std::vector<std::string> &cvec = sourcefiles[nf];
          cvec.push_back(fname);
          }
        }
      }
    }
  for ( cc = 0; cc < files.size(); cc ++ )
    {
    //std::cout << "File: " << files[cc] << std::endl;
    }

  std::map<std::string, std::vector<std::string> >::iterator it;
  cmCTest::tm_CoverageMap coverageresults;

  log << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      << "<Site BuildName=\"" << m_DartConfiguration["BuildName"]
      << "\" BuildStamp=\"" << m_CurrentTag << "-"
      << this->GetTestModelString() << "\" Name=\""
      << m_DartConfiguration["Site"] << "\">\n"
      << "<Coverage>\n"
      << "\t<StartDateTime>" << start_time << "</StartDateTime>" << std::endl;

  int total_tested = 0;
  int total_untested = 0;

  for ( it = sourcefiles.begin(); it != sourcefiles.end(); it ++ )
    {
    //std::cerr << "Source file: " << it->first << std::endl;
    std::vector<std::string> &gfiles = it->second;
    for ( cc = 0; cc < gfiles.size(); cc ++ )
      {
      //std::cout << "\t" << gfiles[cc] << std::endl;
      std::ifstream ifile(gfiles[cc].c_str());
      if ( !ifile )
        {
        std::cout << "Cannot open file: " << gfiles[cc].c_str() << std::endl;
        }

      ifile.seekg (0, std::ios::end);
      int length = ifile.tellg();
      ifile.seekg (0, std::ios::beg);
      char *buffer = new char [ length + 1 ];
      ifile.read(buffer, length);
      buffer [length] = 0;
      //std::cout << "Read: " << buffer << std::endl;
      std::vector<cmStdString> lines;
      cmSystemTools::Split(buffer, lines);
      delete [] buffer;
      cmCTest::cmCTestCoverage& cov = coverageresults[it->first];
      std::vector<int>& covlines = cov.m_Lines; 
      if ( cov.m_FullPath == "" )
        {
        covlines.insert(covlines.begin(), lines.size(), -1);
        if ( allsourcefiles.find(it->first) != allsourcefiles.end() )
          {
          cov.m_FullPath = allsourcefiles[it->first];
          }
        else if ( allbinaryfiles.find(it->first) != allbinaryfiles.end() )
          {
          cov.m_FullPath = allbinaryfiles[it->first];
          }
        cov.m_AbsolutePath = cov.m_FullPath;
        std::string src_dir = m_DartConfiguration["SourceDirectory"];
        if ( src_dir[src_dir.size()-1] != '/' )
          {
          src_dir = src_dir + "/";
          }
        std::string::size_type spos = cov.m_FullPath.find(src_dir);
        if ( spos == 0 )
          {
          cov.m_FullPath = std::string("./") + cov.m_FullPath.substr(src_dir.size());
          }
        else
          {
          //std::cerr << "Compare -- " << cov.m_FullPath << std::endl;
          //std::cerr << "        -- " << src_dir << std::endl;
          cov.m_Show = false;
          continue;
          }
        cov.m_Show = true;
        }
      for ( cc = 0; cc < lines.size(); cc ++ )
        {
        std::string& line = lines[cc];
        std::string sub = line.substr(0, strlen("      ######"));
        int count = atoi(sub.c_str());
        if ( sub.compare("      ######") == 0 )
          {
          if ( covlines[cc] == -1 )
            {
            covlines[cc] = 0;
            }
          cov.m_UnTested ++;
          //std::cout << "Untested - ";
          }
        else if ( count > 0 )
          {
          if ( covlines[cc] == -1 )
            {
            covlines[cc] = 0;
            }
          cov.m_Tested ++;
          covlines[cc] += count;
          //std::cout << "Tested[" << count << "] - ";
          }

        //std::cout << line << std::endl;
        }
      }
    }

  //std::cerr << "Finalizing" << std::endl;
  cmCTest::tm_CoverageMap::iterator cit;
  int ccount = 0;
  std::ofstream cfileoutput; 
  int cfileoutputcount = 0;
  char cfileoutputname[100];
  std::string local_start_time = ::CurrentTime();
  std::string local_end_time;
  for ( cit = coverageresults.begin(); cit != coverageresults.end(); cit ++ )
    {
    cmCTest::cmCTestCoverage &cov = cit->second;
    if ( !cov.m_Show )
      {
      continue;
      }
    if ( ccount == 100 )
      {
      local_end_time = ::CurrentTime();
      cfileoutput << "\t<EndDateTime>" << local_end_time << "</EndDateTime>\n"
        << "</CoverageLog>\n"
        << "</Site>" << std::endl;
      cfileoutput.close();
      std::cout << "Close file: " << cfileoutputname << std::endl;
      ccount = 0;
      }
    if ( ccount == 0 )
      {
      sprintf(cfileoutputname, "CoverageLog-%d.xml", cfileoutputcount++);
      std::cout << "Open file: " << cfileoutputname << std::endl;
      if (!this->OpenOutputFile(m_CurrentTag, cfileoutputname, cfileoutput))
        {
        std::cout << "Cannot open log file: " << cfileoutputname << std::endl;
        return 1;
        }
      local_start_time = ::CurrentTime();
      cfileoutput << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<Site BuildName=\"" << m_DartConfiguration["BuildName"]
        << "\" BuildStamp=\"" << m_CurrentTag << "-"
        << this->GetTestModelString() << "\" Site=\""
        << m_DartConfiguration["Site"] << "\">\n"
        << "<CoverageLog>\n"
        << "\t<StartDateTime>" << local_start_time << "</StartDateTime>" << std::endl;
      }

    //std::cerr << "Final process of Source file: " << cit->first << std::endl;


    std::ifstream ifile(cov.m_AbsolutePath.c_str());
    if ( !ifile )
      {
      std::cerr << "Cannot open file: " << cov.m_FullPath.c_str() << std::endl;
      }
    ifile.seekg (0, std::ios::end);
    int length = ifile.tellg();
    ifile.seekg (0, std::ios::beg);
    char *buffer = new char [ length + 1 ];
    ifile.read(buffer, length);
    buffer [length] = 0;
    //std::cout << "Read: " << buffer << std::endl;
    std::vector<cmStdString> lines;
    cmSystemTools::Split(buffer, lines);
    delete [] buffer;
    cfileoutput << "\t<File Name=\"" << cit->first << "\" FullPath=\""
      << cov.m_FullPath << "\">\n"
      << "\t\t<Report>" << std::endl;
    for ( cc = 0; cc < lines.size(); cc ++ )
      {
      cfileoutput << "\t\t<Line Number=\"" 
        << static_cast<int>(cc) << "\" Count=\""
        << cov.m_Lines[cc] << "\">"
        << cmCTest::MakeXMLSafe(lines[cc]) << "</Line>" << std::endl;
      }
    cfileoutput << "\t\t</Report>\n"
      << "\t</File>" << std::endl;


    total_tested += cov.m_Tested;
    total_untested += cov.m_UnTested;
    float cper = 0;
    float cmet = 0;
    if ( total_tested + total_untested > 0 && (cov.m_Tested + cov.m_UnTested) > 0)
      {
      cper = (100 * SAFEDIV(static_cast<float>(cov.m_Tested),
        static_cast<float>(cov.m_Tested + cov.m_UnTested)));
      cmet = ( SAFEDIV(static_cast<float>(cov.m_Tested + 10),
        static_cast<float>(cov.m_Tested + cov.m_UnTested + 10)));
      }

    log << "\t<File Name=\"" << cit->first << "\" FullPath=\"" << cov.m_FullPath
      << "\" Covered=\"" << (cmet>0?"true":"false") << "\">\n"
      << "\t\t<LOCTested>" << cov.m_Tested << "</LOCTested>\n"
      << "\t\t<LOCUnTested>" << cov.m_UnTested << "</LOCUnTested>\n"
      << "\t\t<PercentCoverage>";
    log.setf(std::ios::fixed, std::ios::floatfield);
    log.precision(2);
    log << (cper) << "</PercentCoverage>\n"
      << "\t\t<CoverageMetric>";
    log.setf(std::ios::fixed, std::ios::floatfield);
    log.precision(2);
    log << (cmet) << "</CoverageMetric>\n"
      << "\t</File>" << std::endl;
    ccount ++;
    }
  
  if ( ccount > 0 )
    {
    local_end_time = ::CurrentTime();
    cfileoutput << "\t<EndDateTime>" << local_end_time << "</EndDateTime>\n"
                << "</CoverageLog>\n"
                << "</Site>" << std::endl;
    cfileoutput.close();
    }

  int total_lines = total_tested + total_untested;
  float percent_coverage = 100 * SAFEDIV(static_cast<float>(total_tested),
    static_cast<float>(total_lines));
  if ( total_lines == 0 )
    {
    percent_coverage = 0;
    }

  std::string end_time = ::CurrentTime();

  log << "\t<LOCTested>" << total_tested << "</LOCTested>\n"
      << "\t<LOCUntested>" << total_untested << "</LOCUntested>\n"
      << "\t<LOC>" << total_lines << "</LOC>\n"
      << "\t<PercentCoverage>";
  log.setf(std::ios::fixed, std::ios::floatfield);
  log.precision(2);
  log << (percent_coverage)<< "</PercentCoverage>\n"
      << "\t<EndDateTime>" << end_time << "</EndDateTime>\n"
      << "</Coverage>\n"
      << "</Site>" << std::endl;

  std::cout << "\tCovered LOC:         " << total_tested << std::endl
            << "\tNot covered LOC:     " << total_untested << std::endl
            << "\tTotal LOC:           " << total_lines << std::endl
            << "\tPercentage Coverage: ";

  std::cout.setf(std::ios::fixed, std::ios::floatfield);
  std::cout.precision(2);
  std::cout << (percent_coverage) << "%" << std::endl;


  return 1;
}

bool cmCTest::OpenOutputFile(const std::string& path, 
                     const std::string& name, std::ofstream& stream)
{
  std::string testingDir = m_ToplevelPath + "/Testing";
  if ( path.size() > 0 )
    {
    testingDir += "/" + path;
    }
  if ( cmSystemTools::FileExists(testingDir.c_str()) )
    {
    if ( !cmSystemTools::FileIsDirectory(testingDir.c_str()) )
      {
      std::cerr << "File " << testingDir 
                << " is in the place of the testing directory"
                << std::endl;
      return false;
      }
    }
  else
    {
    if ( !cmSystemTools::MakeDirectory(testingDir.c_str()) )
      {
      std::cerr << "Cannot create directory " << testingDir
                << std::endl;
      return false;
      }
    }
  std::string filename = testingDir + "/" + name;
  stream.open(filename.c_str());
  if( !stream )
    {
    std::cout << "Problem opening file: " << filename << std::endl;
    return false;
    }
  return true;
}

void cmCTest::GenerateDartBuildOutput(std::ostream& os, 
                                    std::vector<cmCTestBuildErrorWarning> ew)
{
  os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
     << "<Site BuildName=\"" << m_DartConfiguration["BuildName"]
     << "\" BuildStamp=\"" << m_CurrentTag << "-"
     << this->GetTestModelString() << "\" Name=\""
     << m_DartConfiguration["Site"] << "\">\n"
     << "<Build>\n"
     << "\t<StartDateTime>" << m_StartBuild << "</StartDateTime>\n"
     << "<BuildCommand>" 
     << this->MakeXMLSafe(m_DartConfiguration["MakeCommand"])
     << "</BuildCommand>" << std::endl;
    
  std::vector<cmCTestBuildErrorWarning>::iterator it;
  for ( it = ew.begin(); it != ew.end(); it++ )
    {
    cmCTestBuildErrorWarning *cm = &(*it);
    os << "\t<" << (cm->m_Error ? "Error" : "Warning") << ">\n"
       << "\t\t<BuildLogLine>" << cm->m_LogLine << "</BuildLogLine>\n"
       << "\t\t<Text>" << this->MakeXMLSafe(cm->m_Text) 
       << "\n</Text>" << std::endl;
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
    os << "\t\t<PreContext>" << this->MakeXMLSafe(cm->m_PreContext) 
       << "</PreContext>\n"
       << "\t\t<PostContext>" << this->MakeXMLSafe(cm->m_PostContext) 
       << "</PostContext>\n"
       << "\t\t<RepeatCount>0</RepeatCount>\n"
       << "</" << (cm->m_Error ? "Error" : "Warning") << ">\n\n" 
       << std::endl;
    }
  os << "\t<Log Encoding=\"base64\" Compression=\"/bin/gzip\">\n\t</Log>\n"
     << "\t<EndDateTime>" << m_EndBuild << "</EndDateTime>\n"
     << "</Build>\n"
     << "</Site>" << std::endl;
}
  
void cmCTest::ProcessDirectory(std::vector<std::string> &passed, 
                             std::vector<std::string> &failed)
{
  // does the DartTestfile.txt exist ?
  if(!cmSystemTools::FileExists("DartTestfile.txt"))
    {
    return;
    }
  
  // parse the file
  std::ifstream fin("DartTestfile.txt");
  if(!fin)
    {
    return;
    }

  int firstTest = 1;
  long line = 0;

#define SPACE_REGEX "[ \t\r\n]"
  
  cmsys::RegularExpression ireg(this->m_IncludeRegExp.c_str());
  cmsys::RegularExpression ereg(this->m_ExcludeRegExp.c_str());
  cmsys::RegularExpression dartStuff("(<DartMeasurement.*/DartMeasurement[a-zA-Z]*>)");

  bool parseError;
  while ( fin )
    {
    cmListFileFunction lff;
    if(cmListFileCache::ParseFunction(fin, lff, "DartTestfile.txt",
                                      parseError, line))
      {
      const std::string& name = lff.m_Name;
      const std::vector<cmListFileArgument>& args = lff.m_Arguments;
      if (name == "SUBDIRS")
        {
        std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
        for(std::vector<cmListFileArgument>::const_iterator j = args.begin();
            j != args.end(); ++j)
          {   
          std::string nwd = cwd + "/";
          nwd += j->Value;
          if (cmSystemTools::FileIsDirectory(nwd.c_str()))
            {
            cmSystemTools::ChangeDirectory(nwd.c_str());
            this->ProcessDirectory(passed, failed);
            }
          }
        // return to the original directory
        cmSystemTools::ChangeDirectory(cwd.c_str());
        }
      
      if (name == "ADD_TEST")
        {
        if (this->m_UseExcludeRegExp && 
            this->m_UseExcludeRegExpFirst && 
            ereg.find(args[0].Value.c_str()))
          {
          continue;
          }
        if (this->m_UseIncludeRegExp && !ireg.find(args[0].Value.c_str()))
          {
          continue;
          }
        if (this->m_UseExcludeRegExp && 
            !this->m_UseExcludeRegExpFirst && 
            ereg.find(args[0].Value.c_str()))
          {
          continue;
          }

        cmCTestTestResult cres;
        cres.m_Status = cmCTest::NOT_RUN;

        if (firstTest)
          {
          std::string nwd = cmSystemTools::GetCurrentWorkingDirectory();
          std::cerr << "Changing directory into " << nwd.c_str() << "\n";
          firstTest = 0;
          }
        cres.m_Name = args[0].Value;
        if ( m_ShowOnly )
          {
          std::cout << args[0].Value << std::endl;
          }
        else
          {
          fprintf(stderr,"Testing %-30s ",args[0].Value.c_str());
          fflush(stderr);
          }
        //std::cerr << "Testing " << args[0] << " ... ";
        // find the test executable
        std::string testCommand = this->FindTheExecutable(args[1].Value.c_str());
        testCommand = cmSystemTools::ConvertToOutputPath(testCommand.c_str());

        // continue if we did not find the executable
        if (testCommand == "")
          {
          std::cerr << "Unable to find executable: " << 
            args[1].Value.c_str() << "\n";
          continue;
          }
        
        // add the arguments
        std::vector<cmListFileArgument>::const_iterator j = args.begin();
        ++j;
        ++j;
        for(;j != args.end(); ++j)
          {   
          testCommand += " ";
          testCommand += cmSystemTools::EscapeSpaces(j->Value.c_str());
          }
        /**
         * Run an executable command and put the stdout in output.
         */
        std::string output;
        int retVal = 0;

        double clock_start, clock_finish;
        clock_start = cmSystemTools::GetTime();

        if ( m_Verbose )
          {
          std::cout << std::endl << "Test command: " << testCommand << std::endl;
          }
        int res = 0;
        if ( !m_ShowOnly )
          {
          res = this->RunTest(testCommand.c_str(), &output, &retVal);
          }
        clock_finish = cmSystemTools::GetTime();

        cres.m_ExecutionTime = (double)(clock_finish - clock_start);
        cres.m_FullCommandLine = testCommand;

        if ( !m_ShowOnly )
          {
          if (res == cmsysProcess_State_Exited && retVal )
            {
            fprintf(stderr,"   Passed\n");
            passed.push_back(args[0].Value); 
            }
          else
            {
            if ( res == cmsysProcess_State_Expired )
              {
              fprintf(stderr,"***Timeout\n");
              cres.m_Status = cmCTest::TIMEOUT;
              }
            else if ( res == cmsysProcess_State_Exception )
              {
              fprintf(stderr,"***Exception: ");
              switch ( retVal )
                {
              case cmsysProcess_Exception_Fault:
                fprintf(stderr,"SegFault");
                cres.m_Status = cmCTest::SEGFAULT;
                break;
              case cmsysProcess_Exception_Illegal:
                fprintf(stderr,"SegFault");
                cres.m_Status = cmCTest::ILLEGAL;
                break;
              case cmsysProcess_Exception_Interrupt:
                fprintf(stderr,"SegFault");
                cres.m_Status = cmCTest::INTERRUPT;
                break;
              case cmsysProcess_Exception_Numerical:
                fprintf(stderr,"SegFault");
                cres.m_Status = cmCTest::NUMERICAL;
                break;
              default:
                fprintf(stderr,"Other");
                cres.m_Status = cmCTest::OTHER_FAULT;
                }
              }
            else if ( res == cmsysProcess_State_Error )
              {
              fprintf(stderr,"***Bad command\n");
              cres.m_Status = cmCTest::BAD_COMMAND;
              }
            else
              {
              fprintf(stderr,"***Failed\n");
              }
            failed.push_back(args[0].Value); 
            }
          if (output != "")
            {
            if (dartStuff.find(output.c_str()))
              {
              std::string dartString = dartStuff.match(1);
              cmSystemTools::ReplaceString(output, dartString.c_str(),"");
              cres.m_RegressionImages = this->GenerateRegressionImages(dartString);
              }
            }
          }
        cres.m_Output = output;
        cres.m_ReturnValue = retVal;
        std::string nwd = cmSystemTools::GetCurrentWorkingDirectory();
        if ( nwd.size() > m_ToplevelPath.size() )
          {
          nwd = "." + nwd.substr(m_ToplevelPath.size(), nwd.npos);
          }
        cmSystemTools::ReplaceString(nwd, "\\", "/");
        cres.m_Path = nwd;
        cres.m_CompletionStatus = "Completed";
        m_TestResults.push_back( cres );
        }
      }
    }
}

int cmCTest::TestDirectory()
{
  std::cout << "Test project" << std::endl;
  std::vector<std::string> passed;
  std::vector<std::string> failed;
  int total;

  m_StartTest = ::CurrentTime();
  this->ProcessDirectory(passed, failed);
  m_EndTest = ::CurrentTime();

  total = int(passed.size()) + int(failed.size());

  if (total == 0)
    {
    if ( !m_ShowOnly )
      {
      std::cerr << "No tests were found!!!\n";
      }
    }
  else
    {
    if (passed.size() && (m_UseIncludeRegExp || m_UseExcludeRegExp)) 
      {
      std::cerr << "\nThe following tests passed:\n";
      for(std::vector<std::string>::iterator j = passed.begin();
          j != passed.end(); ++j)
        {   
        std::cerr << "\t" << *j << "\n";
        }
      }

    float percent = float(passed.size()) * 100.0f / total;
    fprintf(stderr,"\n%.0f%% tests passed, %i tests failed out of %i\n",
            percent, int(failed.size()), total);

    if (failed.size()) 
      {
      std::cerr << "\nThe following tests FAILED:\n";
      for(std::vector<std::string>::iterator j = failed.begin();
          j != failed.end(); ++j)
        {   
        std::cerr << "\t" << *j << "\n";
        }
      }
    }

  if ( m_DartMode )
    {
    std::ofstream ofs;
    if( !this->OpenOutputFile(m_CurrentTag, "Test.xml", ofs) )
      {
      std::cerr << "Cannot create testing XML file" << std::endl;
      return 1;
      }
    this->GenerateDartTestOutput(ofs);
    }

  return int(failed.size());
}

int cmCTest::SubmitResults()
{
#ifdef HAVE_CURL
  std::vector<std::string> files;
  std::string prefix = this->GetSubmitResultsPrefix();
  // TODO:
  // Check if test is enabled
  if ( this->CTestFileExists("Update.xml") )
    {
    files.push_back("Update.xml");
    }
  if ( this->CTestFileExists("Configure.xml") )
    {
    files.push_back("Configure.xml");
    }
  if ( this->CTestFileExists("Build.xml") )
    {
    files.push_back("Build.xml");
    }
  if ( this->CTestFileExists("Test.xml") )
    {
    files.push_back("Test.xml");
    }
  if ( this->CTestFileExists("Coverage.xml") )
    {
    files.push_back("Coverage.xml");
    std::vector<std::string> gfiles;
    std::string gpath = m_ToplevelPath + "/Testing/" + m_CurrentTag;
    std::string::size_type glen = gpath.size() + 1;
    gpath = gpath + "/CoverageLog*";
    //std::cout << "Globbing for: " << gpath.c_str() << std::endl;
    if ( cmSystemTools::SimpleGlob(gpath, gfiles, 1) )
      {
      size_t cc;
      for ( cc = 0; cc < gfiles.size(); cc ++ )
        {
        gfiles[cc] = gfiles[cc].substr(glen);
        //std::cout << "Glob file: " << gfiles[cc].c_str() << std::endl;
        files.push_back(gfiles[cc]);
        }
      }
    else
      {
      std::cout << "Problem globbing" << std::endl;
      }
    }
  if ( this->CTestFileExists("Purify.xml") )
    {
    files.push_back("Purify.xml");
    }
  cmCTestSubmit submit;
  submit.SetVerbose(m_Verbose);
  if ( m_DartConfiguration["DropMethod"] == "" ||
       m_DartConfiguration["DropMethod"] ==  "ftp" )
    {
    std::cout << "  Using FTP submit method" << std::endl;
    std::string url = "ftp://";
    url += cmCTest::MakeURLSafe(m_DartConfiguration["DropSiteUser"]) + ":" + 
      cmCTest::MakeURLSafe(m_DartConfiguration["DropSitePassword"]) + "@" + 
      m_DartConfiguration["DropSite"] + 
      cmCTest::MakeURLSafe(m_DartConfiguration["DropLocation"]);
    if ( !submit.SubmitUsingFTP(m_ToplevelPath+"/Testing/"+m_CurrentTag, 
        files, prefix, url) )
      {
      std::cout << "  Problems when submitting via FTP" << std::endl;
      return 0;
      }
    if ( !submit.TriggerUsingHTTP(files, prefix, m_DartConfiguration["TriggerSite"]) )
      {
      std::cout << "  Problems when triggering via HTTP" << std::endl;
      return 0;
      }
    std::cout << "  Submission successfull" << std::endl;
    return 1;
    }
  else if ( m_DartConfiguration["DropMethod"] == "http" )
    {
    std::cout << "  Using HTTP submit method" << std::endl;
    std::string url = "http://";
    if ( m_DartConfiguration["DropSiteUser"].size() > 0 )
      {
      url += m_DartConfiguration["DropSiteUser"];
      if ( m_DartConfiguration["DropSitePassword"].size() > 0 )
        {
        url += ":" + m_DartConfiguration["DropSitePassword"];
        }
      url += "@";
      }
    url += m_DartConfiguration["DropSite"] + m_DartConfiguration["DropLocation"];
    if ( !submit.SubmitUsingHTTP(m_ToplevelPath+"/Testing/"+m_CurrentTag, files, prefix, url) )
      {
      std::cout << "  Problems when submitting via HTTP" << std::endl;
      return 0;
      }
    if ( !submit.TriggerUsingHTTP(files, prefix, m_DartConfiguration["TriggerSite"]) )
      {
      std::cout << "  Problems when triggering via HTTP" << std::endl;
      return 0;
      }
    std::cout << "  Submission successfull" << std::endl;
    return 1;
    }
  else
    {
    std::cout << "SCP submit not yet implemented" << std::endl;
    }
                          
#endif
  return 0;
}

bool cmCTest::CTestFileExists(const std::string& filename)
{
  std::string testingDir = m_ToplevelPath + "/Testing/" + m_CurrentTag + "/" +
    filename;
  return cmSystemTools::FileExists(testingDir.c_str());
}

std::string cmCTest::GetSubmitResultsPrefix()
{
  std::string name = m_DartConfiguration["Site"] +
    "___" + m_DartConfiguration["BuildName"] +
    "___" + m_CurrentTag + "-" +
    this->GetTestModelString() + "___XML___";
  return name;
}

void cmCTest::GenerateDartTestOutput(std::ostream& os)
{
  if ( !m_DartMode )
    {
    return;
    }

  os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
     << "<Site BuildName=\"" << m_DartConfiguration["BuildName"]
     << "\" BuildStamp=\"" << m_CurrentTag << "-"
     << this->GetTestModelString() << "\" Name=\""
     << m_DartConfiguration["Site"] << "\">\n"
     << "<Testing>\n"
     << "\t<StartDateTime>" << m_StartTest << "</StartDateTime>\n"
     << "\t<TestList>\n";
  tm_TestResultsVector::size_type cc;
  for ( cc = 0; cc < m_TestResults.size(); cc ++ )
    {
    cmCTestTestResult *result = &m_TestResults[cc];
    os << "\t\t<Test>" << this->MakeXMLSafe(result->m_Path) 
       << "/" << this->MakeXMLSafe(result->m_Name)
       << "</Test>" << std::endl;
    }
  os << "\t</TestList>\n";
  for ( cc = 0; cc < m_TestResults.size(); cc ++ )
    {
    cmCTestTestResult *result = &m_TestResults[cc];
    os << "\t<Test Status=\"" << (result->m_Status==cmCTest::COMPLETED?"passed":"failed")
       << "\">\n"
       << "\t\t<Name>" << this->MakeXMLSafe(result->m_Name) << "</Name>\n"
       << "\t\t<Path>" << this->MakeXMLSafe(result->m_Path) << "</Path>\n"
       << "\t\t<FullName>" << this->MakeXMLSafe(result->m_Path) 
       << "/" << this->MakeXMLSafe(result->m_Name) << "</FullName>\n"
       << "\t\t<FullCommandLine>" 
       << this->MakeXMLSafe(result->m_FullCommandLine) 
       << "</FullCommandLine>\n"
       << "\t\t<Results>" << std::endl;
    if ( result->m_Status != cmCTest::COMPLETED || result->m_ReturnValue )
      {
      os << "\t\t\t<NamedMeasurement type=\"text/string\" name=\"Exit Code\"><Value>"
         << this->GetTestStatus(result->m_Status) << "</Value></NamedMeasurement>\n"
         << "\t\t\t<NamedMeasurement type=\"text/string\" name=\"Exit Value\"><Value>"
         << result->m_ReturnValue << "</Value></NamedMeasurement>" << std::endl;
      }
    os << result->m_RegressionImages;
    os << "\t\t\t<NamedMeasurement type=\"numeric/double\" "
       << "name=\"Execution Time\"><Value>"
       << result->m_ExecutionTime << "</Value></NamedMeasurement>\n"
       << "\t\t\t<NamedMeasurement type=\"text/string\" "
       << "name=\"Completion Status\"><Value>"
       << result->m_CompletionStatus << "</Value></NamedMeasurement>\n"
       << "\t\t\t<Measurement>\n"
       << "\t\t\t\t<Value>" << this->MakeXMLSafe(result->m_Output) 
       << "</Value>\n"
       << "\t\t\t</Measurement>\n"
       << "\t\t</Results>\n"
       << "\t</Test>" << std::endl;
    }
  
  os << "\t<EndDateTime>" << m_EndTest << "</EndDateTime>\n"
     << "</Testing>\n"
     << "</Site>" << std::endl;
}

int cmCTest::ProcessTests()
{
  int res = 0;
  bool notest = true;
  int cc;
  int update_count = 0;

  for ( cc = 0; cc < LAST_TEST; cc ++ )
    {
    if ( m_Tests[cc] )
      {
      notest = false;
      break;
      }
    }
  if ( m_Tests[UPDATE_TEST] || m_Tests[ALL_TEST] )
    {
    update_count = this->UpdateDirectory(); 
    if ( update_count < 0 )
      {
      res += 1;
      }
    }
  if ( m_TestModel == cmCTest::CONTINUOUS && !update_count )
    {
    return 0;
    }
  if ( m_Tests[CONFIGURE_TEST] || m_Tests[ALL_TEST] )
    {
    res += this->ConfigureDirectory();
    }
  if ( m_Tests[BUILD_TEST] || m_Tests[ALL_TEST] )
    {
    res += this->BuildDirectory();
    }
  if ( m_Tests[TEST_TEST] || m_Tests[ALL_TEST] || notest )
    {
    res += this->TestDirectory();
    }
  if ( m_Tests[COVERAGE_TEST] || m_Tests[ALL_TEST] )
    {
    this->CoverageDirectory();
    }
  if ( m_Tests[PURIFY_TEST] || m_Tests[ALL_TEST] )
    {
    std::cerr << "Purify test is not yet implemented" << std::endl;
    }
  if ( m_Tests[SUBMIT_TEST] || m_Tests[ALL_TEST] )
    {
#ifdef HAVE_CURL
    this->SubmitResults();
#else
    std::cerr << "Submit test is not yet implemented" << std::endl;
#endif
    }
  return res;
}

std::string cmCTest::GetTestModelString()
{
  switch ( m_TestModel )
    {
    case cmCTest::NIGHTLY:
      return "Nightly";
    case cmCTest::CONTINUOUS:
      return "Continuous";
    }
  return "Experimental";
}

std::string cmCTest::GenerateRegressionImages(const std::string& xml)
{
  cmsys::RegularExpression twoattributes(
    "<DartMeasurement" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*>([^<]*)</DartMeasurement>");
  cmsys::RegularExpression threeattributes(
    "<DartMeasurement" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*>([^<]*)</DartMeasurement>");
  cmsys::RegularExpression fourattributes(
    "<DartMeasurement" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*>([^<]*)</DartMeasurement>");
  cmsys::RegularExpression measurementfile(
    "<DartMeasurementFile" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*>([^<]*)</DartMeasurementFile>");

  cmOStringStream ostr;
  bool done = false;
  std::string cxml = xml;
  while ( ! done )
    {
    if ( twoattributes.find(cxml) )
      {
      ostr
        << "\t\t\t<NamedMeasurement" 
        << " " << twoattributes.match(1) << "=\"" << twoattributes.match(2) << "\""
        << " " << twoattributes.match(3) << "=\"" << twoattributes.match(4) << "\""
        << "><Value>" << twoattributes.match(5) 
        << "</Value></NamedMeasurement>" 
        << std::endl;
      cxml.erase(twoattributes.start(), twoattributes.end() - twoattributes.start());
      }
    else if ( threeattributes.find(cxml) )
      {
      ostr
        << "\t\t\t<NamedMeasurement" 
        << " " << threeattributes.match(1) << "=\"" << threeattributes.match(2) << "\""
        << " " << threeattributes.match(3) << "=\"" << threeattributes.match(4) << "\""
        << " " << threeattributes.match(5) << "=\"" << threeattributes.match(6) << "\""
        << "><Value>" << threeattributes.match(7) 
        << "</Value></NamedMeasurement>" 
        << std::endl;
      cxml.erase(threeattributes.start(), threeattributes.end() - threeattributes.start());
      }
    else if ( fourattributes.find(cxml) )
      {
      ostr
        << "\t\t\t<NamedMeasurement" 
        << " " << fourattributes.match(1) << "=\"" << fourattributes.match(2) << "\""
        << " " << fourattributes.match(3) << "=\"" << fourattributes.match(4) << "\""
        << " " << fourattributes.match(5) << "=\"" << fourattributes.match(6) << "\""
        << " " << fourattributes.match(7) << "=\"" << fourattributes.match(8) << "\""
        << "><Value>" << fourattributes.match(9) 
        << "</Value></NamedMeasurement>" 
        << std::endl;
      cxml.erase(fourattributes.start(), fourattributes.end() - fourattributes.start());
      }
    else if ( measurementfile.find(cxml) )
      {
      const std::string& filename = measurementfile.match(5);
      if ( cmSystemTools::FileExists(filename.c_str()) )
        {
        long len = cmSystemTools::FileLength(filename.c_str());
        std::ifstream ifs(filename.c_str(), std::ios::in 
#ifdef _WIN32
          | std::ios::binary
#endif
          );
        unsigned char *file_buffer = new unsigned char [ len + 1 ];
        ifs.read(reinterpret_cast<char*>(file_buffer), len);
        unsigned char *encoded_buffer = new unsigned char [ static_cast<int>(len * 1.5 + 1) ];

        unsigned long rlen = cmsysBase64_Encode(file_buffer, len, encoded_buffer, 1);
        unsigned long cc;

        ostr
          << "\t\t\t<NamedMeasurement" 
          << " " << measurementfile.match(1) << "=\"" << measurementfile.match(2) << "\""
          << " " << measurementfile.match(3) << "=\"" << measurementfile.match(4) << "\""
          << " encoding=\"base64\""
          << ">" << std::endl << "\t\t\t\t<Value>";
        for ( cc = 0; cc < rlen; cc ++ )
          {
          ostr << encoded_buffer[cc];
          if ( cc % 60 == 0 && cc )
            {
            ostr << std::endl;
            }
          }
        ostr
          << "</Value>" << std::endl << "\t\t\t</NamedMeasurement>" 
          << std::endl;
        delete [] file_buffer;
        delete [] encoded_buffer;
        }
      else
        {
        int idx = 4;
        if ( measurementfile.match(1) == "name" )
          {
          idx = 2;
          }
        ostr
          << "\t\t\t<NamedMeasurement" 
          << " name=\"" << measurementfile.match(idx) << "\""
          << " text=\"text/string\""
          << "><Value>File " << filename << " not found</Value></NamedMeasurement>" 
          << std::endl;
        }
      cxml.erase(measurementfile.start(), measurementfile.end() - measurementfile.start());
      }
    else
      {
      done = true;
      }
    }
  return ostr.str();
}

bool cmCTest::RunMakeCommand(const char* command, std::string* output,
    int* retVal, const char* dir, bool verbose, int timeout, std::ofstream& ofs)
{
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

  if ( output )
    {
    *output = "";
    }

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*argv.begin());
  cmsysProcess_SetWorkingDirectory(cp, dir);
  cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
  cmsysProcess_SetTimeout(cp, timeout);
  cmsysProcess_Execute(cp);
  
  std::string::size_type tick = 0;
  std::string::size_type tick_len = 1024;
  std::string::size_type tick_line_len = 50;

  char* data;
  int length;
  if ( !verbose )
    {
    std::cout << "   Each . represents " << tick_len << " bytes of output" << std::endl;
    std::cout << "    " << std::flush;
    }
  while(cmsysProcess_WaitForData(cp, (cmsysProcess_Pipe_STDOUT |
                                      cmsysProcess_Pipe_STDERR),
                                 &data, &length, 0))
    {
    if ( output )
      {
      output->append(data, length);
      while ( output->size() > (tick * tick_len) )
        {
        tick ++;
        std::cout << "." << std::flush;
        if ( tick % tick_line_len == 0 && tick > 0 )
          {
          std::cout << "  Size: ";
          std::cout << int((output->size() / 1024.0) + 1) << "K" << std::endl;
          std::cout << "    " << std::flush;
          }
        }
      }
    if(verbose)
      {
      std::cout.write(data, length);
      }
    if ( ofs )
      {
      ofs.write(data, length);
      ofs.flush();
      }
    }
  std::cout << " Size of output: ";
  std::cout << int(output->size() / 1024.0) << "K" << std::endl;
  
  cmsysProcess_WaitForExit(cp, 0);
  
  bool result = true;
  if(cmsysProcess_GetState(cp) == cmsysProcess_State_Exited)
    {
    if ( retVal )
      {
      *retVal = cmsysProcess_GetExitValue(cp);
      }
    else
      {
      if ( cmsysProcess_GetExitValue(cp) !=  0 )
        {
        result = false;
        }
      }
    }
  else
    {
    result = false;
    }
  
  cmsysProcess_Delete(cp);
  
  return result;
}

int cmCTest::RunTest( const char* command, std::string* output, int *retVal)
{
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

  if ( output )
    {
    *output = "";
    }

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*argv.begin());
  if(cmSystemTools::GetRunCommandHideConsole())
    {
    cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
    }
  cmsysProcess_SetTimeout(cp, m_TimeOut);
  cmsysProcess_Execute(cp);
  
  char* data;
  int length;
  while(cmsysProcess_WaitForData(cp, (cmsysProcess_Pipe_STDOUT |
                                      cmsysProcess_Pipe_STDERR),
                                 &data, &length, 0))
    {
    if ( output )
      {
      output->append(data, length);
      }
    }
  
  cmsysProcess_WaitForExit(cp, 0);

  int result = cmsysProcess_GetState(cp);

  if(result == cmsysProcess_State_Exited)
    {
    *retVal = cmsysProcess_GetExitValue(cp);
    }
  else if(result == cmsysProcess_State_Exception)
    {
    *retVal = cmsysProcess_GetExitException(cp);
    }
  else if(result == cmsysProcess_State_Error)
    {
    *output += "\n*** ERROR executing: ";
    *output += cmsysProcess_GetErrorString(cp);
    }
  
  cmsysProcess_Delete(cp);
  
  return result;
}

const char* cmCTest::GetTestStatus(int status)
{
  static const char statuses[][100] = {
    "Not Run",
    "Timeout",
    "SEGFAULT",
    "ILLEGAL",
    "INTERRUPT",
    "NUMERICAL",
    "OTHER_FAULT",
    "BAD_COMMAND",
    "Completed"
  };

  if ( status < cmCTest::NOT_RUN || status > cmCTest::COMPLETED )
    {
    return "No Status";
    }
  return statuses[status];
}
