/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDepends.h"

#include <utility>

#include "cmsys/FStream.hxx"

#include "cmFileTime.h"
#include "cmFileTimeCache.h"
#include "cmGeneratedFileStream.h"
#include "cmList.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

cmDepends::cmDepends(cmLocalUnixMakefileGenerator3* lg, std::string targetDir)
  : LocalGenerator(lg)
  , TargetDirectory(std::move(targetDir))
{
}

cmDepends::~cmDepends() = default;

bool cmDepends::Write(std::ostream& makeDepends, std::ostream& internalDepends)
{
  std::map<std::string, std::set<std::string>> dependencies;
  {
    // Lookup the set of sources to scan.
    cmList pairs;
    {
      std::string const srcLang = "CMAKE_DEPENDS_CHECK_" + this->Language;
      cmMakefile* mf = this->LocalGenerator->GetMakefile();
      pairs.assign(mf->GetSafeDefinition(srcLang));
    }
    for (auto si = pairs.begin(); si != pairs.end();) {
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
                      DependencyMap& validDeps)
{
  // Check whether dependencies must be regenerated.
  bool okay = true;
  cmsys::ifstream fin(internalFile.c_str());
  if (!(fin && this->CheckDependencies(fin, internalFile, validDeps))) {
    // Clear all dependencies so they will be regenerated.
    this->Clear(makeFile);
    cmSystemTools::RemoveFile(internalFile);
    this->FileTimeCache->Remove(internalFile);
    okay = false;
  }
  return okay;
}

void cmDepends::Clear(const std::string& file) const
{
  // Print verbose output.
  if (this->Verbose) {
    cmSystemTools::Stdout(
      cmStrCat("Clearing dependencies in \"", file, "\".\n"));
  }

  // Write an empty dependency file.
  cmGeneratedFileStream depFileStream(file);
  depFileStream << "# Empty dependencies file\n"
                   "# This may be replaced when dependencies are built.\n";
}

bool cmDepends::WriteDependencies(const std::set<std::string>& /*unused*/,
                                  const std::string& /*unused*/,
                                  std::ostream& /*unused*/,
                                  std::ostream& /*unused*/)
{
  // This should be implemented by the subclass.
  return false;
}

bool cmDepends::CheckDependencies(std::istream& internalDepends,
                                  const std::string& internalDependsFileName,
                                  DependencyMap& validDeps)
{
  // Read internal depends file time
  cmFileTime internalDependsTime;
  if (!this->FileTimeCache->Load(internalDependsFileName,
                                 internalDependsTime)) {
    return false;
  }

  // Parse dependencies from the stream.  If any dependee is missing
  // or newer than the depender then dependencies should be
  // regenerated.
  bool okay = true;
  bool dependerExists = false;

  std::string line;
  line.reserve(1024);
  std::string depender;
  std::string dependee;
  cmFileTime dependerTime;
  cmFileTime dependeeTime;
  std::vector<std::string>* currentDependencies = nullptr;

  while (std::getline(internalDepends, line)) {
    // Check if this an empty or a comment line
    if (line.empty() || line.front() == '#') {
      continue;
    }
    // Drop carriage return character at the end
    if (line.back() == '\r') {
      line.pop_back();
      if (line.empty()) {
        continue;
      }
    }
    // Check if this a depender line
    if (line.front() != ' ') {
      depender = line;
      dependerExists = this->FileTimeCache->Load(depender, dependerTime);
      // If we erase validDeps[this->Depender] by overwriting it with an empty
      // vector, we lose dependencies for dependers that have multiple
      // entries. No need to initialize the entry, std::map will do so on first
      // access.
      currentDependencies = &validDeps[depender];
      continue;
    }

    // This is a dependee line
    dependee = line.substr(1);

    // Add dependee to depender's list
    if (currentDependencies != nullptr) {
      currentDependencies->push_back(dependee);
    }

    // Dependencies must be regenerated
    // * if the dependee does not exist
    // * if the depender exists and is older than the dependee.
    // * if the depender does not exist, but the dependee is newer than the
    //   depends file
    bool regenerate = false;
    bool dependeeExists = this->FileTimeCache->Load(dependee, dependeeTime);
    if (!dependeeExists) {
      // The dependee does not exist.
      regenerate = true;

      // Print verbose output.
      if (this->Verbose) {
        cmSystemTools::Stdout(cmStrCat("Dependee \"", dependee,
                                       "\" does not exist for depender \"",
                                       depender, "\".\n"));
      }
    } else if (dependerExists) {
      // The dependee and depender both exist.  Compare file times.
      if (dependerTime.Older(dependeeTime)) {
        // The depender is older than the dependee.
        regenerate = true;

        // Print verbose output.
        if (this->Verbose) {
          cmSystemTools::Stdout(cmStrCat("Dependee \"", dependee,
                                         "\" is newer than depender \"",
                                         depender, "\".\n"));
        }
      }
    } else {
      // The dependee exists, but the depender doesn't. Regenerate if the
      // internalDepends file is older than the dependee.
      if (internalDependsTime.Older(dependeeTime)) {
        // The depends-file is older than the dependee.
        regenerate = true;

        // Print verbose output.
        if (this->Verbose) {
          cmSystemTools::Stdout(cmStrCat("Dependee \"", dependee,
                                         "\" is newer than depends file \"",
                                         internalDependsFileName, "\".\n"));
        }
      }
    }

    if (regenerate) {
      // Dependencies must be regenerated.
      okay = false;

      // Remove the information of this depender from the map, it needs
      // to be rescanned
      if (currentDependencies != nullptr) {
        validDeps.erase(depender);
        currentDependencies = nullptr;
      }

      // Remove the depender to be sure it is rebuilt.
      if (dependerExists) {
        cmSystemTools::RemoveFile(depender);
        this->FileTimeCache->Remove(depender);
        dependerExists = false;
      }
    }
  }

  return okay;
}

void cmDepends::SetIncludePathFromLanguage(const std::string& lang)
{
  // Look for the new per "TARGET_" variant first:
  std::string includePathVar =
    cmStrCat("CMAKE_", lang, "_TARGET_INCLUDE_PATH");
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  cmValue includePath = mf->GetDefinition(includePathVar);
  if (includePath) {
    cmExpandList(*includePath, this->IncludePath);
  } else {
    // Fallback to the old directory level variable if no per-target var:
    includePathVar = cmStrCat("CMAKE_", lang, "_INCLUDE_PATH");
    includePath = mf->GetDefinition(includePathVar);
    if (includePath) {
      cmExpandList(*includePath, this->IncludePath);
    }
  }
}
