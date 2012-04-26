#include "cmStandardIncludes.h"
#include <stdio.h>
#include <stdlib.h>
#include "cmSystemTools.h"
#include "cmParseGTMCoverage.h"
#include <cmsys/Directory.hxx>
#include <cmsys/Glob.hxx>


cmParseGTMCoverage::cmParseGTMCoverage(cmCTestCoverageHandlerContainer& cont,
    cmCTest* ctest)
    :Coverage(cont), CTest(ctest)
{
}

bool cmParseGTMCoverage::ReadGTMCoverage(const char* file)
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

void cmParseGTMCoverage::InitializeFile(std::string& file)
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

bool cmParseGTMCoverage::LoadPackages(const char* d)
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
//    this->InitializeFile(*fileIt);
    }
  return true;
}

bool cmParseGTMCoverage::LoadCoverageData(const char* d)
{
  // load all the .mcov files in the specified directory
  cmsys::Directory dir;
  if(!dir.Load(d))
    {
    return false;
    }
  size_t numf;
  unsigned int i;
  numf = dir.GetNumberOfFiles();
  for (i = 0; i < numf; i++)
    {
    std::string file = dir.GetFile(i);
    if(file != "." && file != ".."
       && !cmSystemTools::FileIsDirectory(file.c_str()))
      {
      std::string path = d;
      path += "/";
      path += file;
      if(cmSystemTools::GetFilenameLastExtension(path) == ".mcov")
        {
        if(!this->ReadMCovFile(path.c_str()))
          {
          return false;
          }
        }
      }
    }
  return true;
}

bool cmParseGTMCoverage::ParseFile(std::string& filepath,
                                   std::string& function,
                                   int& lineoffset)
{
  std::ifstream in(filepath.c_str());
  if(!in)
    {
    return false;
    }
  std::string line;
  int linenum = 0;
  while(  cmSystemTools::GetLineFromStream(in, line))
    {
    std::string::size_type pos = line.find(function.c_str());
    if(pos == 0)
      {
      char nextchar = line[function.size()];
      if(nextchar == ' ' || nextchar == '(')
        {
        lineoffset = linenum;
        return true;
        }
      }
    if(pos == 1)
      {
      char prevchar = line[0];
      char nextchar = line[function.size()+1];
      if(prevchar == '%' && (nextchar == ' ' || nextchar == '('))
        {
        lineoffset = linenum;
        return true;
        }
      }
    linenum++; // move to next line count
    }
  lineoffset = 0;
  cmCTestLog(this->CTest, ERROR_MESSAGE,
             "Could not find entry point : "
             << function << " in " << filepath << "\n");
  return false;
}

bool cmParseGTMCoverage::ParseLine(std::string const& line,
                                   std::string& routine,
                                   std::string& function,
                                   int& linenumber,
                                   int& count)
{
  // this method parses lines from the .mcov file
  // each line has ^COVERAGE(...) in it, and there
  // are several varients of coverage lines:
  //
  // ^COVERAGE("DIC11","PR1",0)="2:0:0:0"
  //          ( file  , entry, line ) = "number_executed:timing_info"
  // ^COVERAGE("%RSEL","SRC")="1:0:0:0"
  //          ( file  , entry ) = "number_executed:timing_info"
  // ^COVERAGE("%RSEL","init",8,"FOR_LOOP",1)=1
  //          ( file  , entry, line, IGNORE ) =number_executed
  std::vector<cmStdString> args;
  std::string::size_type pos = line.find('(', 0);
  // if no ( is found, then return line has no coverage
  if(pos == std::string::npos)
    {
    return false;
    }
  std::string arg;
  bool done = false;
  // separate out all of the comma separated arguments found
  // in the COVERAGE(...) line
  while(line[pos] && !done)
    {
    // save the char we are looking at
    char cur = line[pos];
    // , or ) means end of argument
    if(cur == ',' || cur == ')')
      {
      // save the argument into the argument vector
      args.push_back(arg);
      // start on a new argument
      arg = "";
      // if we are at the end of the ), then finish while loop
      if(cur == ')')
        {
        done = true;
        }
      }
    else
      {
      // all chars except ", (, and % get stored in the arg string
      if(cur != '\"' && cur != '(' && cur != '%')
        {
        arg.append(1, line[pos]);
        }
      }
    // move to next char
    pos++;
    }
  // now parse the right hand side of the =
  pos = line.find('=');
  // no = found, this is an error
  if(pos == line.npos)
    {
    return false;
    }
  pos++; // move past =

  // if the next positing is not a ", then this is a
  // COVERAGE(..)=count line and turn the rest of the string
  // past the = into an integer and set it to count
  if(line[pos] != '\"')
    {
    count = atoi(line.substr(pos).c_str());
    }
  else
    {
    // this means line[pos] is a ", and we have a
    // COVERAGE(...)="1:0:0:0" type of line
    pos++; // move past "
    // find the first : past the "
    std::string::size_type pos2 = line.find(':', pos);
    // turn the string between the " and the first : into an integer
    // and set it to count
    count = atoi(line.substr(pos, pos2-pos).c_str());
    }
  // less then two arguments is an error
  if(args.size() < 2)
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Error parsing mcov line: [" << line << "]\n");
    return false;
    }
  routine = args[0]; // the routine is the first argument
  function = args[1]; // the function in the routine is the second
  // in the two argument only format
  // ^COVERAGE("%RSEL","SRC"), the line offset is 0
  if(args.size() == 2)
    {
    linenumber = 0;
    }
  else
    {
    // this is the format for this line
    // ^COVERAGE("%RSEL","SRC",count)
    linenumber = atoi(args[2].c_str());
    }
  return true;
}

bool cmParseGTMCoverage::ReadMCovFile(const char* file)
{
  std::ifstream in(file);
  if(!in)
    {
    return false;
    }
  std::string line;
  std::string lastfunction;
  std::string lastroutine;
  std::string lastpath;
  int lastoffset = 0;
  while(  cmSystemTools::GetLineFromStream(in, line))
    {
    // only look at lines that have coverage data
    if(line.find("^COVERAGE") == line.npos)
      {
      continue;
      }
    std::string filepath;
    std::string function;
    std::string routine;
    int linenumber = 0;
    int count = 0;
    this->ParseLine(line, routine, function, linenumber, count);
    // skip this one
    if(routine == "RSEL")
      {
      continue;
      }
    // no need to search the file if we just did it
    if(function == lastfunction && lastroutine == routine)
      {
      this->Coverage.TotalCoverage[lastpath][lastoffset + linenumber] += count;
      continue;
      }
    // Find the full path to the file
    std::map<cmStdString, cmStdString>::iterator i =
      this->RoutineToDirectory.find(routine);
    bool found = false;
    if(i != this->RoutineToDirectory.end())
      {
      filepath = i->second;
      found = true;
      }
    else
      {
      // try some alternate names
      const char* tryname[] = {"GUX", "GTM", "ONT", 0};
      for(int k=0; tryname[k] != 0; k++)
        {
        std::string routine2 = routine + tryname[k];
        i = this->RoutineToDirectory.find(routine2);
        if(i != this->RoutineToDirectory.end())
          {
          found = true;
          filepath = i->second;
          break; // break out of tryname loop if found
          }
        }
      }
    if(found)
      {
      int lineoffset;
      if(this->ParseFile(filepath,
                         function,
                         lineoffset))
        {
        // hack, this should be done on every file, but for now
        // just do it on the ones that have coverage at all
        if( this->Coverage.TotalCoverage[filepath].size() == 0)
          {
          this->InitializeFile(filepath);
          }
        cmCTestCoverageHandlerContainer::SingleFileCoverageVector&
          coverageVector = this->Coverage.TotalCoverage[filepath];
        coverageVector[lineoffset + linenumber] += count;
        }
      lastoffset = lineoffset;
      }
    else
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Can not find mumps file : "
               << routine << "  referenced in this line of mcov data:\n"
                 "[" << line << "]\n");
      }
    lastfunction = function;
    lastroutine = routine;
    lastpath = filepath;
    }
  return true;
}
