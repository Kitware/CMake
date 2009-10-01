/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDepends.h"

#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmSystemTools.h"
#include "cmFileTimeComparison.h"
#include <string.h>

//----------------------------------------------------------------------------
cmDepends::cmDepends(cmLocalGenerator* lg, const char* targetDir):
  CompileDirectory(),
  LocalGenerator(lg),
  Verbose(false),
  FileComparison(0),
  TargetDirectory(targetDir),
  MaxPath(cmSystemTools::GetMaximumFilePathLength()),
  Dependee(new char[MaxPath]),
  Depender(new char[MaxPath])
{
}

//----------------------------------------------------------------------------
cmDepends::~cmDepends()
{
  delete [] this->Dependee;
  delete [] this->Depender;
}

//----------------------------------------------------------------------------
bool cmDepends::Write(std::ostream &makeDepends,
                      std::ostream &internalDepends)
{
  // Lookup the set of sources to scan.
  std::string srcLang = "CMAKE_DEPENDS_CHECK_";
  srcLang += this->Language;
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  const char* srcStr = mf->GetSafeDefinition(srcLang.c_str());
  std::vector<std::string> pairs;
  cmSystemTools::ExpandListArgument(srcStr, pairs);

  for(std::vector<std::string>::iterator si = pairs.begin();
      si != pairs.end();)
    {
    // Get the source and object file.
    std::string const& src = *si++;
    if(si == pairs.end()) { break; }
    std::string obj = *si++;

    // Make sure the object file is relative to the top of the build tree.
    obj = this->LocalGenerator->Convert(obj.c_str(),
                                        cmLocalGenerator::HOME_OUTPUT,
                                        cmLocalGenerator::MAKEFILE);

    // Write the dependencies for this pair.
    if(!this->WriteDependencies(src.c_str(), obj.c_str(),
                                makeDepends, internalDepends))
      {
      return false;
      }
    }

  return this->Finalize(makeDepends, internalDepends);
}

//----------------------------------------------------------------------------
bool cmDepends::Finalize(std::ostream&,
                         std::ostream&)
{
  return true;
}

//----------------------------------------------------------------------------
bool cmDepends::Check(const char *makeFile, const char *internalFile,
                      std::map<std::string, DependencyVector>& validDeps)
{
  // Dependency checks must be done in proper working directory.
  std::string oldcwd = ".";
  if(this->CompileDirectory != ".")
    {
    // Get the CWD but do not call CollapseFullPath because
    // we only need it to cd back, and the form does not matter
    oldcwd = cmSystemTools::GetCurrentWorkingDirectory(false);
    cmSystemTools::ChangeDirectory(this->CompileDirectory.c_str());
    }

  // Check whether dependencies must be regenerated.
  bool okay = true;
  std::ifstream fin(internalFile);
  if(!(fin && this->CheckDependencies(fin, validDeps)))
    {
    // Clear all dependencies so they will be regenerated.
    this->Clear(makeFile);
    cmSystemTools::RemoveFile(internalFile);
    okay = false;
    }

  // Restore working directory.
  if(oldcwd != ".")
    {
    cmSystemTools::ChangeDirectory(oldcwd.c_str());
    }

  return okay;
}

//----------------------------------------------------------------------------
void cmDepends::Clear(const char *file)
{
  // Print verbose output.
  if(this->Verbose)
    {
    cmOStringStream msg;
    msg << "Clearing dependencies in \"" << file << "\"." << std::endl;
    cmSystemTools::Stdout(msg.str().c_str());
    }

  // Write an empty dependency file.
  cmGeneratedFileStream depFileStream(file);
  depFileStream
    << "# Empty dependencies file\n"
    << "# This may be replaced when dependencies are built." << std::endl;
}

//----------------------------------------------------------------------------
bool cmDepends::WriteDependencies(const char*, const char*,
                                  std::ostream&, std::ostream&)
{
  // This should be implemented by the subclass.
  return false;
}

//----------------------------------------------------------------------------
bool cmDepends::CheckDependencies(std::istream& internalDepends,
                            std::map<std::string, DependencyVector>& validDeps)
{
  // Parse dependencies from the stream.  If any dependee is missing
  // or newer than the depender then dependencies should be
  // regenerated.
  bool okay = true;
  bool dependerExists = false;
  DependencyVector* currentDependencies = 0;

  while(internalDepends.getline(this->Dependee, this->MaxPath))
    {
    if ( this->Dependee[0] == 0 || this->Dependee[0] == '#' || 
         this->Dependee[0] == '\r' )
      {
      continue;
      }
    size_t len = internalDepends.gcount()-1;
    if ( this->Dependee[len-1] == '\r' )
      {
      len --;
      this->Dependee[len] = 0;
      }
    if ( this->Dependee[0] != ' ' )
      {
      memcpy(this->Depender, this->Dependee, len+1);
      // Calling FileExists() for the depender here saves in many cases 50%
      // of the calls to FileExists() further down in the loop. E.g. for
      // kdelibs/khtml this reduces the number of calls from 184k down to 92k,
      // or the time for cmake -E cmake_depends from 0.3 s down to 0.21 s.
      dependerExists = cmSystemTools::FileExists(this->Depender);
      DependencyVector tmp;
      validDeps[this->Depender] = tmp;
      currentDependencies = &validDeps[this->Depender];
      continue;
      }
    /*
    // Parse the dependency line.
    if(!this->ParseDependency(line.c_str()))
      {
      continue;
      }
      */

    // Dependencies must be regenerated if the dependee does not exist
    // or if the depender exists and is older than the dependee.
    bool regenerate = false;
    const char* dependee = this->Dependee+1;
    const char* depender = this->Depender;
    if (currentDependencies != 0)
      {
      currentDependencies->push_back(dependee);
      }

    if(!cmSystemTools::FileExists(dependee))
      {
      // The dependee does not exist.
      regenerate = true;

      // Print verbose output.
      if(this->Verbose)
        {
        cmOStringStream msg;
        msg << "Dependee \"" << dependee
            << "\" does not exist for depender \""
            << depender << "\"." << std::endl;
        cmSystemTools::Stdout(msg.str().c_str());
        }
      }
    else if(dependerExists)
      {
      // The dependee and depender both exist.  Compare file times.
      int result = 0;
      if((!this->FileComparison->FileTimeCompare(depender, dependee,
                                             &result) || result < 0))
        {
        // The depender is older than the dependee.
        regenerate = true;

        // Print verbose output.
        if(this->Verbose)
          {
          cmOStringStream msg;
          msg << "Dependee \"" << dependee
              << "\" is newer than depender \""
              << depender << "\"." << std::endl;
          cmSystemTools::Stdout(msg.str().c_str());
          }
        }
      }
    if(regenerate)
      {
      // Dependencies must be regenerated.
      okay = false;

      // Remove the information of this depender from the map, it needs
      // to be rescanned
      if (currentDependencies != 0)
        {
        validDeps.erase(this->Depender);
        currentDependencies = 0;
        }

      // Remove the depender to be sure it is rebuilt.
      if (dependerExists)
        {
        cmSystemTools::RemoveFile(depender);
        dependerExists = false;
        }
      }
    }

  return okay;
}

//----------------------------------------------------------------------------
void cmDepends::SetIncludePathFromLanguage(const char* lang)
{
  std::string includePathVar = "CMAKE_";
  includePathVar += lang;
  includePathVar += "_INCLUDE_PATH";
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  if(const char* includePath = mf->GetDefinition(includePathVar.c_str()))
    {
    cmSystemTools::ExpandListArgument(includePath, this->IncludePath);
    }
}
