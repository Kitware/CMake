
/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudioGenerator.h"

#include <cassert>
#include <future>
#include <iostream>
#include <sstream>
#include <system_error>
#include <utility>

#include <cm/iterator>
#include <cm/memory>

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
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmake.h"

cmGlobalVisualStudioGenerator::cmGlobalVisualStudioGenerator(
  cmake* cm, std::string const& platformInGeneratorName)
  : cmGlobalGenerator(cm)
{
  cm->GetState()->SetIsGeneratorMultiConfig(true);
  cm->GetState()->SetWindowsShell(true);
  cm->GetState()->SetWindowsVSIDE(true);

  if (platformInGeneratorName.empty()) {
    this->DefaultPlatformName = "Win32";
  } else {
    this->DefaultPlatformName = platformInGeneratorName;
    this->PlatformInGeneratorName = true;
  }
}

cmGlobalVisualStudioGenerator::~cmGlobalVisualStudioGenerator()
{
}

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
  if (this->GetPlatformName() == "x64") {
    mf->AddDefinition("CMAKE_FORCE_WIN64", "TRUE");
  } else if (this->GetPlatformName() == "Itanium") {
    mf->AddDefinition("CMAKE_FORCE_IA64", "TRUE");
  }
  mf->AddDefinition("CMAKE_VS_PLATFORM_NAME", this->GetPlatformName());
  return this->cmGlobalGenerator::SetGeneratorPlatform(p, mf);
}

std::string const& cmGlobalVisualStudioGenerator::GetPlatformName() const
{
  if (!this->GeneratorPlatform.empty()) {
    return this->GeneratorPlatform;
  }
  return this->DefaultPlatformName;
}

const char* cmGlobalVisualStudioGenerator::GetIDEVersion() const
{
  switch (this->Version) {
    case cmGlobalVisualStudioGenerator::VSVersion::VS9:
      return "9.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS10:
      return "10.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS11:
      return "11.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS12:
      return "12.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS14:
      return "14.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS15:
      return "15.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS16:
      return "16.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS17:
      return "17.0";
  }
  return "";
}

void cmGlobalVisualStudioGenerator::WriteSLNHeader(std::ostream& fout)
{
  char utf8bom[] = { char(0xEF), char(0xBB), char(0xBF) };
  fout.write(utf8bom, 3);
  fout << '\n';

  switch (this->Version) {
    case cmGlobalVisualStudioGenerator::VSVersion::VS9:
      fout << "Microsoft Visual Studio Solution File, Format Version 10.00\n";
      fout << "# Visual Studio 2008\n";
      break;
    case cmGlobalVisualStudioGenerator::VSVersion::VS10:
      fout << "Microsoft Visual Studio Solution File, Format Version 11.00\n";
      if (this->ExpressEdition) {
        fout << "# Visual C++ Express 2010\n";
      } else {
        fout << "# Visual Studio 2010\n";
      }
      break;
    case cmGlobalVisualStudioGenerator::VSVersion::VS11:
      fout << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
      if (this->ExpressEdition) {
        fout << "# Visual Studio Express 2012 for Windows Desktop\n";
      } else {
        fout << "# Visual Studio 2012\n";
      }
      break;
    case cmGlobalVisualStudioGenerator::VSVersion::VS12:
      fout << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
      if (this->ExpressEdition) {
        fout << "# Visual Studio Express 2013 for Windows Desktop\n";
      } else {
        fout << "# Visual Studio 2013\n";
      }
      break;
    case cmGlobalVisualStudioGenerator::VSVersion::VS14:
      // Visual Studio 14 writes .sln format 12.00
      fout << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
      if (this->ExpressEdition) {
        fout << "# Visual Studio Express 14 for Windows Desktop\n";
      } else {
        fout << "# Visual Studio 14\n";
      }
      break;
    case cmGlobalVisualStudioGenerator::VSVersion::VS15:
      // Visual Studio 15 writes .sln format 12.00
      fout << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
      if (this->ExpressEdition) {
        fout << "# Visual Studio Express 15 for Windows Desktop\n";
      } else {
        fout << "# Visual Studio 15\n";
      }
      break;
    case cmGlobalVisualStudioGenerator::VSVersion::VS16:
      // Visual Studio 16 writes .sln format 12.00
      fout << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
      if (this->ExpressEdition) {
        fout << "# Visual Studio Express 16 for Windows Desktop\n";
      } else {
        fout << "# Visual Studio Version 16\n";
      }
      break;
    case cmGlobalVisualStudioGenerator::VSVersion::VS17:
      // Visual Studio 17 writes .sln format 12.00
      fout << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
      if (this->ExpressEdition) {
        fout << "# Visual Studio Express 17 for Windows Desktop\n";
      } else {
        fout << "# Visual Studio Version 17\n";
      }
      break;
  }
}

std::string cmGlobalVisualStudioGenerator::GetRegistryBase()
{
  return cmGlobalVisualStudioGenerator::GetRegistryBase(this->GetIDEVersion());
}

std::string cmGlobalVisualStudioGenerator::GetRegistryBase(const char* version)
{
  std::string key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\";
  return key + version;
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
      cc->SetCMP0116Status(cmPolicies::NEW);
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
        for (const auto& tgt : i->GetGeneratorTargets()) {
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
    cmStrCat(gt->LocalGenerator->GetCurrentBinaryDirectory(), '/');
  std::string tgtDir = gt->LocalGenerator->GetTargetDirectory(gt);
  if (!tgtDir.empty()) {
    dir += tgtDir;
    dir += "/";
  }
  const char* cd = this->GetCMakeCFGIntDir();
  if (cd && *cd) {
    dir += cd;
    dir += "/";
  }
  gt->ObjectDirectory = dir;
}

bool IsVisualStudioMacrosFileRegistered(const std::string& macrosFile,
                                        const std::string& regKeyBase,
                                        std::string& nextAvailableSubKeyName);

void RegisterVisualStudioMacros(const std::string& macrosFile,
                                const std::string& regKeyBase);

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

    std::string dst = dir + "/CMakeMacros/" CMAKE_VSMACROS_FILENAME;

    // Copy the macros file to the user directory only if the
    // destination does not exist or the source location is newer.
    // This will allow the user to edit the macros for development
    // purposes but newer versions distributed with CMake will replace
    // older versions in user directories.
    int res;
    if (!cmSystemTools::FileTimeCompare(src, dst, &res) || res > 0) {
      if (!cmSystemTools::CopyFileAlways(src, dst)) {
        std::ostringstream oss;
        oss << "Could not copy from: " << src << std::endl;
        oss << "                 to: " << dst << std::endl;
        cmSystemTools::Message(oss.str(), "Warning");
      }
    }

    RegisterVisualStudioMacros(dst, this->GetUserMacrosRegKeyBase());
  }
}

void cmGlobalVisualStudioGenerator::CallVisualStudioMacro(
  MacroName m, const std::string& vsSolutionFile)
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
    std::string macrosFile = dir + "/CMakeMacros/" CMAKE_VSMACROS_FILENAME;
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

void cmGlobalVisualStudioGenerator::FillLinkClosure(
  const cmGeneratorTarget* target, TargetSet& linked)
{
  if (linked.insert(target).second) {
    TargetDependSet const& depends = this->GetTargetDirectDepends(target);
    for (cmTargetDepend const& di : depends) {
      if (di.IsLink()) {
        this->FillLinkClosure(di, linked);
      }
    }
  }
}

cmGlobalVisualStudioGenerator::TargetSet const&
cmGlobalVisualStudioGenerator::GetTargetLinkClosure(cmGeneratorTarget* target)
{
  auto i = this->TargetLinkClosure.find(target);
  if (i == this->TargetLinkClosure.end()) {
    TargetSetMap::value_type entry(target, TargetSet());
    i = this->TargetLinkClosure.insert(entry).first;
    this->FillLinkClosure(target, i->second);
  }
  return i->second;
}

void cmGlobalVisualStudioGenerator::FollowLinkDepends(
  const cmGeneratorTarget* target, std::set<const cmGeneratorTarget*>& linked)
{
  if (!target->IsInBuildSystem()) {
    return;
  }
  if (linked.insert(target).second &&
      target->GetType() == cmStateEnums::STATIC_LIBRARY) {
    // Static library targets do not list their link dependencies so
    // we must follow them transitively now.
    TargetDependSet const& depends = this->GetTargetDirectDepends(target);
    for (cmTargetDepend const& di : depends) {
      if (di.IsLink()) {
        this->FollowLinkDepends(di, linked);
      }
    }
  }
}

bool cmGlobalVisualStudioGenerator::ComputeTargetDepends()
{
  if (!this->cmGlobalGenerator::ComputeTargetDepends()) {
    return false;
  }
  for (auto const& it : this->ProjectMap) {
    for (const cmLocalGenerator* i : it.second) {
      for (const auto& ti : i->GetGeneratorTargets()) {
        this->ComputeVSTargetDepends(ti.get());
      }
    }
  }
  return true;
}

static bool VSLinkable(cmGeneratorTarget const* t)
{
  return t->IsLinkable() || t->GetType() == cmStateEnums::OBJECT_LIBRARY;
}

void cmGlobalVisualStudioGenerator::ComputeVSTargetDepends(
  cmGeneratorTarget* target)
{
  if (this->VSTargetDepends.find(target) != this->VSTargetDepends.end()) {
    return;
  }
  VSDependSet& vsTargetDepend = this->VSTargetDepends[target];
  // VS <= 7.1 has two behaviors that affect solution dependencies.
  //
  // (1) Solution-level dependencies between a linkable target and a
  // library cause that library to be linked.  We use an intermedite
  // empty utility target to express the dependency.  (VS 8 and above
  // provide a project file "LinkLibraryDependencies" setting to
  // choose whether to activate this behavior.  We disable it except
  // when linking external project files.)
  //
  // (2) We cannot let static libraries depend directly on targets to
  // which they "link" because the librarian tool will copy the
  // targets into the static library.  While the work-around for
  // behavior (1) would also avoid this, it would create a large
  // number of extra utility targets for little gain.  Instead, use
  // the above work-around only for dependencies explicitly added by
  // the add_dependencies() command.  Approximate link dependencies by
  // leaving them out for the static library itself but following them
  // transitively for other targets.

  bool allowLinkable = (target->GetType() != cmStateEnums::STATIC_LIBRARY &&
                        target->GetType() != cmStateEnums::SHARED_LIBRARY &&
                        target->GetType() != cmStateEnums::MODULE_LIBRARY &&
                        target->GetType() != cmStateEnums::EXECUTABLE);

  TargetDependSet const& depends = this->GetTargetDirectDepends(target);

  // Collect implicit link dependencies (target_link_libraries).
  // Static libraries cannot depend on their link implementation
  // due to behavior (2), but they do not really need to.
  std::set<cmGeneratorTarget const*> linkDepends;
  if (target->GetType() != cmStateEnums::STATIC_LIBRARY) {
    for (cmTargetDepend const& di : depends) {
      if (di.IsLink()) {
        this->FollowLinkDepends(di, linkDepends);
      }
    }
  }

  // Collect explicit util dependencies (add_dependencies).
  std::set<cmGeneratorTarget const*> utilDepends;
  for (cmTargetDepend const& di : depends) {
    if (di.IsUtil()) {
      this->FollowLinkDepends(di, utilDepends);
    }
  }

  // Collect all targets linked by this target so we can avoid
  // intermediate targets below.
  TargetSet linked;
  if (target->GetType() != cmStateEnums::STATIC_LIBRARY) {
    linked = this->GetTargetLinkClosure(target);
  }

  // Emit link dependencies.
  for (cmGeneratorTarget const* dep : linkDepends) {
    vsTargetDepend.insert(dep->GetName());
  }

  // Emit util dependencies.  Possibly use intermediate targets.
  for (cmGeneratorTarget const* dgt : utilDepends) {
    if (allowLinkable || !VSLinkable(dgt) || linked.count(dgt)) {
      // Direct dependency allowed.
      vsTargetDepend.insert(dgt->GetName());
    } else {
      // Direct dependency on linkable target not allowed.
      // Use an intermediate utility target.
      vsTargetDepend.insert(this->GetUtilityDepend(dgt));
    }
  }
}

bool cmGlobalVisualStudioGenerator::FindMakeProgram(cmMakefile* mf)
{
  // Visual Studio generators know how to lookup their build tool
  // directly instead of needing a helper module to do it, so we
  // do not actually need to put CMAKE_MAKE_PROGRAM into the cache.
  if (cmIsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM"))) {
    mf->AddDefinition("CMAKE_MAKE_PROGRAM", this->GetVSMakeProgram());
  }
  return true;
}

std::string cmGlobalVisualStudioGenerator::GetUtilityDepend(
  cmGeneratorTarget const* target)
{
  auto i = this->UtilityDepends.find(target);
  if (i == this->UtilityDepends.end()) {
    std::string name = this->WriteUtilityDepend(target);
    UtilityDependsMap::value_type entry(target, name);
    i = this->UtilityDepends.insert(entry).first;
  }
  return i->second;
}

std::string cmGlobalVisualStudioGenerator::GetStartupProjectName(
  cmLocalGenerator const* root) const
{
  cmValue n = root->GetMakefile()->GetProperty("VS_STARTUP_PROJECT");
  if (cmNonempty(n)) {
    std::string startup = *n;
    if (this->FindTarget(startup)) {
      return startup;
    } else {
      root->GetMakefile()->IssueMessage(
        MessageType::AUTHOR_WARNING,
        "Directory property VS_STARTUP_PROJECT specifies target "
        "'" +
          startup + "' that does not exist.  Ignoring.");
    }
  }

  // default, if not specified
  return this->GetAllTargetName();
}

bool IsVisualStudioMacrosFileRegistered(const std::string& macrosFile,
                                        const std::string& regKeyBase,
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
  HKEY hkey = NULL;
  LONG result = ERROR_SUCCESS;
  DWORD index = 0;

  keyname = regKeyBase + "\\OtherProjects7";
  hkey = NULL;
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
      HKEY hsubkey = NULL;
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
        std::cout << "error opening subkey: " << subkeyname << std::endl;
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

  keyname = regKeyBase + "\\RecordingProject7";
  hkey = NULL;
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

void WriteVSMacrosFileRegistryEntry(const std::string& nextAvailableSubKeyName,
                                    const std::string& macrosFile,
                                    const std::string& regKeyBase)
{
  std::string keyname = regKeyBase + "\\OtherProjects7";
  HKEY hkey = NULL;
  LONG result =
    RegOpenKeyExW(HKEY_CURRENT_USER, cmsys::Encoding::ToWide(keyname).c_str(),
                  0, KEY_READ | KEY_WRITE, &hkey);
  if (ERROR_SUCCESS == result) {
    // Create the subkey and set the values of interest:
    HKEY hsubkey = NULL;
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

void RegisterVisualStudioMacros(const std::string& macrosFile,
                                const std::string& regKeyBase)
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
  cmGeneratorTarget const* gt)
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

  return languages.size() == 1 && *languages.begin() == "Fortran";
}

bool cmGlobalVisualStudioGenerator::IsInSolution(
  const cmGeneratorTarget* gt) const
{
  return gt->IsInBuildSystem();
}

bool cmGlobalVisualStudioGenerator::IsDepInSolution(
  const std::string& targetName) const
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
  const std::string& str, const std::string& config) const
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
  std::map<cmSourceFile const*, std::string> mapping;
  for (cmSourceFile const* it : objectSources) {
    mapping[it];
  }
  gt->LocalGenerator->ComputeObjectFilenames(mapping, gt);
  std::string obj_dir = gt->ObjectDirectory;
  std::string cmakeCommand = cmSystemTools::GetCMakeCommand();
  std::string obj_dir_expanded = obj_dir;
  cmSystemTools::ReplaceString(obj_dir_expanded, this->GetCMakeCFGIntDir(),
                               configName.c_str());
  cmSystemTools::MakeDirectory(obj_dir_expanded);
  std::string const objs_file = obj_dir_expanded + "/objects.txt";
  cmGeneratedFileStream fout(objs_file.c_str());
  if (!fout) {
    cmSystemTools::Error("could not open " + objs_file);
    return;
  }

  if (mdi->WindowsExportAllSymbols) {
    std::vector<std::string> objs;
    for (cmSourceFile const* it : objectSources) {
      // Find the object file name corresponding to this source file.
      // It must exist because we populated the mapping just above.
      const auto& v = mapping[it];
      assert(!v.empty());
      std::string objFile = obj_dir + v;
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

static bool OpenSolution(std::string sln)
{
  HRESULT comInitialized =
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  if (FAILED(comInitialized)) {
    return false;
  }

  HINSTANCE hi =
    ShellExecuteA(NULL, "open", sln.c_str(), NULL, NULL, SW_SHOWNORMAL);

  CoUninitialize();

  return reinterpret_cast<intptr_t>(hi) > 32;
}

bool cmGlobalVisualStudioGenerator::Open(const std::string& bindir,
                                         const std::string& projectName,
                                         bool dryRun)
{
  std::string sln = bindir + "/" + projectName + ".sln";

  if (dryRun) {
    return cmSystemTools::FileExists(sln, true);
  }

  sln = cmSystemTools::ConvertToOutputPath(sln);

  return std::async(std::launch::async, OpenSolution, sln).get();
}
