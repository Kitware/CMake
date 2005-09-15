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
  this->Superclass::Initialize();
  m_CustomCoverageExclude.empty();
}

//----------------------------------------------------------------------
bool cmCTestCoverageHandler::StartCoverageLogFile(cmGeneratedFileStream& covLogFile, int logFileCount)
{
  char covLogFilename[1024];
  sprintf(covLogFilename, "CoverageLog-%d", logFileCount);
  cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Open file: " << covLogFilename << std::endl);
  if (!this->StartResultingXML(covLogFilename, covLogFile) )
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
void cmCTestCoverageHandler::EndCoverageLogFile(cmGeneratedFileStream& ostr, int logFileCount)
{
  std::string local_end_time = m_CTest->CurrentTime();
  ostr << "\t<EndDateTime>" << local_end_time << "</EndDateTime>" << std::endl
    << "</CoverageLog>" << std::endl;
  m_CTest->EndXML(ostr);
  char covLogFilename[1024];
  sprintf(covLogFilename, "CoverageLog-%d.xml", logFileCount);
  cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Close file: " << covLogFilename << std::endl);
  ostr.Close();
}

//----------------------------------------------------------------------
bool cmCTestCoverageHandler::ShouldIDoCoverage(const char* file, const char* srcDir,
  const char* binDir)
{
  std::vector<cmsys::RegularExpression>::iterator sit;
  for ( sit = m_CustomCoverageExcludeRegex.begin();
    sit != m_CustomCoverageExcludeRegex.end(); ++ sit )
    {
    if ( sit->find(file) )
      {
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "  File " << file
        << " is excluded in CTestCustom.ctest" << std::endl;);
      return false;
      }
    }

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
  if ( !this->StartLogFile("Coverage", ofs) )
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot create LastCoverage.log file" << std::endl);
    }

  ofs << "Performing coverage: " << elapsed_time_start << std::endl;

  cmSystemTools::ConvertToUnixSlashes(sourceDir);
  cmSystemTools::ConvertToUnixSlashes(binaryDir);

  std::string asfGlob = sourceDir + "/*";
  std::string abfGlob = binaryDir + "/*";
 
  
  // Style 1
  std::string st1gcovOutputRex1 = "[0-9]+\\.[0-9]+% of [0-9]+ (source |)lines executed in file (.*)$";
  std::string st1gcovOutputRex2 = "^Creating (.*\\.gcov)\\.";
  cmsys::RegularExpression st1re1(st1gcovOutputRex1.c_str());
  cmsys::RegularExpression st1re2(st1gcovOutputRex2.c_str());
 
 
  // Style 2
  std::string st2gcovOutputRex1 = "^File *[`'](.*)'$";
  std::string st2gcovOutputRex2 = "Lines executed: *[0-9]+\\.[0-9]+% of [0-9]+$";
  std::string st2gcovOutputRex3 = "^(.*):creating [`'](.*\\.gcov)'";
  std::string st2gcovOutputRex4 = "^(.*):unexpected EOF *$";
  std::string st2gcovOutputRex5 = "^(.*):cannot open source file*$";
  std::string st2gcovOutputRex6 = "^(.*):source file is newer than graph file `(.*)'$";
  cmsys::RegularExpression st2re1(st2gcovOutputRex1.c_str());
  cmsys::RegularExpression st2re2(st2gcovOutputRex2.c_str());
  cmsys::RegularExpression st2re3(st2gcovOutputRex3.c_str());
  cmsys::RegularExpression st2re4(st2gcovOutputRex4.c_str());
  cmsys::RegularExpression st2re5(st2gcovOutputRex5.c_str());
  cmsys::RegularExpression st2re6(st2gcovOutputRex6.c_str());
 
  cmCTestLog(m_CTest, HANDLER_OUTPUT, "Performing coverage" << std::endl);
 
  std::string coverage_start_time = m_CTest->CurrentTime();
 
  std::string testingDir = m_CTest->GetBinaryDir() + "/Testing";
  std::string tempDir = testingDir + "/CoverageInfo";
  std::string currentDirectory = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::MakeDirectory(tempDir.c_str());
  cmSystemTools::ChangeDirectory(tempDir.c_str());
 
  cmGlob gl;
  gl.RecurseOn();
  std::string daGlob = binaryDir + "/*.da";
  gl.FindFiles(daGlob);
  std::vector<std::string> files = gl.GetFiles();
  daGlob = binaryDir + "/*.gcda";
  gl.FindFiles(daGlob);
  std::vector<std::string>& moreFiles = gl.GetFiles();
  files.insert(files.end(), moreFiles.begin(), moreFiles.end());
  std::vector<std::string>::iterator it;
 
  if ( files.size() == 0 )
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, " Cannot find any coverage files." << std::endl);
    // No coverage files is a valid thing, so the exit code is 0
    return 0;
    }

  m_CustomCoverageExcludeRegex.empty();
  std::vector<cmStdString>::iterator rexIt;
  for ( rexIt = m_CustomCoverageExclude.begin();
    rexIt != m_CustomCoverageExclude.end();
    ++ rexIt )
    {
    m_CustomCoverageExcludeRegex.push_back(cmsys::RegularExpression(rexIt->c_str()));
    }
 
  typedef std::vector<int> singleFileCoverageVector;
  typedef std::map<std::string, singleFileCoverageVector> totalCoverageMap;
 
  totalCoverageMap totalCoverage;
 
  int gcovStyle = 0;

  std::set<std::string> missingFiles;
 
  std::string actualSourceFile = "";
  cmCTestLog(m_CTest, HANDLER_OUTPUT, "   Processing coverage (each . represents one file):" << std::endl);
  cmCTestLog(m_CTest, HANDLER_OUTPUT, "    ");
  int file_count = 0;
  for ( it = files.begin(); it != files.end(); ++ it )
    {
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "." << std::flush);
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
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Command produced error: " << errors << std::endl);
      error ++;
      continue;
      }
    if ( retVal != 0 )
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Coverage command returned: " << retVal << " while processing: " << it->c_str() << std::endl);
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Command produced error: " << error << std::endl);
      }
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT,
      "--------------------------------------------------------------" << std::endl
      << output << std::endl
      << "--------------------------------------------------------------" << std::endl);
    std::vector<cmStdString> lines;
    std::vector<cmStdString>::iterator line;
 
 
    // Globals for storing current source file and current gcov file;
    cmSystemTools::Split(output.c_str(), lines);
    for ( line = lines.begin(); line != lines.end(); ++line)
      {
      std::string sourceFile;
      std::string gcovFile;
      cmCTestLog(m_CTest, DEBUG, "Line: [" << line->c_str() << "]" << std::endl);
      if ( line->size() == 0 )
        {
        // Ignore empty line; probably style 2
        }
      else if ( st1re1.find(line->c_str()) )
        {
        if ( gcovStyle != 0 )
          {
          if ( gcovStyle != 1 )
            {
            cmCTestLog(m_CTest, ERROR_MESSAGE, "Unknown gcov output style" << std::endl);
            error ++;
            break;
            }
          gcovStyle = 1;
          }
 
        actualSourceFile = "";
        sourceFile = st1re1.match(2);
        }
      else if ( st1re2.find(line->c_str() ) )
        {
        if ( gcovStyle != 0 )
          {
          if ( gcovStyle != 1 )
            {
            cmCTestLog(m_CTest, ERROR_MESSAGE, "Unknown gcov output style" << std::endl);
            error ++;
            break;
            }
          gcovStyle = 1;
          }
 
        gcovFile = st1re2.match(1);
        }
      else if ( st2re1.find(line->c_str() ) )
        {
        if ( gcovStyle != 0 )
          {
          if ( gcovStyle != 2 )
            {
            cmCTestLog(m_CTest, ERROR_MESSAGE, "Unknown gcov output style" << std::endl);
            error ++;
            break;
            }
          gcovStyle = 2;
          }
 
        actualSourceFile = "";
        sourceFile = st2re1.match(1);
        }
      else if ( st2re2.find(line->c_str() ) )
        {
        if ( gcovStyle != 0 )
          {
          if ( gcovStyle != 2 )
            {
            cmCTestLog(m_CTest, ERROR_MESSAGE, "Unknown gcov output style" << std::endl);
            error ++;
            break;
            }
          gcovStyle = 2;
          }
        }
      else if ( st2re3.find(line->c_str() ) )
        {
        if ( gcovStyle != 0 )
          {
          if ( gcovStyle != 2 )
            {
            cmCTestLog(m_CTest, ERROR_MESSAGE, "Unknown gcov output style" << std::endl);
            error ++;
            break;
            }
          gcovStyle = 2;
          }
 
        gcovFile = st2re3.match(2);
        }
      else if ( st2re4.find(line->c_str() ) )
        {
        if ( gcovStyle != 0 )
          {
          if ( gcovStyle != 2 )
            {
            cmCTestLog(m_CTest, ERROR_MESSAGE, "Unknown gcov output style" << std::endl);
            error ++;
            break;
            }
          gcovStyle = 2;
          }
 
        cmCTestLog(m_CTest, WARNING, "Warning: " << st2re4.match(1) << " had unexpected EOF" << std::endl);
        }
      else if ( st2re5.find(line->c_str() ) )
        {
        if ( gcovStyle != 0 )
          {
          if ( gcovStyle != 2 )
            {
            cmCTestLog(m_CTest, ERROR_MESSAGE, "Unknown gcov output style" << std::endl);
            error ++;
            break;
            }
          gcovStyle = 2;
          }
 
        cmCTestLog(m_CTest, WARNING, "Warning: Cannot open file: " << st2re5.match(1) << std::endl);
        }
      else if ( st2re6.find(line->c_str() ) )
        {
        if ( gcovStyle != 0 )
          {
          if ( gcovStyle != 2 )
            {
            cmCTestLog(m_CTest, ERROR_MESSAGE, "Unknown gcov output style" << std::endl);
            error ++;
            break;
            }
          gcovStyle = 2;
          }
 
        cmCTestLog(m_CTest, WARNING, "Warning: File: " << st2re6.match(1)
          << " is newer than " << st2re6.match(2) << std::endl);
        }
      else
        {
        cmCTestLog(m_CTest, ERROR_MESSAGE, "Unknown line: [" << line->c_str() << "]" << std::endl);
        error ++;
        //abort();
        }
      if ( !gcovFile.empty() && actualSourceFile.size() )
        {
        singleFileCoverageVector* vec = &totalCoverage[actualSourceFile];
        cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "   in file: " << gcovFile << std::endl);
        std::ifstream ifile(gcovFile.c_str());
        if ( ! ifile )
          {
          cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot open file: " << gcovFile << std::endl);
          }
        else
          {
          long cnt = -1;
          std::string nl;
          while ( cmSystemTools::GetLineFromStream(ifile, nl) )
            {
            cnt ++;
 
            //TODO: Handle gcov 3.0 non-coverage lines
 
            // Skip empty lines
            if ( !nl.size() )
              {
              continue;
              }
 
            // Skip unused lines
            if ( nl.size() < 12 )
              {
              continue;
              }

            // Read the coverage count from the beginning of the gcov output line
            std::string prefix = nl.substr(0, 12);
            int cov = atoi(prefix.c_str());
            // Read the line number starting at the 10th character of the gcov output line
            std::string lineNumber = nl.substr(10, 5);
            int lineIdx = atoi(lineNumber.c_str())-1;
            if ( lineIdx >= 0 )
              {
              while ( vec->size() <= static_cast<singleFileCoverageVector::size_type>(lineIdx) )
                {
                vec->push_back(-1);
                }
              // Initially all entries are -1 (not used). If we get coverage
              // information, increment it to 0 first.
              if ( (*vec)[lineIdx] < 0 )
                {
                if ( cov > 0 || prefix.find("#") != prefix.npos )
                  {
                  (*vec)[lineIdx] = 0;
                  }
                }
              (*vec)[lineIdx] += cov;
              }
            }
          }
        actualSourceFile = "";
        }
      if ( !sourceFile.empty() && actualSourceFile.empty() )
        {
        gcovFile = "";
        // Is it in the source dir?
        if ( sourceFile.size() > sourceDir.size() &&
          sourceFile.substr(0, sourceDir.size()) == sourceDir &&
          sourceFile[sourceDir.size()] == '/' )
          {
          cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "   produced s: " << sourceFile.c_str() << std::endl);
          ofs << "  produced in source dir: " << sourceFile.c_str() << std::endl;
          actualSourceFile = cmSystemTools::CollapseFullPath(sourceFile.c_str());
          }
        // Binary dir?
        if ( sourceFile.size() > binaryDir.size() &&
          sourceFile.substr(0, binaryDir.size()) == binaryDir &&
          sourceFile[binaryDir.size()] == '/' )
          {
          cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "   produced b: " << sourceFile.c_str() << std::endl);
          ofs << "  produced in binary dir: " << sourceFile.c_str() << std::endl;
          actualSourceFile = cmSystemTools::CollapseFullPath(sourceFile.c_str());
          }
        if ( actualSourceFile.empty() )
          {
          if ( missingFiles.find(actualSourceFile) == missingFiles.end() )
            {
            cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Something went wrong" << std::endl);
            cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "File: [" << sourceFile.c_str() << "]" << std::endl);
            cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "s: [" << sourceFile.substr(0, sourceDir.size()) << "]" << std::endl);
            cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "b: [" << sourceFile.substr(0, binaryDir.size()) << "]" << std::endl);
            ofs << "  Something went wrong. Cannot find: " << sourceFile.c_str()
              << " in source dir: " << sourceDir.c_str()
              << " or binary dir: " << binaryDir.c_str() << std::endl;
            missingFiles.insert(actualSourceFile);
            }
          }
        }
      }
    file_count ++;
    if ( file_count % 50 == 0 )
      {
      cmCTestLog(m_CTest, HANDLER_OUTPUT, " processed: " << file_count << " out of " << files.size() << std::endl);
      cmCTestLog(m_CTest, HANDLER_OUTPUT, "    ");
      }
    }
 
  cmGeneratedFileStream covSumFile;
  cmGeneratedFileStream covLogFile;

  if (!this->StartResultingXML("Coverage", covSumFile))
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot open coverage summary file." << std::endl);

    return -1;
    }

  m_CTest->StartXML(covSumFile);
  // Produce output xml files

  covSumFile << "<Coverage>" << std::endl
    << "\t<StartDateTime>" << coverage_start_time << "</StartDateTime>" << std::endl;
  int logFileCount = 0;
  if ( !this->StartCoverageLogFile(covLogFile, logFileCount) )
    {
    return -1;
    }
  totalCoverageMap::iterator fileIterator;
  int cnt = 0;
  long total_tested = 0;
  long total_untested = 0;
  //std::string fullSourceDir = sourceDir + "/";
  //std::string fullBinaryDir = binaryDir + "/";
  cmCTestLog(m_CTest, HANDLER_OUTPUT, std::endl);
  cmCTestLog(m_CTest, HANDLER_OUTPUT, "   Acumulating results (each . represents one file):" << std::endl);
  cmCTestLog(m_CTest, HANDLER_OUTPUT, "    ");

  std::vector<std::string> errorsWhileAccumulating;

  file_count = 0;
  for ( fileIterator = totalCoverage.begin();
    fileIterator != totalCoverage.end();
    ++fileIterator )
    {
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "." << std::flush);
    file_count ++;
    if ( file_count % 50 == 0 )
      {
      cmCTestLog(m_CTest, HANDLER_OUTPUT, " processed: " << file_count << " out of "
        << totalCoverage.size() << std::endl);
      cmCTestLog(m_CTest, HANDLER_OUTPUT, "    ");
      }
    if ( cnt % 100 == 0 )
      {
      this->EndCoverageLogFile(covLogFile, logFileCount);
      logFileCount ++;
      if ( !this->StartCoverageLogFile(covLogFile, logFileCount) )
        {
        return -1;
        }
      }
    const std::string fullFileName = fileIterator->first;
    const std::string fileName = cmSystemTools::GetFilenameName(fullFileName.c_str());
    std::string fullFilePath = cmSystemTools::GetFilenamePath(fullFileName.c_str());
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Process file: " << fullFileName << std::endl);

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
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, ".NoDartCoverage found, so skip coverage check for: "
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
      cmOStringStream ostr;
      ostr <<  "Cannot open source file: " << fullFileName.c_str();
      errorsWhileAccumulating.push_back(ostr.str());
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
        cmOStringStream ostr;
        ostr << "Problem reading source file: " << fullFileName.c_str() << " line:" << cc;
        errorsWhileAccumulating.push_back(ostr.str());
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
      cmOStringStream ostr;
      ostr <<  "Looks like there are more lines in the file: " << line;
      errorsWhileAccumulating.push_back(ostr.str());
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
  this->EndCoverageLogFile(covLogFile, logFileCount);

  if ( errorsWhileAccumulating.size() > 0 )
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, std::endl);
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Error(s) while acumulating results:" << std::endl);
    std::vector<std::string>::iterator erIt;
    for ( erIt = errorsWhileAccumulating.begin();
      erIt != errorsWhileAccumulating.end();
      ++ erIt )
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "  " << erIt->c_str() << std::endl);
      }
    }

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

//----------------------------------------------------------------------
void cmCTestCoverageHandler::PopulateCustomVectors(cmMakefile *mf)
{
  cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, " Add coverage exclude regular expressions." << std::endl);
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_COVERAGE_EXCLUDE", 
                                m_CustomCoverageExclude);
  std::vector<cmStdString>::iterator it;
  for ( it = m_CustomCoverageExclude.begin();
    it != m_CustomCoverageExclude.end();
    ++ it )
    {
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, " Add coverage exclude: " << it->c_str() << std::endl);
    }
}
