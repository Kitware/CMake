/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportCommand.h"

#include "cm_static_string_view.hxx"
#include "cmsys/RegularExpression.hxx"

#include <algorithm>
#include <map>
#include <sstream>
#include <utility>

#include "cmArgumentParser.h"
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

class cmExportSet;
class cmExecutionStatus;

#if defined(__HAIKU__)
#  include <FindDirectory.h>
#  include <StorageDefs.h>
#endif

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

  struct Arguments
  {
    std::string ExportSetName;
    std::vector<std::string> Targets;
    std::string Namespace;
    std::string Filename;
    std::string AndroidMKFile;
    bool Append = false;
    bool ExportOld = false;
  };

  auto parser = cmArgumentParser<Arguments>{}
                  .Bind("NAMESPACE"_s, &Arguments::Namespace)
                  .Bind("FILE"_s, &Arguments::Filename);

  if (args[0] == "EXPORT") {
    parser.Bind("EXPORT"_s, &Arguments::ExportSetName);
  } else {
    parser.Bind("TARGETS"_s, &Arguments::Targets);
    parser.Bind("ANDROID_MK"_s, &Arguments::AndroidMKFile);
    parser.Bind("APPEND"_s, &Arguments::Append);
    parser.Bind("EXPORT_LINK_INTERFACE_LIBRARIES"_s, &Arguments::ExportOld);
  }

  std::vector<std::string> unknownArgs;
  std::vector<std::string> keywordsMissingValue;
  Arguments const arguments =
    parser.Parse(args, &unknownArgs, &keywordsMissingValue);

  if (!unknownArgs.empty()) {
    this->SetError("Unknown argument: \"" + unknownArgs.front() + "\".");
    return false;
  }

  std::string fname;
  bool android = false;
  if (!arguments.AndroidMKFile.empty()) {
    fname = arguments.AndroidMKFile;
    android = true;
  }
  if (arguments.Filename.empty() && fname.empty()) {
    if (args[0] != "EXPORT") {
      this->SetError("FILE <filename> option missing.");
      return false;
    }
    fname = arguments.ExportSetName + ".cmake";
  } else if (fname.empty()) {
    // Make sure the file has a .cmake extension.
    if (cmSystemTools::GetFilenameLastExtension(arguments.Filename) !=
        ".cmake") {
      std::ostringstream e;
      e << "FILE option given filename \"" << arguments.Filename
        << "\" which does not have an extension of \".cmake\".\n";
      this->SetError(e.str());
      return false;
    }
    fname = arguments.Filename;
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

  cmExportSet* ExportSet = nullptr;
  if (args[0] == "EXPORT") {
    cmExportSetMap& setMap = gg->GetExportSets();
    auto const it = setMap.find(arguments.ExportSetName);
    if (it == setMap.end()) {
      std::ostringstream e;
      e << "Export set \"" << arguments.ExportSetName << "\" not found.";
      this->SetError(e.str());
      return false;
    }
    ExportSet = it->second;
  } else if (!arguments.Targets.empty() ||
             std::find(keywordsMissingValue.begin(),
                       keywordsMissingValue.end(),
                       "TARGETS") != keywordsMissingValue.end()) {
    for (std::string const& currentTarget : arguments.Targets) {
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
    if (arguments.Append) {
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
  ebfg->SetNamespace(arguments.Namespace);
  ebfg->SetAppendMode(arguments.Append);
  if (ExportSet != nullptr) {
    ebfg->SetExportSet(ExportSet);
  } else {
    ebfg->SetTargets(targets);
  }
  this->Makefile->AddExportBuildFileGenerator(ebfg);
  ebfg->SetExportOld(arguments.ExportOld);

  // Compute the set of configurations exported.
  std::vector<std::string> configurationTypes;
  this->Makefile->GetConfigurations(configurationTypes);
  if (configurationTypes.empty()) {
    configurationTypes.emplace_back();
  }
  for (std::string const& ct : configurationTypes) {
    ebfg->AddConfiguration(ct);
  }
  if (ExportSet != nullptr) {
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
