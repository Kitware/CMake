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

#include "cmCTestCoverageHandler.h"

#include "cmCTest.h"
#include "cmake.h"
#include <cmsys/Process.h>

#include <stdlib.h> 
#include <math.h>
#include <float.h>

#define SAFEDIV(x,y) (((y)!=0)?((x)/(y)):(0))


//----------------------------------------------------------------------
cmCTestCoverageHandler::cmCTestCoverageHandler()
{
  m_Verbose = false; 
  m_CTest = 0;
}


//----------------------------------------------------------------------
//clearly it would be nice if this were broken up into a few smaller
//functions and commented...
int cmCTestCoverageHandler::CoverageDirectory(cmCTest *ctest_inst)
{
  m_CTest = ctest_inst;

  std::cout << "Performing coverage" << std::endl;
  double elapsed_time_start = cmSystemTools::GetTime();
  cmCTest::tm_VectorOfStrings files;
  cmCTest::tm_VectorOfStrings cfiles;
  cmCTest::tm_VectorOfStrings cdirs;
  bool done = false;
  std::string::size_type cc;
  std::string glob;
  std::map<std::string, std::string> allsourcefiles;
  std::map<std::string, std::string> allbinaryfiles;

  std::string start_time = m_CTest->CurrentTime();

  // Find all source files.
  std::string sourceDirectory = m_CTest->GetDartConfiguration("SourceDirectory");
  if ( sourceDirectory.size() == 0 )
    {
    std::cerr << "Cannot find SourceDirectory key in the DartConfiguration.tcl" << std::endl;
    return 1;
    }

  std::string coverageCommand = m_CTest->GetDartConfiguration("CoverageCommand");
  if ( coverageCommand.size() == 0 )
    {
    std::cerr << "Coverage command not defined in DartConfiguration.tcl" << std::endl;
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
    std::cerr << "Cannot find any coverage information files (.da)" << std::endl;
    return 1;
    }

  std::ofstream log; 
  if (!m_CTest->OpenOutputFile("Temporary", "Coverage.log", log))
    {
    std::cerr << "Cannot open log file" << std::endl;
    return 1;
    }
  log.close();
  if (!m_CTest->OpenOutputFile(m_CTest->GetCurrentTag(), "Coverage.xml", log))
    {
    std::cerr << "Cannot open log file" << std::endl;
    return 1;
    }

  std::string opath = m_CTest->GetToplevelPath() + "/Testing/Temporary/Coverage";
  cmSystemTools::MakeDirectory(opath.c_str());
  cfiles.clear();
  cmCTest::tm_VectorOfStrings ncfiles;
  cmCTest::tm_VectorOfStrings missing_files;

  for ( cc = 0; cc < files.size(); cc ++ )
    {
    std::string currPath = cmSystemTools::GetFilenamePath(files[cc]);
    std::string command = coverageCommand + " -o \"" + currPath + "\" -l \"" + files[cc] + "\"";
    std::string output;
    int retVal = 0;
    if ( m_Verbose )
      {
      std::cerr << "Run gcov on " << files[cc] << " in directory: " << currPath.c_str() << std::endl;
      }
    //std::cout << "   --- Run [" << command << "]" << std::endl;
    bool res = true;
    if ( !m_CTest->GetShowOnly() )
      {
      res = cmSystemTools::RunSingleCommand(command.c_str(), &output, 
        &retVal, currPath.c_str(),
        m_Verbose, 0 /*m_TimeOut*/);
      }
    if ( res && retVal == 0 )
      {
      //std::cout << " - done" << std::endl;
      glob = currPath + "/*";
      if ( !cmSystemTools::SimpleGlob(glob, ncfiles, 1) )
        {
        std::cerr << "Cannot found any coverage files" << std::endl;
        return 1;
        }
      cfiles.insert(cfiles.end(), ncfiles.begin(), ncfiles.end());
      std::vector<cmStdString> gcovlines;
      cmSystemTools::Split(output.c_str(), gcovlines);
      std::vector<cmStdString>::iterator git;
      const char* message = "Could not open source file";
      for ( git = gcovlines.begin(); git != gcovlines.end(); ++git )
        {
        if ( strncmp(git->c_str(), message, strlen(message) ) == 0 )
          {
          std::cerr << "Problem: " << git->c_str() << std::endl;
          missing_files.push_back(git->c_str() + strlen(message));
          }
        }
      }
    else
      {
      std::cerr << "Run gcov on " << files[cc] << std::flush;
      std::cerr << " [" << command << "]" << std::endl;
      std::cerr << " - fail" << std::endl;
      }
    }
  
  files.clear();
  std::map<std::string, cmCTest::tm_VectorOfStrings > sourcefiles;
  for ( cc = 0; cc < cfiles.size(); cc ++ )
    {
    std::string& fname = cfiles[cc];
    //    std::cout << "File: " << fname << std::endl;
    if ( strcmp(fname.substr(fname.size()-5, 5).c_str(), ".gcov") == 0 )
      {
      files.push_back(fname);
      std::string::size_type pos = fname.find(".da.");
      std::string::size_type pos2 = fname.find(".da##");
      if(pos2 != fname.npos)
        {
        pos = pos2+1;
        }
      if ( pos != fname.npos )
        {
        pos += 4;
        std::string::size_type epos = fname.size() - pos - strlen(".gcov");
        std::string nf = fname.substr(pos, epos);
        //std::cout << "Substring: " << nf << std::endl;
        if ( allsourcefiles.find(nf) != allsourcefiles.end() || 
             allbinaryfiles.find(nf) != allbinaryfiles.end() )
          {
          cmCTest::tm_VectorOfStrings &cvec = sourcefiles[nf];
          cvec.push_back(fname);
          }
        }
      }
    }
  // for ( cc = 0; cc < files.size(); cc ++ )
  //     {
  //     std::cout << "File: " << files[cc] << std::endl;
  //     }
  if ( missing_files.size() > 0 )
    {
    std::cout << "---------------------------------------------------------------" << std::endl;
    std::cout << "The following files were missing:" << std::endl;
    for ( cc = 0; cc < missing_files.size(); cc ++ )
      {
      std::cout << "File: " << missing_files[cc] << std::endl;
      }
    std::cout << "---------------------------------------------------------------" << std::endl;
    }

  std::map<std::string, cmCTest::tm_VectorOfStrings >::iterator it;
  cmCTestCoverageHandler::tm_CoverageMap coverageresults;

  m_CTest->StartXML(log);
  log << "<Coverage>\n"
      << "\t<StartDateTime>" << start_time << "</StartDateTime>" << std::endl;

  int total_tested = 0;
  int total_untested = 0;

  for ( it = sourcefiles.begin(); it != sourcefiles.end(); it ++ )
    {
    //std::cerr << "Source file: " << it->first << std::endl;
    cmCTest::tm_VectorOfStrings &gfiles = it->second;

    for ( cc = 0; cc < gfiles.size(); cc ++ )
      {
      int do_coverage = 1;
      std::string coverage_dir = cmSystemTools::GetFilenamePath(gfiles[cc].c_str());
      std::string builDir = m_CTest->GetDartConfiguration("BuildDirectory");
      do
        {
        std::string coverage_file = coverage_dir + "/.NoDartCoverage";
        if ( cmSystemTools::FileExists(coverage_file.c_str()) )
          {
          do_coverage = 0;
          break;
          }
        // is there a parent directory we can check
        std::string::size_type pos = coverage_dir.rfind('/');
        // if we could not find the directory return 0
        if(pos == std::string::npos)
          {
          break;
          }
        coverage_dir = coverage_dir.substr(0, pos);
        }
      while (coverage_dir.size() >= builDir.size());
      if ( !do_coverage )
        {
        continue;
        }
      //std::cout << "\t" << gfiles[cc] << std::endl;
      std::ifstream ifile(gfiles[cc].c_str());
      if ( !ifile )
        {
        std::cerr << "Cannot open file: " << gfiles[cc].c_str() << std::endl;
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
      cmCTestCoverageHandler::cmCTestCoverage& cov = coverageresults[it->first];
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
        std::string src_dir = m_CTest->GetDartConfiguration("SourceDirectory");
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
      std::string::size_type kk;
      //      std::cerr << "number of lines " << lines.size() << "\n";
      for ( kk = 0; kk < lines.size(); kk ++ )
        {
        std::string& line = lines[kk];
        //std::cerr << line << "\n";
        std::string sub1 = line.substr(0, strlen("    #####"));
        std::string sub2 = line.substr(0, strlen("      ######"));
        int count = atoi(sub2.c_str());
        if ( sub1.compare("    #####") == 0 ||
             sub2.compare("      ######") == 0 )
          {
          if ( covlines[kk] == -1 )
            {
            covlines[kk] = 0;
            }
          cov.m_UnTested ++;
          //std::cout << "Untested - ";
          }
        else if ( count > 0 )
          {
          if ( covlines[kk] == -1 )
            {
            covlines[kk] = 0;
            }
          cov.m_Tested ++;
          covlines[kk] ++;
          //std::cout << "Tested[" << count << "] - ";
          }

        //std::cout << line << std::endl;
        }
      }
    }

  //std::cerr << "Finalizing" << std::endl;
  cmCTestCoverageHandler::tm_CoverageMap::iterator cit;
  int ccount = 0;
  std::ofstream cfileoutput; 
  int cfileoutputcount = 0;
  char cfileoutputname[100];
  std::string local_start_time = m_CTest->CurrentTime();
  std::string local_end_time;
  for ( cit = coverageresults.begin(); cit != coverageresults.end(); cit ++ )
    {
    cmCTestCoverageHandler::cmCTestCoverage &cov = cit->second;
    if ( !cov.m_Show )
      {
      continue;
      }

    // Check if we should ignore the directory, if we find a NoDartCoverage
    // file in it or any of its parents
    int do_coverage = 1;
    std::string coverage_dir = cmSystemTools::GetFilenamePath(cov.m_AbsolutePath.c_str());
    do
      {
      std::string coverage_file = coverage_dir + "/.NoDartCoverage";
      if ( cmSystemTools::FileExists(coverage_file.c_str()) )
        {
        do_coverage = 0;
        break;
        }
      // is there a parent directory we can check
      std::string::size_type pos = coverage_dir.rfind('/');
      // if we could not find the directory return 0
      if(pos == std::string::npos)
        {
        break;
        }
      coverage_dir = coverage_dir.substr(0, pos);

      }
    while (coverage_dir.size() >= sourceDirectory.size());

    if (!do_coverage)
      {
      if ( m_Verbose )
        {
        std::cout << "Ignore file: " << cov.m_FullPath.c_str() << std::endl;
        }
      continue;
      }
    
    if ( ccount == 100 )
      {
      local_end_time = m_CTest->CurrentTime();
      cfileoutput << "\t<EndDateTime>" << local_end_time << "</EndDateTime>\n"
        << "</CoverageLog>" << std::endl;
      m_CTest->EndXML(cfileoutput);
      cfileoutput.close();
      std::cout << "Close file: " << cfileoutputname << std::endl;
      ccount = 0;
      }
    if ( ccount == 0 )
      {
      sprintf(cfileoutputname, "CoverageLog-%d.xml", cfileoutputcount++);
      std::cout << "Open file: " << cfileoutputname << std::endl;
      if (!m_CTest->OpenOutputFile(m_CTest->GetCurrentTag(), 
                                   cfileoutputname, cfileoutput))
        {
        std::cerr << "Cannot open log file: " << cfileoutputname << std::endl;
        return 1;
        }
      local_start_time = m_CTest->CurrentTime();
      m_CTest->StartXML(cfileoutput);
      cfileoutput << "<CoverageLog>\n"
        << "\t<StartDateTime>" << local_start_time << "</StartDateTime>" << std::endl;
      }

    //std::cerr << "Final process of Source file: " << cit->first << std::endl;
    cov.m_UnTested = 0;
    cov.m_Tested = 0;
    for ( cc = 0; cc < cov.m_Lines.size(); cc ++ )
      {
      if ( cov.m_Lines[cc] == 0 )
        {
        cov.m_UnTested ++;
        }
      else if ( cov.m_Lines[cc] > 0 )
        {
        cov.m_Tested ++;
        }
      }

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
    local_end_time = m_CTest->CurrentTime();
    cfileoutput << "\t<EndDateTime>" << local_end_time << "</EndDateTime>\n"
                << "</CoverageLog>" << std::endl;
    m_CTest->EndXML(cfileoutput);
    cfileoutput.close();
    }

  int total_lines = total_tested + total_untested;
  float percent_coverage = 100 * SAFEDIV(static_cast<float>(total_tested),
    static_cast<float>(total_lines));
  if ( total_lines == 0 )
    {
    percent_coverage = 0;
    }

  std::string end_time = m_CTest->CurrentTime();

  log << "\t<LOCTested>" << total_tested << "</LOCTested>\n"
      << "\t<LOCUntested>" << total_untested << "</LOCUntested>\n"
      << "\t<LOC>" << total_lines << "</LOC>\n"
      << "\t<PercentCoverage>";
  log.setf(std::ios::fixed, std::ios::floatfield);
  log.precision(2);
  log << (percent_coverage)<< "</PercentCoverage>\n"
      << "\t<EndDateTime>" << end_time << "</EndDateTime>\n";
  log << "<ElapsedMinutes>" << 
    static_cast<int>((cmSystemTools::GetTime() - elapsed_time_start)/6)/10.0
      << "</ElapsedMinutes>"
      << "</Coverage>" << std::endl;
  m_CTest->EndXML(log);

  std::cout << "\tCovered LOC:         " << total_tested << std::endl
            << "\tNot covered LOC:     " << total_untested << std::endl
            << "\tTotal LOC:           " << total_lines << std::endl
            << "\tPercentage Coverage: ";

  std::cout.setf(std::ios::fixed, std::ios::floatfield);
  std::cout.precision(2);
  std::cout << (percent_coverage) << "%" << std::endl;


  return 1;
}
