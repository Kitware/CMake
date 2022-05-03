/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFindProgramCommand.h"

#include <algorithm>
#include <string>
#include <utility>

#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmWindowsRegistry.h"

class cmExecutionStatus;

#if defined(__APPLE__)
#  include <CoreFoundation/CoreFoundation.h>
#endif

struct cmFindProgramHelper
{
  cmFindProgramHelper(std::string debugName, cmMakefile* makefile,
                      cmFindBase const* base)
    : DebugSearches(std::move(debugName), base)
    , Makefile(makefile)
    , PolicyCMP0109(makefile->GetPolicyStatus(cmPolicies::CMP0109))
  {
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
    // Consider platform-specific extensions.
    this->Extensions.push_back(".com");
    this->Extensions.push_back(".exe");
#endif
    // Consider original name with no extensions.
    this->Extensions.emplace_back();
  }

  // List of valid extensions.
  std::vector<std::string> Extensions;

  // Keep track of the best program file found so far.
  std::string BestPath;

  // Current names under consideration.
  std::vector<std::string> Names;

  // Current name with extension under consideration.
  std::string TestNameExt;

  // Current full path under consideration.
  std::string TestPath;

  // Debug state
  cmFindBaseDebugState DebugSearches;
  cmMakefile* Makefile;

  cmPolicies::PolicyStatus PolicyCMP0109;

  void AddName(std::string const& name) { this->Names.push_back(name); }
  void SetName(std::string const& name)
  {
    this->Names.clear();
    this->AddName(name);
  }
  bool CheckCompoundNames()
  {
    return std::any_of(this->Names.begin(), this->Names.end(),
                       [this](std::string const& n) -> bool {
                         // Only perform search relative to current directory
                         // if the file name contains a directory separator.
                         return n.find('/') != std::string::npos &&
                           this->CheckDirectoryForName("", n);
                       });
  }
  bool CheckDirectory(std::string const& path)
  {
    return std::any_of(this->Names.begin(), this->Names.end(),
                       [this, &path](std::string const& n) -> bool {
                         // Only perform search relative to current directory
                         // if the file name contains a directory separator.
                         return this->CheckDirectoryForName(path, n);
                       });
  }
  bool CheckDirectoryForName(std::string const& path, std::string const& name)
  {
    return std::any_of(this->Extensions.begin(), this->Extensions.end(),
                       [this, &path, &name](std::string const& ext) -> bool {
                         if (!ext.empty() && cmHasSuffix(name, ext)) {
                           return false;
                         }
                         this->TestNameExt = cmStrCat(name, ext);
                         this->TestPath = cmSystemTools::CollapseFullPath(
                           this->TestNameExt, path);
                         bool exists = this->FileIsExecutable(this->TestPath);
                         exists ? this->DebugSearches.FoundAt(this->TestPath)
                                : this->DebugSearches.FailedAt(this->TestPath);
                         if (exists) {
                           this->BestPath = this->TestPath;
                           return true;
                         }
                         return false;
                       });
  }
  bool FileIsExecutable(std::string const& file) const
  {
#ifdef _WIN32
    if (!this->FileIsExecutableCMP0109(file)) {
      return false;
    }
    // Pretend the Windows "python" app installer alias does not exist.
    if (cmSystemTools::LowerCase(file).find("/windowsapps/python") !=
        std::string::npos) {
      std::string dest;
      if (cmSystemTools::ReadSymlink(file, dest) &&
          cmHasLiteralSuffix(dest, "\\AppInstallerPythonRedirector.exe")) {
        return false;
      }
    }
    return true;
#else
    return this->FileIsExecutableCMP0109(file);
#endif
  }
  bool FileIsExecutableCMP0109(std::string const& file) const
  {
    switch (this->PolicyCMP0109) {
      case cmPolicies::OLD:
        return cmSystemTools::FileExists(file, true);
      case cmPolicies::NEW:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
        return cmSystemTools::FileIsExecutable(file);
      default:
        break;
    }
    bool const isExeOld = cmSystemTools::FileExists(file, true);
    bool const isExeNew = cmSystemTools::FileIsExecutable(file);
    if (isExeNew == isExeOld) {
      return isExeNew;
    }
    if (isExeNew) {
      this->Makefile->IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0109),
                 "\n"
                 "The file\n"
                 "  ",
                 file,
                 "\n"
                 "is executable but not readable.  "
                 "CMake is ignoring it for compatibility."));
    } else {
      this->Makefile->IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0109),
                 "\n"
                 "The file\n"
                 "  ",
                 file,
                 "\n"
                 "is readable but not executable.  "
                 "CMake is using it for compatibility."));
    }
    return isExeOld;
  }
};

cmFindProgramCommand::cmFindProgramCommand(cmExecutionStatus& status)
  : cmFindBase("find_program", status)
{
  this->NamesPerDirAllowed = true;
  this->VariableDocumentation = "Path to a program.";
  this->VariableType = cmStateEnums::FILEPATH;
  // Windows Registry views
  // When policy CMP0134 is not NEW, rely on previous behavior:
  if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0134) !=
      cmPolicies::NEW) {
    if (this->Makefile->GetDefinition("CMAKE_SIZEOF_VOID_P") == "8") {
      this->RegistryView = cmWindowsRegistry::View::Reg64_32;
    } else {
      this->RegistryView = cmWindowsRegistry::View::Reg32_64;
    }
  } else {
    this->RegistryView = cmWindowsRegistry::View::Both;
  }
}

// cmFindProgramCommand
bool cmFindProgramCommand::InitialPass(std::vector<std::string> const& argsIn)
{

  this->CMakePathName = "PROGRAM";

  // call cmFindBase::ParseArguments
  if (!this->ParseArguments(argsIn)) {
    return false;
  }
  this->DebugMode = this->ComputeIfDebugModeWanted(this->VariableName);

  if (this->AlreadyDefined) {
    this->NormalizeFindResult();
    return true;
  }

  std::string const result = this->FindProgram();
  this->StoreFindResult(result);
  return true;
}

std::string cmFindProgramCommand::FindProgram()
{
  std::string program;

  if (this->SearchAppBundleFirst || this->SearchAppBundleOnly) {
    program = this->FindAppBundle();
  }
  if (program.empty() && !this->SearchAppBundleOnly) {
    program = this->FindNormalProgram();
  }

  if (program.empty() && this->SearchAppBundleLast) {
    program = this->FindAppBundle();
  }
  return program;
}

std::string cmFindProgramCommand::FindNormalProgram()
{
  if (this->NamesPerDir) {
    return this->FindNormalProgramNamesPerDir();
  }
  return this->FindNormalProgramDirsPerName();
}

std::string cmFindProgramCommand::FindNormalProgramNamesPerDir()
{
  // Search for all names in each directory.
  cmFindProgramHelper helper(this->FindCommandName, this->Makefile, this);
  for (std::string const& n : this->Names) {
    helper.AddName(n);
  }

  // Check for the names themselves if they contain a directory separator.
  if (helper.CheckCompoundNames()) {
    return helper.BestPath;
  }

  // Search every directory.
  for (std::string const& sp : this->SearchPaths) {
    if (helper.CheckDirectory(sp)) {
      return helper.BestPath;
    }
  }
  // Couldn't find the program.
  return "";
}

std::string cmFindProgramCommand::FindNormalProgramDirsPerName()
{
  // Search the entire path for each name.
  cmFindProgramHelper helper(this->FindCommandName, this->Makefile, this);
  for (std::string const& n : this->Names) {
    // Switch to searching for this name.
    helper.SetName(n);

    // Check for the names themselves if they contain a directory separator.
    if (helper.CheckCompoundNames()) {
      return helper.BestPath;
    }

    // Search every directory.
    for (std::string const& sp : this->SearchPaths) {
      if (helper.CheckDirectory(sp)) {
        return helper.BestPath;
      }
    }
  }
  // Couldn't find the program.
  return "";
}

std::string cmFindProgramCommand::FindAppBundle()
{
  for (std::string const& name : this->Names) {

    std::string appName = name + std::string(".app");
    std::string appPath =
      cmSystemTools::FindDirectory(appName, this->SearchPaths, true);

    if (!appPath.empty()) {
      std::string executable = this->GetBundleExecutable(appPath);
      if (!executable.empty()) {
        return cmSystemTools::CollapseFullPath(executable);
      }
    }
  }

  // Couldn't find app bundle
  return "";
}

std::string cmFindProgramCommand::GetBundleExecutable(
  std::string const& bundlePath)
{
  std::string executable;
  (void)bundlePath;
#if defined(__APPLE__)
  // Started with an example on developer.apple.com about finding bundles
  // and modified from that.

  // Get a CFString of the app bundle path
  // XXX - Is it safe to assume everything is in UTF8?
  CFStringRef bundlePathCFS = CFStringCreateWithCString(
    kCFAllocatorDefault, bundlePath.c_str(), kCFStringEncodingUTF8);

  // Make a CFURLRef from the CFString representation of the
  // bundle’s path.
  CFURLRef bundleURL = CFURLCreateWithFileSystemPath(
    kCFAllocatorDefault, bundlePathCFS, kCFURLPOSIXPathStyle, true);

  // Make a bundle instance using the URLRef.
  CFBundleRef appBundle = CFBundleCreate(kCFAllocatorDefault, bundleURL);

  // returned executableURL is relative to <appbundle>/Contents/MacOS/
  CFURLRef executableURL = CFBundleCopyExecutableURL(appBundle);

  if (executableURL != nullptr) {
    const int MAX_OSX_PATH_SIZE = 1024;
    UInt8 buffer[MAX_OSX_PATH_SIZE];

    if (CFURLGetFileSystemRepresentation(executableURL, false, buffer,
                                         MAX_OSX_PATH_SIZE)) {
      executable = bundlePath + "/Contents/MacOS/" +
        std::string(reinterpret_cast<char*>(buffer));
    }
    // Only release CFURLRef if it's not null
    CFRelease(executableURL);
  }

  // Any CF objects returned from functions with "create" or
  // "copy" in their names must be released by us!
  CFRelease(bundlePathCFS);
  CFRelease(bundleURL);
  CFRelease(appBundle);
#endif

  return executable;
}

bool cmFindProgram(std::vector<std::string> const& args,
                   cmExecutionStatus& status)
{
  return cmFindProgramCommand(status).InitialPass(args);
}
