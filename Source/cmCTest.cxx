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
#include "cmcurl/curl/curl.h"

#include "cmCTest.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include <cmsys/Directory.hxx>
#include "cmGlob.h"
#include "cmDynamicLoader.h"
#include "cmGeneratedFileStream.h"
#include "cmCTestCommand.h"

#include "cmCTestBuildHandler.h"
#include "cmCTestBuildAndTestHandler.h"
#include "cmCTestConfigureHandler.h"
#include "cmCTestCoverageHandler.h"
#include "cmCTestMemCheckHandler.h"
#include "cmCTestScriptHandler.h"
#include "cmCTestTestHandler.h"
#include "cmCTestUpdateHandler.h"
#include "cmCTestSubmitHandler.h"

#include "cmVersion.h"

#include <cmsys/RegularExpression.hxx>
#include <cmsys/Process.h>

#include <stdlib.h> 
#include <math.h>
#include <float.h>

#include <memory> // auto_ptr

#define DEBUGOUT std::cout << __LINE__ << " "; std::cout
#define DEBUGERR std::cerr << __LINE__ << " "; std::cerr

//----------------------------------------------------------------------
struct tm* cmCTest::GetNightlyTime(std::string str, 
                                   bool tomorrowtag)
{
  struct tm* lctime;
  time_t tctime = time(0);
  cmCTestLog(this, OUTPUT, "Determine Nightly Start Time" << std::endl
    << "   Specified time: " << str.c_str() << std::endl);
  //Convert the nightly start time to seconds. Since we are
  //providing only a time and a timezone, the current date of
  //the local machine is assumed. Consequently, nightlySeconds
  //is the time at which the nightly dashboard was opened or
  //will be opened on the date of the current client machine.
  //As such, this time may be in the past or in the future.
  time_t ntime = curl_getdate(str.c_str(), &tctime);
  cmCTestLog(this, DEBUG, "   Get curl time: " << ntime << std::endl);
  tctime = time(0);
  cmCTestLog(this, DEBUG, "   Get the current time: " << tctime << std::endl);

  const int dayLength = 24 * 60 * 60;
  cmCTestLog(this, DEBUG, "Seconds: " << tctime << std::endl);
  while ( ntime > tctime )
    {
    // If nightlySeconds is in the past, this is the current
    // open dashboard, then return nightlySeconds.  If
    // nightlySeconds is in the future, this is the next
    // dashboard to be opened, so subtract 24 hours to get the
    // time of the current open dashboard
    ntime -= dayLength;
    cmCTestLog(this, DEBUG, "Pick yesterday" << std::endl);
    cmCTestLog(this, DEBUG, "   Future time, subtract day: " << ntime << std::endl);
    }
  while ( tctime > (ntime + dayLength) )
    {
    ntime += dayLength;
    cmCTestLog(this, DEBUG, "   Past time, add day: " << ntime << std::endl);
    }
  cmCTestLog(this, DEBUG, "nightlySeconds: " << ntime << std::endl);
  cmCTestLog(this, DEBUG, "   Current time: " << tctime
    << " Nightly time: " << ntime << std::endl);
  if ( tomorrowtag )
    {
    cmCTestLog(this, OUTPUT, "   Use future tag, Add a day" << std::endl);
    ntime += dayLength;
    }
  lctime = gmtime(&ntime);
  return lctime;
}

//----------------------------------------------------------------------
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

//----------------------------------------------------------------------
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
  cmCTestLog(this, DEBUG, "   Current_Time: " << current_time << std::endl);
  return cmCTest::MakeXMLSafe(cmCTest::CleanString(current_time));
}


//----------------------------------------------------------------------
std::string cmCTest::MakeXMLSafe(const std::string& str)
{
  std::vector<char> result;
  result.reserve(500);
  const char* pos = str.c_str();
  for ( ;*pos; ++pos)
    {
    char ch = *pos;
    if ( (ch > 126 || ch < 32) && ch != 9  && ch != 10 && ch != 13 && ch != '\r' )
      {
      char buffer[33];
      sprintf(buffer, "&lt;%d&gt;", (int)ch);
      //sprintf(buffer, "&#x%0x;", (unsigned int)ch);
      result.insert(result.end(), buffer, buffer+strlen(buffer));
      }
    else
      {
      const char* const encodedChars[] = {
        "&amp;",
        "&lt;",
        "&gt;"
      };
      switch ( ch )
        {
        case '&':
          result.insert(result.end(), encodedChars[0], encodedChars[0]+5);
          break;
        case '<':
          result.insert(result.end(), encodedChars[1], encodedChars[1]+4);
          break;
        case '>':
          result.insert(result.end(), encodedChars[2], encodedChars[2]+4);
          break;
        case '\n':
          result.push_back('\n');
          break;
        case '\r': break; // Ignore \r
        default:
          result.push_back(ch);
        }
      }
    }
  return std::string(&*result.begin(), result.size());
}

//----------------------------------------------------------------------
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

//----------------------------------------------------------------------
cmCTest::cmCTest() 
{ 
  m_ForceNewCTestProcess   = false;
  m_TomorrowTag            = false;
  m_Verbose                = false;
  m_Debug                  = false;
  m_ShowLineNumbers        = false;
  m_Quiet                  = false;
  m_ExtraVerbose           = false;
  m_ProduceXML             = false;
  m_ShowOnly               = false;
  m_RunConfigurationScript = false;
  m_TestModel              = cmCTest::EXPERIMENTAL;
  m_InteractiveDebugMode   = true;
  m_TimeOut                = 0;
  m_CompressXMLFiles       = false;
  m_CTestConfigFile        = "";
  m_OutputLogFile          = 0;
  m_OutputLogFileLastTag   = -1;
  m_SuppressUpdatingCTestConfiguration = false;
  m_DartVersion            = 1;

  int cc; 
  for ( cc=0; cc < cmCTest::LAST_TEST; cc ++ )
    {
    m_Tests[cc] = 0;
    }
  m_ShortDateFormat        = true;

  m_TestingHandlers["build"]     = new cmCTestBuildHandler;
  m_TestingHandlers["buildtest"] = new cmCTestBuildAndTestHandler;
  m_TestingHandlers["coverage"]  = new cmCTestCoverageHandler;
  m_TestingHandlers["script"]    = new cmCTestScriptHandler;
  m_TestingHandlers["test"]      = new cmCTestTestHandler;
  m_TestingHandlers["update"]    = new cmCTestUpdateHandler;
  m_TestingHandlers["configure"] = new cmCTestConfigureHandler;
  m_TestingHandlers["memcheck"]  = new cmCTestMemCheckHandler;
  m_TestingHandlers["submit"]    = new cmCTestSubmitHandler;

  cmCTest::t_TestingHandlers::iterator it;
  for ( it = m_TestingHandlers.begin(); it != m_TestingHandlers.end(); ++ it )
    {
    it->second->SetCTestInstance(this);
    }
}

//----------------------------------------------------------------------
cmCTest::~cmCTest() 
{ 
  cmCTest::t_TestingHandlers::iterator it;
  for ( it = m_TestingHandlers.begin(); it != m_TestingHandlers.end(); ++ it )
    {
    delete it->second;
    it->second = 0;
    }
  this->SetOutputLogFileName(0);
}

//----------------------------------------------------------------------
int cmCTest::Initialize(const char* binary_dir, bool new_tag, bool verbose_tag)
{
  if(!m_InteractiveDebugMode)
    {
    this->BlockTestErrorDiagnostics();
    }
  
  m_BinaryDir = binary_dir;
  cmSystemTools::ConvertToUnixSlashes(m_BinaryDir);

  this->UpdateCTestConfiguration();

  if ( m_ProduceXML )
    {
    cmCTestLog(this, OUTPUT,
      "   Site: " << this->GetCTestConfiguration("Site") << std::endl
      << "   Build name: " << this->GetCTestConfiguration("BuildName") << std::endl
      );
    cmCTestLog(this, DEBUG, "Produce XML is on" << std::endl);
    if ( this->GetCTestConfiguration("NightlyStartTime").empty() )
      {
      cmCTestLog(this, DEBUG, "No nightly start time" << std::endl);
      return 0;
      }
    }

  if ( !this->ReadCustomConfigurationFileTree(m_BinaryDir.c_str()) )
    {
    cmCTestLog(this, DEBUG, "Cannot find custom configuration file tree" << std::endl);
    return 0;
    }

  if ( m_ProduceXML )
    {
    std::string testingDir = m_BinaryDir + "/Testing";
    if ( cmSystemTools::FileExists(testingDir.c_str()) )
      {
      if ( !cmSystemTools::FileIsDirectory(testingDir.c_str()) )
        {
        cmCTestLog(this, ERROR_MESSAGE, "File " << testingDir << " is in the place of the testing directory"
                  << std::endl);
        return 0;
        }
      }
    else
      {
      if ( !cmSystemTools::MakeDirectory(testingDir.c_str()) )
        {
        cmCTestLog(this, ERROR_MESSAGE, "Cannot create directory " << testingDir
                  << std::endl);
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
    if ( tag.size() == 0 || new_tag || m_Tests[cmCTest::START_TEST] || m_Tests[ALL_TEST])
      {
      cmCTestLog(this, DEBUG, "TestModel: " << this->GetTestModelString() << std::endl);
      cmCTestLog(this, DEBUG, "TestModel: " << m_TestModel << std::endl);
      if ( m_TestModel == cmCTest::NIGHTLY )
        {
        lctime = this->GetNightlyTime(m_CTestConfiguration["NightlyStartTime"], m_TomorrowTag);
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
      if ( verbose_tag )
        {
        cmCTestLog(this, OUTPUT, "Create new tag: " << tag << " - " 
          << this->GetTestModelString() << std::endl);
        }
      }
    m_CurrentTag = tag;
    }
  return 1;
}

//----------------------------------------------------------------------
bool cmCTest::InitializeFromCommand(cmCTestCommand* command, bool first)
{
  if ( !first && !m_CurrentTag.empty() )
    {
    return true;
    }

  const char* src_dir = this->GetCTestConfiguration("SourceDirectory").c_str();
  const char* bld_dir = this->GetCTestConfiguration("BuildDirectory").c_str();
  m_DartVersion = 1;
  m_SubmitFiles.clear();

  cmMakefile* mf = command->GetMakefile();
  std::string fname = src_dir;
  fname += "/CTestConfig.cmake";
  cmSystemTools::ConvertToUnixSlashes(fname);
  if ( cmSystemTools::FileExists(fname.c_str()) )
    {
    cmCTestLog(this, OUTPUT, "   Reading ctest configuration file: " << fname.c_str() << std::endl);
    bool readit = mf->ReadListFile(mf->GetCurrentListFile(), 
      fname.c_str() );
    if(!readit)
      {
      std::string m = "Could not find include file: ";
      m += fname;
      command->SetError(m.c_str());
      return false;
      }
    }
  else if ( !first )
    {
    cmCTestLog(this, WARNING, "Cannot locate CTest configuration: " << fname.c_str() << std::endl);
    }
  else
    {
    cmCTestLog(this, HANDLER_OUTPUT, "   Cannot locate CTest configuration: " << fname.c_str() << std::endl
      << "   Delay the initialization of CTest" << std::endl);
    }

  this->SetCTestConfigurationFromCMakeVariable(mf, "NightlyStartTime", "CTEST_NIGHTLY_START_TIME");
  this->SetCTestConfigurationFromCMakeVariable(mf, "Site", "CTEST_SITE");
  this->SetCTestConfigurationFromCMakeVariable(mf, "BuildName", "CTEST_BUILD_NAME");
  const char* dartVersion = mf->GetDefinition("CTEST_DART_SERVER_VERSION");
  if ( dartVersion )
    {
    m_DartVersion = atoi(dartVersion);
    if ( m_DartVersion < 0 )
      {
      cmCTestLog(this, ERROR_MESSAGE, "Invalid Dart server version: " << dartVersion << ". Please specify the version number." << std::endl);
      return false;
      }
    }

  if ( !this->Initialize(bld_dir, true, false) )
    {
    if ( this->GetCTestConfiguration("NightlyStartTime").empty() && first)
      {
      return true;
      }
    return false;
    }
  cmCTestLog(this, OUTPUT, "   Use " << this->GetTestModelString() << " tag: " << this->GetCurrentTag() << std::endl);
  return true;
}


//----------------------------------------------------------------------
bool cmCTest::UpdateCTestConfiguration()
{
  if ( m_SuppressUpdatingCTestConfiguration )
    {
    return true;
    }
  std::string fileName = m_CTestConfigFile;
  if ( fileName.empty() )
    {
    fileName = m_BinaryDir + "/DartConfiguration.tcl";
    if ( !cmSystemTools::FileExists(fileName.c_str()) )
      {
      fileName = m_BinaryDir + "/CTestConfiguration.ini";
      }
    }
  if ( !cmSystemTools::FileExists(fileName.c_str()) )
    {
    // No need to exit if we are not producing XML
    if ( m_ProduceXML )
      {
      cmCTestLog(this, ERROR_MESSAGE, "Cannot find file: " << fileName.c_str() << std::endl);
      return false;
      }
    }
  else
    {
    // parse the dart test file
    std::ifstream fin(fileName.c_str());
    if(!fin)
      {
      return false;
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
      m_CTestConfiguration[key] = value;
      }
    fin.close();
    }
  if ( m_ProduceXML )
    {
    m_TimeOut = atoi(m_CTestConfiguration["TimeOut"].c_str());
    m_CompressXMLFiles = cmSystemTools::IsOn(m_CTestConfiguration["CompressSubmission"].c_str());
    }
  return true;
}

//----------------------------------------------------------------------
void cmCTest::BlockTestErrorDiagnostics()
{
  cmSystemTools::PutEnv("DART_TEST_FROM_DART=1");
  cmSystemTools::PutEnv("DASHBOARD_TEST_FROM_CTEST=1");
#if defined(_WIN32)
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX); 
#endif
}

//----------------------------------------------------------------------
void cmCTest::SetTestModel(int mode)
{
  m_InteractiveDebugMode = false;
  m_TestModel = mode;
}

//----------------------------------------------------------------------
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
      cmCTestLog(this, ERROR_MESSAGE, "Don't know about test \"" << ttype << "\" yet..." << std::endl);
      }
    return false;
    }
  return true;
}

//----------------------------------------------------------------------
void cmCTest::Finalize()
{
}

//----------------------------------------------------------------------
bool cmCTest::OpenOutputFile(const std::string& path, 
                     const std::string& name, cmGeneratedFileStream& stream,
                     bool compress)
{
  std::string testingDir = m_BinaryDir + "/Testing";
  if ( path.size() > 0 )
    {
    testingDir += "/" + path;
    }
  if ( cmSystemTools::FileExists(testingDir.c_str()) )
    {
    if ( !cmSystemTools::FileIsDirectory(testingDir.c_str()) )
      {
      cmCTestLog(this, ERROR_MESSAGE, "File " << testingDir 
                << " is in the place of the testing directory"
                << std::endl);
      return false;
      }
    }
  else
    {
    if ( !cmSystemTools::MakeDirectory(testingDir.c_str()) )
      {
      cmCTestLog(this, ERROR_MESSAGE, "Cannot create directory " << testingDir
                << std::endl);
      return false;
      }
    }
  std::string filename = testingDir + "/" + name;
  stream.Open(filename.c_str());
  if( !stream )
    {
    cmCTestLog(this, ERROR_MESSAGE, "Problem opening file: " << filename << std::endl);
    return false;
    }
  if ( compress )
    {
    if ( m_CompressXMLFiles )
      {
      stream.SetCompression(true);
      }
    }
  return true;
}

//----------------------------------------------------------------------
bool cmCTest::AddIfExists(tm_SetOfStrings& files, const char* file)
{
  if ( this->CTestFileExists(file) )
    {
    files.insert(file);
    }
  else
    {
    std::string name = file;
    name += ".gz";
    if ( this->CTestFileExists(name.c_str()) )
      {
      files.insert(name.c_str());
      }
    else
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------
bool cmCTest::CTestFileExists(const std::string& filename)
{
  std::string testingDir = m_BinaryDir + "/Testing/" + m_CurrentTag + "/" +
    filename;
  return cmSystemTools::FileExists(testingDir.c_str());
}

//----------------------------------------------------------------------
cmCTestGenericHandler* cmCTest::GetInitializedHandler(const char* handler)
{
  cmCTest::t_TestingHandlers::iterator it = m_TestingHandlers.find(handler);
  if ( it == m_TestingHandlers.end() )
    {
    return 0;
    }
  it->second->Initialize();
  return it->second;
}

//----------------------------------------------------------------------
cmCTestGenericHandler* cmCTest::GetHandler(const char* handler)
{
  cmCTest::t_TestingHandlers::iterator it = m_TestingHandlers.find(handler);
  if ( it == m_TestingHandlers.end() )
    {
    return 0;
    }
  return it->second;
}

//----------------------------------------------------------------------
int cmCTest::ExecuteHandler(const char* shandler)
{
  cmCTestGenericHandler* handler = this->GetHandler(shandler);
  if ( !handler )
    {
    return -1;
    }
  handler->Initialize();
  return handler->ProcessHandler(); 
}

//----------------------------------------------------------------------
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
    cmCTestGenericHandler* uphandler = this->GetHandler("update");
    uphandler->SetOption("SourceDirectory", this->GetCTestConfiguration("SourceDirectory").c_str());
    update_count = uphandler->ProcessHandler(); 
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
    if (this->GetHandler("configure")->ProcessHandler() < 0)
      {
      res |= cmCTest::CONFIGURE_ERRORS;
      }
    }
  if ( m_Tests[BUILD_TEST] || m_Tests[ALL_TEST] )
    {
    this->UpdateCTestConfiguration();
    if (this->GetHandler("build")->ProcessHandler() < 0)
      {
      res |= cmCTest::BUILD_ERRORS;
      }
    }
  if ( m_Tests[TEST_TEST] || m_Tests[ALL_TEST] || notest )
    {
    this->UpdateCTestConfiguration();
    if (this->GetHandler("test")->ProcessHandler() < 0)
      {
      res |= cmCTest::TEST_ERRORS;
      }
    }
  if ( m_Tests[COVERAGE_TEST] || m_Tests[ALL_TEST] )
    {
    this->UpdateCTestConfiguration();
    if (this->GetHandler("coverage")->ProcessHandler() < 0)
      {
      res |= cmCTest::COVERAGE_ERRORS;
      }
    }
  if ( m_Tests[MEMCHECK_TEST] || m_Tests[ALL_TEST] )
    {
    this->UpdateCTestConfiguration();
    if (this->GetHandler("memcheck")->ProcessHandler() < 0)
      {
      res |= cmCTest::MEMORY_ERRORS;
      }
    }
  if ( !notest )
    {
    std::string notes_dir = m_BinaryDir + "/Testing/Notes";
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
    if (this->GetHandler("submit")->ProcessHandler() < 0)
      {
      res |= cmCTest::SUBMIT_ERRORS;
      }
    }
  return res;
}

//----------------------------------------------------------------------
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

//----------------------------------------------------------------------
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

//######################################################################
//######################################################################
//######################################################################
//######################################################################

//----------------------------------------------------------------------
int cmCTest::RunMakeCommand(const char* command, std::string* output,
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

  if ( output )
    {
    *output = "";
    }

  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, "Run command:");
  std::vector<const char*>::iterator ait;
  for ( ait = argv.begin(); ait != argv.end() && *ait; ++ ait )
    {
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, " \"" << *ait << "\"");
    }
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, std::endl);
  
  // Now create process object
  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*argv.begin());
  cmsysProcess_SetWorkingDirectory(cp, dir);
  cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
  cmsysProcess_SetTimeout(cp, timeout);
  cmsysProcess_Execute(cp);

  // Initialize tick's
  std::string::size_type tick = 0;
  std::string::size_type tick_len = 1024;
  std::string::size_type tick_line_len = 50;

  char* data;
  int length;
  cmCTestLog(this, HANDLER_OUTPUT,
    "   Each . represents " << tick_len << " bytes of output" << std::endl
    << "    " << std::flush);
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
      while ( output->size() > (tick * tick_len) )
        {
        tick ++;
        cmCTestLog(this, HANDLER_OUTPUT, "." << std::flush);
        if ( tick % tick_line_len == 0 && tick > 0 )
          {
          cmCTestLog(this, HANDLER_OUTPUT, "  Size: "
            << int((output->size() / 1024.0) + 1) << "K" << std::endl
            << "    " << std::flush);
          }
        }
      }
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, cmCTestLogWrite(data, length));
    if ( ofs )
      {
      ofs << cmCTestLogWrite(data, length);
      }
    }
  cmCTestLog(this, OUTPUT, " Size of output: "
    << int(output->size() / 1024.0) << "K" << std::endl);

  cmsysProcess_WaitForExit(cp, 0);

  int result = cmsysProcess_GetState(cp);

  if(result == cmsysProcess_State_Exited)
    {
    *retVal = cmsysProcess_GetExitValue(cp);
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, "Command exited with the value: " << *retVal << std::endl);
    }
  else if(result == cmsysProcess_State_Exception)
    {
    *retVal = cmsysProcess_GetExitException(cp);
    cmCTestLog(this, WARNING, "There was an exception: " << *retVal << std::endl);
    }
  else if(result == cmsysProcess_State_Expired)
    {
    cmCTestLog(this, WARNING, "There was a timeout" << std::endl);
    } 
  else if(result == cmsysProcess_State_Error)
    {
    *output += "\n*** ERROR executing: ";
    *output += cmsysProcess_GetErrorString(cp);
    cmCTestLog(this, ERROR_MESSAGE, "There was an error: " << cmsysProcess_GetErrorString(cp) << std::endl);
    }

  cmsysProcess_Delete(cp);

  return result;
}

//######################################################################
//######################################################################
//######################################################################
//######################################################################

//----------------------------------------------------------------------
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
    
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, "Internal cmCTest object used to run test." << std::endl
      <<  *output << std::endl);
    return cmsysProcess_State_Exited;
    }
  std::vector<char> tempOutput;
  if ( output )
    {
    *output = "";
    }

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*argv.begin());
  cmCTestLog(this, DEBUG, "Command is: " << argv[0] << std::endl);
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
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, cmCTestLogWrite(data, length));
    if ( log )
      {
      log->write(data, length);
      }
    }

  cmsysProcess_WaitForExit(cp, 0);
  if(output)
    {
    output->append(&*tempOutput.begin(), tempOutput.size());
    }
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, "-- Process completed" << std::endl);

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
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, outerr.c_str() << std::endl << std::flush);
    }
  else if(result == cmsysProcess_State_Error)
    {
    std::string outerr = "\n*** ERROR executing: ";
    outerr += cmsysProcess_GetErrorString(cp);
    *output += outerr;
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, outerr.c_str() << std::endl << std::flush);
    }
  cmsysProcess_Delete(cp);

  return result;
}

//----------------------------------------------------------------------
void cmCTest::StartXML(std::ostream& ostr)
{
  ostr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    << "<Site BuildName=\"" << m_CTestConfiguration["BuildName"]
    << "\" BuildStamp=\"" << m_CurrentTag << "-"
    << this->GetTestModelString() << "\" Name=\""
    << m_CTestConfiguration["Site"] << "\" Generator=\"ctest"
    << cmVersion::GetCMakeVersion()
    << "\">" << std::endl;
}

//----------------------------------------------------------------------
void cmCTest::EndXML(std::ostream& ostr)
{
  ostr << "</Site>" << std::endl;
}

//----------------------------------------------------------------------
int cmCTest::GenerateCTestNotesOutput(std::ostream& os, const cmCTest::tm_VectorOfStrings& files)
{
  cmCTest::tm_VectorOfStrings::const_iterator it;
  os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    << "<?xml-stylesheet type=\"text/xsl\" href=\"Dart/Source/Server/XSL/Build.xsl <file:///Dart/Source/Server/XSL/Build.xsl> \"?>\n"
    << "<Site BuildName=\"" << m_CTestConfiguration["BuildName"] << "\" BuildStamp=\"" 
    << m_CurrentTag << "-" << this->GetTestModelString() << "\" Name=\"" 
    << m_CTestConfiguration["Site"] << "\" Generator=\"ctest"
    << cmVersion::GetCMakeVersion()
    << "\">\n"
    << "<Notes>" << std::endl;

  for ( it = files.begin(); it != files.end(); it ++ )
    {
    cmCTestLog(this, OUTPUT, "\tAdd file: " << it->c_str() << std::endl);
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
      cmCTestLog(this, ERROR_MESSAGE, "Problem reading file: " << it->c_str() << " while creating notes" << std::endl);
      }
    os << "</Text>\n"
      << "</Note>" << std::endl;
    }
  os << "</Notes>\n"
    << "</Site>" << std::endl;
  return 1;
}

//----------------------------------------------------------------------
int cmCTest::GenerateNotesFile(const std::vector<cmStdString> &files)
{
  cmGeneratedFileStream ofs;
  if ( !this->OpenOutputFile(m_CurrentTag, "Notes.xml", ofs) )
    {
    cmCTestLog(this, ERROR_MESSAGE, "Cannot open notes file" << std::endl);
    return 1;
    }

  this->GenerateCTestNotesOutput(ofs, files);
  return 0;
}

//----------------------------------------------------------------------
int cmCTest::GenerateNotesFile(const char* cfiles)
{
  if ( !cfiles )
    {
    return 1;
    }

  std::vector<cmStdString> files;

  cmCTestLog(this, OUTPUT, "Create notes file" << std::endl);

  files = cmSystemTools::SplitString(cfiles, ';');
  if ( files.size() == 0 )
    {
    return 1;
    }

  return this->GenerateNotesFile(files);
}

//----------------------------------------------------------------------
int cmCTest::Run(std::vector<std::string>const& args, std::string* output)
{
  this->FindRunningCMake(args[0].c_str());
  const char* ctestExec = "ctest";
  bool cmakeAndTest = false;
  bool performSomeTest = true;
  for(size_t i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("--ctest-config",0) == 0 && i < args.size() - 1)
      {
      i++;
      this->m_CTestConfigFile= args[i];
      }

    if(arg.find("-C",0) == 0 && i < args.size() - 1)
      {
      i++;
      this->m_ConfigType = args[i];
      cmSystemTools::ReplaceString(this->m_ConfigType, ".\\", "");
      }

    if( arg.find("--debug",0) == 0 )
      {
      this->m_Debug = true;
      }
    if( arg.find("--show-line-numbers",0) == 0 )
      {
      this->m_ShowLineNumbers = true;
      }
    if( arg.find("-Q",0) == 0 || arg.find("--quiet",0) == 0 )
      {
      this->m_Quiet = true;
      }
    if( arg.find("-V",0) == 0 || arg.find("--verbose",0) == 0 )
      {
      this->m_Verbose = true;
      }
    if( arg.find("-VV",0) == 0 || arg.find("--extra-verbose",0) == 0 )
      {
      this->m_ExtraVerbose = true;
      this->m_Verbose = true;
      }

    if( arg.find("-N",0) == 0 || arg.find("--show-only",0) == 0 )
      {
      this->m_ShowOnly = true;
      }

    if( arg.find("-S",0) == 0 && i < args.size() - 1 )
      {
      this->m_RunConfigurationScript = true;
      i++;
      cmCTestScriptHandler* ch = static_cast<cmCTestScriptHandler*>(this->GetHandler("script"));
      ch->AddConfigurationScript(args[i].c_str());
      }

    if( ( arg.find("-O",0) == 0 || arg.find("--output-log") == 0 ) && i < args.size() - 1 )
      {
      i++;
      this->SetOutputLogFileName(args[i].c_str());
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
      this->m_ProduceXML = true;
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
        cmCTestLog(this, ERROR_MESSAGE, "CTest -D called with incorrect option: " << targ << std::endl);
        cmCTestLog(this, ERROR_MESSAGE, "Available options are:" << std::endl
          << "  " << ctestExec << " -D Continuous" << std::endl
          << "  " << ctestExec << " -D Continuous(Start|Update|Configure|Build)" << std::endl
          << "  " << ctestExec << " -D Continuous(Test|Coverage|MemCheck|Submit)" << std::endl
          << "  " << ctestExec << " -D Experimental" << std::endl
          << "  " << ctestExec << " -D Experimental(Start|Update|Configure|Build)" << std::endl
          << "  " << ctestExec << " -D Experimental(Test|Coverage|MemCheck|Submit)" << std::endl
          << "  " << ctestExec << " -D Nightly" << std::endl
          << "  " << ctestExec << " -D Nightly(Start|Update|Configure|Build)" << std::endl
          << "  " << ctestExec << " -D Nightly(Test|Coverage|MemCheck|Submit)" << std::endl
          << "  " << ctestExec << " -D NightlyMemoryCheck" << std::endl);
        }
      }

    if( ( arg.find("-T",0) == 0 ) && 
      (i < args.size() -1) )
      {
      this->m_ProduceXML = true;
      i++;
      if ( !this->SetTest(args[i].c_str(), false) )
        {
        performSomeTest = false;
        cmCTestLog(this, ERROR_MESSAGE, "CTest -T called with incorrect option: " << args[i].c_str() << std::endl);
        cmCTestLog(this, ERROR_MESSAGE, "Available options are:" << std::endl
          << "  " << ctestExec << " -T all" << std::endl
          << "  " << ctestExec << " -T start" << std::endl
          << "  " << ctestExec << " -T update" << std::endl
          << "  " << ctestExec << " -T configure" << std::endl
          << "  " << ctestExec << " -T build" << std::endl
          << "  " << ctestExec << " -T test" << std::endl
          << "  " << ctestExec << " -T coverage" << std::endl
          << "  " << ctestExec << " -T memcheck" << std::endl
          << "  " << ctestExec << " -T notes" << std::endl
          << "  " << ctestExec << " -T submit" << std::endl);
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
        cmCTestLog(this, ERROR_MESSAGE, "CTest -M called with incorrect option: " << str.c_str() << std::endl);
        cmCTestLog(this, ERROR_MESSAGE, "Available options are:" << std::endl
          << "  " << ctestExec << " -M Continuous" << std::endl
          << "  " << ctestExec << " -M Experimental" << std::endl
          << "  " << ctestExec << " -M Nightly" << std::endl);
        }
      }

    if(arg.find("-I",0) == 0 && i < args.size() - 1)
      {
      i++;
      this->GetHandler("test")->SetOption("TestsToRunInformation", args[i].c_str());
      }                                                       
    if(arg.find("-U",0) == 0)                                 
      {                                                       
      this->GetHandler("test")->SetOption("UseUnion", "true");
      }                                                       
    if(arg.find("-R",0) == 0 && i < args.size() - 1)          
      {                                                       
      i++;                                                    
      this->GetHandler("test")->SetOption("IncludeRegularExpression", args[i].c_str());
      }                                                       

    if(arg.find("-E",0) == 0 && i < args.size() - 1)          
      {                                                       
      i++;
      this->GetHandler("test")->SetOption("ExcludeRegularExpression", args[i].c_str());
      }

    if(arg.find("-A",0) == 0 && i < args.size() - 1)
      {
      this->m_ProduceXML = true;
      this->SetTest("Notes");
      i++;
      this->SetNotesFiles(args[i].c_str());
      }
    // --build-and-test options
    if(arg.find("--build-and-test",0) == 0 && i < args.size() - 1)
      {
      cmakeAndTest = true;
      }
    cmCTest::t_TestingHandlers::iterator it;
    for ( it = m_TestingHandlers.begin(); it != m_TestingHandlers.end(); ++ it )
      {
      if ( !it->second->ProcessCommandLineArguments(arg, i, args) )
        {
        cmCTestLog(this, ERROR_MESSAGE, "Problem parsing command line arguments within a handler");
        return 0;
        }
      }
    }

  // default to the build type of ctest itself
  if(m_ConfigType.size() == 0)
    {
#ifdef  CMAKE_INTDIR
    m_ConfigType = CMAKE_INTDIR;
#endif
    }


  if(cmakeAndTest)
    {
    m_Verbose = true;
    cmCTestBuildAndTestHandler* handler = 
      static_cast<cmCTestBuildAndTestHandler*>(this->GetHandler("buildtest"));
    int retv = handler->ProcessHandler();
    /*
    cmSystemTools::ResetErrorOccuredFlag();
    cmListFileCache::ClearCache();
    int retv = this->RunCMakeAndTest(output);
    cmSystemTools::ResetErrorOccuredFlag();
    cmListFileCache::ClearCache();
    */
    *output = handler->GetOutput();
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
      cmCTest::t_TestingHandlers::iterator it;
      for ( it = m_TestingHandlers.begin(); it != m_TestingHandlers.end(); ++ it )
        {
        it->second->SetVerbose(this->m_ExtraVerbose);
        }
      this->GetHandler("script")->SetVerbose(m_Verbose);
      res = this->GetHandler("script")->ProcessHandler();
      }
    else
      {
      m_ExtraVerbose = m_Verbose;
      m_Verbose = true;
      cmCTest::t_TestingHandlers::iterator it;
      for ( it = m_TestingHandlers.begin(); it != m_TestingHandlers.end(); ++ it )
        {
        it->second->SetVerbose(this->m_Verbose);
        }
      if ( !this->Initialize(cmSystemTools::GetCurrentWorkingDirectory().c_str()) )
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

//----------------------------------------------------------------------
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
    msg << "CTEST can not find the command line program ctest.\n";
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
                                     dir, file, true))
    {
    m_CMakeSelf = dir += "/cmake";
    m_CMakeSelf += cmSystemTools::GetExecutableExtension();
    if(cmSystemTools::FileExists(m_CMakeSelf.c_str()))
      {
      return;
      }
    }
  failures.push_back(m_CMakeSelf);
#ifdef CMAKE_BUILD_DIR
  std::string intdir = ".";
#ifdef  CMAKE_INTDIR
  intdir = CMAKE_INTDIR;
#endif
  m_CMakeSelf = CMAKE_BUILD_DIR;
  m_CMakeSelf += "/bin/";
  m_CMakeSelf += intdir;
  m_CMakeSelf += "/cmake";
  m_CMakeSelf += cmSystemTools::GetExecutableExtension();
#endif
  if(!cmSystemTools::FileExists(m_CMakeSelf.c_str()))
    {
    failures.push_back(m_CMakeSelf);
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
}

//----------------------------------------------------------------------
void cmCTest::SetNotesFiles(const char* notes)
{
  if ( !notes )
    {
    return;
    }
  m_NotesFiles = notes;
}

//----------------------------------------------------------------------
int cmCTest::ReadCustomConfigurationFileTree(const char* dir)
{
  tm_VectorOfStrings dirs;
  tm_VectorOfStrings ndirs;
  cmake cm;
  cmGlobalGenerator gg;
  gg.SetCMakeInstance(&cm);
  std::auto_ptr<cmLocalGenerator> lg(gg.CreateLocalGenerator());
  lg->SetGlobalGenerator(&gg);
  cmMakefile *mf = lg->GetMakefile();

  std::string rexpr = dir;
  rexpr += "/*/CTestCustom.ctest";
  cmGlob gl;
  gl.RecurseOn();
  gl.FindFiles(rexpr);
  std::vector<std::string>& files = gl.GetFiles();
  std::vector<std::string>::iterator fileIt;
  for ( fileIt = files.begin(); fileIt != files.end();
    ++ fileIt )
    {
    cmCTestLog(this, DEBUG, "* Read custom CTest configuration file: " << fileIt->c_str() << std::endl);
    if ( !lg->GetMakefile()->ReadListFile(0, fileIt->c_str()) ||
      cmSystemTools::GetErrorOccuredFlag() )
      {
      cmCTestLog(this, ERROR_MESSAGE, "Problem reading custom configuration: " << fileIt->c_str() << std::endl);
      }
    }

  cmCTest::t_TestingHandlers::iterator it;
  for ( it = m_TestingHandlers.begin(); it != m_TestingHandlers.end(); ++ it )
    {
    it->second->PopulateCustomVectors(mf);
    }
  
  return 1;
}

//----------------------------------------------------------------------
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

//----------------------------------------------------------------------
void cmCTest::PopulateCustomInteger(cmMakefile* mf, const char* def, int& val)
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
  val = atoi(dval);
}

//----------------------------------------------------------------------
std::string cmCTest::GetShortPathToFile(const char* cfname)
{
  const std::string& sourceDir = this->GetCTestConfiguration("SourceDirectory");
  const std::string& buildDir = this->GetCTestConfiguration("BuildDirectory");
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

//----------------------------------------------------------------------
std::string cmCTest::GetCTestConfiguration(const char *name)
{
  return m_CTestConfiguration[name];
}

//----------------------------------------------------------------------
void cmCTest::EmptyCTestConfiguration()
{
  m_CTestConfiguration.clear();
}

//----------------------------------------------------------------------
void cmCTest::SetCTestConfiguration(const char *name, const char* value)
{
  if ( !name )
    {
    return;
    }
  if ( !value )
    {
    m_CTestConfiguration.erase(name);
    return;
    }
  m_CTestConfiguration[name] = value;
}

  
//----------------------------------------------------------------------
std::string cmCTest::GetCurrentTag()
{
  return m_CurrentTag;
}

//----------------------------------------------------------------------
std::string cmCTest::GetBinaryDir()
{
  return m_BinaryDir;
}

//----------------------------------------------------------------------
std::string cmCTest::GetConfigType()
{
  return m_ConfigType;
}

//----------------------------------------------------------------------
bool cmCTest::GetShowOnly()
{
  return m_ShowOnly;
}

//----------------------------------------------------------------------
void cmCTest::SetProduceXML(bool v)
{
  m_ProduceXML = v;
}

//----------------------------------------------------------------------
bool cmCTest::GetProduceXML()
{
  return m_ProduceXML;
}

//----------------------------------------------------------------------
void cmCTest::AddSubmitFile(const char* name)
{
  m_SubmitFiles.insert(name);
}

//----------------------------------------------------------------------
bool cmCTest::SetCTestConfigurationFromCMakeVariable(cmMakefile* mf, const char* dconfig, const char* cmake_var)
{
  const char* ctvar;
  ctvar = mf->GetDefinition(cmake_var);
  if ( !ctvar )
    {
    return false;
    }
  this->SetCTestConfiguration(dconfig, ctvar);
  return true;
}

bool cmCTest::RunCommand(
  const char* command, 
  std::string* stdOut,
  std::string* stdErr,
  int *retVal, 
  const char* dir,
  double timeout)
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

  *stdOut = "";
  *stdErr = "";

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*argv.begin());
  cmsysProcess_SetWorkingDirectory(cp, dir);
  if(cmSystemTools::GetRunCommandHideConsole())
    {
    cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
    }
  cmsysProcess_SetTimeout(cp, timeout);
  cmsysProcess_Execute(cp);
  
  std::vector<char> tempOutput;
  std::vector<char> tempError;
  char* data;
  int length;
  int res;
  bool done = false;
  while(!done)
    {
    res = cmsysProcess_WaitForData(cp, &data, &length, 0);
    switch ( res )
      {
    case cmsysProcess_Pipe_STDOUT:
      tempOutput.insert(tempOutput.end(), data, data+length);
      break;
    case cmsysProcess_Pipe_STDERR:
      tempError.insert(tempError.end(), data, data+length);
      break;
    default:
      done = true;
      }
    if ( (res == cmsysProcess_Pipe_STDOUT || res == cmsysProcess_Pipe_STDERR) && m_ExtraVerbose )
      {
      cmSystemTools::Stdout(data, length);
      }
    }
  
  cmsysProcess_WaitForExit(cp, 0);
  stdOut->append(&*tempOutput.begin(), tempOutput.size());
  stdErr->append(&*tempError.begin(), tempError.size());
  
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
  else if(cmsysProcess_GetState(cp) == cmsysProcess_State_Exception)
    {
    const char* exception_str = cmsysProcess_GetExceptionString(cp);
    cmCTestLog(this, ERROR_MESSAGE, exception_str << std::endl);
    stdErr->append(exception_str, strlen(exception_str));
    result = false;
    }
  else if(cmsysProcess_GetState(cp) == cmsysProcess_State_Error)
    {
    const char* error_str = cmsysProcess_GetErrorString(cp);
    cmCTestLog(this, ERROR_MESSAGE, error_str << std::endl);
    stdErr->append(error_str, strlen(error_str));
    result = false;
    }
  else if(cmsysProcess_GetState(cp) == cmsysProcess_State_Expired)
    {
    const char* error_str = "Process terminated due to timeout\n";
    cmCTestLog(this, ERROR_MESSAGE, error_str << std::endl);
    stdErr->append(error_str, strlen(error_str));
    result = false;
    }
  
  cmsysProcess_Delete(cp);
  return result;
}

//----------------------------------------------------------------------
void cmCTest::SetOutputLogFileName(const char* name)
{
  if ( m_OutputLogFile)
    {
    delete m_OutputLogFile;
    m_OutputLogFile= 0;
    }
  if ( name )
    {
    m_OutputLogFile = new cmGeneratedFileStream(name);
    }
}

//----------------------------------------------------------------------
static const char* cmCTestStringLogType[] = 
{
  "DEBUG",
  "OUTPUT",
  "HANDLER_OUTPUT",
  "HANDLER_VERBOSE_OUTPUT",
  "WARNING",
  "ERROR_MESSAGE",
  0
};

//----------------------------------------------------------------------
#ifdef cerr
#  undef cerr
#endif
#ifdef cout
#  undef cout
#endif

#define cmCTestLogOutputFileLine(stream) \
  if ( m_ShowLineNumbers ) \
    { \
    (stream) << std::endl << file << ":" << line << " "; \
    }

void cmCTest::Log(int logType, const char* file, int line, const char* msg)
{
  if ( !msg || !*msg )
    {
    return;
    }
  if ( m_OutputLogFile )
    {
    bool display = true;
    if ( logType == cmCTest::DEBUG && !m_Debug ) { display = false; }
    if ( logType == cmCTest::HANDLER_VERBOSE_OUTPUT && !m_Debug && !m_ExtraVerbose ) { display = false; }
    if ( display )
      {
      cmCTestLogOutputFileLine(*m_OutputLogFile);
      if ( logType != m_OutputLogFileLastTag )
        {
        *m_OutputLogFile << "[";
        if ( logType >= OTHER || logType < 0 )
          {
          *m_OutputLogFile << "OTHER";
          }
        else
          {
          *m_OutputLogFile << cmCTestStringLogType[logType];
          }
        *m_OutputLogFile << "] " << std::endl << std::flush;
        }
      *m_OutputLogFile << msg << std::flush;
      if ( logType != m_OutputLogFileLastTag )
        {
        *m_OutputLogFile << std::endl << std::flush;
        m_OutputLogFileLastTag = logType;
        }
      }
    }
  if ( !m_Quiet )
    {
    switch ( logType )
      {
    case DEBUG:
      if ( m_Debug )
        {
        cmCTestLogOutputFileLine(std::cout);
        std::cout << msg;
        std::cout.flush();
        }
      break;
    case OUTPUT: case HANDLER_OUTPUT:
      if ( m_Debug || m_Verbose )
        {
        cmCTestLogOutputFileLine(std::cout);
        std::cout << msg;
        std::cout.flush();
        }
      break;
    case HANDLER_VERBOSE_OUTPUT:
      if ( m_Debug || m_ExtraVerbose )
        {
        cmCTestLogOutputFileLine(std::cout);
        std::cout << msg;
        std::cout.flush();
        }
      break;
    case WARNING:
      cmCTestLogOutputFileLine(std::cerr);
      std::cerr << msg;
      std::cerr.flush();
      break;
    case ERROR_MESSAGE:
      cmCTestLogOutputFileLine(std::cerr);
      std::cerr << msg;
      std::cerr.flush();
      cmSystemTools::SetErrorOccured();
      break;
    default:
      cmCTestLogOutputFileLine(std::cout);
      std::cout << msg;
      std::cout.flush();
      }
    }
}

