/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmRuntimeDependencyArchive.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>

#include "cmBinUtilsLinuxELFLinker.h"
#include "cmBinUtilsMacOSMachOLinker.h"
#include "cmBinUtilsWindowsPELinker.h"
#include "cmExecutionStatus.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"

#if defined(_WIN32)
#  include "cmGlobalGenerator.h"
#  ifndef CMAKE_BOOTSTRAP
#    include "cmGlobalVisualStudioVersionedGenerator.h"
#  endif
#  include "cmsys/Glob.hxx"

#  include "cmVSSetupHelper.h"
#endif

#if defined(_WIN32)
static void AddVisualStudioPath(std::vector<std::string>& paths,
                                const std::string& prefix,
                                unsigned int version, cmGlobalGenerator* gg)
{
  // If generating for the VS IDE, use the same instance.
  std::string vsloc;
  bool found = false;
#  ifndef CMAKE_BOOTSTRAP
  if (cmHasPrefix(gg->GetName(), prefix)) {
    cmGlobalVisualStudioVersionedGenerator* vsgen =
      static_cast<cmGlobalVisualStudioVersionedGenerator*>(gg);
    if (vsgen->GetVSInstance(vsloc)) {
      found = true;
    }
  }
#  endif

  // Otherwise, find a VS instance ourselves.
  if (!found) {
    cmVSSetupAPIHelper vsSetupAPIHelper(version);
    if (vsSetupAPIHelper.GetVSInstanceInfo(vsloc)) {
      cmSystemTools::ConvertToUnixSlashes(vsloc);
      found = true;
    }
  }

  if (found) {
    cmsys::Glob glob;
    glob.SetListDirs(true);
    glob.FindFiles(vsloc + "/VC/Tools/MSVC/*");
    for (auto const& vcdir : glob.GetFiles()) {
      paths.push_back(vcdir + "/bin/Hostx64/x64");
      paths.push_back(vcdir + "/bin/Hostx86/x64");
      paths.push_back(vcdir + "/bin/Hostx64/x86");
      paths.push_back(vcdir + "/bin/Hostx86/x86");
    }
  }
}

static void AddRegistryPath(std::vector<std::string>& paths,
                            const std::string& path, cmMakefile* mf)
{
  // We should view the registry as the target application would view
  // it.
  cmSystemTools::KeyWOW64 view = cmSystemTools::KeyWOW64_32;
  cmSystemTools::KeyWOW64 other_view = cmSystemTools::KeyWOW64_64;
  if (mf->PlatformIs64Bit()) {
    view = cmSystemTools::KeyWOW64_64;
    other_view = cmSystemTools::KeyWOW64_32;
  }

  // Expand using the view of the target application.
  std::string expanded = path;
  cmSystemTools::ExpandRegistryValues(expanded, view);
  cmSystemTools::GlobDirs(expanded, paths);

  // Executables can be either 32-bit or 64-bit, so expand using the
  // alternative view.
  expanded = path;
  cmSystemTools::ExpandRegistryValues(expanded, other_view);
  cmSystemTools::GlobDirs(expanded, paths);
}

static void AddEnvPath(std::vector<std::string>& paths, const std::string& var,
                       const std::string& suffix)
{
  std::string value;
  if (cmSystemTools::GetEnv(var, value)) {
    paths.push_back(value + suffix);
  }
}
#endif

static cmsys::RegularExpression TransformCompile(const std::string& str)
{
  return cmsys::RegularExpression(str);
}

cmRuntimeDependencyArchive::cmRuntimeDependencyArchive(
  cmExecutionStatus& status, std::vector<std::string> searchDirectories,
  std::string bundleExecutable,
  const std::vector<std::string>& preIncludeRegexes,
  const std::vector<std::string>& preExcludeRegexes,
  const std::vector<std::string>& postIncludeRegexes,
  const std::vector<std::string>& postExcludeRegexes,
  std::vector<std::string> postIncludeFiles,
  std::vector<std::string> postExcludeFiles,
  std::vector<std::string> postExcludeFilesStrict)
  : Status(status)
  , SearchDirectories(std::move(searchDirectories))
  , BundleExecutable(std::move(bundleExecutable))
  , PreIncludeRegexes(preIncludeRegexes.size())
  , PreExcludeRegexes(preExcludeRegexes.size())
  , PostIncludeRegexes(postIncludeRegexes.size())
  , PostExcludeRegexes(postExcludeRegexes.size())
  , PostIncludeFiles(std::move(postIncludeFiles))
  , PostExcludeFiles(std::move(postExcludeFiles))
  , PostExcludeFilesStrict(std::move(postExcludeFilesStrict))
{
  std::transform(preIncludeRegexes.begin(), preIncludeRegexes.end(),
                 this->PreIncludeRegexes.begin(), TransformCompile);
  std::transform(preExcludeRegexes.begin(), preExcludeRegexes.end(),
                 this->PreExcludeRegexes.begin(), TransformCompile);
  std::transform(postIncludeRegexes.begin(), postIncludeRegexes.end(),
                 this->PostIncludeRegexes.begin(), TransformCompile);
  std::transform(postExcludeRegexes.begin(), postExcludeRegexes.end(),
                 this->PostExcludeRegexes.begin(), TransformCompile);
}

bool cmRuntimeDependencyArchive::Prepare()
{
  std::string platform = this->GetMakefile()->GetSafeDefinition(
    "CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM");
  if (platform.empty()) {
    std::string systemName =
      this->GetMakefile()->GetSafeDefinition("CMAKE_HOST_SYSTEM_NAME");
    if (systemName == "Windows") {
      platform = "windows+pe";
    } else if (systemName == "Darwin") {
      platform = "macos+macho";
    } else if (systemName == "Linux") {
      platform = "linux+elf";
    }
  }
  if (platform == "linux+elf") {
    this->Linker = cm::make_unique<cmBinUtilsLinuxELFLinker>(this);
  } else if (platform == "windows+pe") {
    this->Linker = cm::make_unique<cmBinUtilsWindowsPELinker>(this);
  } else if (platform == "macos+macho") {
    this->Linker = cm::make_unique<cmBinUtilsMacOSMachOLinker>(this);
  } else {
    std::ostringstream e;
    e << "Invalid value for CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM: "
      << platform;
    this->SetError(e.str());
    return false;
  }

  return this->Linker->Prepare();
}

bool cmRuntimeDependencyArchive::GetRuntimeDependencies(
  const std::vector<std::string>& executables,
  const std::vector<std::string>& libraries,
  const std::vector<std::string>& modules)
{
  for (auto const& exe : executables) {
    if (!this->Linker->ScanDependencies(exe, cmStateEnums::EXECUTABLE)) {
      return false;
    }
  }
  for (auto const& lib : libraries) {
    if (!this->Linker->ScanDependencies(lib, cmStateEnums::SHARED_LIBRARY)) {
      return false;
    }
  }
  return std::all_of(
    modules.begin(), modules.end(), [this](std::string const& mod) -> bool {
      return this->Linker->ScanDependencies(mod, cmStateEnums::MODULE_LIBRARY);
    });
}

void cmRuntimeDependencyArchive::SetError(const std::string& e)
{
  this->Status.SetError(e);
}

const std::string& cmRuntimeDependencyArchive::GetBundleExecutable() const
{
  return this->BundleExecutable;
}

const std::vector<std::string>&
cmRuntimeDependencyArchive::GetSearchDirectories() const
{
  return this->SearchDirectories;
}

const std::string& cmRuntimeDependencyArchive::GetGetRuntimeDependenciesTool()
  const
{
  return this->GetMakefile()->GetSafeDefinition(
    "CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL");
}

bool cmRuntimeDependencyArchive::GetGetRuntimeDependenciesCommand(
  const std::string& search, std::vector<std::string>& command) const
{
  // First see if it was supplied by the user
  std::string toolCommand = this->GetMakefile()->GetSafeDefinition(
    "CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND");
  if (toolCommand.empty() && search == "objdump") {
    toolCommand = this->GetMakefile()->GetSafeDefinition("CMAKE_OBJDUMP");
  }
  if (!toolCommand.empty()) {
    cmExpandList(toolCommand, command);
    return true;
  }

  // Now go searching for it
  std::vector<std::string> paths;
#ifdef _WIN32
  cmGlobalGenerator* gg = this->GetMakefile()->GetGlobalGenerator();

  // Add newer Visual Studio paths
  AddVisualStudioPath(paths, "Visual Studio 17 ", 17, gg);
  AddVisualStudioPath(paths, "Visual Studio 16 ", 16, gg);
  AddVisualStudioPath(paths, "Visual Studio 15 ", 15, gg);

  // Add older Visual Studio paths
  AddRegistryPath(
    paths,
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\14.0;InstallDir]/"
    "../../VC/bin",
    this->GetMakefile());
  AddEnvPath(paths, "VS140COMNTOOLS", "/../../VC/bin");
  paths.push_back(
    "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin");
  AddRegistryPath(
    paths,
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\12.0;InstallDir]/"
    "../../VC/bin",
    this->GetMakefile());
  AddEnvPath(paths, "VS120COMNTOOLS", "/../../VC/bin");
  paths.push_back(
    "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/bin");
  AddRegistryPath(
    paths,
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\11.0;InstallDir]/"
    "../../VC/bin",
    this->GetMakefile());
  AddEnvPath(paths, "VS110COMNTOOLS", "/../../VC/bin");
  paths.push_back(
    "C:/Program Files (x86)/Microsoft Visual Studio 11.0/VC/bin");
  AddRegistryPath(
    paths,
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\10.0;InstallDir]/"
    "../../VC/bin",
    this->GetMakefile());
  AddEnvPath(paths, "VS100COMNTOOLS", "/../../VC/bin");
  paths.push_back(
    "C:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/bin");
  AddRegistryPath(
    paths,
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\9.0;InstallDir]/"
    "../../VC/bin",
    this->GetMakefile());
  AddEnvPath(paths, "VS90COMNTOOLS", "/../../VC/bin");
  paths.push_back("C:/Program Files/Microsoft Visual Studio 9.0/VC/bin");
  paths.push_back("C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/bin");
  AddRegistryPath(
    paths,
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0;InstallDir]/"
    "../../VC/bin",
    this->GetMakefile());
  AddEnvPath(paths, "VS80COMNTOOLS", "/../../VC/bin");
  paths.push_back("C:/Program Files/Microsoft Visual Studio 8/VC/BIN");
  paths.push_back("C:/Program Files (x86)/Microsoft Visual Studio 8/VC/BIN");
  AddRegistryPath(
    paths,
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\7.1;InstallDir]/"
    "../../VC7/bin",
    this->GetMakefile());
  AddEnvPath(paths, "VS71COMNTOOLS", "/../../VC7/bin");
  paths.push_back(
    "C:/Program Files/Microsoft Visual Studio .NET 2003/VC7/BIN");
  paths.push_back(
    "C:/Program Files (x86)/Microsoft Visual Studio .NET 2003/VC7/BIN");
#endif

  std::string program = cmSystemTools::FindProgram(search, paths);
  if (!program.empty()) {
    command = { program };
    return true;
  }

  // Couldn't find it
  return false;
}

bool cmRuntimeDependencyArchive::IsPreExcluded(const std::string& name) const
{
  cmsys::RegularExpressionMatch match;
  auto const regexMatch =
    [&match, name](const cmsys::RegularExpression& regex) -> bool {
    return regex.find(name.c_str(), match);
  };
  auto const regexSearch =
    [&regexMatch](
      const std::vector<cmsys::RegularExpression>& regexes) -> bool {
    return std::any_of(regexes.begin(), regexes.end(), regexMatch);
  };

  return !regexSearch(this->PreIncludeRegexes) &&
    regexSearch(this->PreExcludeRegexes);
}

bool cmRuntimeDependencyArchive::IsPostExcluded(const std::string& name) const
{
  cmsys::RegularExpressionMatch match;
  auto const regexMatch =
    [&match, name](const cmsys::RegularExpression& regex) -> bool {
    return regex.find(name.c_str(), match);
  };
  auto const regexSearch =
    [&regexMatch](
      const std::vector<cmsys::RegularExpression>& regexes) -> bool {
    return std::any_of(regexes.begin(), regexes.end(), regexMatch);
  };
  auto const fileMatch = [name](const std::string& file) -> bool {
    return cmSystemTools::SameFile(file, name);
  };
  auto const fileSearch =
    [&fileMatch](const std::vector<std::string>& files) -> bool {
    return std::any_of(files.begin(), files.end(), fileMatch);
  };

  return fileSearch(this->PostExcludeFilesStrict) ||
    (!(regexSearch(this->PostIncludeRegexes) ||
       fileSearch(this->PostIncludeFiles)) &&
     (regexSearch(this->PostExcludeRegexes) ||
      fileSearch(this->PostExcludeFiles)));
}

void cmRuntimeDependencyArchive::AddResolvedPath(
  const std::string& name, const std::string& path, bool& unique,
  std::vector<std::string> rpaths)
{
  auto it = this->ResolvedPaths.emplace(name, std::set<std::string>{}).first;
  unique = true;
  for (auto const& other : it->second) {
    if (cmSystemTools::SameFile(path, other)) {
      unique = false;
      break;
    }
  }
  it->second.insert(path);
  this->RPaths[path] = std::move(rpaths);
}

void cmRuntimeDependencyArchive::AddUnresolvedPath(const std::string& name)
{
  this->UnresolvedPaths.insert(name);
}

cmMakefile* cmRuntimeDependencyArchive::GetMakefile() const
{
  return &this->Status.GetMakefile();
}

const std::map<std::string, std::set<std::string>>&
cmRuntimeDependencyArchive::GetResolvedPaths() const
{
  return this->ResolvedPaths;
}

const std::set<std::string>& cmRuntimeDependencyArchive::GetUnresolvedPaths()
  const
{
  return this->UnresolvedPaths;
}

const std::map<std::string, std::vector<std::string>>&
cmRuntimeDependencyArchive::GetRPaths() const
{
  return this->RPaths;
}

bool cmRuntimeDependencyArchive::PlatformSupportsRuntimeDependencies(
  const std::string& platform)
{
  static const std::set<std::string> supportedPlatforms = { "Windows", "Linux",
                                                            "Darwin" };
  return supportedPlatforms.count(platform);
}
