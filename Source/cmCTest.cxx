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
#include "CTest/Curl/curl/curl.h"

#include "cmCTest.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include <cmsys/Directory.hxx>
#include "cmGlob.h"
#include "cmDynamicLoader.h"

#include "cmCTestBuildHandler.h"
#include "cmCTestCoverageHandler.h"
#include "cmCTestScriptHandler.h"
#include "cmCTestTestHandler.h"
#include "cmCTestUpdateHandler.h"
#include "cmCTestConfigureHandler.h"

#include "cmCTestSubmit.h"
#include "cmVersion.h"

#include <cmsys/RegularExpression.hxx>
#include <cmsys/Process.h>

#include <stdlib.h> 
#include <math.h>
#include <float.h>

#include <memory> // auto_ptr

#define DEBUGOUT std::cout << __LINE__ << " "; std::cout
#define DEBUGERR std::cerr << __LINE__ << " "; std::cerr

struct tm* cmCTest::GetNightlyTime(std::string str, 
                                   bool verbose, 
                                   bool tomorrowtag)
{
  struct tm* lctime;
  time_t tctime = time(0);
  if ( verbose )
    {
    std::cout << "Determine Nightly Start Time" << std::endl;
    std::cout << "   Specified time: " << str.c_str() << std::endl;
    }
  //Convert the nightly start time to seconds. Since we are
  //providing only a time and a timezone, the current date of
  //the local machine is assumed. Consequently, nightlySeconds
  //is the time at which the nightly dashboard was opened or
  //will be opened on the date of the current client machine.
  //As such, this time may be in the past or in the future.
  time_t ntime = curl_getdate(str.c_str(), &tctime);
  if ( verbose )
    {
    std::cout << "   Get curl time: " << ntime << std::endl;
    }
  tctime = time(0);
  if ( verbose )
    {
    std::cout << "   Get the current time: " << tctime << std::endl;
    }

  const int dayLength = 24 * 60 * 60;
  //std::cout << "Seconds: " << tctime << std::endl;
  if ( ntime > tctime )
    {
    // If nightlySeconds is in the past, this is the current
    // open dashboard, then return nightlySeconds.  If
    // nightlySeconds is in the future, this is the next
    // dashboard to be opened, so subtract 24 hours to get the
    // time of the current open dashboard
    ntime -= dayLength;
    //std::cout << "Pick yesterday" << std::endl;
    if ( verbose )
      {
      std::cout << "   Future time, subtract day: " << ntime << std::endl;
      }
    }
  if ( (tctime - ntime) >  dayLength )
    {
    ntime += dayLength;
    if ( verbose )
      {
      std::cout << "   Past time, subtract day: " << ntime << std::endl;
      }
    }
  //std::cout << "nightlySeconds: " << ntime << std::endl;
  if ( verbose )
    {
    std::cout << "   Current time: " << tctime
              << " Nightly time: " << ntime << std::endl;
    }
  if ( tomorrowtag )
    {
    std::cout << "Use future tag, Add a day" << std::endl;
    ntime += dayLength;
    }
  lctime = gmtime(&ntime);
  return lctime;
}

std::string cmCTest::CleanString(const std::string& str)
{
  std::string::size_type spos = str.find_first_not_of(" \n\t\r\f\v");
  std::string::size_type epos = str.find_last_not_of(" \n\t\r\f\v");
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

std::string cmCTest::CurrentTime()
{
  time_t currenttime = time(0);
  struct tm* t = localtime(&currenttime);
  //return ::CleanString(ctime(&currenttime));
  char current_time[1024];
  if ( m_ShortDateFormat )
    {
    strftime(current_time, 1000, "%b %d %H:%M %Z", t);
    }
  else
    {
    strftime(current_time, 1000, "%a %b %d %H:%M:%S %Z %Y", t);
    }
  //std::cout << "Current_Time: " << current_time << std::endl;
  return cmCTest::MakeXMLSafe(cmCTest::CleanString(current_time));
}


std::string cmCTest::MakeXMLSafe(const std::string& str)
{
  cmOStringStream ost;
  // By uncommenting the lcnt code, it will put newline every 120 characters
  //int lcnt = 0;
  for (std::string::size_type  pos = 0; pos < str.size(); pos ++ )
    {
    unsigned char ch = str[pos];
    if ( ch == '\r' )
      {
      // Ignore extra CR characters.
      }
    else if ( (ch > 126 || ch < 32) && ch != 9  && ch != 10 && ch != 13 )
      {
      char buffer[33];
      sprintf(buffer, "&lt;%d&gt;", (int)ch);
      //sprintf(buffer, "&#x%0x;", (unsigned int)ch);
      ost << buffer;
      //lcnt += 4;
      }
    else
      {
      switch ( ch )
        {
        case '&': ost << "&amp;"; break;
        case '<': ost << "&lt;"; break;
        case '>': ost << "&gt;"; break;
        case '\n': ost << "\n"; 
          //lcnt = 0; 
          break;
        default: ost << ch;
        }
      //lcnt ++;
      }
    //if ( lcnt > 120 )
    //  {
    //  ost << "\n";
    //  lcnt = 0;
    //  }
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

cmCTest::cmCTest() 
{ 
  m_ForceNewCTestProcess   = false;
  m_TomorrowTag            = false;
  m_BuildNoCMake           = false;
  m_BuildNoClean           = false;
  m_BuildTwoConfig         = false;
  m_Verbose                = false;
  m_DartMode               = false;
  m_ShowOnly               = false;
  m_RunConfigurationScript = false;
  m_TestModel              = cmCTest::EXPERIMENTAL;
  m_InteractiveDebugMode   = true;
  m_TimeOut                = 0;
  int cc; 
  for ( cc=0; cc < cmCTest::LAST_TEST; cc ++ )
    {
    m_Tests[cc] = 0;
    }
  m_ShortDateFormat        = true;

  this->BuildHandler     = new cmCTestBuildHandler;
  this->CoverageHandler  = new cmCTestCoverageHandler;
  this->ScriptHandler    = new cmCTestScriptHandler;
  this->TestHandler      = new cmCTestTestHandler;
  this->UpdateHandler    = new cmCTestUpdateHandler;
  this->ConfigureHandler = new cmCTestConfigureHandler;
}

cmCTest::~cmCTest() 
{ 
  delete this->BuildHandler;
  delete this->CoverageHandler;
  delete this->ScriptHandler;
  delete this->TestHandler;
  delete this->UpdateHandler;
  delete this->ConfigureHandler;
}

int cmCTest::Initialize()
{
  if(!m_InteractiveDebugMode)
    {
    this->BlockTestErrorDiagnostics();
    }
  
  m_ToplevelPath = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ConvertToUnixSlashes(m_ToplevelPath);
  if ( !this->ReadCustomConfigurationFileTree(m_ToplevelPath.c_str()) )
    {
    return 0;
    }
  this->UpdateCTestConfiguration();
  if ( m_DartMode )
    {
    std::string testingDir = m_ToplevelPath + "/Testing";
    if ( cmSystemTools::FileExists(testingDir.c_str()) )
      {
      if ( !cmSystemTools::FileIsDirectory(testingDir.c_str()) )
        {
        std::cerr << "File " << testingDir << " is in the place of the testing directory"
                  << std::endl;
        return 0;
        }
      }
    else
      {
      if ( !cmSystemTools::MakeDirectory(testingDir.c_str()) )
        {
        std::cerr << "Cannot create directory " << testingDir
                  << std::endl;
        return 0;
        }
      }
    std::string tagfile = testingDir + "/TAG";
    std::ifstream tfin(tagfile.c_str());
    std::string tag;
    time_t tctime = time(0);
    if ( m_TomorrowTag )
      {
      tctime += ( 24 * 60 * 60 );
      }
    struct tm *lctime = gmtime(&tctime);
    if ( tfin && cmSystemTools::GetLineFromStream(tfin, tag) )
      {
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
      std::string tagmode;
      if ( cmSystemTools::GetLineFromStream(tfin, tagmode) )
        {
        if ( tagmode.size() > 4 && !( m_Tests[cmCTest::START_TEST] || m_Tests[ALL_TEST] ))
          {
          m_TestModel = cmCTest::GetTestModelFromString(tagmode.c_str());
          }
        }
      tfin.close();
      }
    if ( tag.size() == 0 || m_Tests[cmCTest::START_TEST] || m_Tests[ALL_TEST])
      {
      //std::cout << "TestModel: " << this->GetTestModelString() << std::endl;
      //std::cout << "TestModel: " << m_TestModel << std::endl;
      if ( m_TestModel == cmCTest::NIGHTLY )
        {
        lctime = cmCTest::GetNightlyTime(m_DartConfiguration["NightlyStartTime"],
          m_Verbose,
          m_TomorrowTag);
        }
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
        ofs << this->GetTestModelString() << std::endl;
        }
      ofs.close();
      std::cout << "Create new tag: " << tag << " - " 
        << this->GetTestModelString() << std::endl;
      }
    m_CurrentTag = tag;
    }
  return 1;
}

void cmCTest::UpdateCTestConfiguration()
{
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
    std::string line = cmCTest::CleanString(buffer);
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
      line += cmCTest::CleanString(buffer);
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
    std::string value = cmCTest::CleanString(line.substr(cpos+1, line.npos));
    m_DartConfiguration[key] = value;
    }
  fin.close();
  if ( m_DartMode )
    {
    m_TimeOut = atoi(m_DartConfiguration["TimeOut"].c_str());
    }
}

void cmCTest::BlockTestErrorDiagnostics()
{
  cmSystemTools::PutEnv("DART_TEST_FROM_DART=1");
  cmSystemTools::PutEnv("DASHBOARD_TEST_FROM_CTEST=1");
#if defined(_WIN32)
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX); 
#endif
}

void cmCTest::SetTestModel(int mode)
{
  m_InteractiveDebugMode = false;
  m_TestModel = mode;
}

bool cmCTest::SetTest(const char* ttype, bool report)
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
  else if ( cmSystemTools::LowerCase(ttype) == "memcheck" )
    {
    m_Tests[cmCTest::MEMCHECK_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "notes" )
    {
    m_Tests[cmCTest::NOTES_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "submit" )
    {
    m_Tests[cmCTest::SUBMIT_TEST] = 1;
    }
  else
    {
    if ( report )
      {
      std::cerr << "Don't know about test \"" << ttype << "\" yet..." << std::endl;
      }
    return false;
    }
  return true;
}

void cmCTest::Finalize()
{
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
    std::cerr << "Problem opening file: " << filename << std::endl;
    return false;
    }
  return true;
}

int cmCTest::SubmitResults()
{
  std::ofstream ofs;
  this->OpenOutputFile("Temporary", "LastSubmit.log", ofs);

  cmCTest::tm_VectorOfStrings files;
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
    cmCTest::tm_VectorOfStrings gfiles;
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
      std::cerr << "Problem globbing" << std::endl;
      }
    }
  if ( this->CTestFileExists("DynamicAnalysis.xml") )
    {
    files.push_back("DynamicAnalysis.xml");
    }
  if ( this->CTestFileExists("Purify.xml") )
    {
    files.push_back("Purify.xml");
    }
  if ( this->CTestFileExists("Notes.xml") )
    {
    files.push_back("Notes.xml");
    }

  if ( ofs )
    {
    ofs << "Upload files:" << std::endl;
    int cnt = 0;
    cmCTest::tm_VectorOfStrings::iterator it;
    for ( it = files.begin(); it != files.end(); ++ it )
      {
      ofs << cnt << "\t" << it->c_str() << std::endl;
      cnt ++;
      }
    }
  std::cout << "Submit files (using " << m_DartConfiguration["DropMethod"] << ")"
    << std::endl;
  cmCTestSubmit submit;
  submit.SetVerbose(m_Verbose);
  submit.SetLogFile(&ofs);
  if ( m_DartConfiguration["DropMethod"] == "" ||
    m_DartConfiguration["DropMethod"] ==  "ftp" )
    {
    ofs << "Using drop method: FTP" << std::endl;
    std::cout << "  Using FTP submit method" << std::endl;
    std::string url = "ftp://";
    url += cmCTest::MakeURLSafe(m_DartConfiguration["DropSiteUser"]) + ":" + 
      cmCTest::MakeURLSafe(m_DartConfiguration["DropSitePassword"]) + "@" + 
      m_DartConfiguration["DropSite"] + 
      cmCTest::MakeURLSafe(m_DartConfiguration["DropLocation"]);
    if ( !submit.SubmitUsingFTP(m_ToplevelPath+"/Testing/"+m_CurrentTag, 
        files, prefix, url) )
      {
      std::cerr << "  Problems when submitting via FTP" << std::endl;
      ofs << "  Problems when submitting via FTP" << std::endl;
      return 0;
      }
    if ( !submit.TriggerUsingHTTP(files, prefix, m_DartConfiguration["TriggerSite"]) )
      {
      std::cerr << "  Problems when triggering via HTTP" << std::endl;
      ofs << "  Problems when triggering via HTTP" << std::endl;
      return 0;
      }
    std::cout << "  Submission successfull" << std::endl;
    ofs << "  Submission succesfull" << std::endl;
    return 1;
    }
  else if ( m_DartConfiguration["DropMethod"] == "http" )
    {
    ofs << "Using drop method: HTTP" << std::endl;
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
      std::cerr << "  Problems when submitting via HTTP" << std::endl;
      ofs << "  Problems when submitting via HTTP" << std::endl;
      return 0;
      }
    if ( !submit.TriggerUsingHTTP(files, prefix, m_DartConfiguration["TriggerSite"]) )
      {
      std::cerr << "  Problems when triggering via HTTP" << std::endl;
      ofs << "  Problems when triggering via HTTP" << std::endl;
      return 0;
      }
    std::cout << "  Submission successfull" << std::endl;
    ofs << "  Submission succesfull" << std::endl;
    return 1;
    }
  else
    {
    std::string url;
    if ( m_DartConfiguration["DropSiteUser"].size() > 0 )
      {
      url += m_DartConfiguration["DropSiteUser"] + "@";
      }
    url += m_DartConfiguration["DropSite"] + ":" + m_DartConfiguration["DropLocation"];
    
    if ( !submit.SubmitUsingSCP(m_DartConfiguration["ScpCommand"],
        m_ToplevelPath+"/Testing/"+m_CurrentTag, files, prefix, url) )
      {
      std::cerr << "  Problems when submitting via SCP" << std::endl;
      ofs << "  Problems when submitting via SCP" << std::endl;
      return 0;
      }
    std::cout << "  Submission successfull" << std::endl;
    ofs << "  Submission succesfull" << std::endl;
    }

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
    update_count = this->UpdateHandler->UpdateDirectory(this); 
    if ( update_count < 0 )
      {
      res |= cmCTest::UPDATE_ERRORS;
      }
    }
  if ( m_TestModel == cmCTest::CONTINUOUS && !update_count )
    {
    return 0;
    }
  if ( m_Tests[CONFIGURE_TEST] || m_Tests[ALL_TEST] )
    {
    if (this->ConfigureHandler->ConfigureDirectory(this))
      {
      res |= cmCTest::CONFIGURE_ERRORS;
      }
    }
  if ( m_Tests[BUILD_TEST] || m_Tests[ALL_TEST] )
    {
    this->UpdateCTestConfiguration();
    if (this->BuildHandler->BuildDirectory(this))
      {
      res |= cmCTest::BUILD_ERRORS;
      }
    }
  if ( m_Tests[TEST_TEST] || m_Tests[ALL_TEST] || notest )
    {
    this->UpdateCTestConfiguration();
    if (this->TestHandler->TestDirectory(this,false))
      {
      res |= cmCTest::TEST_ERRORS;
      }
    }
  if ( m_Tests[COVERAGE_TEST] || m_Tests[ALL_TEST] )
    {
    this->UpdateCTestConfiguration();
    if (this->CoverageHandler->CoverageDirectory(this))
      {
      res |= cmCTest::COVERAGE_ERRORS;
      }
    }
  if ( m_Tests[MEMCHECK_TEST] || m_Tests[ALL_TEST] )
    {
    this->UpdateCTestConfiguration();
    if (this->TestHandler->TestDirectory(this,true))
      {
      res |= cmCTest::MEMORY_ERRORS;
      }
    }
  if ( !notest )
    {
    std::string notes_dir = m_ToplevelPath + "/Testing/Notes";
    if ( cmSystemTools::FileIsDirectory(notes_dir.c_str()) )
      {
      cmsys::Directory d;
      d.Load(notes_dir.c_str());
      unsigned long kk;
      for ( kk = 0; kk < d.GetNumberOfFiles(); kk ++ )
        {
        const char* file = d.GetFile(kk);
        std::string fullname = notes_dir + "/" + file;
        if ( cmSystemTools::FileExists(fullname.c_str()) &&
          !cmSystemTools::FileIsDirectory(fullname.c_str()) )
          {
          if ( m_NotesFiles.size() > 0 )
            {
            m_NotesFiles += ";";
            }
          m_NotesFiles += fullname;
          m_Tests[NOTES_TEST] = 1;
          }
        }
      }
    }
  if ( m_Tests[NOTES_TEST] || m_Tests[ALL_TEST] )
    {
    this->UpdateCTestConfiguration();
    if ( m_NotesFiles.size() )
      {
      this->GenerateNotesFile(m_NotesFiles.c_str());
      }
    }
  if ( m_Tests[SUBMIT_TEST] || m_Tests[ALL_TEST] )
    {
    this->UpdateCTestConfiguration();
    this->SubmitResults();
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

int cmCTest::GetTestModelFromString(const char* str)
{
  if ( !str )
    {
    return cmCTest::EXPERIMENTAL;
    }
  std::string rstr = cmSystemTools::LowerCase(str);
  if ( strncmp(rstr.c_str(), "cont", 4) == 0 )
    {
    return cmCTest::CONTINUOUS;
    }
  if ( strncmp(rstr.c_str(), "nigh", 4) == 0 )
    {
    return cmCTest::NIGHTLY;
    }
  return cmCTest::EXPERIMENTAL;
}

int cmCTest::RunMakeCommand(const char* command, std::string* output,
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
  while(cmsysProcess_WaitForData(cp, &data, &length, 0))
    {
    if ( output )
      {
      for(int cc =0; cc < length; ++cc)
        {
        if(data[cc] == 0)
          {
          data[cc] = '\n';
          }
        }

      output->append(data, length);
      if ( !verbose )
        {
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
      }
    if(verbose)
      {
      std::cout.write(data, length);
      std::cout.flush();
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

  int result = cmsysProcess_GetState(cp);

  if(result == cmsysProcess_State_Exited)
    {
    *retVal = cmsysProcess_GetExitValue(cp);
    }
  else if(result == cmsysProcess_State_Exception)
    {
    *retVal = cmsysProcess_GetExitException(cp);
    std::cout << "There was an exception: " << *retVal << std::endl;
    }
  else if(result == cmsysProcess_State_Expired)
    {
    std::cout << "There was a timeout" << std::endl;
    } 
  else if(result == cmsysProcess_State_Error)
    {
    *output += "\n*** ERROR executing: ";
    *output += cmsysProcess_GetErrorString(cp);
    }

  cmsysProcess_Delete(cp);

  return result;
}

int cmCTest::RunTest(std::vector<const char*> argv, 
                     std::string* output, int *retVal,
                     std::ostream* log)
{
  if(cmSystemTools::SameFile(argv[0], m_CTestSelf.c_str()) && 
     !m_ForceNewCTestProcess)
    {
    cmCTest inst;
    inst.m_ConfigType = m_ConfigType;
    inst.m_TimeOut = m_TimeOut;
    std::vector<std::string> args;
    for(unsigned int i =0; i < argv.size(); ++i)
      {
      if(argv[i])
        {
        args.push_back(argv[i]);
        }
      }
    if ( *log )
      {
      *log << "* Run internal CTest" << std::endl;
      }
    std::string oldpath = cmSystemTools::GetCurrentWorkingDirectory();
    
    *retVal = inst.Run(args, output);
    if ( *log )
      {
      *log << output->c_str();
      }
    cmSystemTools::ChangeDirectory(oldpath.c_str());
    
    if(m_Verbose)
      {
      std::cout << "Internal cmCTest object used to run test.\n";
      std::cout <<  *output << "\n";
      }
    return cmsysProcess_State_Exited;
    }
  std::vector<char> tempOutput;
  if ( output )
    {
    *output = "";
    }

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*argv.begin());
  //  std::cout << "Command is: " << argv[0] << std::endl;
  if(cmSystemTools::GetRunCommandHideConsole())
    {
    cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
    }
  cmsysProcess_SetTimeout(cp, m_TimeOut);
  cmsysProcess_Execute(cp);

  char* data;
  int length;
  while(cmsysProcess_WaitForData(cp, &data, &length, 0))
    {
    if ( output )
      {
      tempOutput.insert(tempOutput.end(), data, data+length);
      }
    if ( m_Verbose )
      {
      std::cout.write(data, length);
      std::cout.flush();
      }
    if ( log )
      {
      log->write(data, length);
      log->flush();
      }
    }

  cmsysProcess_WaitForExit(cp, 0);
  if(output)
    {
    output->append(&*tempOutput.begin(), tempOutput.size());
    }
  if ( m_Verbose )
    {
    std::cout << "-- Process completed" << std::endl;
    }

  int result = cmsysProcess_GetState(cp);

  if(result == cmsysProcess_State_Exited)
    {
    *retVal = cmsysProcess_GetExitValue(cp);
    }
  else if(result == cmsysProcess_State_Exception)
    {
    *retVal = cmsysProcess_GetExitException(cp);
    std::string outerr = "\n*** Exception executing: ";
    outerr += cmsysProcess_GetExceptionString(cp);
    *output += outerr;
    if ( m_Verbose )
      {
      std::cout << outerr.c_str() << "\n";
      std::cout.flush();
      }
    }
  else if(result == cmsysProcess_State_Error)
    {
    std::string outerr = "\n*** ERROR executing: ";
    outerr += cmsysProcess_GetErrorString(cp);
    *output += outerr;
    if ( m_Verbose )
      {
      std::cout << outerr.c_str() << "\n";
      std::cout.flush();
      }
    }
  cmsysProcess_Delete(cp);

  return result;
}

void cmCTest::StartXML(std::ostream& ostr)
{
  ostr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    << "<Site BuildName=\"" << m_DartConfiguration["BuildName"]
    << "\" BuildStamp=\"" << m_CurrentTag << "-"
    << this->GetTestModelString() << "\" Name=\""
    << m_DartConfiguration["Site"] << "\" Generator=\"ctest"
    << cmVersion::GetCMakeVersion()
    << "\">" << std::endl;
}

void cmCTest::EndXML(std::ostream& ostr)
{
  ostr << "</Site>" << std::endl;
}

int cmCTest::GenerateDartNotesOutput(std::ostream& os, const cmCTest::tm_VectorOfStrings& files)
{
  cmCTest::tm_VectorOfStrings::const_iterator it;
  os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    << "<?xml-stylesheet type=\"text/xsl\" href=\"Dart/Source/Server/XSL/Build.xsl <file:///Dart/Source/Server/XSL/Build.xsl> \"?>\n"
    << "<Site BuildName=\"" << m_DartConfiguration["BuildName"] << "\" BuildStamp=\"" 
    << m_CurrentTag << "-" << this->GetTestModelString() << "\" Name=\"" 
    << m_DartConfiguration["Site"] << "\" Generator=\"ctest"
    << cmVersion::GetCMakeVersion()
    << "\">\n"
    << "<Notes>" << std::endl;

  for ( it = files.begin(); it != files.end(); it ++ )
    {
    std::cout << "\tAdd file: " << it->c_str() << std::endl;
    std::string note_time = this->CurrentTime();
    os << "<Note Name=\"" << this->MakeXMLSafe(it->c_str()) << "\">\n"
      << "<DateTime>" << note_time << "</DateTime>\n"
      << "<Text>" << std::endl;
    std::ifstream ifs(it->c_str());
    if ( ifs )
      {
      std::string line;
      while ( cmSystemTools::GetLineFromStream(ifs, line) )
        {
        os << this->MakeXMLSafe(line) << std::endl;
        }
      ifs.close();
      }
    else
      {
      os << "Problem reading file: " << it->c_str() << std::endl;
      std::cerr << "Problem reading file: " << it->c_str() << " while creating notes" << std::endl;
      }
    os << "</Text>\n"
      << "</Note>" << std::endl;
    }
  os << "</Notes>\n"
    << "</Site>" << std::endl;
  return 1;
}

int cmCTest::GenerateNotesFile(const char* cfiles)
{
  if ( !cfiles )
    {
    return 1;
    }

  std::vector<cmStdString> files;

  std::cout << "Create notes file" << std::endl;

  files = cmSystemTools::SplitString(cfiles, ';');
  if ( files.size() == 0 )
    {
    return 1;
    }

  std::ofstream ofs;
  if ( !this->OpenOutputFile(m_CurrentTag, "Notes.xml", ofs) )
    {
    std::cerr << "Cannot open notes file" << std::endl;
    return 1;
    }

  this->GenerateDartNotesOutput(ofs, files);
  return 0;
}

int cmCTest::Run(std::vector<std::string>const& args, std::string* output)
{
  this->FindRunningCMake(args[0].c_str());
  const char* ctestExec = "ctest";
  bool cmakeAndTest = false;
  bool performSomeTest = true;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-C",0) == 0 && i < args.size() - 1)
      {
      i++;
      this->m_ConfigType = args[i];
      cmSystemTools::ReplaceString(this->m_ConfigType, ".\\", "");
      }

    if( arg.find("-V",0) == 0 || arg.find("--verbose",0) == 0 )
      {
      this->m_Verbose = true;
      this->BuildHandler->SetVerbose(this->m_Verbose);
      this->ConfigureHandler->SetVerbose(this->m_Verbose);
      this->CoverageHandler->SetVerbose(this->m_Verbose);
      this->ScriptHandler->SetVerbose(this->m_Verbose);
      this->TestHandler->SetVerbose(this->m_Verbose);
      this->UpdateHandler->SetVerbose(this->m_Verbose);
      }

    if( arg.find("-N",0) == 0 || arg.find("--show-only",0) == 0 )
      {
      this->m_ShowOnly = true;
      }

    if( arg.find("-S",0) == 0 && i < args.size() - 1 )
      {
      this->m_RunConfigurationScript = true;
      i++;
      this->ScriptHandler->AddConfigurationScript(args[i].c_str());
      }

    if( arg.find("--tomorrow-tag",0) == 0 )
      {
      m_TomorrowTag = true;
      }
    if( arg.find("--force-new-ctest-process",0) == 0 )
      {
      m_ForceNewCTestProcess = true;
      }
    if( arg.find("--interactive-debug-mode",0) == 0 && i < args.size() - 1 )
      {
      i++;
      m_InteractiveDebugMode = cmSystemTools::IsOn(args[i].c_str());
      }
    if( arg.find("-D",0) == 0 && i < args.size() - 1 )
      {
      this->m_DartMode = true;
      i++;
      std::string targ = args[i];
      if ( targ == "Experimental" )
        {
        this->SetTestModel(cmCTest::EXPERIMENTAL);
        this->SetTest("Start");
        this->SetTest("Configure");
        this->SetTest("Build");
        this->SetTest("Test");
        this->SetTest("Coverage");
        this->SetTest("Submit");
        }
      else if ( targ == "ExperimentalStart" )
        {
        this->SetTestModel(cmCTest::EXPERIMENTAL);
        this->SetTest("Start");
        }
      else if ( targ == "ExperimentalUpdate" )
        {
        this->SetTestModel(cmCTest::EXPERIMENTAL);
        this->SetTest("Update");
        }
      else if ( targ == "ExperimentalConfigure" )
        {
        this->SetTestModel(cmCTest::EXPERIMENTAL);
        this->SetTest("Configure");
        }
      else if ( targ == "ExperimentalBuild" )
        {
        this->SetTestModel(cmCTest::EXPERIMENTAL);
        this->SetTest("Build");
        }
      else if ( targ == "ExperimentalTest" )
        {
        this->SetTestModel(cmCTest::EXPERIMENTAL);
        this->SetTest("Test");
        }
      else if ( targ == "ExperimentalMemCheck"
        || targ == "ExperimentalPurify" )
        {
        this->SetTestModel(cmCTest::EXPERIMENTAL);
        this->SetTest("MemCheck");
        }
      else if ( targ == "ExperimentalCoverage" )
        {
        this->SetTestModel(cmCTest::EXPERIMENTAL);
        this->SetTest("Coverage");
        }
      else if ( targ == "ExperimentalSubmit" )
        {
        this->SetTestModel(cmCTest::EXPERIMENTAL);
        this->SetTest("Submit");
        }
      else if ( targ == "Continuous" )
        {
        this->SetTestModel(cmCTest::CONTINUOUS);
        this->SetTest("Start");
        this->SetTest("Update");
        this->SetTest("Configure");
        this->SetTest("Build");
        this->SetTest("Test");
        this->SetTest("Coverage");
        this->SetTest("Submit");
        }
      else if ( targ == "ContinuousStart" )
        {
        this->SetTestModel(cmCTest::CONTINUOUS);
        this->SetTest("Start");
        }
      else if ( targ == "ContinuousUpdate" )
        {
        this->SetTestModel(cmCTest::CONTINUOUS);
        this->SetTest("Update");
        }  
      else if ( targ == "ContinuousConfigure" )
        {
        this->SetTestModel(cmCTest::CONTINUOUS);
        this->SetTest("Configure");
        }
      else if ( targ == "ContinuousBuild" )
        {
        this->SetTestModel(cmCTest::CONTINUOUS);
        this->SetTest("Build");
        }
      else if ( targ == "ContinuousTest" )
        {
        this->SetTestModel(cmCTest::CONTINUOUS);
        this->SetTest("Test");
        }
      else if ( targ == "ContinuousMemCheck"
        || targ == "ContinuousPurify" )
        {
        this->SetTestModel(cmCTest::CONTINUOUS);
        this->SetTest("MemCheck");
        }
      else if ( targ == "ContinuousCoverage" )
        {
        this->SetTestModel(cmCTest::CONTINUOUS);
        this->SetTest("Coverage");
        }
      else if ( targ == "ContinuousSubmit" )
        {
        this->SetTestModel(cmCTest::CONTINUOUS);
        this->SetTest("Submit");
        }
      else if ( targ == "Nightly" )
        {
        this->SetTestModel(cmCTest::NIGHTLY);
        this->SetTest("Start");
        this->SetTest("Update");
        this->SetTest("Configure");
        this->SetTest("Build");
        this->SetTest("Test");
        this->SetTest("Coverage");
        this->SetTest("Submit");
        }
      else if ( targ == "NightlyStart" )
        {
        this->SetTestModel(cmCTest::NIGHTLY);
        this->SetTest("Start");
        }
      else if ( targ == "NightlyUpdate" )
        {
        this->SetTestModel(cmCTest::NIGHTLY);
        this->SetTest("Update");
        }
      else if ( targ == "NightlyConfigure" )
        {
        this->SetTestModel(cmCTest::NIGHTLY);
        this->SetTest("Configure");
        }
      else if ( targ == "NightlyBuild" )
        {
        this->SetTestModel(cmCTest::NIGHTLY);
        this->SetTest("Build");
        }
      else if ( targ == "NightlyTest" )
        {
        this->SetTestModel(cmCTest::NIGHTLY);
        this->SetTest("Test");
        }
      else if ( targ == "NightlyMemCheck"
        || targ == "NightlyPurify" )
        {
        this->SetTestModel(cmCTest::NIGHTLY);
        this->SetTest("MemCheck");
        }
      else if ( targ == "NightlyCoverage" )
        {
        this->SetTestModel(cmCTest::NIGHTLY);
        this->SetTest("Coverage");
        }
      else if ( targ == "NightlySubmit" )
        {
        this->SetTestModel(cmCTest::NIGHTLY);
        this->SetTest("Submit");
        }
      else if ( targ == "MemoryCheck" )
        {
        this->SetTestModel(cmCTest::EXPERIMENTAL);
        this->SetTest("Start");
        this->SetTest("Configure");
        this->SetTest("Build");
        this->SetTest("MemCheck");
        this->SetTest("Coverage");
        this->SetTest("Submit");
        }
      else if ( targ == "NightlyMemoryCheck" )
        {
        this->SetTestModel(cmCTest::NIGHTLY);
        this->SetTest("Start");
        this->SetTest("Update");
        this->SetTest("Configure");
        this->SetTest("Build");
        this->SetTest("MemCheck");
        this->SetTest("Coverage");
        this->SetTest("Submit");
        }
      else
        {
        performSomeTest = false;
        std::cerr << "CTest -D called with incorrect option: " << targ << std::endl;
        std::cerr << "Available options are:" << std::endl
          << "  " << ctestExec << " -D Continuous" << std::endl
          << "  " << ctestExec << " -D Continuous(Start|Update|Configure|Build)" << std::endl
          << "  " << ctestExec << " -D Continuous(Test|Coverage|MemCheck|Submit)" << std::endl
          << "  " << ctestExec << " -D Experimental" << std::endl
          << "  " << ctestExec << " -D Experimental(Start|Update|Configure|Build)" << std::endl
          << "  " << ctestExec << " -D Experimental(Test|Coverage|MemCheck|Submit)" << std::endl
          << "  " << ctestExec << " -D Nightly" << std::endl
          << "  " << ctestExec << " -D Nightly(Start|Update|Configure|Build)" << std::endl
          << "  " << ctestExec << " -D Nightly(Test|Coverage|MemCheck|Submit)" << std::endl
          << "  " << ctestExec << " -D NightlyMemoryCheck" << std::endl;
          ;
        }
      }

    if( ( arg.find("-T",0) == 0 ) && 
      (i < args.size() -1) )
      {
      this->m_DartMode = true;
      i++;
      if ( !this->SetTest(args[i].c_str(), false) )
        {
        performSomeTest = false;
        std::cerr << "CTest -T called with incorrect option: " << args[i].c_str() << std::endl;
        std::cerr << "Available options are:" << std::endl
          << "  " << ctestExec << " -T all" << std::endl
          << "  " << ctestExec << " -T start" << std::endl
          << "  " << ctestExec << " -T update" << std::endl
          << "  " << ctestExec << " -T configure" << std::endl
          << "  " << ctestExec << " -T build" << std::endl
          << "  " << ctestExec << " -T test" << std::endl
          << "  " << ctestExec << " -T coverage" << std::endl
          << "  " << ctestExec << " -T memcheck" << std::endl
          << "  " << ctestExec << " -T notes" << std::endl
          << "  " << ctestExec << " -T submit" << std::endl;
        }
      }

    if( ( arg.find("-M",0) == 0 || arg.find("--test-model",0) == 0 ) &&
      (i < args.size() -1) )
      {
      i++;
      std::string const& str = args[i];
      if ( cmSystemTools::LowerCase(str) == "nightly" )
        {
        this->SetTestModel(cmCTest::NIGHTLY);
        }
      else if ( cmSystemTools::LowerCase(str) == "continuous" )
        {
        this->SetTestModel(cmCTest::CONTINUOUS);
        }
      else if ( cmSystemTools::LowerCase(str) == "experimental" )
        {
        this->SetTestModel(cmCTest::EXPERIMENTAL);
        }
      else
        {
        performSomeTest = false;
        std::cerr << "CTest -M called with incorrect option: " << str.c_str() << std::endl;
        std::cerr << "Available options are:" << std::endl
          << "  " << ctestExec << " -M Continuous" << std::endl
          << "  " << ctestExec << " -M Experimental" << std::endl
          << "  " << ctestExec << " -M Nightly" << std::endl;
        }
      }

    if(arg.find("-I",0) == 0 && i < args.size() - 1)
      {
      i++;
      this->TestHandler->SetTestsToRunInformation(args[i].c_str());
      }
    if(arg.find("-U",0) == 0)
      {
      this->TestHandler->SetUseUnion(true);
      }
    if(arg.find("-R",0) == 0 && i < args.size() - 1)
      {
      this->TestHandler->UseIncludeRegExp();
      i++;
      this->TestHandler->SetIncludeRegExp(args[i].c_str());
      }

    if(arg.find("-E",0) == 0 && i < args.size() - 1)
      {
      this->TestHandler->UseExcludeRegExp();
      i++;
      this->TestHandler->SetExcludeRegExp(args[i].c_str());
      }

    if(arg.find("-A",0) == 0 && i < args.size() - 1)
      {
      this->m_DartMode = true;
      this->SetTest("Notes");
      i++;
      this->SetNotesFiles(args[i].c_str());
      }

    // --build-and-test options
    if(arg.find("--build-and-test",0) == 0 && i < args.size() - 1)
      {
      cmakeAndTest = true;
      if(i+2 < args.size())
        {
        i++;
        m_SourceDir = args[i];
        i++;
        m_BinaryDir = args[i];
        // dir must exist before CollapseFullPath is called
        cmSystemTools::MakeDirectory(m_BinaryDir.c_str());
        m_BinaryDir = cmSystemTools::CollapseFullPath(m_BinaryDir.c_str());
        m_SourceDir = cmSystemTools::CollapseFullPath(m_SourceDir.c_str());
        }
      else
        {
        std::cerr << "--build-and-test must have source and binary dir\n";
        }
      }
    if(arg.find("--build-target",0) == 0 && i < args.size() - 1)
      {
      i++;
      m_BuildTarget = args[i];
      }
    if(arg.find("--build-nocmake",0) == 0)
      {
      m_BuildNoCMake = true;
      }
    if(arg.find("--build-run-dir",0) == 0 && i < args.size() - 1)
      {
      i++;
      m_BuildRunDir = args[i];
      }
    if(arg.find("--build-two-config",0) == 0)
      {
      m_BuildTwoConfig = true;
      }
    if(arg.find("--build-exe-dir",0) == 0 && i < args.size() - 1)
      {
      i++;
      m_ExecutableDirectory = args[i];
      }
    if(arg.find("--build-generator",0) == 0 && i < args.size() - 1)
      {
      i++;
      m_BuildGenerator = args[i];
      }
    if(arg.find("--build-project",0) == 0 && i < args.size() - 1)
      {
      i++;
      m_BuildProject = args[i];
      }
    if(arg.find("--build-makeprogram",0) == 0 && i < args.size() - 1)
      {
      i++;
      m_BuildMakeProgram = args[i];
      }
    if(arg.find("--build-noclean",0) == 0)
      {
      m_BuildNoClean = true;
      }
    if(arg.find("--build-options",0) == 0 && i < args.size() - 1)
      {
      ++i;
      bool done = false;
      while(i < args.size() && !done)
        {
        m_BuildOptions.push_back(args[i]);
        if(i+1 < args.size() 
          && (args[i+1] == "--build-target" || args[i+1] == "--test-command"))
          {
          done = true;
          }
        else
          {
          ++i;
          }
        }
      }
    if(arg.find("--test-command",0) == 0 && i < args.size() - 1)
      {
      ++i;
      m_TestCommand = args[i];
      while(i+1 < args.size())
        {
        ++i;
        m_TestCommandArgs.push_back(args[i]);
        }
      }
    }

  if(cmakeAndTest)
    {
    cmSystemTools::ResetErrorOccuredFlag();
    cmListFileCache::ClearCache();
    int retv = this->RunCMakeAndTest(output);
    cmSystemTools::ResetErrorOccuredFlag();
    cmListFileCache::ClearCache();
#ifdef CMAKE_BUILD_WITH_CMAKE
    cmDynamicLoader::FlushCache();
#endif
    return retv;
    }

  if(performSomeTest )
    {
    int res;
    // call process directory
    if (this->m_RunConfigurationScript)
      {
      res = this->ScriptHandler->RunConfigurationScript(this);
      }
    else
      {
      if ( !this->Initialize() )
        {
        res = 12;
        }
      else
        {
        res = this->ProcessTests();
        }
      this->Finalize();
      }
    return res;
    }
  return 1;
}

void cmCTest::FindRunningCMake(const char* arg0)
{
  // Find our own executable.
  std::vector<cmStdString> failures;
  m_CTestSelf = arg0;
  cmSystemTools::ConvertToUnixSlashes(m_CTestSelf);
  failures.push_back(m_CTestSelf);
  m_CTestSelf = cmSystemTools::FindProgram(m_CTestSelf.c_str());
  if(!cmSystemTools::FileExists(m_CTestSelf.c_str()))
    {
    failures.push_back(m_CTestSelf);
    m_CTestSelf =  "/usr/local/bin/ctest";
    }
  if(!cmSystemTools::FileExists(m_CTestSelf.c_str()))
    {
    failures.push_back(m_CTestSelf);
    cmOStringStream msg;
    msg << "CTEST can not find the command line program cmake.\n";
    msg << "  argv[0] = \"" << arg0 << "\"\n";
    msg << "  Attempted paths:\n";
    std::vector<cmStdString>::iterator i;
    for(i=failures.begin(); i != failures.end(); ++i)
      {
      msg << "    \"" << i->c_str() << "\"\n";
      }
    cmSystemTools::Error(msg.str().c_str());
    }
  std::string dir;
  std::string file;
  if(cmSystemTools::SplitProgramPath(m_CTestSelf.c_str(),
      dir,
      file,
      true))
    {
    m_CMakeSelf = dir += "/cmake";
    m_CMakeSelf += cmSystemTools::GetExecutableExtension();
    if(!cmSystemTools::FileExists(m_CMakeSelf.c_str()))
      {
      cmOStringStream msg;
      failures.push_back(m_CMakeSelf);
      msg << "CTEST can not find the command line program cmake.\n";
      msg << "  argv[0] = \"" << arg0 << "\"\n";
      msg << "  Attempted path:\n";
      msg << "    \"" << m_CMakeSelf.c_str() << "\"\n"; 
      cmSystemTools::Error(msg.str().c_str());
      }
    }
}

void CMakeMessageCallback(const char* m, const char*, bool&, void* s)
{
  std::string* out = (std::string*)s;
  *out += m;
  *out += "\n";
}

void CMakeStdoutCallback(const char* m, int len, void* s)
{
  std::string* out = (std::string*)s;
  out->append(m, len);
}


int cmCTest::RunCMakeAndTest(std::string* outstring)
{  
  unsigned int k;
  std::string cmakeOutString;
  cmSystemTools::SetErrorCallback(CMakeMessageCallback, &cmakeOutString);
  cmSystemTools::SetStdoutCallback(CMakeStdoutCallback, &cmakeOutString);
  cmOStringStream out;
  cmake cm;
  double timeout = m_TimeOut;
  // default to the build type of ctest itself
  if(m_ConfigType.size() == 0)
    {
#ifdef  CMAKE_INTDIR
    m_ConfigType = CMAKE_INTDIR;
#endif
    }

  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  out << "Internal cmake changing into directory: " << m_BinaryDir << "\n";  
  if (!cmSystemTools::FileIsDirectory(m_BinaryDir.c_str()))
    {
    cmSystemTools::MakeDirectory(m_BinaryDir.c_str());
    }
  cmSystemTools::ChangeDirectory(m_BinaryDir.c_str());
  if(!m_BuildNoCMake)
    {
    std::vector<std::string> args;
    args.push_back(m_CMakeSelf);
    args.push_back(m_SourceDir);
    if(m_BuildGenerator.size())
      {
      std::string generator = "-G";
      generator += m_BuildGenerator;
      args.push_back(generator);
      }
    if ( m_ConfigType.size() > 0 )
      {
      std::string btype = "-DBUILD_TYPE:STRING=" + m_ConfigType;
      args.push_back(btype);
      }

    for(k=0; k < m_BuildOptions.size(); ++k)
      {
      args.push_back(m_BuildOptions[k]);
      }
    if (cm.Run(args) != 0)
      {
      out << "Error: cmake execution failed\n";
      out << cmakeOutString << "\n";
      // return to the original directory
      cmSystemTools::ChangeDirectory(cwd.c_str());
      if(outstring)
        {
        *outstring = out.str();
        }
      else
        {
        std::cerr << out.str() << "\n";
        }
      return 1;
      }
    if(m_BuildTwoConfig)
      {
      if (cm.Run(args) != 0)
        {
        out << "Error: cmake execution failed\n";
        out << cmakeOutString << "\n";
        // return to the original directory
        cmSystemTools::ChangeDirectory(cwd.c_str());
        if(outstring)
          {
          *outstring = out.str();
          }
        else
          {
          std::cerr << out.str() << "\n";
          }
        return 1;
        }
      } 
    }

  cmSystemTools::SetErrorCallback(0, 0);
  out << cmakeOutString << "\n";
  if(m_BuildMakeProgram.size() == 0)
    {
    out << "Error: cmake does not have a valid MAKEPROGRAM\n";
    out << "Did you specify a --build-makeprogram and a --build-generator?\n";
    if(outstring)
      {
      *outstring = out.str();
      }
    else
      {
      std::cerr << out.str() << "\n";
      }
    return 1;
    }
  int retVal = 0;
  std::string makeCommand = cmSystemTools::ConvertToOutputPath(m_BuildMakeProgram.c_str());
  std::string lowerCaseCommand = cmSystemTools::LowerCase(makeCommand);
  // if msdev is the make program then do the following
  // MSDEV 6.0
  if(lowerCaseCommand.find("msdev") != std::string::npos)
    {
    // if there are spaces in the makeCommand, assume a full path
    // and convert it to a path with no spaces in it as the
    // RunSingleCommand does not like spaces
#if defined(_WIN32) && !defined(__CYGWIN__)      
    if(makeCommand.find(' ') != std::string::npos)
      {
      cmSystemTools::GetShortPath(makeCommand.c_str(), makeCommand);
      }
#endif
    makeCommand += " ";
    makeCommand += m_BuildProject;
    makeCommand += ".dsw /MAKE \"ALL_BUILD - ";
    makeCommand += m_ConfigType;
    if(m_BuildNoClean)
      {
      makeCommand += "\" /BUILD";
      }
    else
      {
      makeCommand += "\" /REBUILD";
      }
    }
  // MSDEV 7.0 .NET
  else if (lowerCaseCommand.find("devenv") != std::string::npos)
    {
#if defined(_WIN32) && !defined(__CYGWIN__)      
    if(makeCommand.find(' ') != std::string::npos)
      {
      cmSystemTools::GetShortPath(makeCommand.c_str(), makeCommand);
      }
#endif
    makeCommand += " ";
    makeCommand += m_BuildProject;
    makeCommand += ".sln ";
    if(m_BuildNoClean)
      {
      makeCommand += "/build ";
      }
    else
      {
      makeCommand += "/rebuild ";
      }
    makeCommand += m_ConfigType + " /project ALL_BUILD";
    }
  else if (lowerCaseCommand.find("make") != std::string::npos)
    {
    // assume a make sytle program
    // clean first
    if(!m_BuildNoClean)
      {
      std::string cleanCommand = makeCommand;
      cleanCommand += " clean";
      out << "Running make clean command: " << cleanCommand.c_str() << " ...\n";
      retVal = 0;
      std::string output;
      if (!cmSystemTools::RunSingleCommand(cleanCommand.c_str(), &output, &retVal, 0,
                                           false, timeout) ||
        retVal)
        {
        out << "Error: " << cleanCommand.c_str() << "  execution failed\n";
        out << output.c_str() << "\n";
        // return to the original directory
        cmSystemTools::ChangeDirectory(cwd.c_str());
        out << "Return value: " << retVal << std::endl;
        if(outstring)
          {
          *outstring = out.str();
          }
        else
          {
          std::cerr << out.str() << "\n";
          }
        return 1;
        }
      out << output;
      }

    if(m_BuildTarget.size())
      {
      makeCommand +=  " ";
      makeCommand += m_BuildTarget;
      }
    }

  // command line make program

  out << "Running make command: " << makeCommand.c_str() << "\n";
  retVal = 0;
  std::string output;
  if (!cmSystemTools::RunSingleCommand(makeCommand.c_str(), &output, &retVal, 0, false, timeout))
    {
    out << "Error: " << makeCommand.c_str() <<  "  execution failed\n";
    out << output.c_str() << "\n";
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    out << "Return value: " << retVal << std::endl;
    if(outstring)
      {
      *outstring = out.str();
      }
    else
      {
      std::cerr << out.str() << "\n";
      }
    return 1;
    }
  if ( retVal )
    {
    if(outstring)
      {
      *outstring = out.str();
      *outstring += "Building of project failed\n";
      *outstring += output;
      *outstring += "\n";
      }
    else
      {
      std::cerr << "Building of project failed\n";
      std::cerr << out.str() << output << "\n";
      }
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }
  out << output;

  if(m_TestCommand.size() == 0)
    {
    if(outstring)
      {
      *outstring = out.str();
      }
    else
      {
      std::cout << out.str() << "\n";
      }
    return retVal;
    }

  // now run the compiled test if we can find it
  std::vector<std::string> attempted;
  std::vector<std::string> failed;
  std::string tempPath;
  std::string filepath = 
    cmSystemTools::GetFilenamePath(m_TestCommand);
  std::string filename = 
    cmSystemTools::GetFilenameName(m_TestCommand);
  // if full path specified then search that first
  if (filepath.size())
    {
    tempPath = filepath;
    tempPath += "/";
    tempPath += filename;
    attempted.push_back(tempPath);
    if(m_ConfigType.size())
      {
      tempPath = filepath;
      tempPath += "/";
      tempPath += m_ConfigType;
      tempPath += "/";
      tempPath += filename;
      attempted.push_back(tempPath);
      }
    }
  // otherwise search local dirs
  else
    {
    attempted.push_back(filename);
    if(m_ConfigType.size())
      {
      tempPath = m_ConfigType;
      tempPath += "/";
      tempPath += filename;
      attempted.push_back(tempPath);
      }
    }
  // if m_ExecutableDirectory is set try that as well
  if (m_ExecutableDirectory.size())
    {
    tempPath = m_ExecutableDirectory;
    tempPath += "/";
    tempPath += m_TestCommand;
    attempted.push_back(tempPath);
    if(m_ConfigType.size())
      {
      tempPath = m_ExecutableDirectory;
      tempPath += "/";
      tempPath += m_ConfigType;
      tempPath += "/";
      tempPath += filename;
      attempted.push_back(tempPath);
      }
    }

  // store the final location in fullPath
  std::string fullPath;
  
  // now look in the paths we specified above
  for(unsigned int ai=0; 
      ai < attempted.size() && fullPath.size() == 0; ++ai)
    {
    // first check without exe extension
    if(cmSystemTools::FileExists(attempted[ai].c_str())
       && !cmSystemTools::FileIsDirectory(attempted[ai].c_str()))
      {
      fullPath = cmSystemTools::CollapseFullPath(attempted[ai].c_str());
      }
    // then try with the exe extension
    else
      {
      failed.push_back(attempted[ai].c_str());
      tempPath = attempted[ai];
      tempPath += cmSystemTools::GetExecutableExtension();
      if(cmSystemTools::FileExists(tempPath.c_str())
         && !cmSystemTools::FileIsDirectory(tempPath.c_str()))
        {
        fullPath = cmSystemTools::CollapseFullPath(tempPath.c_str());
        }
      else
        {
        failed.push_back(tempPath.c_str());
        }
      }
    }

  if(!cmSystemTools::FileExists(fullPath.c_str()))
    {
    out << "Could not find path to executable, perhaps it was not built: " <<
      m_TestCommand << "\n";
    out << "tried to find it in these places:\n";
    out << fullPath.c_str() << "\n";
    for(unsigned int i=0; i < failed.size(); ++i)
      {
      out << failed[i] << "\n";
      }
    if(outstring)
      {
      *outstring =  out.str();
      }
    else
      {
      std::cerr << out.str();
      }
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }

  std::vector<const char*> testCommand;
  testCommand.push_back(fullPath.c_str());
  for(k=0; k < m_TestCommandArgs.size(); ++k)
    {
    testCommand.push_back(m_TestCommandArgs[k].c_str());
    }
  testCommand.push_back(0);
  std::string outs;
  int retval = 0;
  // run the test from the m_BuildRunDir if set
  if(m_BuildRunDir.size())
    {
    out << "Run test in directory: " << m_BuildRunDir << "\n";
    cmSystemTools::ChangeDirectory(m_BuildRunDir.c_str());
    }
  out << "Running test executable: " << fullPath << " ";
  for(k=0; k < m_TestCommandArgs.size(); ++k)
    {
    out << m_TestCommandArgs[k] << " ";
    }
  out << "\n";
  m_TimeOut = timeout;
  int runTestRes = this->RunTest(testCommand, &outs, &retval, 0);
  if(runTestRes != cmsysProcess_State_Exited || retval != 0)
    {
    out << "Failed to run test command: " << testCommand[0] << "\n";
    retval = 1;
    }

  out << outs << "\n";
  if(outstring)
    {
    *outstring = out.str();
    }
  else
    {
    std::cout << out.str() << "\n";
    }
  return retval;
}

void cmCTest::SetNotesFiles(const char* notes)
{
  if ( !notes )
    {
    return;
    }
  m_NotesFiles = notes;
}

int cmCTest::ReadCustomConfigurationFileTree(const char* dir)
{
  tm_VectorOfStrings dirs;
  tm_VectorOfStrings ndirs;
  dirs.push_back(dir);
  cmake cm;
  cmGlobalGenerator gg;
  gg.SetCMakeInstance(&cm);
  std::auto_ptr<cmLocalGenerator> lg(gg.CreateLocalGenerator());
  lg->SetGlobalGenerator(&gg);
  cmMakefile *mf = lg->GetMakefile();

  while ( dirs.size() > 0 )
    {
    tm_VectorOfStrings::iterator cdir = dirs.end()-1;
    std::string rexpr = *cdir + "/*";
    std::string fname = *cdir + "/CTestCustom.ctest";
    if ( cmSystemTools::FileExists(fname.c_str()) && 
      (!lg->GetMakefile()->ReadListFile(0, fname.c_str()) ||
       cmSystemTools::GetErrorOccuredFlag() ) )
      {
      std::cerr << "Problem reading custom configuration" << std::endl;
      }
    dirs.erase(dirs.end()-1, dirs.end());
    cmSystemTools::SimpleGlob(rexpr, ndirs, -1);
    dirs.insert(dirs.end(), ndirs.begin(), ndirs.end());
    }

  this->BuildHandler->PopulateCustomVectors(mf);
  this->TestHandler->PopulateCustomVectors(mf);
  
  return 1;
}

void cmCTest::PopulateCustomVector(cmMakefile* mf, const char* def, tm_VectorOfStrings& vec)
{
  if ( !def)
    {
    return;
    }
  const char* dval = mf->GetDefinition(def);
  if ( !dval )
    {
    return;
    }
  std::vector<std::string> slist;
  cmSystemTools::ExpandListArgument(dval, slist);
  std::vector<std::string>::iterator it;

  for ( it = slist.begin(); it != slist.end(); ++it )
    {
    vec.push_back(it->c_str());
    }
}

std::string cmCTest::GetShortPathToFile(const char* cfname)
{
  const std::string& sourceDir = GetDartConfiguration("SourceDirectory");
  const std::string& buildDir = GetDartConfiguration("BuildDirectory");
  std::string fname = cmSystemTools::CollapseFullPath(cfname);

  // Find relative paths to both directories
  std::string srcRelpath
    = cmSystemTools::RelativePath(sourceDir.c_str(), fname.c_str());
  std::string bldRelpath
    = cmSystemTools::RelativePath(buildDir.c_str(), fname.c_str());

  // If any contains "." it is not parent directory
  bool inSrc = srcRelpath.find("..") == srcRelpath.npos;
  bool inBld = bldRelpath.find("..") == bldRelpath.npos;
  // TODO: Handle files with .. in their name

  std::string* res = 0;

  if ( inSrc && inBld )
    {
    // If both have relative path with no dots, pick the shorter one
    if ( srcRelpath.size() < bldRelpath.size() )
      {
      res = &srcRelpath;
      }
    else
      {
      res = &bldRelpath;
      }
    }
  else if ( inSrc )
    {
    res = &srcRelpath;
    }
  else if ( inBld )
    {
    res = &bldRelpath;
    }
  if ( !res )
    {
    return fname;
    }
  cmSystemTools::ConvertToUnixSlashes(*res);

  std::string path = "./" + *res;
  if ( path[path.size()-1] == '/' )
    {
    path = path.substr(0, path.size()-1);
    }
  return path;
}

std::string cmCTest::GetDartConfiguration(const char *name)
{
  return m_DartConfiguration[name];
}

  
std::string cmCTest::GetCurrentTag()
{
  return m_CurrentTag;
}

std::string cmCTest::GetToplevelPath()
{
  return m_ToplevelPath;
}

std::string cmCTest::GetConfigType()
{
  return m_ConfigType;
}

bool cmCTest::GetShowOnly()
{
  return m_ShowOnly;
}

bool cmCTest::GetProduceXML()
{
  return m_DartMode;
}
