/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportCommand.h"

#include "cmsys/RegularExpression.hxx"
#include <map>
#include <sstream>

#include "cmExportBuildAndroidMKGenerator.h"
#include "cmExportBuildFileGenerator.h"
#include "cmExportSetMap.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

class cmExecutionStatus;

#if defined(__HAIKU__)
#  include <FindDirectory.h>
#  include <StorageDefs.h>
#endif

cmExportCommand::cmExportCommand()
  : Targets(&Helper, "TARGETS")
  , Append(&Helper, "APPEND", &ArgumentGroup)
  , ExportSetName(&Helper, "EXPORT", &ArgumentGroup)
  , Namespace(&Helper, "NAMESPACE", &ArgumentGroup)
  , Filename(&Helper, "FILE", &ArgumentGroup)
  , ExportOld(&Helper, "EXPORT_LINK_INTERFACE_LIBRARIES", &ArgumentGroup)
  , AndroidMKFile(&Helper, "ANDROID_MK")
{
  this->ExportSet = nullptr;
}

// cmExportCommand
bool cmExportCommand::InitialPass(std::vector<std::string> const& args,
                                  cmExecutionStatus&)
{
  if (args.size() < 2) {
    this->SetError("called with too few arguments");
    return false;
  }

  if (args[0] == "PACKAGE") {
    return this->HandlePackage(args);
  }
  if (args[0] == "EXPORT") {
    this->ExportSetName.Follows(nullptr);
    this->ArgumentGroup.Follows(&this->ExportSetName);
  } else {
    this->Targets.Follows(nullptr);
    this->ArgumentGroup.Follows(&this->Targets);
  }

  std::vector<std::string> unknownArgs;
  this->Helper.Parse(&args, &unknownArgs);

  if (!unknownArgs.empty()) {
    this->SetError("Unknown arguments.");
    return false;
  }

  std::string fname;
  bool android = false;
  if (this->AndroidMKFile.WasFound()) {
    fname = this->AndroidMKFile.GetString();
    android = true;
  }
  if (!this->Filename.WasFound() && fname.empty()) {
    if (args[0] != "EXPORT") {
      this->SetError("FILE <filename> option missing.");
      return false;
    }
    fname = this->ExportSetName.GetString() + ".cmake";
  } else if (fname.empty()) {
    // Make sure the file has a .cmake extension.
    if (cmSystemTools::GetFilenameLastExtension(this->Filename.GetCString()) !=
        ".cmake") {
      std::ostringstream e;
      e << "FILE option given filename \"" << this->Filename.GetString()
        << "\" which does not have an extension of \".cmake\".\n";
      this->SetError(e.str());
      return false;
    }
    fname = this->Filename.GetString();
  }

  // Get the file to write.
  if (cmSystemTools::FileIsFullPath(fname)) {
    if (!this->Makefile->CanIWriteThisFile(fname)) {
      std::ostringstream e;
      e << "FILE option given filename \"" << fname
        << "\" which is in the source tree.\n";
      this->SetError(e.str());
      return false;
    }
  } else {
    // Interpret relative paths with respect to the current build dir.
    std::string dir = this->Makefile->GetCurrentBinaryDirectory();
    fname = dir + "/" + fname;
  }

  std::vector<std::string> targets;

  cmGlobalGenerator* gg = this->Makefile->GetGlobalGenerator();

  if (args[0] == "EXPORT") {
    if (this->Append.IsEnabled()) {
      std::ostringstream e;
      e << "EXPORT signature does not recognise the APPEND option.";
      this->SetError(e.str());
      return false;
    }

    if (this->ExportOld.IsEnabled()) {
      std::ostringstream e;
      e << "EXPORT signature does not recognise the "
           "EXPORT_LINK_INTERFACE_LIBRARIES option.";
      this->SetError(e.str());
      return false;
    }

    cmExportSetMap& setMap = gg->GetExportSets();
    std::string setName = this->ExportSetName.GetString();
    if (setMap.find(setName) == setMap.end()) {
      std::ostringstream e;
      e << "Export set \"" << setName << "\" not found.";
      this->SetError(e.str());
      return false;
    }
    this->ExportSet = setMap[setName];
  } else if (this->Targets.WasFound()) {
    for (std::string const& currentTarget : this->Targets.GetVector()) {
      if (this->Makefile->IsAlias(currentTarget)) {
        std::ostringstream e;
        e << "given ALIAS target \"" << currentTarget
          << "\" which may not be exported.";
        this->SetError(e.str());
        return false;
      }

      if (cmTarget* target = gg->FindTarget(currentTarget)) {
        if (target->GetType() == cmStateEnums::UTILITY) {
          this->SetError("given custom target \"" + currentTarget +
                         "\" which may not be exported.");
          return false;
        }
      } else {
        std::ostringstream e;
        e << "given target \"" << currentTarget
          << "\" which is not built by this project.";
        this->SetError(e.str());
        return false;
      }
      targets.push_back(currentTarget);
    }
    if (this->Append.IsEnabled()) {
      if (cmExportBuildFileGenerator* ebfg =
            gg->GetExportedTargetsFile(fname)) {
        ebfg->AppendTargets(targets);
        return true;
      }
    }
  } else {
    this->SetError("EXPORT or TARGETS specifier missing.");
    return false;
  }

  // Setup export file generation.
  cmExportBuildFileGenerator* ebfg = nullptr;
  if (android) {
    ebfg = new cmExportBuildAndroidMKGenerator;
  } else {
    ebfg = new cmExportBuildFileGenerator;
  }
  ebfg->SetExportFile(fname.c_str());
  ebfg->SetNamespace(this->Namespace.GetCString());
  ebfg->SetAppendMode(this->Append.IsEnabled());
  if (this->ExportSet) {
    ebfg->SetExportSet(this->ExportSet);
  } else {
    ebfg->SetTargets(targets);
  }
  this->Makefile->AddExportBuildFileGenerator(ebfg);
  ebfg->SetExportOld(this->ExportOld.IsEnabled());

  // Compute the set of configurations exported.
  std::vector<std::string> configurationTypes;
  this->Makefile->GetConfigurations(configurationTypes);
  if (configurationTypes.empty()) {
    configurationTypes.emplace_back();
  }
  for (std::string const& ct : configurationTypes) {
    ebfg->AddConfiguration(ct);
  }
  if (this->ExportSet) {
    gg->AddBuildExportExportSet(ebfg);
  } else {
    gg->AddBuildExportSet(ebfg);
  }

  return true;
}

bool cmExportCommand::HandlePackage(std::vector<std::string> const& args)
{
  // Parse PACKAGE mode arguments.
  enum Doing
  {
    DoingNone,
    DoingPackage
  };
  Doing doing = DoingPackage;
  std::string package;
  for (unsigned int i = 1; i < args.size(); ++i) {
    if (doing == DoingPackage) {
      package = args[i];
      doing = DoingNone;
    } else {
      std::ostringstream e;
      e << "PACKAGE given unknown argument: " << args[i];
      this->SetError(e.str());
      return false;
    }
  }

  // Verify the package name.
  if (package.empty()) {
    this->SetError("PACKAGE must be given a package name.");
    return false;
  }
  const char* packageExpr = "^[A-Za-z0-9_.-]+$";
  cmsys::RegularExpression packageRegex(packageExpr);
  if (!packageRegex.find(package)) {
    std::ostringstream e;
    e << "PACKAGE given invalid package name \"" << package << "\".  "
      << "Package names must match \"" << packageExpr << "\".";
    this->SetError(e.str());
    return false;
  }

  // CMP0090 decides both the default and what variable changes it.
  switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0090)) {
    case cmPolicies::WARN:
    case cmPolicies::OLD:
      // Default is to export, but can be disabled.
      if (this->Makefile->IsOn("CMAKE_EXPORT_NO_PACKAGE_REGISTRY")) {
        return true;
      }
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      // Default is to not export, but can be enabled.
      if (!this->Makefile->IsOn("CMAKE_EXPORT_PACKAGE_REGISTRY")) {
        return true;
      }
      break;
  }

  // We store the current build directory in the registry as a value
  // named by a hash of its own content.  This is deterministic and is
  // unique with high probability.
  const std::string& outDir = this->Makefile->GetCurrentBinaryDirectory();
  std::string hash = cmSystemTools::ComputeStringMD5(outDir);
#if defined(_WIN32) && !defined(__CYGWIN__)
  this->StorePackageRegistryWin(package, outDir.c_str(), hash.c_str());
#else
  this->StorePackageRegistryDir(package, outDir.c_str(), hash.c_str());
#endif

  return true;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <windows.h>

void cmExportCommand::ReportRegistryError(std::string const& msg,
                                          std::string const& key, long err)
{
  std::ostringstream e;
  e << msg << "\n"
    << "  HKEY_CURRENT_USER\\" << key << "\n";
  wchar_t winmsg[1024];
  if (FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), winmsg, 1024, 0) > 0) {
    e << "Windows reported:\n"
      << "  " << cmsys::Encoding::ToNarrow(winmsg);
  }
  this->Makefile->IssueMessage(MessageType::WARNING, e.str());
}

void cmExportCommand::StorePackageRegistryWin(std::string const& package,
                                              const char* content,
                                              const char* hash)
{
  std::string key = "Software\\Kitware\\CMake\\Packages\\";
  key += package;
  HKEY hKey;
  LONG err =
    RegCreateKeyExW(HKEY_CURRENT_USER, cmsys::Encoding::ToWide(key).c_str(), 0,
                    0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0, &hKey, 0);
  if (err != ERROR_SUCCESS) {
    this->ReportRegistryError("Cannot create/open registry key", key, err);
    return;
  }

  std::wstring wcontent = cmsys::Encoding::ToWide(content);
  err =
    RegSetValueExW(hKey, cmsys::Encoding::ToWide(hash).c_str(), 0, REG_SZ,
                   (BYTE const*)wcontent.c_str(),
                   static_cast<DWORD>(wcontent.size() + 1) * sizeof(wchar_t));
  RegCloseKey(hKey);
  if (err != ERROR_SUCCESS) {
    std::ostringstream msg;
    msg << "Cannot set registry value \"" << hash << "\" under key";
    this->ReportRegistryError(msg.str(), key, err);
    return;
  }
}
#else
void cmExportCommand::StorePackageRegistryDir(std::string const& package,
                                              const char* content,
                                              const char* hash)
{
#  if defined(__HAIKU__)
  char dir[B_PATH_NAME_LENGTH];
  if (find_directory(B_USER_SETTINGS_DIRECTORY, -1, false, dir, sizeof(dir)) !=
      B_OK) {
    return;
  }
  std::string fname = dir;
  fname += "/cmake/packages/";
  fname += package;
#  else
  std::string fname;
  if (!cmSystemTools::GetEnv("HOME", fname)) {
    return;
  }
  cmSystemTools::ConvertToUnixSlashes(fname);
  fname += "/.cmake/packages/";
  fname += package;
#  endif
  cmSystemTools::MakeDirectory(fname);
  fname += "/";
  fname += hash;
  if (!cmSystemTools::FileExists(fname)) {
    cmGeneratedFileStream entry(fname, true);
    if (entry) {
      entry << content << "\n";
    } else {
      std::ostringstream e;
      /* clang-format off */
      e << "Cannot create package registry file:\n"
        << "  " << fname << "\n"
        << cmSystemTools::GetLastSystemError() << "\n";
      /* clang-format on */
      this->Makefile->IssueMessage(MessageType::WARNING, e.str());
    }
  }
}
#endif
