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
#include "cmSystemTools.h"
#include "cmGeneratedFileStream.h"
#include "cmGlob.h"
#include <cmsys/Process.h>
#include <cmsys/RegularExpression.hxx>

#include <stdlib.h> 
#include <math.h>
#include <float.h>

#define SAFEDIV(x,y) (((y)!=0)?((x)/(y)):(0))

//----------------------------------------------------------------------
cmCTestCoverageHandler::cmCTestCoverageHandler()
{
}

//----------------------------------------------------------------------
void cmCTestCoverageHandler::Initialize()
{
}

//----------------------------------------------------------------------
bool cmCTestCoverageHandler::StartLogFile(cmGeneratedFileStream& covLogFile, int logFileCount)
{
  char covLogFilename[1024];
  sprintf(covLogFilename, "CoverageLog-%d.xml", logFileCount);
  cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Open file: " << covLogFilename << std::endl);
  if (!m_CTest->OpenOutputFile(m_CTest->GetCurrentTag(), 
      covLogFilename, covLogFile, true))
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot open log file: " << covLogFilename << std::endl);
    return false;
    }
  std::string local_start_time = m_CTest->CurrentTime();
  m_CTest->StartXML(covLogFile);
  covLogFile << "<CoverageLog>" << std::endl
    << "\t<StartDateTime>" << local_start_time << "</StartDateTime>" << std::endl;
  return true;
}

//----------------------------------------------------------------------
void cmCTestCoverageHandler::EndLogFile(cmGeneratedFileStream& ostr, int logFileCount)
{
  std::string local_end_time = m_CTest->CurrentTime();
  ostr << "\t<EndDateTime>" << local_end_time << "</EndDateTime>" << std::endl
    << "</CoverageLog>" << std::endl;
  m_CTest->EndXML(ostr);
  char covLogFilename[1024];
  sprintf(covLogFilename, "CoverageLog-%d.xml", logFileCount);
  cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Close file: " << covLogFilename << std::endl);
  ostr.close();
}

//----------------------------------------------------------------------
bool cmCTestCoverageHandler::ShouldIDoCoverage(const char* file, const char* srcDir,
  const char* binDir)
{
  std::string fSrcDir = cmSystemTools::CollapseFullPath(srcDir);
  std::string fBinDir = cmSystemTools::CollapseFullPath(binDir);
  std::string fFile = cmSystemTools::CollapseFullPath(file);
  bool sourceSubDir = cmSystemTools::IsSubDirectory(fFile.c_str(), fSrcDir.c_str());
  bool buildSubDir = cmSystemTools::IsSubDirectory(fFile.c_str(), fBinDir.c_str());
  // Always check parent directory of the file.
  std::string fileDir = cmSystemTools::GetFilenamePath(fFile.c_str());
  std::string checkDir;

  // We also need to check the binary/source directory pair.
  if ( sourceSubDir && buildSubDir )
    {
    if ( fSrcDir.size() > fBinDir.size() )
      {
      checkDir = fSrcDir;
      }
    else
      {
      checkDir = fBinDir;
      }
    }
  else if ( sourceSubDir )
    {
    checkDir = fSrcDir;
    }
  else if ( buildSubDir )
    {
    checkDir = fBinDir;
    }
  std::string ndc
    = cmSystemTools::FileExistsInParentDirectories(".NoDartCoverage",
      fFile.c_str(), checkDir.c_str());
  if ( ndc.size() )
    {
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Found: " << ndc.c_str() << " so skip coverage of " << file << std::endl);
    return false;
    }

  // By now checkDir should be set to parent directory of the file.
  // Get the relative path to the file an apply it to the opposite directory.
  // If it is the same as fileDir, then ignore, otherwise check.
  std::string relPath = cmSystemTools::RelativePath(checkDir.c_str(),
    fFile.c_str());
  if ( checkDir == fSrcDir )
    {
    checkDir = fBinDir;
    }
  else
    {
    checkDir = fSrcDir;
    }
  fFile = checkDir + "/" + relPath;
  fFile = cmSystemTools::GetFilenamePath(fFile.c_str());

  if ( fileDir == fFile )
    {
    // This is in-source build, so we trust the previous check.
    return true;
    }

  ndc = cmSystemTools::FileExistsInParentDirectories(".NoDartCoverage",
    fFile.c_str(), checkDir.c_str());
  if ( ndc.size() )
    {
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Found: " << ndc.c_str() << " so skip coverage of: " << file << std::endl);
    return false;
    }
  // Ok, nothing in source tree, nothing in binary tree
  return true;
}

//----------------------------------------------------------------------
//clearly it would be nice if this were broken up into a few smaller
//functions and commented...
int cmCTestCoverageHandler::ProcessHandler()
{
  int error = 0;

  std::string sourceDir = m_CTest->GetCTestConfiguration("SourceDirectory");
  std::string binaryDir = m_CTest->GetCTestConfiguration("BuildDirectory");
  std::string gcovCommand = m_CTest->GetCTestConfiguration("CoverageCommand");

  cmGeneratedFileStream ofs;
  double elapsed_time_start = cmSystemTools::GetTime();
  if ( !m_CTest->OpenOutputFile("Temporary", "LastCoverage.log", ofs) )
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot create LastCoverage.log file" << std::endl);
    }

  ofs << "Performing coverage: " << elapsed_time_start << std::endl;

  cmSystemTools::ConvertToUnixSlashes(sourceDir);
  cmSystemTools::ConvertToUnixSlashes(binaryDir);

  //std::string asfGlob = sourceDir + "/*";
  //std::string abfGlob = binaryDir + "/*";
  std::string daGlob = binaryDir + "/*.da";
  std::string gcovOutputRex = "[0-9]+\\.[0-9]+% of [0-9]+ (source |)lines executed in file (.*)$";
  std::string gcovOutputRex2 = "^Creating (.*\\.gcov)\\.";

  cmCTestLog(m_CTest, HANDLER_OUTPUT, "Performing coverage" << std::endl);

  std::string coverage_start_time = m_CTest->CurrentTime();

  std::string testingDir = m_CTest->GetBinaryDir() + "/Testing";
  std::string tempDir = testingDir + "/CoverageInfo";
  std::string currentDirectory = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::MakeDirectory(tempDir.c_str());
  cmSystemTools::ChangeDirectory(tempDir.c_str());

  cmGlob gl;
  gl.RecurseOn();
  gl.FindFiles(daGlob);
  std::vector<std::string> files = gl.GetFiles();
  std::vector<std::string>::iterator it;

  if ( files.size() == 0 )
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, " Cannot find any coverage files." << std::endl);
    // No coverage files is a valid thing, so the exit code is 0
    return 0;
    }

  // Regular expressions for output of gcov
  cmsys::RegularExpression re(gcovOutputRex.c_str());
  cmsys::RegularExpression re2(gcovOutputRex2.c_str());

  typedef std::vector<int> singleFileCoverageVector;
  typedef std::map<std::string, singleFileCoverageVector> totalCoverageMap;

  totalCoverageMap totalCoverage;

  std::string cfile = "";
  for ( it = files.begin(); it != files.end(); ++ it )
    {
    std::string fileDir = cmSystemTools::GetFilenamePath(it->c_str());
    std::string command = "\"" + gcovCommand + "\" -l -o \"" + fileDir + "\" \"" + *it + "\"";
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, command.c_str() << std::endl);
    std::string output = "";
    std::string errors = "";
    int retVal = 0;
    ofs << "* Run coverage for: " << fileDir.c_str() << std::endl;
    ofs << "  Command: " << command.c_str() << std::endl;
    int res = m_CTest->RunCommand(command.c_str(), &output, &errors,
      &retVal, tempDir.c_str(), 0 /*m_TimeOut*/);

    ofs << "  Output: " << output.c_str() << std::endl;
    ofs << "  Errors: " << errors.c_str() << std::endl;
    if ( ! res )
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Problem running coverage on file: " << it->c_str() << std::endl);
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Command produced error: " << error << std::endl);
      error ++;
      continue;
      }
    if ( retVal != 0 )
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Coverage command returned: " << retVal << " while processing: " << it->c_str() << std::endl);
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Command produced error: " << error << std::endl);
      }
    std::vector<cmStdString> lines;
    std::vector<cmStdString>::iterator line;
    cmSystemTools::Split(output.c_str(), lines);
    for ( line = lines.begin(); line != lines.end(); ++line)
      {
      if ( re.find(line->c_str()) )
        {
        cfile = "";
        std::string file = re.match(2);
        // Is it in the source dir?
        if ( file.size() > sourceDir.size() &&
          file.substr(0, sourceDir.size()) == sourceDir &&
          file[sourceDir.size()] == '/' )
          {
          cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "   produced s: " << file.c_str() << std::endl);
          ofs << "  produced in source dir: " << file.c_str() << std::endl;
          cfile = file;
          }
        // Binary dir?
        if ( file.size() > binaryDir.size() &&
          file.substr(0, binaryDir.size()) == binaryDir &&
          file[binaryDir.size()] == '/' )
          {
          cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "   produce b: " << file.c_str() << std::endl);
          ofs << "  produced in binary dir: " << file.c_str() << std::endl;
          cfile = file;
          }
        if ( cfile.empty() )
          {
          cmCTestLog(m_CTest, ERROR_MESSAGE, "Something went wrong" << std::endl);
          cmCTestLog(m_CTest, ERROR_MESSAGE, "File: [" << file << "]" << std::endl);
          cmCTestLog(m_CTest, ERROR_MESSAGE, "s: [" << file.substr(0, sourceDir.size()) << "]" << std::endl);
          cmCTestLog(m_CTest, ERROR_MESSAGE, "b: [" << file.substr(0, binaryDir.size()) << "]" << std::endl);
          ofs << "  Something went wrong. Cannot find: " << file.c_str()
            << " in source dir: " << sourceDir.c_str()
            << " or binary dir: " << binaryDir.c_str() << std::endl;
          }
        }
      else if ( re2.find(line->c_str() ) )
        {
        std::string fname = re2.match(1);
        if ( cfile.size() )
          {
          singleFileCoverageVector* vec = &totalCoverage[cfile];
          cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "   in file: " << fname << std::endl);
          ofs << "  In file: " << fname << std::endl;
          std::ifstream ifile(fname.c_str());
          if ( ! ifile )
            {
            cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot open file: " << fname << std::endl);
            }
          else
            {
            long cnt = -1;
            std::string nl;
            while ( cmSystemTools::GetLineFromStream(ifile, nl) )
              {
              cnt ++;
              if ( vec->size() <= static_cast<singleFileCoverageVector::size_type>(cnt) )
                {
                vec->push_back(-1);
                }

              //TODO: Handle gcov 3.0 non-coverage lines

              // Skip empty lines
              if ( !nl.size() )
                {
                continue;
                }

              // Skip unused lines
              if ( nl[0] == '\t' || nl.size() < 12 )
                {
                continue;
                }

              std::string prefix = nl.substr(0, 12);
              int cov = atoi(prefix.c_str());
              (*vec)[cnt] += cov;
              }
            }
          }
        }
      else
        {
        cmCTestLog(m_CTest, ERROR_MESSAGE, "Unknown line: " << line->c_str() << std::endl);
        ofs << "  Unknown line: " << line->c_str() << std::endl;
        error ++;
        }
      }
    }

  cmGeneratedFileStream covSumFile;
  cmGeneratedFileStream covLogFile;

  if (!m_CTest->OpenOutputFile(m_CTest->GetCurrentTag(), 
      "Coverage.xml", covSumFile, true))
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot open coverage summary file: Coverage.xml" << std::endl);

    return -1;
    }

  m_CTest->StartXML(covSumFile);
  // Produce output xml files

  covSumFile << "<Coverage>" << std::endl
    << "\t<StartDateTime>" << coverage_start_time << "</StartDateTime>" << std::endl;
  int logFileCount = 0;
  if ( !this->StartLogFile(covLogFile, logFileCount) )
    {
    return -1;
    }
  totalCoverageMap::iterator fileIterator;
  int cnt = 0;
  long total_tested = 0;
  long total_untested = 0;
  //std::string fullSourceDir = sourceDir + "/";
  //std::string fullBinaryDir = binaryDir + "/";
  for ( fileIterator = totalCoverage.begin();
    fileIterator != totalCoverage.end();
    ++fileIterator )
    {
    if ( cnt == 100 )
      {
      cnt = 0;
      this->EndLogFile(covLogFile, logFileCount);
      logFileCount ++;
      if ( !this->StartLogFile(covLogFile, logFileCount) )
        {
        return -1;
        }
      }
    const std::string fullFileName = fileIterator->first;
    const std::string fileName = cmSystemTools::GetFilenameName(fullFileName.c_str());
    std::string fullFilePath = cmSystemTools::GetFilenamePath(fullFileName.c_str());
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Process file: " << fullFileName << std::endl);

    cmSystemTools::ConvertToUnixSlashes(fullFilePath);

    if ( !cmSystemTools::FileExists(fullFileName.c_str()) )
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot find file: " << fullFileName.c_str() << std::endl);
      continue;
      }

    bool shouldIDoCoverage
      = this->ShouldIDoCoverage(fullFileName.c_str(),
        sourceDir.c_str(), binaryDir.c_str());
    if ( !shouldIDoCoverage )
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, ".NoDartCoverage found, so skip coverage check for: "
        << fullFileName.c_str()
        << std::endl);
      continue;
      }

    const singleFileCoverageVector& fcov = fileIterator->second;
    covLogFile << "\t<File Name=\""
      << m_CTest->MakeXMLSafe(fileName.c_str())
      << "\" FullPath=\"" << m_CTest->MakeXMLSafe(m_CTest->GetShortPathToFile(
          fileIterator->first.c_str())) << "\">" << std::endl
      << "\t\t<Report>" << std::endl;

    std::ifstream ifs(fullFileName.c_str());
    if ( !ifs)
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot open source file: " << fullFileName.c_str() << std::endl);
      error ++;
      continue;
      }

    int tested = 0;
    int untested = 0;

    singleFileCoverageVector::size_type cc;
    std::string line;
    for ( cc= 0; cc < fcov.size(); cc ++ )
      {
      if ( !cmSystemTools::GetLineFromStream(ifs, line) )
        {
        cmCTestLog(m_CTest, ERROR_MESSAGE, "Problem reading source file: " << fullFileName.c_str() << " line:" << cc << std::endl);
        error ++;
        break;
        }
      covLogFile << "\t\t<Line Number=\"" << cc << "\" Count=\"" << fcov[cc] << "\">"
        << m_CTest->MakeXMLSafe(line.c_str()) << "</Line>" << std::endl;
      if ( fcov[cc] == 0 )
        {
        untested ++;
        }
      else if ( fcov[cc] > 0 )
        {
        tested ++;
        }
      }
    if ( cmSystemTools::GetLineFromStream(ifs, line) )
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Looks like there are more lines in the file: " << line << std::endl);
      }
    float cper = 0;
    float cmet = 0;
    if ( tested + untested > 0 )
      {
      cper = (100 * SAFEDIV(static_cast<float>(tested),
          static_cast<float>(tested + untested)));
      cmet = ( SAFEDIV(static_cast<float>(tested + 10),
          static_cast<float>(tested + untested + 10)));
      }
    total_tested += tested;
    total_untested += untested;
    covLogFile << "\t\t</Report>" << std::endl
      << "\t</File>" << std::endl;
    covSumFile << "\t<File Name=\"" << m_CTest->MakeXMLSafe(fileName)
      << "\" FullPath=\"" << m_CTest->MakeXMLSafe(
        m_CTest->GetShortPathToFile(fullFileName.c_str()))
      << "\" Covered=\"" << (cmet>0?"true":"false") << "\">\n"
      << "\t\t<LOCTested>" << tested << "</LOCTested>\n"
      << "\t\t<LOCUnTested>" << untested << "</LOCUnTested>\n"
      << "\t\t<PercentCoverage>";
    covSumFile.setf(std::ios::fixed, std::ios::floatfield);
    covSumFile.precision(2);
    covSumFile << (cper) << "</PercentCoverage>\n"
      << "\t\t<CoverageMetric>";
    covSumFile.setf(std::ios::fixed, std::ios::floatfield);
    covSumFile.precision(2);
    covSumFile << (cmet) << "</CoverageMetric>\n"
      << "\t</File>" << std::endl;
    cnt ++;
    }
  this->EndLogFile(covLogFile, logFileCount);

  int total_lines = total_tested + total_untested;
  float percent_coverage = 100 * SAFEDIV(static_cast<float>(total_tested),
    static_cast<float>(total_lines));
  if ( total_lines == 0 )
    {
    percent_coverage = 0;
    }

  std::string end_time = m_CTest->CurrentTime();

  covSumFile << "\t<LOCTested>" << total_tested << "</LOCTested>\n"
    << "\t<LOCUntested>" << total_untested << "</LOCUntested>\n"
    << "\t<LOC>" << total_lines << "</LOC>\n"
    << "\t<PercentCoverage>";
  covSumFile.setf(std::ios::fixed, std::ios::floatfield);
  covSumFile.precision(2);
  covSumFile << (percent_coverage)<< "</PercentCoverage>\n"
    << "\t<EndDateTime>" << end_time << "</EndDateTime>\n";
  covSumFile << "<ElapsedMinutes>" << 
    static_cast<int>((cmSystemTools::GetTime() - elapsed_time_start)/6)/10.0
    << "</ElapsedMinutes>"
    << "</Coverage>" << std::endl;
  m_CTest->EndXML(covSumFile);

  cmCTestLog(m_CTest, HANDLER_OUTPUT, "\tCovered LOC:         " << total_tested << std::endl
    << "\tNot covered LOC:     " << total_untested << std::endl
    << "\tTotal LOC:           " << total_lines << std::endl
    << "\tPercentage Coverage: " 
    << std::setiosflags(std::ios::fixed)
    << std::setprecision(2)
    << (percent_coverage) << "%" << std::endl);

  ofs << "\tCovered LOC:         " << total_tested << std::endl
    << "\tNot covered LOC:     " << total_untested << std::endl
    << "\tTotal LOC:           " << total_lines << std::endl
    << "\tPercentage Coverage: " 
    << std::setiosflags(std::ios::fixed)
    << std::setprecision(2)
    << (percent_coverage) << "%" << std::endl;

  cmSystemTools::ChangeDirectory(currentDirectory.c_str());

  if ( error )
    {
    return -1;
    }
  return 0;
}
