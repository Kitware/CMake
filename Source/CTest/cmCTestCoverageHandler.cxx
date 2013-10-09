/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestCoverageHandler.h"
#include "cmParsePHPCoverage.h"
#include "cmParseGTMCoverage.h"
#include "cmParseCacheCoverage.h"
#include "cmCTest.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmGeneratedFileStream.h"
#include "cmXMLSafe.h"

#include <cmsys/Process.h>
#include <cmsys/RegularExpression.hxx>
#include <cmsys/Glob.hxx>
#include <cmsys/stl/iterator>
#include <cmsys/stl/algorithm>

#include <stdlib.h>
#include <math.h>
#include <float.h>

#define SAFEDIV(x,y) (((y)!=0)?((x)/(y)):(0))

class cmCTestRunProcess
{
public:
  cmCTestRunProcess()
    {
      this->Process = cmsysProcess_New();
      this->PipeState = -1;
      this->TimeOut = -1;
    }
  ~cmCTestRunProcess()
    {
      if(!(this->PipeState == -1)
         && !(this->PipeState == cmsysProcess_Pipe_None )
         && !(this->PipeState == cmsysProcess_Pipe_Timeout))
        {
        this->WaitForExit();
        }
      cmsysProcess_Delete(this->Process);
    }
  void SetCommand(const char* command)
    {
      this->CommandLineStrings.clear();
      this->CommandLineStrings.push_back(command);;
    }
  void AddArgument(const char* arg)
    {
      if(arg)
        {
        this->CommandLineStrings.push_back(arg);
        }
    }
  void SetWorkingDirectory(const char* dir)
    {
      this->WorkingDirectory = dir;
    }
  void SetTimeout(double t)
    {
      this->TimeOut = t;
    }
  bool StartProcess()
    {
      std::vector<const char*> args;
      for(std::vector<std::string>::iterator i =
            this->CommandLineStrings.begin();
          i != this->CommandLineStrings.end(); ++i)
        {
        args.push_back(i->c_str());
        }
      args.push_back(0); // null terminate
      cmsysProcess_SetCommand(this->Process, &*args.begin());
      if(this->WorkingDirectory.size())
        {
        cmsysProcess_SetWorkingDirectory(this->Process,
                                         this->WorkingDirectory.c_str());
        }

      cmsysProcess_SetOption(this->Process,
                             cmsysProcess_Option_HideWindow, 1);
      if(this->TimeOut != -1)
        {
        cmsysProcess_SetTimeout(this->Process, this->TimeOut);
        }
      cmsysProcess_Execute(this->Process);
      this->PipeState = cmsysProcess_GetState(this->Process);
      // if the process is running or exited return true
      if(this->PipeState == cmsysProcess_State_Executing
         || this->PipeState == cmsysProcess_State_Exited)
        {
        return true;
        }
      return false;
    }
  void SetStdoutFile(const char* fname)
    {
    cmsysProcess_SetPipeFile(this->Process, cmsysProcess_Pipe_STDOUT, fname);
    }
  void SetStderrFile(const char* fname)
    {
    cmsysProcess_SetPipeFile(this->Process, cmsysProcess_Pipe_STDERR, fname);
    }
  int WaitForExit(double* timeout =0)
    {
      this->PipeState = cmsysProcess_WaitForExit(this->Process,
                                                 timeout);
      return this->PipeState;
    }
  int GetProcessState() { return this->PipeState;}
private:
  int PipeState;
  cmsysProcess* Process;
  std::vector<std::string> CommandLineStrings;
  std::string WorkingDirectory;
  double TimeOut;
};


//----------------------------------------------------------------------

//----------------------------------------------------------------------
cmCTestCoverageHandler::cmCTestCoverageHandler()
{
}

//----------------------------------------------------------------------
void cmCTestCoverageHandler::Initialize()
{
  this->Superclass::Initialize();
  this->CustomCoverageExclude.clear();
  this->SourceLabels.clear();
  this->LabelIdMap.clear();
  this->Labels.clear();
  this->LabelFilter.clear();
}

//----------------------------------------------------------------------------
void cmCTestCoverageHandler::CleanCoverageLogFiles(std::ostream& log)
{
  std::string logGlob = this->CTest->GetCTestConfiguration("BuildDirectory");
  logGlob += "/Testing/";
  logGlob += this->CTest->GetCurrentTag();
  logGlob += "/CoverageLog*";
  cmsys::Glob gl;
  gl.FindFiles(logGlob.c_str());
  std::vector<std::string> const& files = gl.GetFiles();
  for(std::vector<std::string>::const_iterator fi = files.begin();
      fi != files.end(); ++fi)
    {
    log << "Removing old coverage log: " << *fi << "\n";
    cmSystemTools::RemoveFile(fi->c_str());
    }
}

//----------------------------------------------------------------------
bool cmCTestCoverageHandler::StartCoverageLogFile(
  cmGeneratedFileStream& covLogFile, int logFileCount)
{
  char covLogFilename[1024];
  sprintf(covLogFilename, "CoverageLog-%d", logFileCount);
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Open file: "
    << covLogFilename << std::endl);
  if(!this->StartResultingXML(cmCTest::PartCoverage,
                              covLogFilename, covLogFile))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Cannot open log file: "
      << covLogFilename << std::endl);
    return false;
    }
  std::string local_start_time = this->CTest->CurrentTime();
  this->CTest->StartXML(covLogFile, this->AppendXML);
  covLogFile << "<CoverageLog>" << std::endl
             << "\t<StartDateTime>" << local_start_time << "</StartDateTime>"
             << "\t<StartTime>"
             << static_cast<unsigned int>(cmSystemTools::GetTime())
             << "</StartTime>"
    << std::endl;
  return true;
}

//----------------------------------------------------------------------
void cmCTestCoverageHandler::EndCoverageLogFile(cmGeneratedFileStream& ostr,
  int logFileCount)
{
  std::string local_end_time = this->CTest->CurrentTime();
  ostr << "\t<EndDateTime>" << local_end_time << "</EndDateTime>" << std::endl
       << "\t<EndTime>" <<
       static_cast<unsigned int>(cmSystemTools::GetTime())
       << "</EndTime>" << std::endl
    << "</CoverageLog>" << std::endl;
  this->CTest->EndXML(ostr);
  char covLogFilename[1024];
  sprintf(covLogFilename, "CoverageLog-%d.xml", logFileCount);
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Close file: "
    << covLogFilename << std::endl);
  ostr.Close();
}

//----------------------------------------------------------------------
bool cmCTestCoverageHandler::ShouldIDoCoverage(const char* file,
  const char* srcDir,
  const char* binDir)
{
  if(this->IsFilteredOut(file))
    {
    return false;
    }

  std::vector<cmsys::RegularExpression>::iterator sit;
  for ( sit = this->CustomCoverageExcludeRegex.begin();
    sit != this->CustomCoverageExcludeRegex.end(); ++ sit )
    {
    if ( sit->find(file) )
      {
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "  File " << file
        << " is excluded in CTestCustom.ctest" << std::endl;);
      return false;
      }
    }

  std::string fSrcDir = cmSystemTools::CollapseFullPath(srcDir);
  std::string fBinDir = cmSystemTools::CollapseFullPath(binDir);
  std::string fFile = cmSystemTools::CollapseFullPath(file);
  bool sourceSubDir = cmSystemTools::IsSubDirectory(fFile.c_str(),
    fSrcDir.c_str());
  bool buildSubDir = cmSystemTools::IsSubDirectory(fFile.c_str(),
    fBinDir.c_str());
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
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Found: " << ndc.c_str()
      << " so skip coverage of " << file << std::endl);
    return false;
    }

  // By now checkDir should be set to parent directory of the file.
  // Get the relative path to the file an apply it to the opposite directory.
  // If it is the same as fileDir, then ignore, otherwise check.
  std::string relPath;
  if(checkDir.size() )
    {
    relPath = cmSystemTools::RelativePath(checkDir.c_str(),
                                          fFile.c_str());
    }
  else
    {
    relPath = fFile;
    }
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
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Found: " << ndc.c_str()
      << " so skip coverage of: " << file << std::endl);
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
  this->CTest->ClearSubmitFiles(cmCTest::PartCoverage);
  int error = 0;
  // do we have time for this
  if (this->CTest->GetRemainingTimeAllowed() < 120)
    {
    return error;
    }

  std::string coverage_start_time = this->CTest->CurrentTime();
  unsigned int coverage_start_time_time = static_cast<unsigned int>(
    cmSystemTools::GetTime());
  std::string sourceDir
    = this->CTest->GetCTestConfiguration("SourceDirectory");
  std::string binaryDir
    = this->CTest->GetCTestConfiguration("BuildDirectory");

  this->LoadLabels();

  cmGeneratedFileStream ofs;
  double elapsed_time_start = cmSystemTools::GetTime();
  if ( !this->StartLogFile("Coverage", ofs) )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Cannot create LastCoverage.log file" << std::endl);
    }

  ofs << "Performing coverage: " << elapsed_time_start << std::endl;
  this->CleanCoverageLogFiles(ofs);

  cmSystemTools::ConvertToUnixSlashes(sourceDir);
  cmSystemTools::ConvertToUnixSlashes(binaryDir);

  cmCTestLog(this->CTest, HANDLER_OUTPUT, "Performing coverage" << std::endl);

  cmCTestCoverageHandlerContainer cont;
  cont.Error = error;
  cont.SourceDir = sourceDir;
  cont.BinaryDir = binaryDir;
  cont.OFS = &ofs;

  // setup the regex exclude stuff
  this->CustomCoverageExcludeRegex.clear();
  std::vector<cmStdString>::iterator rexIt;
  for ( rexIt = this->CustomCoverageExclude.begin();
    rexIt != this->CustomCoverageExclude.end();
    ++ rexIt )
    {
    this->CustomCoverageExcludeRegex.push_back(
      cmsys::RegularExpression(rexIt->c_str()));
    }

  if(this->HandleBullseyeCoverage(&cont))
    {
    return cont.Error;
    }
  int file_count = 0;
  file_count += this->HandleGCovCoverage(&cont);
  error = cont.Error;
  if ( file_count < 0 )
    {
    return error;
    }
  file_count += this->HandleLCovCoverage(&cont);
  error = cont.Error;
  if ( file_count < 0 )
    {
    return error;
    }
  file_count += this->HandleTracePyCoverage(&cont);
  error = cont.Error;
  if ( file_count < 0 )
    {
    return error;
    }
  file_count += this->HandlePHPCoverage(&cont);
  error = cont.Error;
  if ( file_count < 0 )
    {
    return error;
    }
  file_count += this->HandleMumpsCoverage(&cont);
  error = cont.Error;
  if ( file_count < 0 )
    {
    return error;
    }

  std::set<std::string> uncovered = this->FindUncoveredFiles(&cont);

  if ( file_count == 0 )
    {
    cmCTestLog(this->CTest, WARNING,
      " Cannot find any coverage files. Ignoring Coverage request."
      << std::endl);
    return error;
    }
  cmGeneratedFileStream covSumFile;
  cmGeneratedFileStream covLogFile;

  if(!this->StartResultingXML(cmCTest::PartCoverage, "Coverage", covSumFile))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Cannot open coverage summary file." << std::endl);
    return -1;
    }

  this->CTest->StartXML(covSumFile, this->AppendXML);
  // Produce output xml files

  covSumFile << "<Coverage>" << std::endl
    << "\t<StartDateTime>" << coverage_start_time << "</StartDateTime>"
    << std::endl
    << "\t<StartTime>" << coverage_start_time_time << "</StartTime>"
    << std::endl;
  int logFileCount = 0;
  if ( !this->StartCoverageLogFile(covLogFile, logFileCount) )
    {
    return -1;
    }
  cmCTestCoverageHandlerContainer::TotalCoverageMap::iterator fileIterator;
  int cnt = 0;
  long total_tested = 0;
  long total_untested = 0;
  //std::string fullSourceDir = sourceDir + "/";
  //std::string fullBinaryDir = binaryDir + "/";
  cmCTestLog(this->CTest, HANDLER_OUTPUT, std::endl);
  cmCTestLog(this->CTest, HANDLER_OUTPUT,
    "   Accumulating results (each . represents one file):" << std::endl);
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "    ");

  std::vector<std::string> errorsWhileAccumulating;

  file_count = 0;
  for ( fileIterator = cont.TotalCoverage.begin();
    fileIterator != cont.TotalCoverage.end();
    ++fileIterator )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "." << std::flush);
    file_count ++;
    if ( file_count % 50 == 0 )
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT, " processed: " << file_count
        << " out of "
        << cont.TotalCoverage.size() << std::endl);
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "    ");
      }

    const std::string fullFileName = fileIterator->first;
    bool shouldIDoCoverage
      = this->ShouldIDoCoverage(fullFileName.c_str(),
        sourceDir.c_str(), binaryDir.c_str());
    if ( !shouldIDoCoverage )
      {
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
        ".NoDartCoverage found, so skip coverage check for: "
        << fullFileName.c_str()
        << std::endl);
      continue;
      }

    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
      "Process file: " << fullFileName << std::endl);

    if ( !cmSystemTools::FileExists(fullFileName.c_str()) )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "Cannot find file: "
        << fullFileName.c_str() << std::endl);
      continue;
      }

    if ( ++cnt % 100 == 0 )
      {
      this->EndCoverageLogFile(covLogFile, logFileCount);
      logFileCount ++;
      if ( !this->StartCoverageLogFile(covLogFile, logFileCount) )
        {
        return -1;
        }
      }

    const std::string fileName
      = cmSystemTools::GetFilenameName(fullFileName.c_str());
    std::string shortFileName =
      this->CTest->GetShortPathToFile(fullFileName.c_str());
    const cmCTestCoverageHandlerContainer::SingleFileCoverageVector& fcov
      = fileIterator->second;
    covLogFile << "\t<File Name=\"" << cmXMLSafe(fileName)
      << "\" FullPath=\"" << cmXMLSafe(shortFileName) << "\">\n"
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

    cmCTestCoverageHandlerContainer::SingleFileCoverageVector::size_type cc;
    std::string line;
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
      "Actually performing coverage for: " << fullFileName << std::endl);
    for ( cc= 0; cc < fcov.size(); cc ++ )
      {
      if ( !cmSystemTools::GetLineFromStream(ifs, line) &&
        cc != fcov.size() -1 )
        {
        cmOStringStream ostr;
        ostr << "Problem reading source file: " << fullFileName.c_str()
             << " line:" << cc << "  out total: " << fcov.size()-1;
        errorsWhileAccumulating.push_back(ostr.str());
        error ++;
        break;
        }
      covLogFile << "\t\t<Line Number=\"" << cc << "\" Count=\"" << fcov[cc]
        << "\">"
        << cmXMLSafe(line) << "</Line>" << std::endl;
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
    covSumFile << "\t<File Name=\"" << cmXMLSafe(fileName)
      << "\" FullPath=\"" << cmXMLSafe(
        this->CTest->GetShortPathToFile(fullFileName.c_str()))
      << "\" Covered=\"" << (tested+untested > 0 ? "true":"false") << "\">\n"
      << "\t\t<LOCTested>" << tested << "</LOCTested>\n"
      << "\t\t<LOCUnTested>" << untested << "</LOCUnTested>\n"
      << "\t\t<PercentCoverage>";
    covSumFile.setf(std::ios::fixed, std::ios::floatfield);
    covSumFile.precision(2);
    covSumFile << (cper) << "</PercentCoverage>\n"
      << "\t\t<CoverageMetric>";
    covSumFile.setf(std::ios::fixed, std::ios::floatfield);
    covSumFile.precision(2);
    covSumFile << (cmet) << "</CoverageMetric>\n";
    this->WriteXMLLabels(covSumFile, shortFileName);
    covSumFile << "\t</File>" << std::endl;
    }

  //Handle all the files in the extra coverage globs that have no cov data
  for(std::set<std::string>::iterator i = uncovered.begin();
      i != uncovered.end(); ++i)
    {
    std::string fileName = cmSystemTools::GetFilenameName(*i);
    std::string fullPath = cont.SourceDir + "/" + *i;

    covLogFile << "\t<File Name=\"" << cmXMLSafe(fileName)
      << "\" FullPath=\"" << cmXMLSafe(*i) << "\">\n"
      << "\t\t<Report>" << std::endl;

    std::ifstream ifs(fullPath.c_str());
    if (!ifs)
      {
      cmOStringStream ostr;
      ostr <<  "Cannot open source file: " << fullPath.c_str();
      errorsWhileAccumulating.push_back(ostr.str());
      error ++;
      continue;
      }
    int untested = 0;
    std::string line;
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
      "Actually performing coverage for: " << i->c_str() << std::endl);
    while (cmSystemTools::GetLineFromStream(ifs, line))
      {
      covLogFile << "\t\t<Line Number=\"" << untested << "\" Count=\"0\">"
        << cmXMLSafe(line) << "</Line>" << std::endl;
      untested ++;
      }
    covLogFile << "\t\t</Report>\n\t</File>" << std::endl;

    total_untested += untested;
    covSumFile << "\t<File Name=\"" << cmXMLSafe(fileName)
      << "\" FullPath=\"" << cmXMLSafe(i->c_str())
      << "\" Covered=\"true\">\n"
      << "\t\t<LOCTested>0</LOCTested>\n"
      << "\t\t<LOCUnTested>" << untested << "</LOCUnTested>\n"
      << "\t\t<PercentCoverage>0</PercentCoverage>\n"
      << "\t\t<CoverageMetric>0</CoverageMetric>\n";
    this->WriteXMLLabels(covSumFile, *i);
    covSumFile << "\t</File>" << std::endl;
    }

  this->EndCoverageLogFile(covLogFile, logFileCount);

  if ( errorsWhileAccumulating.size() > 0 )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, std::endl);
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Error(s) while accumulating results:" << std::endl);
    std::vector<std::string>::iterator erIt;
    for ( erIt = errorsWhileAccumulating.begin();
      erIt != errorsWhileAccumulating.end();
      ++ erIt )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "  " << erIt->c_str() << std::endl);
      }
    }

  long total_lines = total_tested + total_untested;
  float percent_coverage = 100 * SAFEDIV(static_cast<float>(total_tested),
    static_cast<float>(total_lines));
  if ( total_lines == 0 )
    {
    percent_coverage = 0;
    }

  std::string end_time = this->CTest->CurrentTime();

  covSumFile << "\t<LOCTested>" << total_tested << "</LOCTested>\n"
    << "\t<LOCUntested>" << total_untested << "</LOCUntested>\n"
    << "\t<LOC>" << total_lines << "</LOC>\n"
    << "\t<PercentCoverage>";
  covSumFile.setf(std::ios::fixed, std::ios::floatfield);
  covSumFile.precision(2);
  covSumFile << (percent_coverage)<< "</PercentCoverage>\n"
    << "\t<EndDateTime>" << end_time << "</EndDateTime>\n"
    << "\t<EndTime>" <<
         static_cast<unsigned int>(cmSystemTools::GetTime())
    << "</EndTime>\n";
  covSumFile << "<ElapsedMinutes>" <<
    static_cast<int>((cmSystemTools::GetTime() - elapsed_time_start)/6)/10.0
    << "</ElapsedMinutes>"
    << "</Coverage>" << std::endl;
  this->CTest->EndXML(covSumFile);

  cmCTestLog(this->CTest, HANDLER_OUTPUT, "" << std::endl
    << "\tCovered LOC:         "
    << total_tested << std::endl
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


  if ( error )
    {
    return -1;
    }
  return 0;
}

//----------------------------------------------------------------------
void cmCTestCoverageHandler::PopulateCustomVectors(cmMakefile *mf)
{
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
    " Add coverage exclude regular expressions." << std::endl);
  this->CTest->PopulateCustomVector(mf, "CTEST_CUSTOM_COVERAGE_EXCLUDE",
                                this->CustomCoverageExclude);
  this->CTest->PopulateCustomVector(mf, "CTEST_EXTRA_COVERAGE_GLOB",
                                this->ExtraCoverageGlobs);
  std::vector<cmStdString>::iterator it;
  for ( it = this->CustomCoverageExclude.begin();
    it != this->CustomCoverageExclude.end();
    ++ it )
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, " Add coverage exclude: "
      << it->c_str() << std::endl);
    }
  for ( it = this->ExtraCoverageGlobs.begin();
    it != this->ExtraCoverageGlobs.end(); ++it)
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, " Add coverage glob: "
      << it->c_str() << std::endl);
    }
}

//----------------------------------------------------------------------
// Fix for issue #4971 where the case of the drive letter component of
// the filenames might be different when analyzing gcov output.
//
// Compare file names: fnc(fn1) == fnc(fn2) // fnc == file name compare
//
#ifdef _WIN32
#define fnc(s) cmSystemTools::LowerCase(s)
#else
#define fnc(s) s
#endif

//----------------------------------------------------------------------
bool IsFileInDir(const std::string &infile, const std::string &indir)
{
  std::string file = cmSystemTools::CollapseFullPath(infile.c_str());
  std::string dir = cmSystemTools::CollapseFullPath(indir.c_str());

  if (
    file.size() > dir.size() &&
    (fnc(file.substr(0, dir.size())) == fnc(dir)) &&
    file[dir.size()] == '/'
    )
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------
int cmCTestCoverageHandler::HandlePHPCoverage(
  cmCTestCoverageHandlerContainer* cont)
{
  cmParsePHPCoverage cov(*cont, this->CTest);
  std::string coverageDir = this->CTest->GetBinaryDir() + "/xdebugCoverage";
  if(cmSystemTools::FileIsDirectory(coverageDir.c_str()))
    {
    cov.ReadPHPCoverageDirectory(coverageDir.c_str());
    }
  return static_cast<int>(cont->TotalCoverage.size());
}
//----------------------------------------------------------------------
int cmCTestCoverageHandler::HandleMumpsCoverage(
  cmCTestCoverageHandlerContainer* cont)
{
  // try gtm coverage
  cmParseGTMCoverage cov(*cont, this->CTest);
  std::string coverageFile = this->CTest->GetBinaryDir() +
    "/gtm_coverage.mcov";
  if(cmSystemTools::FileExists(coverageFile.c_str()))
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "Parsing Cache Coverage: " << coverageFile
               << std::endl);
    cov.ReadCoverageFile(coverageFile.c_str());
    return static_cast<int>(cont->TotalCoverage.size());
    }
  else
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               " Cannot find foobar GTM coverage file: " << coverageFile
               << std::endl);
    }
  cmParseCacheCoverage ccov(*cont, this->CTest);
  coverageFile = this->CTest->GetBinaryDir() +
    "/cache_coverage.cmcov";
  if(cmSystemTools::FileExists(coverageFile.c_str()))
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "Parsing Cache Coverage: " << coverageFile
               << std::endl);
    ccov.ReadCoverageFile(coverageFile.c_str());
    }
  else
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               " Cannot find Cache coverage file: " << coverageFile
               << std::endl);
    }
  return static_cast<int>(cont->TotalCoverage.size());
}

struct cmCTestCoverageHandlerLocale
{
  cmCTestCoverageHandlerLocale()
    {
    if(const char* l = cmSystemTools::GetEnv("LC_ALL"))
      {
      lc_all = l;
      }
    if(lc_all != "C")
      {
      cmSystemTools::PutEnv("LC_ALL=C");
      }
    }
  ~cmCTestCoverageHandlerLocale()
    {
    if(!lc_all.empty())
      {
      cmSystemTools::PutEnv(("LC_ALL=" + lc_all).c_str());
      }
    else
      {
      cmSystemTools::UnsetEnv("LC_ALL");
      }
    }
  std::string lc_all;
};

//----------------------------------------------------------------------
int cmCTestCoverageHandler::HandleGCovCoverage(
  cmCTestCoverageHandlerContainer* cont)
{
  std::string gcovCommand
    = this->CTest->GetCTestConfiguration("CoverageCommand");
  std::string gcovExtraFlags
    = this->CTest->GetCTestConfiguration("CoverageExtraFlags");

  // Style 1
  std::string st1gcovOutputRex1
    = "[0-9]+\\.[0-9]+% of [0-9]+ (source |)lines executed in file (.*)$";
  std::string st1gcovOutputRex2 = "^Creating (.*\\.gcov)\\.";
  cmsys::RegularExpression st1re1(st1gcovOutputRex1.c_str());
  cmsys::RegularExpression st1re2(st1gcovOutputRex2.c_str());


  // Style 2
  std::string st2gcovOutputRex1 = "^File *[`'](.*)'$";
  std::string st2gcovOutputRex2
    = "Lines executed: *[0-9]+\\.[0-9]+% of [0-9]+$";
  std::string st2gcovOutputRex3 = "^(.*)reating [`'](.*\\.gcov)'";
  std::string st2gcovOutputRex4 = "^(.*):unexpected EOF *$";
  std::string st2gcovOutputRex5 = "^(.*):cannot open source file*$";
  std::string st2gcovOutputRex6
    = "^(.*):source file is newer than graph file `(.*)'$";
  cmsys::RegularExpression st2re1(st2gcovOutputRex1.c_str());
  cmsys::RegularExpression st2re2(st2gcovOutputRex2.c_str());
  cmsys::RegularExpression st2re3(st2gcovOutputRex3.c_str());
  cmsys::RegularExpression st2re4(st2gcovOutputRex4.c_str());
  cmsys::RegularExpression st2re5(st2gcovOutputRex5.c_str());
  cmsys::RegularExpression st2re6(st2gcovOutputRex6.c_str());

  std::vector<std::string> files;
  this->FindGCovFiles(files);
  std::vector<std::string>::iterator it;

  if ( files.size() == 0 )
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
      " Cannot find any GCov coverage files."
      << std::endl);
    // No coverage files is a valid thing, so the exit code is 0
    return 0;
    }

  std::string testingDir = this->CTest->GetBinaryDir() + "/Testing";
  std::string tempDir = testingDir + "/CoverageInfo";
  std::string currentDirectory = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::MakeDirectory(tempDir.c_str());
  cmSystemTools::ChangeDirectory(tempDir.c_str());

  int gcovStyle = 0;

  std::set<std::string> missingFiles;

  std::string actualSourceFile = "";
  cmCTestLog(this->CTest, HANDLER_OUTPUT,
    "   Processing coverage (each . represents one file):" << std::endl);
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "    ");
  int file_count = 0;

  // make sure output from gcov is in English!
  cmCTestCoverageHandlerLocale locale_C;
  static_cast<void>(locale_C);

  // files is a list of *.da and *.gcda files with coverage data in them.
  // These are binary files that you give as input to gcov so that it will
  // give us text output we can analyze to summarize coverage.
  //
  if ( gcovCommand != "codecov" )
    {
    for ( it = files.begin(); it != files.end(); ++ it )
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "." << std::flush);

      // Call gcov to get coverage data for this *.gcda file:
      //
      std::string fileDir = cmSystemTools::GetFilenamePath(it->c_str());
      std::string command = "";

        command = "\"" + gcovCommand + "\" " +
          gcovExtraFlags + " " +
          "-o \"" + fileDir + "\" " +
          "\"" + *it + "\"";
      

      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, command.c_str()
        << std::endl);

      std::string output = "";
      std::string errors = "";
      int retVal = 0;
      *cont->OFS << "* Run coverage for: " << fileDir.c_str() << std::endl;
      *cont->OFS << "  Command: " << command.c_str() << std::endl;
      int res = this->CTest->RunCommand(command.c_str(), &output, &errors,
        &retVal, tempDir.c_str(), 0 /*this->TimeOut*/);

      *cont->OFS << "  Output: " << output.c_str() << std::endl;
      *cont->OFS << "  Errors: " << errors.c_str() << std::endl;
      if ( ! res )
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
          "Problem running coverage on file: " << it->c_str() << std::endl);
        cmCTestLog(this->CTest, ERROR_MESSAGE,
          "Command produced error: " << errors << std::endl);
        cont->Error ++;
        continue;
        }
      if ( retVal != 0 )
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE, "Coverage command returned: "
          << retVal << " while processing: " << it->c_str() << std::endl);
        cmCTestLog(this->CTest, ERROR_MESSAGE,
          "Command produced error: " << cont->Error << std::endl);
        }
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
        "--------------------------------------------------------------"
        << std::endl
        << output << std::endl
        << "--------------------------------------------------------------"
        << std::endl);

      std::vector<cmStdString> lines;
      std::vector<cmStdString>::iterator line;

      cmSystemTools::Split(output.c_str(), lines);

      for ( line = lines.begin(); line != lines.end(); ++line)
        {
        std::string sourceFile;
        std::string gcovFile;

        cmCTestLog(this->CTest, DEBUG, "Line: [" << line->c_str() << "]"
          << std::endl);

        if ( line->size() == 0 )
          {
          // Ignore empty line; probably style 2
          }
        else if ( st1re1.find(line->c_str()) )
          {
          if ( gcovStyle == 0 )
            {
            gcovStyle = 1;
            }
          if ( gcovStyle != 1 )
            {
            cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e1"
              << std::endl);
            cont->Error ++;
            break;
            }

          actualSourceFile = "";
          sourceFile = st1re1.match(2);
          }
        else if ( st1re2.find(line->c_str() ) )
          {
          if ( gcovStyle == 0 )
            {
            gcovStyle = 1;
            }
          if ( gcovStyle != 1 )
            {
            cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e2"
              << std::endl);
            cont->Error ++;
            break;
            }

          gcovFile = st1re2.match(1);
          }
        else if ( st2re1.find(line->c_str() ) )
          {
          if ( gcovStyle == 0 )
            {
            gcovStyle = 2;
            }
          if ( gcovStyle != 2 )
            {
            cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e3"
              << std::endl);
            cont->Error ++;
            break;
            }

          actualSourceFile = "";
          sourceFile = st2re1.match(1);
          }
        else if ( st2re2.find(line->c_str() ) )
          {
          if ( gcovStyle == 0 )
            {
            gcovStyle = 2;
            }
          if ( gcovStyle != 2 )
            {
            cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e4"
              << std::endl);
            cont->Error ++;
            break;
            }
          }
        else if ( st2re3.find(line->c_str() ) )
          {
          if ( gcovStyle == 0 )
            {
            gcovStyle = 2;
            }
          if ( gcovStyle != 2 )
            {
            cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e5"
              << std::endl);
            cont->Error ++;
            break;
            }

          gcovFile = st2re3.match(2);
          }
        else if ( st2re4.find(line->c_str() ) )
          {
          if ( gcovStyle == 0 )
            {
            gcovStyle = 2;
            }
          if ( gcovStyle != 2 )
            {
            cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e6"
              << std::endl);
            cont->Error ++;
            break;
            }

          cmCTestLog(this->CTest, WARNING, "Warning: " << st2re4.match(1)
            << " had unexpected EOF" << std::endl);
          }
        else if ( st2re5.find(line->c_str() ) )
          {
          if ( gcovStyle == 0 )
            {
            gcovStyle = 2;
            }
          if ( gcovStyle != 2 )
            {
            cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e7"
              << std::endl);
            cont->Error ++;
            break;
            }

          cmCTestLog(this->CTest, WARNING, "Warning: Cannot open file: "
            << st2re5.match(1) << std::endl);
          }
        else if ( st2re6.find(line->c_str() ) )
          {
          if ( gcovStyle == 0 )
            {
            gcovStyle = 2;
            }
          if ( gcovStyle != 2 )
            {
            cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e8"
              << std::endl);
            cont->Error ++;
            break;
            }

          cmCTestLog(this->CTest, WARNING, "Warning: File: " << st2re6.match(1)
            << " is newer than " << st2re6.match(2) << std::endl);
          }
        else
          {
          // gcov 4.7 can have output lines saying "No executable lines" and
          // "Removing 'filename.gcov'"... Don't log those as "errors."
          if(*line != "No executable lines" &&
            !cmSystemTools::StringStartsWith(line->c_str(), "Removing "))
            {
            cmCTestLog(this->CTest, ERROR_MESSAGE,
              "Unknown gcov output line: [" << line->c_str() << "]"
              << std::endl);
            cont->Error ++;
            //abort();
            }
          }


        // If the last line of gcov output gave us a valid value for gcovFile,
        // and we have an actualSourceFile, then insert a (or add to existing)
        // SingleFileCoverageVector for actualSourceFile:
        //
        if ( !gcovFile.empty() && !actualSourceFile.empty() )
          {
          cmCTestCoverageHandlerContainer::SingleFileCoverageVector& vec
            = cont->TotalCoverage[actualSourceFile];

          cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "   in gcovFile: "
            << gcovFile << std::endl);

          std::ifstream ifile(gcovFile.c_str());
          if ( ! ifile )
            {
            cmCTestLog(this->CTest, ERROR_MESSAGE, "Cannot open file: "
              << gcovFile << std::endl);
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

              // Read the coverage count from the beginning of the gcov output
              // line
              std::string prefix = nl.substr(0, 12);
              int cov = atoi(prefix.c_str());

              // Read the line number starting at the 10th character of the gcov
              // output line
              std::string lineNumber = nl.substr(10, 5);

              int lineIdx = atoi(lineNumber.c_str())-1;
              if ( lineIdx >= 0 )
                {
                while ( vec.size() <= static_cast<size_t>(lineIdx) )
                  {
                  vec.push_back(-1);
                  }

                // Initially all entries are -1 (not used). If we get coverage
                // information, increment it to 0 first.
                if ( vec[lineIdx] < 0 )
                  {
                  if ( cov > 0 || prefix.find("#") != prefix.npos )
                    {
                    vec[lineIdx] = 0;
                    }
                  }

                vec[lineIdx] += cov;
                }
              }
            }

          actualSourceFile = "";
          }


        if ( !sourceFile.empty() && actualSourceFile.empty() )
          {
          gcovFile = "";

          // Is it in the source dir or the binary dir?
          //
          if ( IsFileInDir(sourceFile, cont->SourceDir) )
            {
            cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "   produced s: "
              << sourceFile.c_str() << std::endl);
            *cont->OFS << "  produced in source dir: " << sourceFile.c_str()
              << std::endl;
            actualSourceFile
              = cmSystemTools::CollapseFullPath(sourceFile.c_str());
            }
          else if ( IsFileInDir(sourceFile, cont->BinaryDir) )
            {
            cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "   produced b: "
              << sourceFile.c_str() << std::endl);
            *cont->OFS << "  produced in binary dir: " << sourceFile.c_str()
              << std::endl;
            actualSourceFile
              = cmSystemTools::CollapseFullPath(sourceFile.c_str());
            }

          if ( actualSourceFile.empty() )
            {
            if ( missingFiles.find(sourceFile) == missingFiles.end() )
              {
              cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                "Something went wrong" << std::endl);
              cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                "Cannot find file: ["
                << sourceFile.c_str() << "]" << std::endl);
              cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                " in source dir: ["
                << cont->SourceDir.c_str() << "]"
                << std::endl);
              cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                " or binary dir: ["
                << cont->BinaryDir.size() << "]"
                << std::endl);
              *cont->OFS << "  Something went wrong. Cannot find file: "
                << sourceFile.c_str()
                << " in source dir: " << cont->SourceDir.c_str()
                << " or binary dir: " << cont->BinaryDir.c_str() << std::endl;

              missingFiles.insert(sourceFile);
              }
            }
          }
        }

      file_count++;

      if ( file_count % 50 == 0 )
        {
        cmCTestLog(this->CTest, HANDLER_OUTPUT, " processed: " << file_count
          << " out of " << files.size() << std::endl);
        cmCTestLog(this->CTest, HANDLER_OUTPUT, "    ");
        }
      }
    }
  cmSystemTools::ChangeDirectory(currentDirectory.c_str());
  return file_count;
}

//----------------------------------------------------------------------
int cmCTestCoverageHandler::HandleLCovCoverage(
  cmCTestCoverageHandlerContainer* cont)
{
  std::string lcovCommand
    = this->CTest->GetCTestConfiguration("CoverageCommand");
  std::string lcovExtraFlags
    = this->CTest->GetCTestConfiguration("CoverageExtraFlags");

  // Style 1
  std::string st1gcovOutputRex1
    = "[0-9]+\\.[0-9]+% of [0-9]+ (source |)lines executed in file (.*)$";
  std::string st1gcovOutputRex2 = "^Creating (.*\\.lcov)\\.";
  cmsys::RegularExpression st1re1(st1gcovOutputRex1.c_str());
  cmsys::RegularExpression st1re2(st1gcovOutputRex2.c_str());


  // Style 2
  std::string st2gcovOutputRex1 = "a[0-9]+%";
  std::string st2gcovOutputRex2
    = "a[0-9]+%";
  std::string st2gcovOutputRex3 = "[0-9]+%";
  std::string st2gcovOutputRex4 = "ERROR";
  std::string st2gcovOutputRex5 = "Warning";
  std::string st2gcovOutputRex6
    = "Aborted";
  cmsys::RegularExpression st2re1(st2gcovOutputRex1.c_str());
  cmsys::RegularExpression st2re2(st2gcovOutputRex2.c_str());
  cmsys::RegularExpression st2re3(st2gcovOutputRex3.c_str());
  cmsys::RegularExpression st2re4(st2gcovOutputRex4.c_str());
  cmsys::RegularExpression st2re5(st2gcovOutputRex5.c_str());
  cmsys::RegularExpression st2re6(st2gcovOutputRex6.c_str());

  std::vector<std::string> files;
  this->FindLCovFiles(files);
  std::vector<std::string>::iterator it;

  if ( files.size() == 0 )
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
      " Cannot find any LCov coverage files."
      << std::endl);
    // No coverage files is a valid thing, so the exit code is 0
    return 0;
    }

  std::string testingDir = this->CTest->GetBinaryDir();// + "/CodeCoverage";
  std::string tempDir = testingDir;// + "/CoverageInfo";
  std::string currentDirectory = cmSystemTools::GetCurrentWorkingDirectory();
//   cmSystemTools::MakeDirectory(tempDir.c_str());
//   cmSystemTools::ChangeDirectory(tempDir.c_str());

  int gcovStyle = 0;

  std::set<std::string> missingFiles;

  std::string actualSourceFile = "";
  cmCTestLog(this->CTest, HANDLER_OUTPUT,
    "   Processing coverage (each . represents one file):" << std::endl);
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "    ");
  int file_count = 0;

  // make sure output from gcov is in English!
  cmCTestCoverageHandlerLocale locale_C;
  static_cast<void>(locale_C);

  // files is a list of *.da and *.gcda files with coverage data in them.
  // These are binary files that you give as input to gcov so that it will
  // give us text output we can analyze to summarize coverage.
  /// IN intel compiler we have to call codecov only once. It collects all dyn files to generate lcov file. So files.size = 1
  //
  for ( it = files.begin(); it != files.end(); ++ it )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "." << std::flush);

    // Call lcov to get coverage data for this *.gcda file:
    //
    cmSystemTools::ChangeDirectory(testingDir.c_str());
    std::string fileDir = cmSystemTools::GetFilenamePath(it->c_str());
    std::string command = "\"" + lcovCommand + "\" " +
      lcovExtraFlags + " ";

    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, command.c_str()
      << std::endl);

    std::string output = "";
    std::string errors = "";
    int retVal = 0;
    *cont->OFS << "* Run coverage for: " << fileDir.c_str() << std::endl;
    *cont->OFS << "  Command: " << command.c_str() << std::endl;
    int res = this->CTest->RunCommand(command.c_str(), &output, &errors,
      &retVal, tempDir.c_str(), 0 /*this->TimeOut*/);

    *cont->OFS << "  Output: " << output.c_str() << std::endl;
    *cont->OFS << "  Errors: " << errors.c_str() << std::endl;
    if ( ! res )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "Problem running coverage on file: " << it->c_str() << std::endl);
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "Command produced error: " << errors << std::endl);
      cont->Error ++;
      continue;
      }
    if ( retVal != 0 )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "Coverage command returned: "
        << retVal << " while processing: " << it->c_str() << std::endl);
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "Command produced error: " << cont->Error << std::endl);
      }
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
      "--------------------------------------------------------------"
      << std::endl
      << output << std::endl
      << "--------------------------------------------------------------"
      << std::endl);

    std::vector<cmStdString> lines;
    std::vector<cmStdString>::iterator line;

    cmSystemTools::Split(output.c_str(), lines);

    for ( line = lines.begin(); line != lines.end(); ++line)
      {
      std::string sourceFile;
      std::string gcovFile;

      cmCTestLog(this->CTest, DEBUG, "Line: [" << line->c_str() << "]"
        << std::endl);

      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "started" << st1re1.find(line->c_str()) << " finished " << std::endl);
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "started" << st1re2.find(line->c_str()) << " finished " << std::endl);
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "started" << st2re1.find(line->c_str()) << " finished " << std::endl);
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "started" << st2re2.find(line->c_str()) << " finished " << std::endl);
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "started" << st2re3.find(line->c_str()) << " finished " << std::endl);
    //cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "nmatch" << st2re3.match(2) << " finished " << std::endl);
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "started" << st2re4.find(line->c_str()) << " finished " << std::endl);
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "started" << st2re5.find(line->c_str()) << " finished " << std::endl);
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "started" << st2re6.find(line->c_str()) << " finished " << std::endl);
    
    
      if ( line->size() == 0 )
        {
        // Ignore empty line; probably style 2
        }
      else if ( st1re1.find(line->c_str()) )
        {
        if ( gcovStyle == 0 )
          {
          gcovStyle = 1;
          }
        if ( gcovStyle != 1 )
          {
          cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e1"
            << std::endl);
          cont->Error ++;
          break;
          }

        actualSourceFile = "";
        sourceFile = st1re1.match(2);
        }
      else if ( st1re2.find(line->c_str() ) )
        {
        if ( gcovStyle == 0 )
          {
          gcovStyle = 1;
          }
        if ( gcovStyle != 1 )
          {
          cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e2"
            << std::endl);
          cont->Error ++;
          break;
          }

        gcovFile = st1re2.match(1);
        }
      else if ( st2re1.find(line->c_str() ) )
        {
        if ( gcovStyle == 0 )
          {
          gcovStyle = 2;
          }
        if ( gcovStyle != 2 )
          {
          cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e3"
            << std::endl);
          cont->Error ++;
          break;
          }

        actualSourceFile = "";
        sourceFile = st2re1.match(1);
        }
      else if ( st2re2.find(line->c_str() ) )
        {
        if ( gcovStyle == 0 )
          {
          gcovStyle = 2;
          }
        if ( gcovStyle != 2 )
          {
          cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e4"
            << std::endl);
          cont->Error ++;
          break;
          }
        }
      else if ( st2re3.find(line->c_str() ) )
        {
        if ( gcovStyle == 0 )
          {
          gcovStyle = 2;
          }
        if ( gcovStyle != 2 )
          {
          cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e5"
            << std::endl);
          cont->Error ++;
          break;
          }

        gcovFile = st2re3.match(0); // was 2
        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Gcov File: " << gcovFile
            << std::endl);
        }
      else if ( st2re4.find(line->c_str() ) )
        {
        if ( gcovStyle == 0 )
          {
          gcovStyle = 2;
          }
        if ( gcovStyle != 2 )
          {
          cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e6"
            << std::endl);
          cont->Error ++;
          break;
          }

        cmCTestLog(this->CTest, WARNING, "Warning: " << st2re4.match(1)
          << " had unexpected EOF" << std::endl);
        }
      else if ( st2re5.find(line->c_str() ) )
        {
        if ( gcovStyle == 0 )
          {
          gcovStyle = 2;
          }
        if ( gcovStyle != 2 )
          {
          cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e7"
            << std::endl);
          cont->Error ++;
          break;
          }

        cmCTestLog(this->CTest, WARNING, "Warning: Cannot open file: "
          << st2re5.match(1) << std::endl);
        }
      else if ( st2re6.find(line->c_str() ) )
        {
        if ( gcovStyle == 0 )
          {
          gcovStyle = 2;
          }
        if ( gcovStyle != 2 )
          {
          cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown gcov output style e8"
            << std::endl);
          cont->Error ++;
          break;
          }

        cmCTestLog(this->CTest, WARNING, "Warning: File: " << st2re6.match(1)
          << " is newer than " << st2re6.match(2) << std::endl);
        }
      else
        {
        // gcov 4.7 can have output lines saying "No executable lines" and
        // "Removing 'filename.gcov'"... Don't log those as "errors."
        if(*line != "No executable lines" &&
           !cmSystemTools::StringStartsWith(line->c_str(), "Removing "))
          {
          cmCTestLog(this->CTest, ERROR_MESSAGE,
            "Unknown lcov output line: [" << line->c_str() << "]"
            << std::endl);
          cont->Error ++;
          //abort();
          }
        }


      // If the last line of gcov output gave us a valid value for gcovFile,
      // and we have an actualSourceFile, then insert a (or add to existing)
      // SingleFileCoverageVector for actualSourceFile:
      //
      if ( !gcovFile.empty() && !actualSourceFile.empty() )
        {
        cmCTestCoverageHandlerContainer::SingleFileCoverageVector& vec
          = cont->TotalCoverage[actualSourceFile];

        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "   in lcovFile: "
          << gcovFile << std::endl);

        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "   im here: "
          << gcovFile << std::endl);

        std::ifstream ifile(gcovFile.c_str());
        if ( ! ifile )
          {
          cmCTestLog(this->CTest, ERROR_MESSAGE, "Cannot open file: "
            << gcovFile << std::endl);
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

            // Read the coverage count from the beginning of the gcov output
            // line
            std::string prefix = nl.substr(0, 12);
            int cov = atoi(prefix.c_str());

            // Read the line number starting at the 10th character of the gcov
            // output line
            std::string lineNumber = nl.substr(10, 5);

            int lineIdx = atoi(lineNumber.c_str())-1;
            if ( lineIdx >= 0 )
              {
              while ( vec.size() <= static_cast<size_t>(lineIdx) )
                {
                vec.push_back(-1);
                }

              // Initially all entries are -1 (not used). If we get coverage
              // information, increment it to 0 first.
              if ( vec[lineIdx] < 0 )
                {
                if ( cov > 0 || prefix.find("#") != prefix.npos )
                  {
                  vec[lineIdx] = 0;
                  }
                }

              vec[lineIdx] += cov;
              }
            }
          }

        actualSourceFile = "";
        }


      if ( !sourceFile.empty() && actualSourceFile.empty() )
        {
        gcovFile = "";

        // Is it in the source dir or the binary dir?
        //
        if ( IsFileInDir(sourceFile, cont->SourceDir) )
          {
          cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "   produced s: "
            << sourceFile.c_str() << std::endl);
          *cont->OFS << "  produced in source dir: " << sourceFile.c_str()
            << std::endl;
          actualSourceFile
            = cmSystemTools::CollapseFullPath(sourceFile.c_str());
          }
        else if ( IsFileInDir(sourceFile, cont->BinaryDir) )
          {
          cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "   produced b: "
            << sourceFile.c_str() << std::endl);
          *cont->OFS << "  produced in binary dir: " << sourceFile.c_str()
            << std::endl;
          actualSourceFile
            = cmSystemTools::CollapseFullPath(sourceFile.c_str());
          }

        if ( actualSourceFile.empty() )
          {
          if ( missingFiles.find(sourceFile) == missingFiles.end() )
            {
            cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
              "Something went wrong" << std::endl);
            cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
              "Cannot find file: ["
              << sourceFile.c_str() << "]" << std::endl);
            cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
              " in source dir: ["
              << cont->SourceDir.c_str() << "]"
              << std::endl);
            cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
              " or binary dir: ["
              << cont->BinaryDir.size() << "]"
              << std::endl);
            *cont->OFS << "  Something went wrong. Cannot find file: "
              << sourceFile.c_str()
              << " in source dir: " << cont->SourceDir.c_str()
              << " or binary dir: " << cont->BinaryDir.c_str() << std::endl;

            missingFiles.insert(sourceFile);
            }
          }
        }
      }

    file_count++;

    if ( file_count % 50 == 0 )
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT, " processed: " << file_count
        << " out of " << files.size() << std::endl);
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "    ");
      }
    }

  cmSystemTools::ChangeDirectory(currentDirectory.c_str());
  return file_count;
}

//----------------------------------------------------------------------------
void cmCTestCoverageHandler::FindGCovFiles(std::vector<std::string>& files)
{
  cmsys::Glob gl;
  gl.RecurseOn();
  gl.RecurseThroughSymlinksOff();

  for(LabelMapType::const_iterator lmi = this->TargetDirs.begin();
      lmi != this->TargetDirs.end(); ++lmi)
    {
    // Skip targets containing no interesting labels.
    if(!this->IntersectsFilter(lmi->second))
      {
      continue;
      }

    // Coverage files appear next to their object files in the target
    // support directory.
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "   globbing for coverage in: " << lmi->first << std::endl);
    std::string daGlob = lmi->first;
    daGlob += "/*.da";
    gl.FindFiles(daGlob);
    files.insert(files.end(), gl.GetFiles().begin(), gl.GetFiles().end());
    daGlob = lmi->first;
    daGlob += "/*.gcda";
    gl.FindFiles(daGlob);
    files.insert(files.end(), gl.GetFiles().begin(), gl.GetFiles().end());
    }
}

//----------------------------------------------------------------------------
void cmCTestCoverageHandler::FindLCovFiles(std::vector<std::string>& files)
{
  cmsys::Glob gl;
  gl.RecurseOn();
  gl.RecurseThroughSymlinksOff();
  std::string prevBinaryDir;
  for(LabelMapType::const_iterator lmi = this->TargetDirs.begin();
      lmi != this->TargetDirs.end(); lmi++)
    {
    if(prevBinaryDir.compare(this->CTest->GetBinaryDir()))
      {
      cmSystemTools::ChangeDirectory(this->CTest->GetBinaryDir().c_str());
      system("profmerge");
      prevBinaryDir = this->CTest->GetBinaryDir();


    //Skip targets containing no interesting labels.
      if(!this->IntersectsFilter(lmi->second))
        {
        continue;
        }

      // Coverage files appear next to their object files in the target
      // support directory.

      std::string daGlob;
      daGlob = this->CTest->GetBinaryDir();
      daGlob += "/*.dpi";
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                "   looking for dpi files in: " << daGlob << std::endl);
      gl.FindFiles(daGlob);
      files.insert(files.end(), gl.GetFiles().begin(), gl.GetFiles().end());
      }
   }
   cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                "im in this directory now: " << this->CTest->GetBinaryDir() << std::endl);
}

//----------------------------------------------------------------------
int cmCTestCoverageHandler::HandleTracePyCoverage(
  cmCTestCoverageHandlerContainer* cont)
{
  cmsys::Glob gl;
  gl.RecurseOn();
  gl.RecurseThroughSymlinksOff();
  std::string daGlob = cont->BinaryDir + "/*.cover";
  gl.FindFiles(daGlob);
  std::vector<std::string> files = gl.GetFiles();

  if ( files.size() == 0 )
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
      " Cannot find any Python Trace.py coverage files."
      << std::endl);
    // No coverage files is a valid thing, so the exit code is 0
    return 0;
    }

  std::string testingDir = this->CTest->GetBinaryDir() + "/Testing";
  std::string tempDir = testingDir + "/CoverageInfo";
  std::string currentDirectory = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::MakeDirectory(tempDir.c_str());
  cmSystemTools::ChangeDirectory(tempDir.c_str());

  cmSystemTools::ChangeDirectory(currentDirectory.c_str());

  std::vector<std::string>::iterator fileIt;
  int file_count = 0;
  for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
    {
    std::string fileName = this->FindFile(cont, *fileIt);
    if ( fileName.empty() )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "Cannot find source Python file corresponding to: "
        << fileIt->c_str() << std::endl);
      continue;
      }

    std::string actualSourceFile
      = cmSystemTools::CollapseFullPath(fileName.c_str());
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
      "   Check coverage for file: " << actualSourceFile.c_str()
      << std::endl);
    cmCTestCoverageHandlerContainer::SingleFileCoverageVector* vec
      = &cont->TotalCoverage[actualSourceFile];
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
      "   in file: " << fileIt->c_str() << std::endl);
    std::ifstream ifile(fileIt->c_str());
    if ( ! ifile )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "Cannot open file: "
        << fileIt->c_str() << std::endl);
      }
    else
      {
      long cnt = -1;
      std::string nl;
      while ( cmSystemTools::GetLineFromStream(ifile, nl) )
        {
        cnt ++;

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

        // Read the coverage count from the beginning of the Trace.py output
        // line
        std::string prefix = nl.substr(0, 6);
        if ( prefix[5] != ' ' && prefix[5] != ':' )
          {
          // This is a hack. We should really do something more elaborate
          prefix = nl.substr(0, 7);
          if ( prefix[6] != ' ' && prefix[6] != ':' )
            {
            prefix = nl.substr(0, 8);
            if ( prefix[7] != ' ' && prefix[7] != ':' )
              {
              cmCTestLog(this->CTest, ERROR_MESSAGE,
                "Currently the limit is maximum coverage of 999999"
                << std::endl);
              }
            }
          }
        int cov = atoi(prefix.c_str());
        if ( prefix[prefix.size()-1] != ':' )
          {
          // This line does not have ':' so no coverage here. That said,
          // Trace.py does not handle not covered lines versus comments etc.
          // So, this will be set to 0.
          cov = 0;
          }
        cmCTestLog(this->CTest, DEBUG, "Prefix: " << prefix.c_str()
          << " cov: " << cov
          << std::endl);
        // Read the line number starting at the 10th character of the gcov
        // output line
        long lineIdx = cnt;
        if ( lineIdx >= 0 )
          {
          while ( vec->size() <=
            static_cast<size_t>(lineIdx) )
            {
            vec->push_back(-1);
            }
          // Initially all entries are -1 (not used). If we get coverage
          // information, increment it to 0 first.
          if ( (*vec)[lineIdx] < 0 )
            {
            if ( cov >= 0 )
              {
              (*vec)[lineIdx] = 0;
              }
            }
          (*vec)[lineIdx] += cov;
          }
        }
      }
    ++ file_count;
    }
  cmSystemTools::ChangeDirectory(currentDirectory.c_str());
  return file_count;
}

//----------------------------------------------------------------------
std::string cmCTestCoverageHandler::FindFile(
  cmCTestCoverageHandlerContainer* cont,
  std::string fileName)
{
  std::string fileNameNoE
    = cmSystemTools::GetFilenameWithoutLastExtension(fileName);
  // First check in source and binary directory
  std::string fullName = cont->SourceDir + "/" + fileNameNoE + ".py";
  if ( cmSystemTools::FileExists(fullName.c_str()) )
    {
    return fullName;
    }
  fullName = cont->BinaryDir + "/" + fileNameNoE + ".py";
  if ( cmSystemTools::FileExists(fullName.c_str()) )
    {
    return fullName;
    }
  return "";
}

// This is a header put on each marked up source file
namespace
{
  const char* bullseyeHelp[] =
  {"    Coverage produced by bullseye covbr tool: ",
   "      www.bullseye.com/help/ref_covbr.html",
   "    * An arrow --> indicates incomplete coverage.",
   "    * An X indicates a function that was invoked, a switch label that ",
   "      was exercised, a try-block that finished, or an exception handler ",
   "      that was invoked.",
   "    * A T or F indicates a boolean decision that evaluated true or false,",
   "      respectively.",
   "    * A t or f indicates a boolean condition within a decision if the ",
   "      condition evaluated true or false, respectively.",
   "    * A k indicates a constant decision or condition.",
   "    * The slash / means this probe is excluded from summary results. ",
   0};
}

//----------------------------------------------------------------------
int cmCTestCoverageHandler::RunBullseyeCoverageBranch(
  cmCTestCoverageHandlerContainer* cont,
  std::set<cmStdString>& coveredFileNames,
  std::vector<std::string>& files,
  std::vector<std::string>& filesFullPath)
{
  if(files.size() != filesFullPath.size())
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Files and full path files not the same size?:\n");
    return 0;
    }
  // create the output stream for the CoverageLog-N.xml file
  cmGeneratedFileStream covLogFile;
  int logFileCount = 0;
  if ( !this->StartCoverageLogFile(covLogFile, logFileCount) )
    {
    return -1;
    }
  // for each file run covbr on that file to get the coverage
  // information for that file
  std::string outputFile;
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                 "run covbr: "
                 << std::endl);

  if(!this->RunBullseyeCommand(cont, "covbr", 0, outputFile))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "error running covbr for." << "\n");
    return -1;
    }
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             "covbr output in  " << outputFile
             << std::endl);
  // open the output file
  std::ifstream fin(outputFile.c_str());
  if(!fin)
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot open coverage file: " <<
               outputFile.c_str() << std::endl);
    return 0;
    }
  std::map<cmStdString, cmStdString> fileMap;
  std::vector<std::string>::iterator fp = filesFullPath.begin();
  for(std::vector<std::string>::iterator f =  files.begin();
      f != files.end(); ++f, ++fp)
    {
    fileMap[*f] = *fp;
    }

  int count =0; // keep count of the number of files
  // Now parse each line from the bullseye cov log file
  std::string lineIn;
  bool valid = false; // are we in a valid output file
  int line = 0; // line of the current file
  cmStdString file;
  while(cmSystemTools::GetLineFromStream(fin, lineIn))
    {
    bool startFile = false;
    if(lineIn.size() > 1 && lineIn[lineIn.size()-1] == ':')
      {
      file = lineIn.substr(0, lineIn.size()-1);
      if(coveredFileNames.find(file) != coveredFileNames.end())
        {
        startFile = true;
        }
      }
    if(startFile)
      {
      // if we are in a valid file close it because a new one started
      if(valid)
        {
        covLogFile << "\t\t</Report>" << std::endl
                   << "\t</File>" << std::endl;
        }
      // only allow 100 files in each log file
      if ( count != 0 && count % 100 == 0 )
        {
        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                   "start a new log file: "
                   << count
                   << std::endl);
        this->EndCoverageLogFile(covLogFile, logFileCount);
        logFileCount ++;
        if ( !this->StartCoverageLogFile(covLogFile, logFileCount) )
          {
          return -1;
          }
        count++; // move on one
        }
      std::map<cmStdString, cmStdString>::iterator
        i = fileMap.find(file);
      // if the file should be covered write out the header for that file
      if(i != fileMap.end())
        {
        // we have a new file so count it in the output
        count++;
        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                   "Produce coverage for file: "
                   << file.c_str() << " " << count
                   << std::endl);
        // start the file output
        covLogFile << "\t<File Name=\""
                   << cmXMLSafe(i->first)
                   << "\" FullPath=\"" << cmXMLSafe(
                     this->CTest->GetShortPathToFile(
                       i->second.c_str())) << "\">" << std::endl
                   << "\t\t<Report>" << std::endl;
        // write the bullseye header
        line =0;
        for(int k =0; bullseyeHelp[k] != 0; ++k)
          {
          covLogFile << "\t\t<Line Number=\"" << line << "\" Count=\"-1\">"
                     << cmXMLSafe(bullseyeHelp[k])
                     << "</Line>" << std::endl;
          line++;
          }
        valid = true; // we are in a valid file section
        }
      else
        {
        // this is not a file that we want coverage for
        valid = false;
        }
      }
    // we are not at a start file, and we are in a valid file output the line
    else if(valid)
      {
      covLogFile << "\t\t<Line Number=\"" << line << "\" Count=\"-1\">"
                 << cmXMLSafe(lineIn)
                 << "</Line>" << std::endl;
      line++;
      }
    }
  // if we ran out of lines a valid file then close that file
  if(valid)
    {
    covLogFile << "\t\t</Report>" << std::endl
               << "\t</File>" << std::endl;
    }
  this->EndCoverageLogFile(covLogFile, logFileCount);
  return 1;
}


//----------------------------------------------------------------------
int cmCTestCoverageHandler::RunBullseyeCommand(
  cmCTestCoverageHandlerContainer* cont,
  const char* cmd,
  const char* arg,
  std::string& outputFile)
{
  std::string program = cmSystemTools::FindProgram(cmd);
  if(program.size() == 0)
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Cannot find :" << cmd << "\n");
    return 0;
    }
  if(arg)
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "Run : " << program.c_str() << " " << arg << "\n");
    }
  else
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "Run : " << program.c_str() << "\n");
    }
  // create a process object and start it
  cmCTestRunProcess runCoverageSrc;
  runCoverageSrc.SetCommand(program.c_str());
  runCoverageSrc.AddArgument(arg);
  std::string stdoutFile = cont->BinaryDir + "/Testing/Temporary/";
  stdoutFile += this->GetCTestInstance()->GetCurrentTag();
  stdoutFile += "-";
  stdoutFile += cmd;
  std::string stderrFile = stdoutFile;
  stdoutFile += ".stdout";
  stderrFile += ".stderr";
  runCoverageSrc.SetStdoutFile(stdoutFile.c_str());
  runCoverageSrc.SetStderrFile(stderrFile.c_str());
  if(!runCoverageSrc.StartProcess())
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Could not run : "
               << program.c_str() << " " << arg << "\n"
               << "kwsys process state : "
               << runCoverageSrc.GetProcessState());
    return 0;
    }
  // since we set the output file names wait for it to end
  runCoverageSrc.WaitForExit();
  outputFile = stdoutFile;
  return 1;
}

//----------------------------------------------------------------------
int cmCTestCoverageHandler::RunBullseyeSourceSummary(
  cmCTestCoverageHandlerContainer* cont)
{
  // Run the covsrc command and create a temp outputfile
  std::string outputFile;
  if(!this->RunBullseyeCommand(cont, "covsrc", "-c", outputFile))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "error running covsrc:\n");
    return 0;
    }

  std::ostream& tmpLog = *cont->OFS;
  // copen the Coverage.xml file in the Testing directory
  cmGeneratedFileStream covSumFile;
  if(!this->StartResultingXML(cmCTest::PartCoverage, "Coverage", covSumFile))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Cannot open coverage summary file." << std::endl);
    return 0;
    }
  this->CTest->StartXML(covSumFile, this->AppendXML);
  double elapsed_time_start = cmSystemTools::GetTime();
  std::string coverage_start_time = this->CTest->CurrentTime();
  covSumFile << "<Coverage>" << std::endl
             << "\t<StartDateTime>"
             << coverage_start_time << "</StartDateTime>"
             << std::endl
             << "\t<StartTime>"
             << static_cast<unsigned int>(cmSystemTools::GetTime())
             << "</StartTime>"
             << std::endl;
  std::string stdline;
  std::string errline;
  // expected output:
  // first line is:
  // "Source","Function Coverage","out of","%","C/D Coverage","out of","%"
  // after that data follows in that format
  std::string sourceFile;
  int functionsCalled = 0;
  int totalFunctions = 0;
  int percentFunction = 0;
  int branchCovered = 0;
  int totalBranches = 0;
  int percentBranch = 0;
  double total_tested = 0;
  double total_untested = 0;
  double total_functions = 0;
  double percent_coverage =0;
  double number_files  = 0;
  std::vector<std::string> coveredFiles;
  std::vector<std::string> coveredFilesFullPath;
  // Read and parse the summary output file
  std::ifstream fin(outputFile.c_str());
  if(!fin)
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot open coverage summary file: " <<
               outputFile.c_str() << std::endl);
    return 0;
    }
  std::set<cmStdString> coveredFileNames;
  while(cmSystemTools::GetLineFromStream(fin, stdline))
    {
    // if we have a line of output from stdout
    if(stdline.size())
      {
      // parse the comma separated output
      this->ParseBullsEyeCovsrcLine(stdline,
                                    sourceFile,
                                    functionsCalled,
                                    totalFunctions,
                                    percentFunction,
                                    branchCovered,
                                    totalBranches,
                                    percentBranch);
      // The first line is the header
      if(sourceFile == "Source" || sourceFile == "Total")
        {
        continue;
        }
      std::string file = sourceFile;
      coveredFileNames.insert(file);
      if(!cmSystemTools::FileIsFullPath(sourceFile.c_str()))
        {
        // file will be relative to the binary dir
        file = cont->BinaryDir;
        file += "/";
        file += sourceFile;
        }
      file = cmSystemTools::CollapseFullPath(file.c_str());
      bool shouldIDoCoverage
        = this->ShouldIDoCoverage(file.c_str(),
                                  cont->SourceDir.c_str(),
                                  cont->BinaryDir.c_str());
      if ( !shouldIDoCoverage )
        {
        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                   ".NoDartCoverage found, so skip coverage check for: "
                   << file.c_str()
                   << std::endl);
        continue;
        }

      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                 "Doing coverage for: "
                 << file.c_str()
                 << std::endl);

      coveredFiles.push_back(sourceFile);
      coveredFilesFullPath.push_back(file);

      number_files++;
      total_functions += totalFunctions;
      total_tested += functionsCalled;
      total_untested += (totalFunctions - functionsCalled);

      std::string fileName = cmSystemTools::GetFilenameName(file.c_str());
      std::string shortFileName =
        this->CTest->GetShortPathToFile(file.c_str());

      float cper = static_cast<float>(percentBranch + percentFunction);
      if(totalBranches > 0)
        {
        cper /= 2.0f;
        }
      percent_coverage += cper;
      float cmet = static_cast<float>(percentFunction + percentBranch);
      if(totalBranches > 0)
        {
        cmet /= 2.0f;
        }
      cmet /= 100.0f;
      tmpLog << stdline.c_str() << "\n";
      tmpLog << fileName << "\n";
      tmpLog << "functionsCalled: " << functionsCalled/100 << "\n";
      tmpLog << "totalFunctions: " << totalFunctions/100 << "\n";
      tmpLog << "percentFunction: " << percentFunction << "\n";
      tmpLog << "branchCovered: " << branchCovered << "\n";
      tmpLog << "totalBranches: " << totalBranches << "\n";
      tmpLog << "percentBranch: " << percentBranch << "\n";
      tmpLog << "percentCoverage: " << percent_coverage << "\n";
      tmpLog << "coverage metric: " << cmet << "\n";
      covSumFile << "\t<File Name=\"" << cmXMLSafe(sourceFile)
                 << "\" FullPath=\"" << cmXMLSafe(shortFileName)
                 << "\" Covered=\"" << (cmet>0?"true":"false") << "\">\n"
                 << "\t\t<BranchesTested>"
                 << branchCovered
                 << "</BranchesTested>\n"
                 << "\t\t<BranchesUnTested>"
                 << totalBranches - branchCovered
                 << "</BranchesUnTested>\n"
                 << "\t\t<FunctionsTested>"
                 << functionsCalled
                 << "</FunctionsTested>\n"
                 << "\t\t<FunctionsUnTested>"
                 << totalFunctions - functionsCalled
                 << "</FunctionsUnTested>\n"
        // Hack for conversion of function to loc assume a function
        // has 100 lines of code
                 << "\t\t<LOCTested>" << functionsCalled *100
                 << "</LOCTested>\n"
                 << "\t\t<LOCUnTested>"
                 << (totalFunctions - functionsCalled)*100
                 << "</LOCUnTested>\n"
                 << "\t\t<PercentCoverage>";
      covSumFile.setf(std::ios::fixed, std::ios::floatfield);
      covSumFile.precision(2);
      covSumFile << (cper) << "</PercentCoverage>\n"
                 << "\t\t<CoverageMetric>";
      covSumFile.setf(std::ios::fixed, std::ios::floatfield);
      covSumFile.precision(2);
      covSumFile << (cmet) << "</CoverageMetric>\n";
      this->WriteXMLLabels(covSumFile, shortFileName);
      covSumFile << "\t</File>" << std::endl;
      }
    }
  std::string end_time = this->CTest->CurrentTime();
  covSumFile << "\t<LOCTested>" << total_tested << "</LOCTested>\n"
    << "\t<LOCUntested>" << total_untested << "</LOCUntested>\n"
    << "\t<LOC>" << total_functions << "</LOC>\n"
    << "\t<PercentCoverage>";
  covSumFile.setf(std::ios::fixed, std::ios::floatfield);
  covSumFile.precision(2);
  covSumFile
    << SAFEDIV(percent_coverage,number_files)<< "</PercentCoverage>\n"
    << "\t<EndDateTime>" << end_time << "</EndDateTime>\n"
    << "\t<EndTime>" << static_cast<unsigned int>(cmSystemTools::GetTime())
    << "</EndTime>\n";
  covSumFile
    << "<ElapsedMinutes>" <<
    static_cast<int>((cmSystemTools::GetTime() - elapsed_time_start)/6)/10.0
    << "</ElapsedMinutes>"
    << "</Coverage>" << std::endl;
  this->CTest->EndXML(covSumFile);

  // Now create the coverage information for each file
  return this->RunBullseyeCoverageBranch(cont,
                                         coveredFileNames,
                                         coveredFiles,
                                         coveredFilesFullPath);
}

//----------------------------------------------------------------------
int cmCTestCoverageHandler::HandleBullseyeCoverage(
  cmCTestCoverageHandlerContainer* cont)
{
  const char* covfile = cmSystemTools::GetEnv("COVFILE");
  if(!covfile || strlen(covfile) == 0)
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               " COVFILE environment variable not found, not running "
               " bullseye\n");
    return 0;
    }
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             " run covsrc with COVFILE=["
             << covfile
             << "]" << std::endl);
  if(!this->RunBullseyeSourceSummary(cont))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Error running bullseye summary.\n");
    return 0;
    }
  cmCTestLog(this->CTest, DEBUG, "HandleBullseyeCoverage return 1 "
             << std::endl);
  return 1;
}

bool cmCTestCoverageHandler::GetNextInt(std::string const& inputLine,
                                        std::string::size_type& pos,
                                        int& value)
{
  std::string::size_type start = pos;
  pos = inputLine.find(',', start);
  value = atoi(inputLine.substr(start, pos).c_str());
  if(pos == inputLine.npos)
    {
    return true;
    }
  pos++;
  return true;
}

bool cmCTestCoverageHandler::ParseBullsEyeCovsrcLine(
  std::string const& inputLine,
  std::string& sourceFile,
  int& functionsCalled,
  int& totalFunctions,
  int& percentFunction,
  int& branchCovered,
  int& totalBranches,
  int& percentBranch)
{
  // find the first comma
  std::string::size_type pos = inputLine.find(',');
  if(pos == inputLine.npos)
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Error parsing string : "
               << inputLine.c_str() << "\n");
    return false;
    }
  // the source file has "" around it so extract out the file name
  sourceFile = inputLine.substr(1,pos-2);
  pos++;
  if(!this->GetNextInt(inputLine, pos, functionsCalled))
    {
    return false;
    }
  if(!this->GetNextInt(inputLine, pos, totalFunctions))
    {
    return false;
    }
  if(!this->GetNextInt(inputLine, pos, percentFunction))
    {
    return false;
    }
  if(!this->GetNextInt(inputLine, pos, branchCovered))
    {
    return false;
    }
  if(!this->GetNextInt(inputLine, pos, totalBranches))
    {
    return false;
    }
  if(!this->GetNextInt(inputLine, pos, percentBranch))
    {
    return false;
    }
  // should be at the end now
  if(pos != inputLine.npos)
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Error parsing input : "
               << inputLine.c_str() << " last pos not npos =  " << pos <<
               "\n");
    }
  return true;
}

//----------------------------------------------------------------------
int cmCTestCoverageHandler::GetLabelId(std::string const& label)
{
  LabelIdMapType::iterator i = this->LabelIdMap.find(label);
  if(i == this->LabelIdMap.end())
    {
    int n = int(this->Labels.size());
    this->Labels.push_back(label);
    LabelIdMapType::value_type entry(label, n);
    i = this->LabelIdMap.insert(entry).first;
    }
  return i->second;
}

//----------------------------------------------------------------------
void cmCTestCoverageHandler::LoadLabels()
{
  std::string fileList = this->CTest->GetBinaryDir();
  fileList += cmake::GetCMakeFilesDirectory();
  fileList += "/TargetDirectories.txt";
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             " target directory list [" << fileList << "]\n");
  std::ifstream finList(fileList.c_str());
  std::string line;
  while(cmSystemTools::GetLineFromStream(finList, line))
    {
    this->LoadLabels(line.c_str());
    }
}

//----------------------------------------------------------------------
void cmCTestCoverageHandler::LoadLabels(const char* dir)
{
  LabelSet& dirLabels = this->TargetDirs[dir];
  std::string fname = dir;
  fname += "/Labels.txt";
  std::ifstream fin(fname.c_str());
  if(!fin)
    {
    return;
    }

  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             " loading labels from [" << fname << "]\n");
  bool inTarget = true;
  std::string source;
  std::string line;
  std::vector<int> targetLabels;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    if(line.empty() || line[0] == '#')
      {
      // Ignore blank and comment lines.
      continue;
      }
    else if(line[0] == ' ')
      {
      // Label lines appear indented by one space.
      std::string label = line.substr(1);
      int id = this->GetLabelId(label);
      dirLabels.insert(id);
      if(inTarget)
        {
        targetLabels.push_back(id);
        }
      else
        {
        this->SourceLabels[source].insert(id);
        }
      }
    else
      {
      // Non-indented lines specify a source file name.  The first one
      // is the end of the target-wide labels.
      inTarget = false;

      source = this->CTest->GetShortPathToFile(line.c_str());

      // Label the source with the target labels.
      LabelSet& labelSet = this->SourceLabels[source];
      for(std::vector<int>::const_iterator li = targetLabels.begin();
          li != targetLabels.end(); ++li)
        {
        labelSet.insert(*li);
        }
      }
    }
}

//----------------------------------------------------------------------
void cmCTestCoverageHandler::WriteXMLLabels(std::ofstream& os,
                                            std::string const& source)
{
  LabelMapType::const_iterator li = this->SourceLabels.find(source);
  if(li != this->SourceLabels.end() && !li->second.empty())
    {
    os << "\t\t<Labels>\n";
    for(LabelSet::const_iterator lsi = li->second.begin();
        lsi != li->second.end(); ++lsi)
      {
      os << "\t\t\t<Label>" << cmXMLSafe(this->Labels[*lsi]) << "</Label>\n";
      }
    os << "\t\t</Labels>\n";
    }
}

//----------------------------------------------------------------------------
void
cmCTestCoverageHandler::SetLabelFilter(std::set<cmStdString> const& labels)
{
  this->LabelFilter.clear();
  for(std::set<cmStdString>::const_iterator li = labels.begin();
      li != labels.end(); ++li)
    {
    this->LabelFilter.insert(this->GetLabelId(*li));
    }
}

//----------------------------------------------------------------------
bool cmCTestCoverageHandler::IntersectsFilter(LabelSet const& labels)
{
  // If there is no label filter then nothing is filtered out.
  if(this->LabelFilter.empty())
    {
    return true;
    }

  std::vector<int> ids;
  cmsys_stl::set_intersection
    (labels.begin(), labels.end(),
     this->LabelFilter.begin(), this->LabelFilter.end(),
     cmsys_stl::back_inserter(ids));
  return !ids.empty();
}

//----------------------------------------------------------------------
bool cmCTestCoverageHandler::IsFilteredOut(std::string const& source)
{
  // If there is no label filter then nothing is filtered out.
  if(this->LabelFilter.empty())
    {
    return false;
    }

  // The source is filtered out if it does not have any labels in
  // common with the filter set.
  std::string shortSrc = this->CTest->GetShortPathToFile(source.c_str());
  LabelMapType::const_iterator li = this->SourceLabels.find(shortSrc);
  if(li != this->SourceLabels.end())
    {
    return !this->IntersectsFilter(li->second);
    }
  return true;
}

//----------------------------------------------------------------------
std::set<std::string> cmCTestCoverageHandler::FindUncoveredFiles(
  cmCTestCoverageHandlerContainer* cont)
{
  std::set<std::string> extraMatches;

  for(std::vector<cmStdString>::iterator i = this->ExtraCoverageGlobs.begin();
      i != this->ExtraCoverageGlobs.end(); ++i)
    {
    cmsys::Glob gl;
    gl.RecurseOn();
    gl.RecurseThroughSymlinksOff();
    std::string glob = cont->SourceDir + "/" + *i;
    gl.FindFiles(glob);
    std::vector<std::string> files = gl.GetFiles();
    for(std::vector<std::string>::iterator f = files.begin();
        f != files.end(); ++f)
      {
      if(this->ShouldIDoCoverage(f->c_str(),
         cont->SourceDir.c_str(), cont->BinaryDir.c_str()))
        {
        extraMatches.insert(this->CTest->GetShortPathToFile(
          f->c_str()));
        }
      }
    }

  if(extraMatches.size())
    {
    for(cmCTestCoverageHandlerContainer::TotalCoverageMap::iterator i =
        cont->TotalCoverage.begin(); i != cont->TotalCoverage.end(); ++i)
      {
      std::string shortPath = this->CTest->GetShortPathToFile(
        i->first.c_str());
      extraMatches.erase(shortPath);
      }
    }
  return extraMatches;
}
