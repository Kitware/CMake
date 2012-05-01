#include "cmStandardIncludes.h"
#include <stdio.h>
#include <stdlib.h>
#include "cmSystemTools.h"
#include "cmParseGTMCoverage.h"
#include <cmsys/Directory.hxx>
#include <cmsys/Glob.hxx>


cmParseMumpsCoverage::cmParseMumpsCoverage(
  cmCTestCoverageHandlerContainer& cont,
  cmCTest* ctest)
  :Coverage(cont), CTest(ctest)
{
}

bool cmParseMumpsCoverage::ReadCoverageFile(const char* file)
{
  // Read the gtm_coverage.mcov file, that has two lines of data:
  // packages:/full/path/to/Vista/Packages
  // coverage_dir:/full/path/to/dir/with/*.mcov
  std::ifstream in(file);
  if(!in)
    {
    return false;
    }
  std::string line;
  cmSystemTools::GetLineFromStream(in, line);
  std::string::size_type pos = line.find(':', 0);
  std::string packages;
  if(pos != std::string::npos)
    {
    packages = line.substr(pos+1);
    }
  cmSystemTools::GetLineFromStream(in, line);
  pos = line.find(':', 0);
  std::string coverage_dir;
  if(pos != std::string::npos)
    {
    coverage_dir = line.substr(pos+1);
    }
  // load the mumps files from the packages directory
  this->LoadPackages(packages.c_str());
  // now load the *.mcov files from the coverage directory
  this->LoadCoverageData(coverage_dir.c_str());
  return true;
}

void cmParseMumpsCoverage::InitializeMumpsFile(std::string& file)
{
  // initialize the coverage information for a given mumps file
  std::ifstream in(file.c_str());
  if(!in)
    {
    return;
    }
  std::string line;
  cmCTestCoverageHandlerContainer::SingleFileCoverageVector&
    coverageVector = this->Coverage.TotalCoverage[file];
  if(!cmSystemTools::GetLineFromStream(in, line))
    {
    return;
    }
  // first line of a .m file can never be run
  coverageVector.push_back(-1);
  while( cmSystemTools::GetLineFromStream(in, line) )
    {
    // putting in a 0 for a line means it is executable code
    // putting in a -1 for a line means it is not executable code
    int val = -1; // assume line is not executable
    bool found = false;
    std::string::size_type i = 0;
    // (1) Search for the first whitespace or semicolon character on a line.
    //This will skip over labels if the line starts with one, or will simply
    //be the first character on the line for non-label lines.
    for(; i < line.size(); ++i)
      {
      if(line[i] == ' ' || line[i] == '\t' || line[i] == ';')
        {
        found = true;
        break;
        }
      }
    if(found)
      {
      // (2) If the first character found above is whitespace then continue the
      // search for the first following non-whitespace character.
      if(line[i] == ' ' || line[i] == '\t')
        {
        while(i < line.size() && (line[i] == ' ' || line[i] == '\t'))
          {
          i++;
          }
        }
      // (3) If the character found is not a semicolon then the line counts for
      // coverage.
      if(i < line.size() && line[i] != ';')
        {
        val = 0;
        }
      }
    coverageVector.push_back(val);
    }
}

bool cmParseMumpsCoverage::LoadPackages(const char* d)
{
  cmsys::Glob glob;
  glob.RecurseOn();
  std::string pat = d;
  pat += "/*.m";
  glob.FindFiles(pat.c_str());
  std::vector<std::string>& files = glob.GetFiles();
  std::vector<std::string>::iterator fileIt;
  for ( fileIt = files.begin(); fileIt != files.end();
        ++ fileIt )
    {
    std::string name = cmSystemTools::GetFilenameName(*fileIt);
    this->RoutineToDirectory[name.substr(0, name.size()-2)] = *fileIt;
    // initialze each file, this is left out until CDash is fixed
    // to handle large numbers of files
//    this->InitializeMumpsFile(*fileIt);
    }
  return true;
}
