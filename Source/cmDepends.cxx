/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDepends.h"

#include "cmFileTimeCache.h"
#include "cmGeneratedFileStream.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"

#include "cmsys/FStream.hxx"
#include <sstream>
#include <string.h>
#include <utility>

cmDepends::cmDepends(cmLocalGenerator* lg, std::string targetDir)
  : LocalGenerator(lg)
  , TargetDirectory(std::move(targetDir))
  , Dependee(new char[MaxPath])
  , Depender(new char[MaxPath])
{
}

cmDepends::~cmDepends()
{
  delete[] this->Dependee;
  delete[] this->Depender;
}

bool cmDepends::Write(std::ostream& makeDepends, std::ostream& internalDepends)
{
  std::map<std::string, std::set<std::string>> dependencies;
  {
    // Lookup the set of sources to scan.
    std::vector<std::string> pairs;
    {
      std::string const srcLang = "CMAKE_DEPENDS_CHECK_" + this->Language;
      cmMakefile* mf = this->LocalGenerator->GetMakefile();
      cmSystemTools::ExpandListArgument(mf->GetSafeDefinition(srcLang), pairs);
    }
    for (std::vector<std::string>::iterator si = pairs.begin();
         si != pairs.end();) {
      // Get the source and object file.
      std::string const& src = *si++;
      if (si == pairs.end()) {
        break;
      }
      std::string const& obj = *si++;
      dependencies[obj].insert(src);
    }
  }
  for (auto const& d : dependencies) {
    // Write the dependencies for this pair.
    if (!this->WriteDependencies(d.second, d.first, makeDepends,
                                 internalDepends)) {
      return false;
    }
  }

  return this->Finalize(makeDepends, internalDepends);
}

bool cmDepends::Finalize(std::ostream& /*unused*/, std::ostream& /*unused*/)
{
  return true;
}

bool cmDepends::Check(const std::string& makeFile,
                      const std::string& internalFile,
                      std::map<std::string, DependencyVector>& validDeps)
{
  // Check whether dependencies must be regenerated.
  bool okay = true;
  cmsys::ifstream fin(internalFile.c_str());
  if (!(fin && this->CheckDependencies(fin, internalFile, validDeps))) {
    // Clear all dependencies so they will be regenerated.
    this->Clear(makeFile);
    cmSystemTools::RemoveFile(internalFile);
    okay = false;
  }

  return okay;
}

void cmDepends::Clear(const std::string& file)
{
  // Print verbose output.
  if (this->Verbose) {
    std::ostringstream msg;
    msg << "Clearing dependencies in \"" << file << "\"." << std::endl;
    cmSystemTools::Stdout(msg.str());
  }

  // Write an empty dependency file.
  cmGeneratedFileStream depFileStream(file);
  depFileStream << "# Empty dependencies file\n"
                << "# This may be replaced when dependencies are built."
                << std::endl;
}

bool cmDepends::WriteDependencies(const std::set<std::string>& /*unused*/,
                                  const std::string& /*unused*/,
                                  std::ostream& /*unused*/,
                                  std::ostream& /*unused*/)
{
  // This should be implemented by the subclass.
  return false;
}

bool cmDepends::CheckDependencies(
  std::istream& internalDepends, const std::string& internalDependsFileName,
  std::map<std::string, DependencyVector>& validDeps)
{
  // Parse dependencies from the stream.  If any dependee is missing
  // or newer than the depender then dependencies should be
  // regenerated.
  bool okay = true;
  bool dependerExists = false;
  DependencyVector* currentDependencies = nullptr;

  while (internalDepends.getline(this->Dependee, this->MaxPath)) {
    if (this->Dependee[0] == 0 || this->Dependee[0] == '#' ||
        this->Dependee[0] == '\r') {
      continue;
    }
    size_t len = internalDepends.gcount() - 1;
    if (this->Dependee[len - 1] == '\r') {
      len--;
      this->Dependee[len] = 0;
    }
    if (this->Dependee[0] != ' ') {
      memcpy(this->Depender, this->Dependee, len + 1);
      // Calling FileExists() for the depender here saves in many cases 50%
      // of the calls to FileExists() further down in the loop. E.g. for
      // kdelibs/khtml this reduces the number of calls from 184k down to 92k,
      // or the time for cmake -E cmake_depends from 0.3 s down to 0.21 s.
      dependerExists = cmSystemTools::FileExists(this->Depender);
      // If we erase validDeps[this->Depender] by overwriting it with an empty
      // vector, we lose dependencies for dependers that have multiple
      // entries. No need to initialize the entry, std::map will do so on first
      // access.
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

    // Dependencies must be regenerated
    // * if the dependee does not exist
    // * if the depender exists and is older than the dependee.
    // * if the depender does not exist, but the dependee is newer than the
    //   depends file
    bool regenerate = false;
    const std::string dependee(this->Dependee + 1);
    const std::string depender(this->Depender);
    if (currentDependencies != nullptr) {
      currentDependencies->push_back(dependee);
    }

    if (!cmSystemTools::FileExists(dependee)) {
      // The dependee does not exist.
      regenerate = true;

      // Print verbose output.
      if (this->Verbose) {
        std::ostringstream msg;
        msg << "Dependee \"" << dependee << "\" does not exist for depender \""
            << depender << "\"." << std::endl;
        cmSystemTools::Stdout(msg.str());
      }
    } else {
      if (dependerExists) {
        // The dependee and depender both exist.  Compare file times.
        int result = 0;
        if ((!this->FileTimeCache->Compare(depender, dependee, &result) ||
             result < 0)) {
          // The depender is older than the dependee.
          regenerate = true;

          // Print verbose output.
          if (this->Verbose) {
            std::ostringstream msg;
            msg << "Dependee \"" << dependee << "\" is newer than depender \""
                << depender << "\"." << std::endl;
            cmSystemTools::Stdout(msg.str());
          }
        }
      } else {
        // The dependee exists, but the depender doesn't. Regenerate if the
        // internalDepends file is older than the dependee.
        int result = 0;
        if ((!this->FileTimeCache->Compare(internalDependsFileName, dependee,
                                           &result) ||
             result < 0)) {
          // The depends-file is older than the dependee.
          regenerate = true;

          // Print verbose output.
          if (this->Verbose) {
            std::ostringstream msg;
            msg << "Dependee \"" << dependee
                << "\" is newer than depends file \""
                << internalDependsFileName << "\"." << std::endl;
            cmSystemTools::Stdout(msg.str());
          }
        }
      }
    }
    if (regenerate) {
      // Dependencies must be regenerated.
      okay = false;

      // Remove the information of this depender from the map, it needs
      // to be rescanned
      if (currentDependencies != nullptr) {
        validDeps.erase(this->Depender);
        currentDependencies = nullptr;
      }

      // Remove the depender to be sure it is rebuilt.
      if (dependerExists) {
        cmSystemTools::RemoveFile(depender);
        dependerExists = false;
      }
    }
  }

  return okay;
}

void cmDepends::SetIncludePathFromLanguage(const std::string& lang)
{
  // Look for the new per "TARGET_" variant first:
  const char* includePath = nullptr;
  std::string includePathVar = "CMAKE_";
  includePathVar += lang;
  includePathVar += "_TARGET_INCLUDE_PATH";
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  includePath = mf->GetDefinition(includePathVar);
  if (includePath) {
    cmSystemTools::ExpandListArgument(includePath, this->IncludePath);
  } else {
    // Fallback to the old directory level variable if no per-target var:
    includePathVar = "CMAKE_";
    includePathVar += lang;
    includePathVar += "_INCLUDE_PATH";
    includePath = mf->GetDefinition(includePathVar);
    if (includePath) {
      cmSystemTools::ExpandListArgument(includePath, this->IncludePath);
    }
  }
}
