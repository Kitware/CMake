/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "ctest.h"
#include "cmRegularExpression.h"
#include "cmSystemTools.h"

#include <stdio.h>
#include <time.h>

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
    epos ++;
    }
  return str.substr(spos, epos);
}

static std::string CurrentTime()
{
  time_t currenttime = time(0);
  return ::CleanString(ctime(&currenttime));
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

ctest::ctest() 
{ 
  m_UseIncludeRegExp      = false;
  m_UseExcludeRegExp      = false;
  m_UseExcludeRegExpFirst = false;
  m_Verbose               = false;
  m_DartMode              = false;
  int cc; 
  for ( cc=0; cc < ctest::LAST_TEST; cc ++ )
    {
    m_Tests[cc] = 0;
    }
}

void ctest::Initialize()
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
    if ( line.size() == 0 )
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
}

bool ctest::SetTest(const char* ttype)
{
  if ( cmSystemTools::LowerCase(ttype) == "all" )
    {
    m_Tests[ctest::ALL_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "update" )
    {
    m_Tests[ctest::UPDATE_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "configure" )
    {
    m_Tests[ctest::CONFIGURE_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "build" )
    {
    m_Tests[ctest::BUILD_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "test" )
    {
    m_Tests[ctest::TEST_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "coverage" )
    {
    m_Tests[ctest::COVERAGE_TEST] = 1;
    }
  else if ( cmSystemTools::LowerCase(ttype) == "purify" )
    {
    m_Tests[ctest::PURIFY_TEST] = 1;
    }
  else
    {
    std::cerr << "Don't know about test \"" << ttype << "\" yet..." << std::endl;
    return false;
    }
  return true;
}

void ctest::Finalize()
{
  if ( m_DartMode )
    {
    std::string testingDir = m_ToplevelPath + "/Testing/CDart";
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
    std::string testxml = testingDir + "/Test.xml";
    std::ofstream ofs(testxml.c_str());
    if( !ofs )
      {
      std::cerr << "Cannot create testing XML file" << std::endl;
      return;
      }
    this->GenerateDartOutput(ofs);
    }
}

std::string ctest::FindExecutable(const char *exe)
{
  std::string fullPath = "";
  std::string dir;
  std::string file;

  cmSystemTools::SplitProgramPath(exe, dir, file);
  if(m_ConfigType != "")
    {
    if(TryExecutable(dir.c_str(), file.c_str(), &fullPath, m_ConfigType.c_str()))
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

void ctest::UpdateDirectory()
{
  std::string cvsCommand = m_DartConfiguration["CVSCommand"];
  if ( cvsCommand.size() == 0 )
    {
    std::cerr << "Cannot find CVSCommand key in the DartConfiguration.tcl" << std::endl;
    return;
    }
  std::string cvsOptions = m_DartConfiguration["CVSUpdateOptions"];
  if ( cvsOptions.size() == 0 )
    {
    std::cerr << "Cannot find CVSUpdateOptions key in the DartConfiguration.tcl" << std::endl;
    return;
    }

  std::string sourceDirectory = m_DartConfiguration["SourceDirectory"];
  if ( sourceDirectory.size() == 0 )
    {
    std::cerr << "Cannot find SourceDirectory  key in the DartConfiguration.tcl" << std::endl;
    return;
    }

  std::string command = cvsCommand + " update " + cvsOptions;

  std::string output;
  int retVal;
  bool res = cmSystemTools::RunCommand(command.c_str(), output, 
                                       retVal, sourceDirectory.c_str(),
                                       m_Verbose);
  if (! res || retVal )
    {
    std::cerr << "Error(s) when updating the project" << std::endl;
    }
}

void ctest::ConfigureDirectory()
{
  std::string cCommand = m_DartConfiguration["ConfigureCommand"];
  if ( cCommand.size() == 0 )
    {
    std::cerr << "Cannot find ConfigureCommand key in the DartConfiguration.tcl" << std::endl;
    return;
    }

  std::string buildDirectory = m_DartConfiguration["BuildDirectory"];
  if ( buildDirectory.size() == 0 )
    {
    std::cerr << "Cannot find BuildDirectory  key in the DartConfiguration.tcl" << std::endl;
    return;
    }

  std::string output;
  int retVal;
  bool res = cmSystemTools::RunCommand(cCommand.c_str(), output, 
                                       retVal, buildDirectory.c_str(),
                                       m_Verbose);
  if (! res || retVal )
    {
    std::cerr << "Error(s) when updating the project" << std::endl;
    }
}

void ctest::BuildDirectory()
{
  std::string makeCommand = m_DartConfiguration["MakeCommand"];
  if ( makeCommand.size() == 0 )
    {
    std::cerr << "Cannot find MakeCommand key in the DartConfiguration.tcl" << std::endl;
    return;
    }
  std::string buildDirectory = m_DartConfiguration["BuildDirectory"];
  if ( buildDirectory.size() == 0 )
    {
    std::cerr << "Cannot find BuildDirectory  key in the DartConfiguration.tcl" << std::endl;
    return;
    }

  std::string output;
  int retVal;
  bool res = cmSystemTools::RunCommand(makeCommand.c_str(), output, 
                                       retVal, buildDirectory.c_str(), 
                                       m_Verbose);
  if (! res || retVal )
    {
    std::cerr << "Error(s) when building project" << std::endl;
    }
  // To do:
  // Add parsing of output for errors and warnings.
}

void ctest::ProcessDirectory(std::vector<std::string> &passed, 
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
  
  std::string name;
  std::vector<std::string> args;
  cmRegularExpression ireg(this->m_IncludeRegExp.c_str());
  cmRegularExpression ereg(this->m_ExcludeRegExp.c_str());
  cmRegularExpression dartStuff("([\t\n ]*<DartMeasurement.*/DartMeasurement[a-zA-Z]*>[\t ]*[\n]*)");

  bool parseError;
  while ( fin )
    {
    if(cmSystemTools::ParseFunction(fin, name, args, "DartTestfile.txt",
                                    parseError))
      {
      if (name == "SUBDIRS")
        {
        std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
        for(std::vector<std::string>::iterator j = args.begin();
            j != args.end(); ++j)
          {   
          std::string nwd = cwd + "/";
          nwd += *j;
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
            ereg.find(args[0].c_str()))
          {
          continue;
          }
        if (this->m_UseIncludeRegExp && !ireg.find(args[0].c_str()))
          {
          continue;
          }
        if (this->m_UseExcludeRegExp && 
            !this->m_UseExcludeRegExpFirst && 
            ereg.find(args[0].c_str()))
          {
          continue;
          }

        cmCTestTestResult cres;

        if (firstTest)
          {
          std::string nwd = cmSystemTools::GetCurrentWorkingDirectory();
          std::cerr << "Changing directory into " << nwd.c_str() << "\n";
          firstTest = 0;
          }
        cres.m_Name = args[0];
        fprintf(stderr,"Testing %-30s ",args[0].c_str());
        fflush(stderr);
        //std::cerr << "Testing " << args[0] << " ... ";
        // find the test executable
        std::string testCommand = 
          cmSystemTools::EscapeSpaces(this->FindExecutable(args[1].c_str()).c_str());
        // continue if we did not find the executable
        if (testCommand == "")
          {
          std::cerr << "Unable to find executable: " << 
            args[1].c_str() << "\n";
          continue;
          }
        
        testCommand = cmSystemTools::ConvertToOutputPath(testCommand.c_str());
        // add the arguments
        std::vector<std::string>::iterator j = args.begin();
        ++j;
        ++j;
        for(;j != args.end(); ++j)
          {   
          testCommand += " ";
          testCommand += cmSystemTools::EscapeSpaces(j->c_str());
          }
        /**
         * Run an executable command and put the stdout in output.
         */
        std::string output;
        int retVal;

        clock_t clock_start, clock_finish;
        double clocks_per_sec = (double)CLOCKS_PER_SEC;
        clock_start = clock();

        bool res = cmSystemTools::RunCommand(testCommand.c_str(), output, 
                                             retVal, 0, false);
        clock_finish = clock();

        cres.m_ExecutionTime = (double)(clock_finish - clock_start) / clocks_per_sec;
        cres.m_FullCommandLine = testCommand;

        if (!res || retVal != 0)
          {
          fprintf(stderr,"***Failed\n");
          if (output != "")
            {
            if (dartStuff.find(output.c_str()))
              {
              cmSystemTools::ReplaceString(output,
                                           dartStuff.match(1).c_str(),"");
              }
            if (output != "" && m_Verbose)
              {
              std::cerr << output.c_str() << "\n";
              }
            }
          failed.push_back(args[0]); 
          }
        else
          {
          fprintf(stderr,"   Passed\n");
          if (output != "")
            {
            if (dartStuff.find(output.c_str()))
              {
              cmSystemTools::ReplaceString(output,
                                           dartStuff.match(1).c_str(),"");
              }
            if (output != "" && m_Verbose)
              {
              std::cerr << output.c_str() << "\n";
              }
            }
          passed.push_back(args[0]); 
          }
        cres.m_Output = output;
        cres.m_ReturnValue = retVal;
        std::string nwd = cmSystemTools::GetCurrentWorkingDirectory();
        if ( nwd.size() > m_ToplevelPath.size() )
          {
          nwd = "." + nwd.substr(m_ToplevelPath.size(), nwd.npos);
          }
        cres.m_Path = nwd;
        cres.m_CompletionStatus = "Completed";
        m_TestResults.push_back( cres );
        }
      }
    }
}

void ctest::GenerateDartOutput(std::ostream& os)
{
  if ( !m_DartMode )
    {
    return;
    }

  if ( m_TestResults.size() == 0 )
    {
    return;
    }

  time_t tctime = time(0);
  struct tm *lctime = gmtime(&tctime);
  char datestring[100];
  sprintf(datestring, "%4d%02d%02d-%d%d",
          lctime->tm_year + 1900,
          lctime->tm_mon,
          lctime->tm_mday,
          lctime->tm_hour,
          lctime->tm_min);

  os << "Try to create dart output file" << std::endl;
  os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
     << "<Site BuildName=\"" << m_DartConfiguration["BuildName"]
     << "\" BuildStamp=\"" << datestring << "-Experimental\" Name=\""
     << m_DartConfiguration["Site"] << "\">\n"
     << "<Testing>\n"
     << "  <StartDateTime>" << ::CurrentTime() << " </StartDateTime>\n"
     << "  <TestList>\n";
  tm_TestResultsVector::size_type cc;
  for ( cc = 0; cc < m_TestResults.size(); cc ++ )
    {
    cmCTestTestResult *result = &m_TestResults[cc];
    os << "    <Test>" << result->m_Path << "/" << result->m_Name 
       << "</Test>" << std::endl;
    }
  os << "  </TestList>\n";
  for ( cc = 0; cc < m_TestResults.size(); cc ++ )
    {
    cmCTestTestResult *result = &m_TestResults[cc];
    os << "  <Test Status=\"" << (result->m_ReturnValue?"failed":"passed") 
       << "\">\n"
       << "    <Name>" << result->m_Name << "</Name>\n"
       << "    <Path>" << result->m_Path << "</Path>\n"
       << "    <FullName>" << result->m_Path << "/" << result->m_Name << "</FullName>\n"
       << "    <FullCommandLine>" << result->m_FullCommandLine << "</FullCommandLine>\n"
       << "    <Results>" << std::endl;
    if ( result->m_ReturnValue )
      {
      os << "      <NamedMeasurement type=\"text/string\" name=\"Exit Code\"><Value>"
         << "CHILDSTATUS" << "</Value></NamedMeasurement>\n"
         << "      <NamedMeasurement type=\"text/string\" name=\"Exit Value\"><Value>"
         << result->m_ReturnValue << "</Value></NamedMeasurement>" << std::endl;
      }
    os << "      <NamedMeasurement type=\"numeric/double\" "
       << "name=\"Execution Time\"><Value>"
       << result->m_ExecutionTime << "</Value></NamedMeasurement>\n"
       << "      <NamedMeasurement type=\"text/string\" "
       << "name=\"Completion Status\"><Value>"
       << result->m_CompletionStatus << "</Value></NamedMeasurement>\n"
       << "      <Measurement>\n"
       << "        <Value>" << result->m_Output << "</value>\n"
       << "      </Measurement>\n"
       << "    </Results>\n"
       << "  </Test>" << std::endl;
    }
  
  os << "<EndDateTime>" << ::CurrentTime() << "</EndDateTime>\n"
     << "</Testing>\n"
     << "</Site>" << std::endl;
}

int ctest::ProcessTests()
{
  int res = 0;
  bool notest = true;
  int cc;

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
    this->UpdateDirectory();
    }
  if ( m_Tests[CONFIGURE_TEST] || m_Tests[ALL_TEST] )
    {
    this->ConfigureDirectory();
    }
  if ( m_Tests[BUILD_TEST] || m_Tests[ALL_TEST] )
    {
    this->BuildDirectory();
    }
  if ( m_Tests[TEST_TEST] || m_Tests[ALL_TEST] || notest )
    {
    std::vector<std::string> passed;
    std::vector<std::string> failed;
    int total;

    this->ProcessDirectory(passed, failed);

    total = int(passed.size()) + int(failed.size());

    if (total == 0)
      {
      std::cerr << "No tests were found!!!\n";
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

    res += int(failed.size());
    }
  if ( m_Tests[COVERAGE_TEST] || m_Tests[ALL_TEST] )
    {
    std::cerr << "Coverage test is not yet implemented" << std::endl;
    }
  if ( m_Tests[PURIFY_TEST] || m_Tests[ALL_TEST] )
    {
    std::cerr << "Purify test is not yet implemented" << std::endl;
    }
  return res;
}


// this is a test driver program for cmake.
int main (int argc, char *argv[])
{
  ctest inst;
  
  // look at the args
  std::vector<std::string> args;
  for(int i =0; i < argc; ++i)
    {
    args.push_back(argv[i]);
    }

#ifdef _WIN32
  std::string comspec = "cmw9xcom.exe";
  cmSystemTools::SetWindows9xComspecSubstitute(comspec.c_str());
#endif
  
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-D",0) == 0 && i < args.size() - 1)
      {
      inst.m_ConfigType = args[i+1];
      }

    if( arg.find("-V",0) == 0 || arg.find("--verbose",0) == 0 )
      {
      inst.m_Verbose = true;
      }

    if( ( arg.find("-T",0) == 0 || arg.find("--dart-mode",0) == 0 ) && (i < args.size() -1) )
      {
      inst.m_DartMode = true;
      inst.SetTest(args[i+1].c_str());
      }
    
    if(arg.find("-R",0) == 0 && i < args.size() - 1)
      {
      inst.m_UseIncludeRegExp = true;
      inst.m_IncludeRegExp  = args[i+1];
      }

    if(arg.find("-E",0) == 0 && i < args.size() - 1)
      {
      inst.m_UseExcludeRegExp = true;
      inst.m_ExcludeRegExp  = args[i+1];
      inst.m_UseExcludeRegExpFirst = inst.m_UseIncludeRegExp ? false : true;
      }

    if(arg.find("-h") == 0 || 
       arg.find("-help") == 0 || 
       arg.find("-H") == 0 || 
       arg.find("--help") == 0 || 
       arg.find("/H") == 0 || 
       arg.find("/HELP") == 0 || 
       arg.find("/?") == 0)
      {
      std::cerr << "Usage: " << argv[0] << " <options>" << std::endl
                << "\t -D type      Specify config type" << std::endl
                << "\t -E test      Specify regular expression for tests to exclude" 
                << std::endl
                << "\t -R test      Specify regular expression for tests to include" 
                << std::endl
                << "\t -V           Verbose testing" << std::endl
                << "\t -H           Help page" << std::endl;
      return 1;
      }
    }

  // call process directory
  inst.Initialize();
  int res = inst.ProcessTests();
  inst.Finalize();

  return res;
}

