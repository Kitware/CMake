/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFindLibraryCommand.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <set>
#include <utility>

#include "cmsys/RegularExpression.hxx"

#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

class cmExecutionStatus;

cmFindLibraryCommand::cmFindLibraryCommand(cmExecutionStatus& status)
  : cmFindBase("find_library", status)
{
  this->EnvironmentPath = "LIB";
  this->NamesPerDirAllowed = true;
  this->VariableDocumentation = "Path to a library.";
  this->VariableType = cmStateEnums::FILEPATH;
}

// cmFindLibraryCommand
bool cmFindLibraryCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  this->CMakePathName = "LIBRARY";

  if (!this->ParseArguments(argsIn)) {
    return false;
  }

  this->DebugMode = this->ComputeIfDebugModeWanted(this->VariableName);

  if (this->AlreadyDefined) {
    this->NormalizeFindResult();
    return true;
  }

  // add custom lib<qual> paths instead of using fixed lib32, lib64 or
  // libx32
  if (cmValue customLib = this->Makefile->GetDefinition(
        "CMAKE_FIND_LIBRARY_CUSTOM_LIB_SUFFIX")) {
    this->AddArchitecturePaths(customLib->c_str());
  }
  // add special 32 bit paths if this is a 32 bit compile.
  else if (this->Makefile->PlatformIs32Bit() &&
           this->Makefile->GetState()->GetGlobalPropertyAsBool(
             "FIND_LIBRARY_USE_LIB32_PATHS")) {
    this->AddArchitecturePaths("32");
  }
  // add special 64 bit paths if this is a 64 bit compile.
  else if (this->Makefile->PlatformIs64Bit() &&
           this->Makefile->GetState()->GetGlobalPropertyAsBool(
             "FIND_LIBRARY_USE_LIB64_PATHS")) {
    this->AddArchitecturePaths("64");
  }
  // add special 32 bit paths if this is an x32 compile.
  else if (this->Makefile->PlatformIsx32() &&
           this->Makefile->GetState()->GetGlobalPropertyAsBool(
             "FIND_LIBRARY_USE_LIBX32_PATHS")) {
    this->AddArchitecturePaths("x32");
  }

  std::string const library = this->FindLibrary();
  this->StoreFindResult(library);
  return true;
}

void cmFindLibraryCommand::AddArchitecturePaths(const char* suffix)
{
  std::vector<std::string> original;
  original.swap(this->SearchPaths);
  for (std::string const& o : original) {
    this->AddArchitecturePath(o, 0, suffix);
    if (this->DebugMode) {
      std::string msg = cmStrCat(
        "find_library(", this->VariableName, ") removed original suffix ", o,
        " from PATH_SUFFIXES while adding architecture paths for suffix '",
        suffix, "'");
      this->DebugMessage(msg);
    }
  }
}

static bool cmLibDirsLinked(std::string const& l, std::string const& r)
{
  // Compare the real paths of the two directories.
  // Since our caller only changed the trailing component of each
  // directory, the real paths can be the same only if at least one of
  // the trailing components is a symlink.  Use this as an optimization
  // to avoid excessive realpath calls.
  return (cmSystemTools::FileIsSymlink(l) ||
          cmSystemTools::FileIsSymlink(r)) &&
    cmSystemTools::GetRealPath(l) == cmSystemTools::GetRealPath(r);
}

void cmFindLibraryCommand::AddArchitecturePath(
  std::string const& dir, std::string::size_type start_pos, const char* suffix,
  bool fresh)
{
  std::string::size_type pos = dir.find("lib/", start_pos);

  if (pos != std::string::npos) {
    // Check for "lib".
    std::string lib = dir.substr(0, pos + 3);
    bool use_lib = cmSystemTools::FileIsDirectory(lib);

    // Check for "lib<suffix>" and use it first.
    std::string libX = lib + suffix;
    bool use_libX = cmSystemTools::FileIsDirectory(libX);

    // Avoid copies of the same directory due to symlinks.
    if (use_libX && use_lib && cmLibDirsLinked(libX, lib)) {
      use_libX = false;
    }

    if (use_libX) {
      libX += dir.substr(pos + 3);
      std::string::size_type libX_pos = pos + 3 + strlen(suffix) + 1;
      this->AddArchitecturePath(libX, libX_pos, suffix);
    }

    if (use_lib) {
      this->AddArchitecturePath(dir, pos + 3 + 1, suffix, false);
    }
  }

  if (fresh) {
    // Check for the original unchanged path.
    bool use_dir = cmSystemTools::FileIsDirectory(dir);

    // Check for <dir><suffix>/ and use it first.
    std::string dirX = dir + suffix;
    bool use_dirX = cmSystemTools::FileIsDirectory(dirX);

    // Avoid copies of the same directory due to symlinks.
    if (use_dirX && use_dir && cmLibDirsLinked(dirX, dir)) {
      use_dirX = false;
    }

    if (use_dirX) {
      dirX += "/";
      if (this->DebugMode) {
        std::string msg = cmStrCat(
          "find_library(", this->VariableName, ") added replacement path ",
          dirX, " to PATH_SUFFIXES for architecture suffix '", suffix, "'");
        this->DebugMessage(msg);
      }
      this->SearchPaths.push_back(std::move(dirX));
    }

    if (use_dir) {
      this->SearchPaths.push_back(dir);
      if (this->DebugMode) {
        std::string msg = cmStrCat(
          "find_library(", this->VariableName, ") added replacement path ",
          dir, " to PATH_SUFFIXES for architecture suffix '", suffix, "'");
        this->DebugMessage(msg);
      }
    }
  }
}

std::string cmFindLibraryCommand::FindLibrary()
{
  std::string library;
  if (this->SearchFrameworkFirst || this->SearchFrameworkOnly) {
    library = this->FindFrameworkLibrary();
  }
  if (library.empty() && !this->SearchFrameworkOnly) {
    library = this->FindNormalLibrary();
  }
  if (library.empty() && this->SearchFrameworkLast) {
    library = this->FindFrameworkLibrary();
  }
  return library;
}

struct cmFindLibraryHelper
{
  cmFindLibraryHelper(std::string debugName, cmMakefile* mf,
                      cmFindBase const* findBase);

  // Context information.
  cmMakefile* Makefile;
  cmFindBase const* FindBase;
  cmGlobalGenerator* GG;

  // List of valid prefixes and suffixes.
  cmList Prefixes;
  cmList Suffixes;
  std::string PrefixRegexStr;
  std::string SuffixRegexStr;

  // Keep track of the best library file found so far.
  using size_type = std::vector<std::string>::size_type;
  std::string BestPath;

  // Support for OpenBSD shared library naming: lib<name>.so.<major>.<minor>
  bool OpenBSD;

  bool DebugMode;

  // Current names under consideration.
  struct Name
  {
    bool TryRaw = false;
    std::string Raw;
    cmsys::RegularExpression Regex;
  };
  std::vector<Name> Names;

  // Current full path under consideration.
  std::string TestPath;

  void RegexFromLiteral(std::string& out, std::string const& in);
  void RegexFromList(std::string& out, cmList const& in);
  size_type GetPrefixIndex(std::string const& prefix)
  {
    return std::find(this->Prefixes.begin(), this->Prefixes.end(), prefix) -
      this->Prefixes.begin();
  }
  size_type GetSuffixIndex(std::string const& suffix)
  {
    return std::find(this->Suffixes.begin(), this->Suffixes.end(), suffix) -
      this->Suffixes.begin();
  }
  bool HasValidSuffix(std::string const& name);
  void AddName(std::string const& name);
  void SetName(std::string const& name);
  bool CheckDirectory(std::string const& path);
  bool CheckDirectoryForName(std::string const& path, Name& name);

  bool Validate(const std::string& path) const
  {
    return this->FindBase->Validate(path);
  }

  cmFindBaseDebugState DebugSearches;

  void DebugLibraryFailed(std::string const& name, std::string const& path)
  {
    if (this->DebugMode) {
      auto regexName =
        cmStrCat(this->PrefixRegexStr, name, this->SuffixRegexStr);
      this->DebugSearches.FailedAt(path, regexName);
    }
  }

  void DebugLibraryFound(std::string const& name, std::string const& path)
  {
    if (this->DebugMode) {
      auto regexName =
        cmStrCat(this->PrefixRegexStr, name, this->SuffixRegexStr);
      this->DebugSearches.FoundAt(path, regexName);
    }
  }
};

namespace {

std::string const& get_prefixes(cmMakefile* mf)
{
#ifdef _WIN32
  static std::string defaultPrefix = ";lib";
#else
  static std::string defaultPrefix = "lib";
#endif
  cmValue prefixProp = mf->GetDefinition("CMAKE_FIND_LIBRARY_PREFIXES");
  return (prefixProp) ? *prefixProp : defaultPrefix;
}

std::string const& get_suffixes(cmMakefile* mf)
{
#ifdef _WIN32
  static std::string defaultSuffix = ".lib;.dll.a;.a";
#elif defined(__APPLE__)
  static std::string defaultSuffix = ".tbd;.dylib;.so;.a";
#elif defined(__hpux)
  static std::string defaultSuffix = ".sl;.so;.a";
#else
  static std::string defaultSuffix = ".so;.a";
#endif
  cmValue suffixProp = mf->GetDefinition("CMAKE_FIND_LIBRARY_SUFFIXES");
  return (suffixProp) ? *suffixProp : defaultSuffix;
}
}
cmFindLibraryHelper::cmFindLibraryHelper(std::string debugName, cmMakefile* mf,
                                         cmFindBase const* base)
  : Makefile(mf)
  , FindBase(base)
  , DebugMode(base->DebugModeEnabled())
  , DebugSearches(std::move(debugName), base)
{
  this->GG = this->Makefile->GetGlobalGenerator();

  // Collect the list of library name prefixes/suffixes to try.
  std::string const& prefixes_list = get_prefixes(this->Makefile);
  std::string const& suffixes_list = get_suffixes(this->Makefile);

  this->Prefixes.assign(prefixes_list, cmList::EmptyElements::Yes);
  this->Suffixes.assign(suffixes_list, cmList::EmptyElements::Yes);
  this->RegexFromList(this->PrefixRegexStr, this->Prefixes);
  this->RegexFromList(this->SuffixRegexStr, this->Suffixes);

  // Check whether to use OpenBSD-style library version comparisons.
  this->OpenBSD = this->Makefile->GetState()->GetGlobalPropertyAsBool(
    "FIND_LIBRARY_USE_OPENBSD_VERSIONING");
}

void cmFindLibraryHelper::RegexFromLiteral(std::string& out,
                                           std::string const& in)
{
  for (char ch : in) {
    if (ch == '[' || ch == ']' || ch == '(' || ch == ')' || ch == '\\' ||
        ch == '.' || ch == '*' || ch == '+' || ch == '?' || ch == '-' ||
        ch == '^' || ch == '$') {
      out += "\\";
    }
#if defined(_WIN32) || defined(__APPLE__)
    out += static_cast<char>(tolower(ch));
#else
    out += ch;
#endif
  }
}

void cmFindLibraryHelper::RegexFromList(std::string& out, cmList const& in)
{
  // Surround the list in parens so the '|' does not apply to anything
  // else and the result can be checked after matching.
  out += "(";
  const char* sep = "";
  for (auto const& s : in) {
    // Separate from previous item.
    out += sep;
    sep = "|";

    // Append this item.
    this->RegexFromLiteral(out, s);
  }
  out += ")";
}

bool cmFindLibraryHelper::HasValidSuffix(std::string const& name)
{
  for (std::string suffix : this->Suffixes) {
    if (name.length() <= suffix.length()) {
      continue;
    }
    // Check if the given name ends in a valid library suffix.
    if (name.substr(name.size() - suffix.length()) == suffix) {
      return true;
    }
    // Check if a valid library suffix is somewhere in the name,
    // this may happen e.g. for versioned shared libraries: libfoo.so.2
    suffix += ".";
    if (name.find(suffix) != std::string::npos) {
      return true;
    }
  }
  return false;
}

void cmFindLibraryHelper::AddName(std::string const& name)
{
  Name entry;

  // Consider checking the raw name too.
  entry.TryRaw = this->HasValidSuffix(name);
  entry.Raw = name;

  // Build a regular expression to match library names.
  std::string regex = cmStrCat('^', this->PrefixRegexStr);
  this->RegexFromLiteral(regex, name);
  regex += this->SuffixRegexStr;
  if (this->OpenBSD) {
    regex += "(\\.[0-9]+\\.[0-9]+)?";
  }
  regex += "$";
  entry.Regex.compile(regex);
  this->Names.push_back(std::move(entry));
}

void cmFindLibraryHelper::SetName(std::string const& name)
{
  this->Names.clear();
  this->AddName(name);
}

bool cmFindLibraryHelper::CheckDirectory(std::string const& path)
{
  for (Name& i : this->Names) {
    if (this->CheckDirectoryForName(path, i)) {
      return true;
    }
  }
  return false;
}

bool cmFindLibraryHelper::CheckDirectoryForName(std::string const& path,
                                                Name& name)
{
  // If the original library name provided by the user matches one of
  // the suffixes, try it first.  This allows users to search
  // specifically for a static library on some platforms (on MS tools
  // one cannot tell just from the library name whether it is a static
  // library or an import library).
  if (name.TryRaw) {
    this->TestPath = cmStrCat(path, name.Raw);

    const bool exists = cmSystemTools::FileExists(this->TestPath, true);
    if (!exists) {
      this->DebugLibraryFailed(name.Raw, path);
    } else {
      auto testPath = cmSystemTools::CollapseFullPath(this->TestPath);
      if (this->Validate(testPath)) {
        this->DebugLibraryFound(name.Raw, path);
        this->BestPath = testPath;
        return true;
      }
      this->DebugLibraryFailed(name.Raw, path);
    }
  }

  // No library file has yet been found.
  size_type bestPrefix = this->Prefixes.size();
  size_type bestSuffix = this->Suffixes.size();
  unsigned int bestMajor = 0;
  unsigned int bestMinor = 0;

  // Search for a file matching the library name regex.
  std::string dir = path;
  cmSystemTools::ConvertToUnixSlashes(dir);
  std::set<std::string> const& files = this->GG->GetDirectoryContent(dir);
  for (std::string const& origName : files) {
#if defined(_WIN32) || defined(__APPLE__)
    std::string testName = cmSystemTools::LowerCase(origName);
#else
    std::string const& testName = origName;
#endif
    if (name.Regex.find(testName)) {
      this->TestPath = cmStrCat(path, origName);
      // Make sure the path is readable and is not a directory.
      if (cmSystemTools::FileExists(this->TestPath, true)) {
        if (!this->Validate(cmSystemTools::CollapseFullPath(this->TestPath))) {
          continue;
        }

        this->DebugLibraryFound(name.Raw, dir);
        // This is a matching file.  Check if it is better than the
        // best name found so far.  Earlier prefixes are preferred,
        // followed by earlier suffixes.  For OpenBSD, shared library
        // version extensions are compared.
        size_type prefix = this->GetPrefixIndex(name.Regex.match(1));
        size_type suffix = this->GetSuffixIndex(name.Regex.match(2));
        unsigned int major = 0;
        unsigned int minor = 0;
        if (this->OpenBSD) {
          sscanf(name.Regex.match(3).c_str(), ".%u.%u", &major, &minor);
        }
        if (this->BestPath.empty() || prefix < bestPrefix ||
            (prefix == bestPrefix && suffix < bestSuffix) ||
            (prefix == bestPrefix && suffix == bestSuffix &&
             (major > bestMajor ||
              (major == bestMajor && minor > bestMinor)))) {
          this->BestPath = this->TestPath;
          bestPrefix = prefix;
          bestSuffix = suffix;
          bestMajor = major;
          bestMinor = minor;
        }
      }
    }
  }

  if (this->BestPath.empty()) {
    this->DebugLibraryFailed(name.Raw, dir);
  } else {
    this->DebugLibraryFound(name.Raw, this->BestPath);
  }

  // Use the best candidate found in this directory, if any.
  return !this->BestPath.empty();
}

std::string cmFindLibraryCommand::FindNormalLibrary()
{
  if (this->NamesPerDir) {
    return this->FindNormalLibraryNamesPerDir();
  }
  return this->FindNormalLibraryDirsPerName();
}

std::string cmFindLibraryCommand::FindNormalLibraryNamesPerDir()
{
  // Search for all names in each directory.
  cmFindLibraryHelper helper(this->FindCommandName, this->Makefile, this);
  for (std::string const& n : this->Names) {
    helper.AddName(n);
  }
  // Search every directory.
  for (std::string const& sp : this->SearchPaths) {
    if (helper.CheckDirectory(sp)) {
      return helper.BestPath;
    }
  }
  // Couldn't find the library.
  return "";
}

std::string cmFindLibraryCommand::FindNormalLibraryDirsPerName()
{
  // Search the entire path for each name.
  cmFindLibraryHelper helper(this->FindCommandName, this->Makefile, this);
  for (std::string const& n : this->Names) {
    // Switch to searching for this name.
    helper.SetName(n);

    // Search every directory.
    for (std::string const& sp : this->SearchPaths) {
      if (helper.CheckDirectory(sp)) {
        return helper.BestPath;
      }
    }
  }
  // Couldn't find the library.
  return "";
}

std::string cmFindLibraryCommand::FindFrameworkLibrary()
{
  if (this->NamesPerDir) {
    return this->FindFrameworkLibraryNamesPerDir();
  }
  return this->FindFrameworkLibraryDirsPerName();
}

std::string cmFindLibraryCommand::FindFrameworkLibraryNamesPerDir()
{
  std::string fwPath;
  // Search for all names in each search path.
  for (std::string const& d : this->SearchPaths) {
    for (std::string const& n : this->Names) {
      fwPath = cmStrCat(d, n, ".framework");
      if (cmSystemTools::FileIsDirectory(fwPath)) {
        auto finalPath = cmSystemTools::CollapseFullPath(fwPath);
        if (this->Validate(finalPath)) {
          return finalPath;
        }
      }
    }
  }

  // No framework found.
  return "";
}

std::string cmFindLibraryCommand::FindFrameworkLibraryDirsPerName()
{
  std::string fwPath;
  // Search for each name in all search paths.
  for (std::string const& n : this->Names) {
    for (std::string const& d : this->SearchPaths) {
      fwPath = cmStrCat(d, n, ".framework");
      if (cmSystemTools::FileIsDirectory(fwPath)) {
        auto finalPath = cmSystemTools::CollapseFullPath(fwPath);
        if (this->Validate(finalPath)) {
          return finalPath;
        }
      }
    }
  }

  // No framework found.
  return "";
}

bool cmFindLibrary(std::vector<std::string> const& args,
                   cmExecutionStatus& status)
{
  return cmFindLibraryCommand(status).InitialPass(args);
}
