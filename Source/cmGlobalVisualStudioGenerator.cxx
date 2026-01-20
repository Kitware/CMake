
/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudioGenerator.h"

#include <algorithm>
#include <cassert>
#include <future>
#include <iostream>
#include <sstream>
#include <system_error>
#include <utility>

#include <cm/iterator>
#include <cm/memory>
#include <cmext/string_view>

#include <windows.h>

#include <objbase.h>
#include <shellapi.h>

#include "cmCallVisualStudioMacro.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmSourceFile.h"
#include "cmSourceGroup.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmUuid.h"
#include "cmake.h"

cmGlobalVisualStudioGenerator::cmGlobalVisualStudioGenerator(cmake* cm)
  : cmGlobalGenerator(cm)
{
  cm->GetState()->SetIsGeneratorMultiConfig(true);
  cm->GetState()->SetWindowsShell(true);
  cm->GetState()->SetWindowsVSIDE(true);
  this->DefaultPlatformName = "Win32";
}

cmGlobalVisualStudioGenerator::~cmGlobalVisualStudioGenerator() = default;

cmGlobalVisualStudioGenerator::VSVersion
cmGlobalVisualStudioGenerator::GetVersion() const
{
  return this->Version;
}

void cmGlobalVisualStudioGenerator::SetVersion(VSVersion v)
{
  this->Version = v;
}

void cmGlobalVisualStudioGenerator::EnableLanguage(
  std::vector<std::string> const& lang, cmMakefile* mf, bool optional)
{
  mf->AddDefinition("CMAKE_VS_PLATFORM_NAME_DEFAULT",
                    this->DefaultPlatformName);
  this->cmGlobalGenerator::EnableLanguage(lang, mf, optional);
}

bool cmGlobalVisualStudioGenerator::SetGeneratorPlatform(std::string const& p,
                                                         cmMakefile* mf)
{
  if (!this->InitializePlatform(mf)) {
    return false;
  }
  if (this->GetPlatformName() == "x64"_s) {
    mf->AddDefinition("CMAKE_FORCE_WIN64", "TRUE");
  } else if (this->GetPlatformName() == "Itanium"_s) {
    mf->AddDefinition("CMAKE_FORCE_IA64", "TRUE");
  }
  mf->AddDefinition("CMAKE_VS_PLATFORM_NAME", this->GetPlatformName());
  return this->cmGlobalGenerator::SetGeneratorPlatform(p, mf);
}

bool cmGlobalVisualStudioGenerator::InitializePlatform(cmMakefile*)
{
  return true;
}

cmValue cmGlobalVisualStudioGenerator::GetDebuggerWorkingDirectory(
  cmGeneratorTarget* gt) const
{
  if (cmValue ret = gt->GetProperty("VS_DEBUGGER_WORKING_DIRECTORY")) {
    return ret;
  } else {
    return cmGlobalGenerator::GetDebuggerWorkingDirectory(gt);
  }
}

std::string const& cmGlobalVisualStudioGenerator::GetPlatformName() const
{
  if (!this->GeneratorPlatform.empty()) {
    return this->GeneratorPlatform;
  }
  return this->DefaultPlatformName;
}

char const* cmGlobalVisualStudioGenerator::GetIDEVersion() const
{
  switch (this->Version) {
    case cmGlobalVisualStudioGenerator::VSVersion::VS14:
      return "14.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS15:
      return "15.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS16:
      return "16.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS17:
      return "17.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS18:
      return "18.0";
  }
  return "";
}

std::string cmGlobalVisualStudioGenerator::GetRegistryBase()
{
  return cmGlobalVisualStudioGenerator::GetRegistryBase(this->GetIDEVersion());
}

std::string cmGlobalVisualStudioGenerator::GetRegistryBase(char const* version)
{
  return cmStrCat(R"(HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\)",
                  version);
}

void cmGlobalVisualStudioGenerator::AddExtraIDETargets()
{
  // Add a special target that depends on ALL projects for easy build
  // of one configuration only.
  for (auto const& it : this->ProjectMap) {
    std::vector<cmLocalGenerator*> const& gen = it.second;
    // add the ALL_BUILD to the first local generator of each project
    if (!gen.empty()) {
      // Use no actual command lines so that the target itself is not
      // considered always out of date.
      auto cc = cm::make_unique<cmCustomCommand>();
      cc->SetEscapeOldStyle(false);
      cc->SetComment("Build all projects");
      cmTarget* allBuild =
        gen[0]->AddUtilityCommand("ALL_BUILD", true, std::move(cc));

      gen[0]->AddGeneratorTarget(
        cm::make_unique<cmGeneratorTarget>(allBuild, gen[0]));

      //
      // Organize in the "predefined targets" folder:
      //
      if (this->UseFolderProperty()) {
        allBuild->SetProperty("FOLDER", this->GetPredefinedTargetsFolder());
      }

      // Now make all targets depend on the ALL_BUILD target
      for (cmLocalGenerator const* i : gen) {
        for (auto const& tgt : i->GetGeneratorTargets()) {
          if (tgt->GetType() == cmStateEnums::GLOBAL_TARGET ||
              tgt->IsImported()) {
            continue;
          }
          if (!this->IsExcluded(gen[0], tgt.get())) {
            allBuild->AddUtility(tgt->GetName(), false);
          }
        }
      }
    }
  }

  // Configure CMake Visual Studio macros, for this user on this version
  // of Visual Studio.
  this->ConfigureCMakeVisualStudioMacros();
}

void cmGlobalVisualStudioGenerator::ComputeTargetObjectDirectory(
  cmGeneratorTarget* gt) const
{
  std::string dir =
    cmStrCat(gt->GetSupportDirectory(), '/', this->GetCMakeCFGIntDir(), '/');
  gt->ObjectDirectory = dir;
}

bool IsVisualStudioMacrosFileRegistered(std::string const& macrosFile,
                                        std::string const& regKeyBase,
                                        std::string& nextAvailableSubKeyName);

void RegisterVisualStudioMacros(std::string const& macrosFile,
                                std::string const& regKeyBase);

#define CMAKE_VSMACROS_FILENAME "CMakeVSMacros2.vsmacros"

#define CMAKE_VSMACROS_RELOAD_MACRONAME                                       \
  "Macros.CMakeVSMacros2.Macros.ReloadProjects"

#define CMAKE_VSMACROS_STOP_MACRONAME "Macros.CMakeVSMacros2.Macros.StopBuild"

void cmGlobalVisualStudioGenerator::ConfigureCMakeVisualStudioMacros()
{
  std::string dir = this->GetUserMacrosDirectory();

  if (!dir.empty()) {
    std::string src = cmStrCat(cmSystemTools::GetCMakeRoot(),
                               "/Templates/" CMAKE_VSMACROS_FILENAME);

    std::string dst = cmStrCat(dir, "/CMakeMacros/" CMAKE_VSMACROS_FILENAME);

    // Copy the macros file to the user directory only if the
    // destination does not exist or the source location is newer.
    // This will allow the user to edit the macros for development
    // purposes but newer versions distributed with CMake will replace
    // older versions in user directories.
    int res;
    if (!cmSystemTools::FileTimeCompare(src, dst, &res) || res > 0) {
      if (!cmSystemTools::CopyFileAlways(src, dst)) {
        std::ostringstream oss;
        oss << "Could not copy from: " << src << std::endl
            << "                 to: " << dst << std::endl;
        cmSystemTools::Message(oss.str(), "Warning");
      }
    }

    RegisterVisualStudioMacros(dst, this->GetUserMacrosRegKeyBase());
  }
}

void cmGlobalVisualStudioGenerator::CallVisualStudioMacro(
  MacroName m, std::string const& vsSolutionFile)
{
  // If any solution or project files changed during the generation,
  // tell Visual Studio to reload them...
  std::string dir = this->GetUserMacrosDirectory();

  // Only really try to call the macro if:
  //  - there is a UserMacrosDirectory
  //  - the CMake vsmacros file exists
  //  - the CMake vsmacros file is registered
  //  - there were .sln/.vcproj files changed during generation
  //
  if (!dir.empty()) {
    std::string macrosFile =
      cmStrCat(dir, "/CMakeMacros/" CMAKE_VSMACROS_FILENAME);
    std::string nextSubkeyName;
    if (cmSystemTools::FileExists(macrosFile) &&
        IsVisualStudioMacrosFileRegistered(
          macrosFile, this->GetUserMacrosRegKeyBase(), nextSubkeyName)) {
      if (m == MacroReload) {
        std::vector<std::string> filenames;
        this->GetFilesReplacedDuringGenerate(filenames);
        if (!filenames.empty()) {
          std::string projects = cmJoin(filenames, ";");
          cmCallVisualStudioMacro::CallMacro(
            vsSolutionFile, CMAKE_VSMACROS_RELOAD_MACRONAME, projects,
            this->GetCMakeInstance()->GetDebugOutput());
        }
      } else if (m == MacroStop) {
        cmCallVisualStudioMacro::CallMacro(
          vsSolutionFile, CMAKE_VSMACROS_STOP_MACRONAME, "",
          this->GetCMakeInstance()->GetDebugOutput());
      }
    }
  }
}

std::string cmGlobalVisualStudioGenerator::GetUserMacrosDirectory()
{
  return "";
}

std::string cmGlobalVisualStudioGenerator::GetUserMacrosRegKeyBase()
{
  return "";
}

bool cmGlobalVisualStudioGenerator::FindMakeProgram(cmMakefile* mf)
{
  // Visual Studio generators know how to lookup their build tool
  // directly instead of needing a helper module to do it, so we
  // do not actually need to put CMAKE_MAKE_PROGRAM into the cache.
  if (mf->GetDefinition("CMAKE_MAKE_PROGRAM").IsOff()) {
    mf->AddDefinition("CMAKE_MAKE_PROGRAM", this->GetVSMakeProgram());
  }
  return true;
}

std::string cmGlobalVisualStudioGenerator::GetStartupProjectName(
  cmLocalGenerator const* root) const
{
  cmValue n = root->GetMakefile()->GetProperty("VS_STARTUP_PROJECT");
  if (cmNonempty(n)) {
    std::string startup = *n;
    if (this->FindTarget(startup)) {
      return startup;
    }
    root->GetMakefile()->IssueMessage(
      MessageType::AUTHOR_WARNING,
      cmStrCat("Directory property VS_STARTUP_PROJECT specifies target "
               "'",
               startup, "' that does not exist.  Ignoring."));
  }

  // default, if not specified
  return this->GetAllTargetName();
}

bool IsVisualStudioMacrosFileRegistered(std::string const& macrosFile,
                                        std::string const& regKeyBase,
                                        std::string& nextAvailableSubKeyName)
{
  bool macrosRegistered = false;

  std::string s1;
  std::string s2;

  // Make lowercase local copies, convert to Unix slashes, and
  // see if the resulting strings are the same:
  s1 = cmSystemTools::LowerCase(macrosFile);
  cmSystemTools::ConvertToUnixSlashes(s1);

  std::string keyname;
  HKEY hkey = nullptr;
  LONG result = ERROR_SUCCESS;
  DWORD index = 0;

  keyname = cmStrCat(regKeyBase, "\\OtherProjects7");
  hkey = nullptr;
  result =
    RegOpenKeyExW(HKEY_CURRENT_USER, cmsys::Encoding::ToWide(keyname).c_str(),
                  0, KEY_READ, &hkey);
  if (ERROR_SUCCESS == result) {
    // Iterate the subkeys and look for the values of interest in each subkey:
    wchar_t subkeyname[256];
    DWORD cch_subkeyname = cm::size(subkeyname);
    wchar_t keyclass[256];
    DWORD cch_keyclass = cm::size(keyclass);
    FILETIME lastWriteTime;
    lastWriteTime.dwHighDateTime = 0;
    lastWriteTime.dwLowDateTime = 0;

    while (ERROR_SUCCESS ==
           RegEnumKeyExW(hkey, index, subkeyname, &cch_subkeyname, 0, keyclass,
                         &cch_keyclass, &lastWriteTime)) {
      // Open the subkey and query the values of interest:
      HKEY hsubkey = nullptr;
      result = RegOpenKeyExW(hkey, subkeyname, 0, KEY_READ, &hsubkey);
      if (ERROR_SUCCESS == result) {
        DWORD valueType = REG_SZ;
        wchar_t data1[256];
        DWORD cch_data1 = sizeof(data1);
        RegQueryValueExW(hsubkey, L"Path", 0, &valueType, (LPBYTE)data1,
                         &cch_data1);

        DWORD data2 = 0;
        DWORD cch_data2 = sizeof(data2);
        RegQueryValueExW(hsubkey, L"Security", 0, &valueType, (LPBYTE)&data2,
                         &cch_data2);

        DWORD data3 = 0;
        DWORD cch_data3 = sizeof(data3);
        RegQueryValueExW(hsubkey, L"StorageFormat", 0, &valueType,
                         (LPBYTE)&data3, &cch_data3);

        s2 = cmSystemTools::LowerCase(cmsys::Encoding::ToNarrow(data1));
        cmSystemTools::ConvertToUnixSlashes(s2);
        if (s2 == s1) {
          macrosRegistered = true;
        }

        std::string fullname = cmsys::Encoding::ToNarrow(data1);
        std::string filename;
        std::string filepath;
        std::string filepathname;
        std::string filepathpath;
        if (cmSystemTools::FileExists(fullname)) {
          filename = cmSystemTools::GetFilenameName(fullname);
          filepath = cmSystemTools::GetFilenamePath(fullname);
          filepathname = cmSystemTools::GetFilenameName(filepath);
          filepathpath = cmSystemTools::GetFilenamePath(filepath);
        }

        // std::cout << keyname << "\\" << subkeyname << ":" << std::endl;
        // std::cout << "  Path: " << data1 << std::endl;
        // std::cout << "  Security: " << data2 << std::endl;
        // std::cout << "  StorageFormat: " << data3 << std::endl;
        // std::cout << "  filename: " << filename << std::endl;
        // std::cout << "  filepath: " << filepath << std::endl;
        // std::cout << "  filepathname: " << filepathname << std::endl;
        // std::cout << "  filepathpath: " << filepathpath << std::endl;
        // std::cout << std::endl;

        RegCloseKey(hsubkey);
      } else {
        std::cout << "error opening subkey: "
                  << cmsys::Encoding::ToNarrow(subkeyname) << std::endl;
        std::cout << std::endl;
      }

      ++index;
      cch_subkeyname = cm::size(subkeyname);
      cch_keyclass = cm::size(keyclass);
      lastWriteTime.dwHighDateTime = 0;
      lastWriteTime.dwLowDateTime = 0;
    }

    RegCloseKey(hkey);
  } else {
    std::cout << "error opening key: " << keyname << std::endl;
    std::cout << std::endl;
  }

  // Pass back next available sub key name, assuming sub keys always
  // follow the expected naming scheme. Expected naming scheme is that
  // the subkeys of OtherProjects7 is 0 to n-1, so it's ok to use "n"
  // as the name of the next subkey.
  nextAvailableSubKeyName = std::to_string(index);

  keyname = cmStrCat(regKeyBase, "\\RecordingProject7");
  hkey = nullptr;
  result =
    RegOpenKeyExW(HKEY_CURRENT_USER, cmsys::Encoding::ToWide(keyname).c_str(),
                  0, KEY_READ, &hkey);
  if (ERROR_SUCCESS == result) {
    DWORD valueType = REG_SZ;
    wchar_t data1[256];
    DWORD cch_data1 = sizeof(data1);
    RegQueryValueExW(hkey, L"Path", 0, &valueType, (LPBYTE)data1, &cch_data1);

    DWORD data2 = 0;
    DWORD cch_data2 = sizeof(data2);
    RegQueryValueExW(hkey, L"Security", 0, &valueType, (LPBYTE)&data2,
                     &cch_data2);

    DWORD data3 = 0;
    DWORD cch_data3 = sizeof(data3);
    RegQueryValueExW(hkey, L"StorageFormat", 0, &valueType, (LPBYTE)&data3,
                     &cch_data3);

    s2 = cmSystemTools::LowerCase(cmsys::Encoding::ToNarrow(data1));
    cmSystemTools::ConvertToUnixSlashes(s2);
    if (s2 == s1) {
      macrosRegistered = true;
    }

    // std::cout << keyname << ":" << std::endl;
    // std::cout << "  Path: " << data1 << std::endl;
    // std::cout << "  Security: " << data2 << std::endl;
    // std::cout << "  StorageFormat: " << data3 << std::endl;
    // std::cout << std::endl;

    RegCloseKey(hkey);
  } else {
    std::cout << "error opening key: " << keyname << std::endl;
    std::cout << std::endl;
  }

  return macrosRegistered;
}

void WriteVSMacrosFileRegistryEntry(std::string const& nextAvailableSubKeyName,
                                    std::string const& macrosFile,
                                    std::string const& regKeyBase)
{
  std::string keyname = cmStrCat(regKeyBase, "\\OtherProjects7");
  HKEY hkey = nullptr;
  LONG result =
    RegOpenKeyExW(HKEY_CURRENT_USER, cmsys::Encoding::ToWide(keyname).c_str(),
                  0, KEY_READ | KEY_WRITE, &hkey);
  if (ERROR_SUCCESS == result) {
    // Create the subkey and set the values of interest:
    HKEY hsubkey = nullptr;
    wchar_t lpClass[] = L"";
    result = RegCreateKeyExW(
      hkey, cmsys::Encoding::ToWide(nextAvailableSubKeyName).c_str(), 0,
      lpClass, 0, KEY_READ | KEY_WRITE, 0, &hsubkey, 0);
    if (ERROR_SUCCESS == result) {
      DWORD dw = 0;

      std::string s(macrosFile);
      std::replace(s.begin(), s.end(), '/', '\\');
      std::wstring ws = cmsys::Encoding::ToWide(s);

      result =
        RegSetValueExW(hsubkey, L"Path", 0, REG_SZ, (LPBYTE)ws.c_str(),
                       static_cast<DWORD>(ws.size() + 1) * sizeof(wchar_t));
      if (ERROR_SUCCESS != result) {
        std::cout << "error result 1: " << result << std::endl;
        std::cout << std::endl;
      }

      // Security value is always "1" for sample macros files (seems to be "2"
      // if you put the file somewhere outside the standard VSMacros folder)
      dw = 1;
      result = RegSetValueExW(hsubkey, L"Security", 0, REG_DWORD, (LPBYTE)&dw,
                              sizeof(DWORD));
      if (ERROR_SUCCESS != result) {
        std::cout << "error result 2: " << result << std::endl;
        std::cout << std::endl;
      }

      // StorageFormat value is always "0" for sample macros files
      dw = 0;
      result = RegSetValueExW(hsubkey, L"StorageFormat", 0, REG_DWORD,
                              (LPBYTE)&dw, sizeof(DWORD));
      if (ERROR_SUCCESS != result) {
        std::cout << "error result 3: " << result << std::endl;
        std::cout << std::endl;
      }

      RegCloseKey(hsubkey);
    } else {
      std::cout << "error creating subkey: " << nextAvailableSubKeyName
                << std::endl;
      std::cout << std::endl;
    }
    RegCloseKey(hkey);
  } else {
    std::cout << "error opening key: " << keyname << std::endl;
    std::cout << std::endl;
  }
}

void RegisterVisualStudioMacros(std::string const& macrosFile,
                                std::string const& regKeyBase)
{
  bool macrosRegistered;
  std::string nextAvailableSubKeyName;

  macrosRegistered = IsVisualStudioMacrosFileRegistered(
    macrosFile, regKeyBase, nextAvailableSubKeyName);

  if (!macrosRegistered) {
    int count =
      cmCallVisualStudioMacro::GetNumberOfRunningVisualStudioInstances("ALL");

    // Only register the macros file if there are *no* instances of Visual
    // Studio running. If we register it while one is running, first, it has
    // no effect on the running instance; second, and worse, Visual Studio
    // removes our newly added registration entry when it quits. Instead,
    // emit a warning asking the user to exit all running Visual Studio
    // instances...
    //
    if (0 != count) {
      std::ostringstream oss;
      oss << "Could not register CMake's Visual Studio macros file '"
          << CMAKE_VSMACROS_FILENAME "' while Visual Studio is running."
          << " Please exit all running instances of Visual Studio before"
          << " continuing." << std::endl
          << std::endl
          << "CMake needs to register Visual Studio macros when its macros"
          << " file is updated or when it detects that its current macros file"
          << " is no longer registered with Visual Studio." << std::endl;
      cmSystemTools::Message(oss.str(), "Warning");

      // Count them again now that the warning is over. In the case of a GUI
      // warning, the user may have gone to close Visual Studio and then come
      // back to the CMake GUI and clicked ok on the above warning. If so,
      // then register the macros *now* if the count is *now* 0...
      //
      count = cmCallVisualStudioMacro::GetNumberOfRunningVisualStudioInstances(
        "ALL");

      // Also re-get the nextAvailableSubKeyName in case Visual Studio
      // wrote out new registered macros information as it was exiting:
      //
      if (0 == count) {
        IsVisualStudioMacrosFileRegistered(macrosFile, regKeyBase,
                                           nextAvailableSubKeyName);
      }
    }

    // Do another if check - 'count' may have changed inside the above if:
    //
    if (0 == count) {
      WriteVSMacrosFileRegistryEntry(nextAvailableSubKeyName, macrosFile,
                                     regKeyBase);
    }
  }
}
bool cmGlobalVisualStudioGenerator::TargetIsFortranOnly(
  cmGeneratorTarget const* gt) const
{
  // If there's only one source language, Fortran has to be used
  // in order for the sources to compile.
  std::set<std::string> languages = gt->GetAllConfigCompileLanguages();
  // Consider an explicit linker language property, but *not* the
  // computed linker language that may depend on linked targets.
  // This allows the project to control the language choice in
  // a target with none of its own sources, e.g. when also using
  // object libraries.
  cmValue linkLang = gt->GetProperty("LINKER_LANGUAGE");
  if (cmNonempty(linkLang)) {
    languages.insert(*linkLang);
  }

  // Intel Fortran .vfproj files do support the resource compiler.
  languages.erase("RC");

  return languages.size() == 1 && *languages.begin() == "Fortran"_s;
}

bool cmGlobalVisualStudioGenerator::IsInSolution(
  cmGeneratorTarget const* gt) const
{
  return gt->IsInBuildSystem();
}

bool cmGlobalVisualStudioGenerator::IsDepInSolution(
  std::string const& targetName) const
{
  return !targetName.empty();
}

bool cmGlobalVisualStudioGenerator::TargetCompare::operator()(
  cmGeneratorTarget const* l, cmGeneratorTarget const* r) const
{
  // Make sure a given named target is ordered first,
  // e.g. to set ALL_BUILD as the default active project.
  // When the empty string is named this is a no-op.
  if (r->GetName() == this->First) {
    return false;
  }
  if (l->GetName() == this->First) {
    return true;
  }
  return l->GetName() < r->GetName();
}

cmGlobalVisualStudioGenerator::OrderedTargetDependSet::OrderedTargetDependSet(
  TargetDependSet const& targets, std::string const& first)
  : derived(TargetCompare(first))
{
  this->insert(targets.begin(), targets.end());
}

cmGlobalVisualStudioGenerator::OrderedTargetDependSet::OrderedTargetDependSet(
  TargetSet const& targets, std::string const& first)
  : derived(TargetCompare(first))
{
  for (cmGeneratorTarget const* it : targets) {
    this->insert(it);
  }
}

std::string cmGlobalVisualStudioGenerator::ExpandCFGIntDir(
  std::string const& str, std::string const& config) const
{
  std::string replace = GetCMakeCFGIntDir();

  std::string tmp = str;
  for (std::string::size_type i = tmp.find(replace); i != std::string::npos;
       i = tmp.find(replace, i)) {
    tmp.replace(i, replace.size(), config);
    i += config.size();
  }
  return tmp;
}

void cmGlobalVisualStudioGenerator::AddSymbolExportCommand(
  cmGeneratorTarget* gt, std::vector<cmCustomCommand>& commands,
  std::string const& configName)
{
  cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
    gt->GetModuleDefinitionInfo(configName);
  if (!mdi || !mdi->DefFileGenerated) {
    return;
  }

  std::vector<std::string> outputs;
  outputs.push_back(mdi->DefFile);
  std::vector<std::string> empty;
  std::vector<cmSourceFile const*> objectSources;
  gt->GetObjectSources(objectSources, configName);
  std::map<cmSourceFile const*, cmObjectLocations> mapping;
  for (cmSourceFile const* it : objectSources) {
    mapping[it];
  }
  gt->LocalGenerator->ComputeObjectFilenames(mapping, configName, gt);
  std::string obj_dir = gt->ObjectDirectory;
  std::string cmakeCommand = cmSystemTools::GetCMakeCommand();
  std::string obj_dir_expanded = obj_dir;
  cmSystemTools::ReplaceString(obj_dir_expanded, this->GetCMakeCFGIntDir(),
                               configName.c_str());
  cmSystemTools::MakeDirectory(obj_dir_expanded);
  std::string const objs_file = cmStrCat(obj_dir_expanded, "/objects.txt");
  cmGeneratedFileStream fout(objs_file.c_str());
  if (!fout) {
    cmSystemTools::Error(cmStrCat("could not open ", objs_file));
    return;
  }

  auto const useShortPaths = this->UseShortObjectNames()
    ? cmObjectLocations::UseShortPath::Yes
    : cmObjectLocations::UseShortPath::No;

  if (mdi->WindowsExportAllSymbols) {
    std::vector<std::string> objs;
    for (cmSourceFile const* it : objectSources) {
      // Find the object file name corresponding to this source file.
      // It must exist because we populated the mapping just above.
      auto const& locs = mapping[it];
      std::string const& v = locs.GetPath(useShortPaths);
      assert(!v.empty());
      std::string objFile = cmStrCat(obj_dir, v);
      objs.push_back(objFile);
    }
    std::vector<cmSourceFile const*> externalObjectSources;
    gt->GetExternalObjects(externalObjectSources, configName);
    for (cmSourceFile const* it : externalObjectSources) {
      objs.push_back(it->GetFullPath());
    }

    for (std::string const& it : objs) {
      std::string objFile = it;
      // replace $(ConfigurationName) in the object names
      cmSystemTools::ReplaceString(objFile, this->GetCMakeCFGIntDir(),
                                   configName);
      if (cmHasLiteralSuffix(objFile, ".obj")) {
        fout << objFile << "\n";
      }
    }
  }

  for (cmSourceFile const* i : mdi->Sources) {
    fout << i->GetFullPath() << "\n";
  }

  cmCustomCommandLines commandLines = cmMakeSingleCommandLine(
    { cmakeCommand, "-E", "__create_def", mdi->DefFile, objs_file });
  cmCustomCommand command;
  command.SetOutputs(outputs);
  command.SetCommandLines(commandLines);
  command.SetComment("Auto build dll exports");
  command.SetBacktrace(gt->Target->GetMakefile()->GetBacktrace());
  command.SetWorkingDirectory(".");
  command.SetStdPipesUTF8(true);
  commands.push_back(std::move(command));
}

static bool OpenSolution(std::string const& sln)
{
  HRESULT comInitialized =
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  if (FAILED(comInitialized)) {
    return false;
  }

  HINSTANCE hi = ShellExecuteA(nullptr, "open", sln.c_str(), nullptr, nullptr,
                               SW_SHOWNORMAL);

  CoUninitialize();

  return reinterpret_cast<intptr_t>(hi) > 32;
}

bool cmGlobalVisualStudioGenerator::Open(std::string const& bindir,
                                         std::string const& projectName,
                                         bool dryRun)
{
  std::string sln = this->GetSLNFile(bindir, projectName);

  if (dryRun) {
    return cmSystemTools::FileExists(sln, true);
  }

  sln = cmSystemTools::ConvertToOutputPath(sln);

  return std::async(std::launch::async, OpenSolution, sln).get();
}

cm::string_view cmGlobalVisualStudioGenerator::ExternalProjectTypeId(
  std::string const& path)
{
  using namespace cm::VS;
  std::string const extension = cmSystemTools::GetFilenameLastExtension(path);
  if (extension == ".vfproj"_s) {
    return Solution::Project::TypeIdFortran;
  }
  if (extension == ".vbproj"_s) {
    return Solution::Project::TypeIdVisualBasic;
  }
  if (extension == ".csproj"_s) {
    return Solution::Project::TypeIdCSharp;
  }
  if (extension == ".fsproj"_s) {
    return Solution::Project::TypeIdFSharp;
  }
  if (extension == ".vdproj"_s) {
    return Solution::Project::TypeIdVDProj;
  }
  if (extension == ".dbproj"_s) {
    return Solution::Project::TypeIdDatabase;
  }
  if (extension == ".njsproj"_s) {
    return Solution::Project::TypeIdNodeJS;
  }
  if (extension == ".wapproj"_s) {
    return Solution::Project::TypeIdWinAppPkg;
  }
  if (extension == ".wixproj"_s) {
    return Solution::Project::TypeIdWiX;
  }
  if (extension == ".pyproj"_s) {
    return Solution::Project::TypeIdPython;
  }
  return Solution::Project::TypeIdDefault;
}

bool cmGlobalVisualStudioGenerator::IsDependedOn(
  TargetDependSet const& projectTargets, cmGeneratorTarget const* gtIn) const
{
  return std::any_of(projectTargets.begin(), projectTargets.end(),
                     [this, gtIn](cmTargetDepend const& l) {
                       TargetDependSet const& tgtdeps =
                         this->GetTargetDirectDepends(l);
                       return tgtdeps.count(gtIn);
                     });
}

std::set<std::string> cmGlobalVisualStudioGenerator::IsPartOfDefaultBuild(
  std::vector<std::string> const& configs,
  TargetDependSet const& projectTargets, cmGeneratorTarget const* target) const
{
  std::set<std::string> activeConfigs;
  // if it is a utility target then only make it part of the
  // default build if another target depends on it
  int type = target->GetType();
  if (type == cmStateEnums::GLOBAL_TARGET) {
    std::vector<std::string> targetNames;
    targetNames.push_back("INSTALL");
    targetNames.push_back("PACKAGE");
    for (std::string const& t : targetNames) {
      // check if target <t> is part of default build
      if (target->GetName() == t) {
        std::string const propertyName =
          cmStrCat("CMAKE_VS_INCLUDE_", t, "_TO_DEFAULT_BUILD");
        // inspect CMAKE_VS_INCLUDE_<t>_TO_DEFAULT_BUILD properties
        for (std::string const& i : configs) {
          cmValue propertyValue =
            target->Target->GetMakefile()->GetDefinition(propertyName);
          if (propertyValue &&
              cmIsOn(cmGeneratorExpression::Evaluate(
                *propertyValue, target->GetLocalGenerator(), i))) {
            activeConfigs.insert(i);
          }
        }
      }
    }
    return activeConfigs;
  }
  if (type == cmStateEnums::UTILITY &&
      !this->IsDependedOn(projectTargets, target)) {
    return activeConfigs;
  }
  // inspect EXCLUDE_FROM_DEFAULT_BUILD[_<CONFIG>] properties
  for (std::string const& i : configs) {
    if (target->GetFeature("EXCLUDE_FROM_DEFAULT_BUILD", i).IsOff()) {
      activeConfigs.insert(i);
    }
  }
  return activeConfigs;
}

std::string cmGlobalVisualStudioGenerator::GetGUID(
  std::string const& name) const
{
  std::string const& guidStoreName = cmStrCat(name, "_GUID_CMAKE");
  if (cmValue storedGUID =
        this->CMakeInstance->GetCacheDefinition(guidStoreName)) {
    return *storedGUID;
  }
  // Compute a GUID that is deterministic but unique to the build tree.
  std::string input =
    cmStrCat(this->CMakeInstance->GetState()->GetBinaryDirectory(), '|', name);

  cmUuid uuidGenerator;

  std::vector<unsigned char> uuidNamespace;
  uuidGenerator.StringToBinary("ee30c4be-5192-4fb0-b335-722a2dffe760",
                               uuidNamespace);

  std::string guid = uuidGenerator.FromMd5(uuidNamespace, input);

  return cmSystemTools::UpperCase(guid);
}

cm::VS::Solution::Folder* cmGlobalVisualStudioGenerator::CreateSolutionFolder(
  cm::VS::Solution& solution, cm::string_view rawName) const
{
  cm::VS::Solution::Folder* folder = nullptr;
  std::string canonicalName;
  for (std::string::size_type cur = 0;;) {
    static std::string delims = "/\\";
    cur = rawName.find_first_not_of(delims, cur);
    if (cur == std::string::npos) {
      break;
    }
    std::string::size_type end = rawName.find_first_of(delims, cur);
    cm::string_view f = end == std::string::npos
      ? rawName.substr(cur)
      : rawName.substr(cur, end - cur);
    canonicalName =
      canonicalName.empty() ? std::string(f) : cmStrCat(canonicalName, '/', f);
    cm::VS::Solution::Folder* nextFolder = solution.GetFolder(canonicalName);
    if (nextFolder->Id.empty()) {
      nextFolder->Id =
        this->GetGUID(cmStrCat("CMAKE_FOLDER_GUID_"_s, canonicalName));
      if (folder) {
        folder->Folders.emplace_back(nextFolder);
      }
      solution.Folders.emplace_back(nextFolder);
    }
    folder = nextFolder;
    cur = end;
  }
  return folder;
}

cm::VS::Solution cmGlobalVisualStudioGenerator::CreateSolution(
  cmLocalGenerator const* root, TargetDependSet const& projectTargets) const
{
  using namespace cm::VS;
  Solution solution;
  solution.VSVersion = this->Version;
  solution.VSExpress =
    this->ExpressEdition ? VersionExpress::Yes : VersionExpress::No;
  solution.Platform = this->GetPlatformName();
  solution.Configs =
    root->GetMakefile()->GetGeneratorConfigs(cmMakefile::ExcludeEmptyConfig);
  solution.StartupProject = this->GetStartupProjectName(root);

  auto addProject = [this, useFolders = this->UseFolderProperty(),
                     &solution](cmGeneratorTarget const* gt,
                                Solution::Project const* p) {
    if (Solution::Folder* const folder = useFolders
          ? this->CreateSolutionFolder(solution, gt->GetEffectiveFolderName())
          : nullptr) {
      folder->Projects.emplace_back(p);
    } else {
      solution.Projects.emplace_back(p);
    }
  };

  for (cmTargetDepend const& projectTarget : projectTargets) {
    cmGeneratorTarget const* gt = projectTarget;
    if (!this->IsInSolution(gt)) {
      continue;
    }

    Solution::Project* project = solution.GetProject(gt->GetName());
    project->Id = this->GetGUID(gt->GetName());

    std::set<std::string> const& includeConfigs =
      this->IsPartOfDefaultBuild(solution.Configs, projectTargets, gt);
    auto addProjectConfig =
      [this, project, gt, &includeConfigs](std::string const& solutionConfig,
                                           std::string const& projectConfig) {
        bool const build =
          includeConfigs.find(solutionConfig) != includeConfigs.end();
        bool const deploy = this->NeedsDeploy(*gt, solutionConfig.c_str());
        project->Configs.emplace_back(
          Solution::ProjectConfig{ projectConfig, build, deploy });
      };

    if (cmValue expath = gt->GetProperty("EXTERNAL_MSPROJECT")) {
      project->Path = *expath;
      cmValue const projectType = gt->GetProperty("VS_PROJECT_TYPE");
      if (!projectType.IsEmpty()) {
        project->TypeId = *projectType;
      } else {
        project->TypeId =
          std::string{ this->ExternalProjectTypeId(project->Path) };
      }
      for (std::string const& config : solution.Configs) {
        cmList mapConfig{ gt->GetProperty(cmStrCat(
          "MAP_IMPORTED_CONFIG_", cmSystemTools::UpperCase(config))) };
        addProjectConfig(config, !mapConfig.empty() ? mapConfig[0] : config);
      }
      cmValue platformMapping = gt->GetProperty("VS_PLATFORM_MAPPING");
      project->Platform =
        !platformMapping.IsEmpty() ? *platformMapping : solution.Platform;
      for (BT<std::pair<std::string, bool>> const& i : gt->GetUtilities()) {
        std::string const& dep = i.Value.first;
        if (this->IsDepInSolution(dep)) {
          project->BuildDependencies.emplace_back(solution.GetProject(dep));
        }
      }
      addProject(gt, project);
      continue;
    }

    if (cmValue vcprojName = gt->GetProperty("GENERATOR_FILE_NAME")) {
      cmLocalGenerator* lg = gt->GetLocalGenerator();
      std::string dir =
        root->MaybeRelativeToCurBinDir(lg->GetCurrentBinaryDirectory());
      if (dir == "."_s) {
        dir.clear();
      } else if (!cmHasSuffix(dir, '/')) {
        dir += "/";
      }

      cm::string_view vcprojExt;
      if (this->TargetIsFortranOnly(gt)) {
        vcprojExt = ".vfproj"_s;
        project->TypeId = std::string{ Solution::Project::TypeIdFortran };
      } else if (gt->IsCSharpOnly()) {
        vcprojExt = ".csproj"_s;
        project->TypeId = std::string{ Solution::Project::TypeIdCSharp };
      } else {
        vcprojExt = ".vcproj"_s;
        project->TypeId = std::string{ Solution::Project::TypeIdDefault };
      }
      if (cmValue genExt = gt->GetProperty("GENERATOR_FILE_NAME_EXT")) {
        vcprojExt = *genExt;
      }
      project->Path = cmStrCat(dir, *vcprojName, vcprojExt);

      if (gt->IsDotNetSdkTarget() &&
          !cmGlobalVisualStudioGenerator::IsReservedTarget(gt->GetName())) {
        cmValue platformTarget = gt->GetProperty("VS_GLOBAL_PlatformTarget");
        if (!platformTarget.IsEmpty()) {
          project->Platform = *platformTarget;
        } else {
          project->Platform =
            // On VS 16 and above, always map .NET SDK projects to "Any CPU".
            this->Version >= VSVersion::VS16 ? "Any CPU" : solution.Platform;
        }
      } else {
        project->Platform = solution.Platform;
      }

      // Add solution-level dependencies.
      TargetDependSet const& depends = this->GetTargetDirectDepends(gt);
      for (cmTargetDepend const& dep : depends) {
        if (this->IsInSolution(dep)) {
          project->BuildDependencies.emplace_back(
            solution.GetProject(dep->GetName()));
        }
      }

      for (std::string const& config : solution.Configs) {
        addProjectConfig(config, config);
      }

      addProject(gt, project);
      continue;
    }
  }

  cmMakefile* mf = root->GetMakefile();
  std::vector<std::string> items =
    cmList{ root->GetMakefile()->GetProperty("VS_SOLUTION_ITEMS") };
  for (std::string item : items) {
    if (!cmSystemTools::FileIsFullPath(item)) {
      item =
        cmSystemTools::CollapseFullPath(item, mf->GetCurrentSourceDirectory());
    }
    cmSourceGroup* sg =
      cmSourceGroup::FindSourceGroup(item, mf->GetSourceGroups());
    std::string folderName = sg->GetFullName();
    if (folderName.empty()) {
      folderName = "Solution Items";
    }
    Solution::Folder* folder =
      this->CreateSolutionFolder(solution, folderName);
    folder->Files.emplace(std::move(item));
  }

  Solution::PropertyGroup* pgExtensibilityGlobals = nullptr;
  Solution::PropertyGroup* pgExtensibilityAddIns = nullptr;
  std::vector<std::string> const propKeys =
    root->GetMakefile()->GetPropertyKeys();
  for (std::string const& it : propKeys) {
    if (!cmHasLiteralPrefix(it, "VS_GLOBAL_SECTION_")) {
      continue;
    }
    std::string name = it.substr(18);
    Solution::PropertyGroup::Load scope;
    if (cmHasLiteralPrefix(name, "PRE_")) {
      name = name.substr(4);
      scope = Solution::PropertyGroup::Load::Pre;
    } else if (cmHasLiteralPrefix(name, "POST_")) {
      name = name.substr(5);
      scope = Solution::PropertyGroup::Load::Post;
    } else {
      continue;
    }
    if (name.empty()) {
      continue;
    }
    Solution::PropertyGroup* pg = solution.GetPropertyGroup(name);
    solution.PropertyGroups.emplace_back(pg);
    pg->Scope = scope;
    cmList keyValuePairs{ root->GetMakefile()->GetProperty(it) };
    for (std::string const& itPair : keyValuePairs) {
      std::string::size_type const posEqual = itPair.find('=');
      if (posEqual != std::string::npos) {
        std::string key = cmTrimWhitespace(itPair.substr(0, posEqual));
        std::string value = cmTrimWhitespace(itPair.substr(posEqual + 1));
        pg->Map.emplace(std::move(key), std::move(value));
      }
    }
    if (name == "ExtensibilityGlobals"_s) {
      pgExtensibilityGlobals = pg;
    } else if (name == "ExtensibilityAddIns"_s) {
      pgExtensibilityAddIns = pg;
    }
  }

  if (this->Version <= cm::VS::Version::VS17) {
    if (!pgExtensibilityGlobals) {
      pgExtensibilityGlobals =
        solution.GetPropertyGroup("ExtensibilityGlobals"_s);
      solution.PropertyGroups.emplace_back(pgExtensibilityGlobals);
    }
    std::string const solutionGuid =
      this->GetGUID(cmStrCat(root->GetProjectName(), ".sln"));
    pgExtensibilityGlobals->Map.emplace("SolutionGuid",
                                        cmStrCat('{', solutionGuid, '}'));

    if (!pgExtensibilityAddIns) {
      pgExtensibilityAddIns =
        solution.GetPropertyGroup("ExtensibilityAddIns"_s);
      solution.PropertyGroups.emplace_back(pgExtensibilityAddIns);
    }
  }

  solution.CanonicalizeOrder();

  return solution;
}

std::string cmGlobalVisualStudioGenerator::GetSLNFile(
  cmLocalGenerator const* root) const
{
  return this->GetSLNFile(root->GetCurrentBinaryDirectory(),
                          root->GetProjectName());
}

std::string cmGlobalVisualStudioGenerator::GetSLNFile(
  std::string const& projectDir, std::string const& projectName) const
{
  std::string slnFile = projectDir;
  if (!slnFile.empty()) {
    slnFile.push_back('/');
  }
  slnFile = cmStrCat(slnFile, projectName, ".sln");
  if (this->Version >= cm::VS::Version::VS18) {
    slnFile += "x";
  }
  return slnFile;
}

void cmGlobalVisualStudioGenerator::Generate()
{
  // first do the superclass method
  this->cmGlobalGenerator::Generate();

  // Now write out the VS Solution files.
  for (auto& it : this->ProjectMap) {
    this->GenerateSolution(it.second[0], it.second);
  }

  // If any solution or project files changed during the generation,
  // tell Visual Studio to reload them...
  if (!cmSystemTools::GetErrorOccurredFlag() &&
      !this->LocalGenerators.empty()) {
    this->CallVisualStudioMacro(MacroReload,
                                GetSLNFile(this->LocalGenerators[0].get()));
  }

  if (this->Version == VSVersion::VS14 &&
      !this->CMakeInstance->GetIsInTryCompile()) {
    std::string cmakeWarnVS14;
    if (cmValue cached = this->CMakeInstance->GetState()->GetCacheEntryValue(
          "CMAKE_WARN_VS14")) {
      this->CMakeInstance->MarkCliAsUsed("CMAKE_WARN_VS14");
      cmakeWarnVS14 = *cached;
    } else {
      cmSystemTools::GetEnv("CMAKE_WARN_VS14", cmakeWarnVS14);
    }
    if (cmakeWarnVS14.empty() || !cmIsOff(cmakeWarnVS14)) {
      this->CMakeInstance->IssueMessage(
        MessageType::WARNING,
        "The \"Visual Studio 14 2015\" generator is deprecated "
        "and will be removed in a future version of CMake."
        "\n"
        "Add CMAKE_WARN_VS14=OFF to the cache to disable this warning.");
    }
  }
}

void cmGlobalVisualStudioGenerator::GenerateSolution(
  cmLocalGenerator const* root,
  std::vector<cmLocalGenerator*> const& generators)
{
  if (generators.empty()) {
    return;
  }

  // Collect all targets under this root generator and the transitive
  // closure of their dependencies.
  TargetDependSet const projectTargets =
    this->GetTargetsForProject(root, generators);

  std::string fname = GetSLNFile(root);
  cmGeneratedFileStream fout(fname);
  fout.SetCopyIfDifferent(true);
  if (!fout) {
    return;
  }

  cm::VS::Solution const solution = this->CreateSolution(root, projectTargets);
  if (this->Version >= cmGlobalVisualStudioGenerator::VSVersion::VS18) {
    WriteSlnx(fout, solution);
  } else {
    WriteSln(fout, solution);
  }

  if (fout.Close()) {
    this->FileReplacedDuringGenerate(fname);
  }
}
