/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportCommand.h"

#include <map>
#include <sstream>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmCryptoHash.h"
#include "cmExecutionStatus.h"
#include "cmExportBuildAndroidMKGenerator.h"
#include "cmExportBuildFileGenerator.h"
#include "cmExportSet.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

#if defined(__HAIKU__)
#  include <FindDirectory.h>
#  include <StorageDefs.h>
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <windows.h>
#endif

static bool HandlePackage(std::vector<std::string> const& args,
                          cmExecutionStatus& status);

static void StorePackageRegistry(cmMakefile& mf, std::string const& package,
                                 const char* content, const char* hash);

bool cmExportCommand(std::vector<std::string> const& args,
                     cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with too few arguments");
    return false;
  }

  if (args[0] == "PACKAGE") {
    return HandlePackage(args, status);
  }

  struct Arguments
  {
    std::string ExportSetName;
    cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> Targets;
    std::string Namespace;
    std::string Filename;
    std::string AndroidMKFile;
    std::string CxxModulesDirectory;
    bool Append = false;
    bool ExportOld = false;
  };

  auto parser =
    cmArgumentParser<Arguments>{}
      .Bind("NAMESPACE"_s, &Arguments::Namespace)
      .Bind("FILE"_s, &Arguments::Filename)
      .Bind("CXX_MODULES_DIRECTORY"_s, &Arguments::CxxModulesDirectory);

  if (args[0] == "EXPORT") {
    parser.Bind("EXPORT"_s, &Arguments::ExportSetName);
  } else {
    parser.Bind("TARGETS"_s, &Arguments::Targets);
    parser.Bind("ANDROID_MK"_s, &Arguments::AndroidMKFile);
    parser.Bind("APPEND"_s, &Arguments::Append);
    parser.Bind("EXPORT_LINK_INTERFACE_LIBRARIES"_s, &Arguments::ExportOld);
  }

  std::vector<std::string> unknownArgs;
  Arguments const arguments = parser.Parse(args, &unknownArgs);

  if (!unknownArgs.empty()) {
    status.SetError("Unknown argument: \"" + unknownArgs.front() + "\".");
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
      status.SetError("FILE <filename> option missing.");
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
      status.SetError(e.str());
      return false;
    }
    fname = arguments.Filename;
  }

  cmMakefile& mf = status.GetMakefile();

  // Get the file to write.
  if (cmSystemTools::FileIsFullPath(fname)) {
    if (!mf.CanIWriteThisFile(fname)) {
      std::ostringstream e;
      e << "FILE option given filename \"" << fname
        << "\" which is in the source tree.\n";
      status.SetError(e.str());
      return false;
    }
  } else {
    // Interpret relative paths with respect to the current build dir.
    std::string const& dir = mf.GetCurrentBinaryDirectory();
    fname = dir + "/" + fname;
  }

  std::vector<std::string> targets;

  cmGlobalGenerator* gg = mf.GetGlobalGenerator();

  cmExportSet* exportSet = nullptr;
  if (args[0] == "EXPORT") {
    cmExportSetMap& setMap = gg->GetExportSets();
    auto const it = setMap.find(arguments.ExportSetName);
    if (it == setMap.end()) {
      std::ostringstream e;
      e << "Export set \"" << arguments.ExportSetName << "\" not found.";
      status.SetError(e.str());
      return false;
    }
    exportSet = &it->second;
  } else if (arguments.Targets) {
    for (std::string const& currentTarget : *arguments.Targets) {
      if (mf.IsAlias(currentTarget)) {
        std::ostringstream e;
        e << "given ALIAS target \"" << currentTarget
          << "\" which may not be exported.";
        status.SetError(e.str());
        return false;
      }

      if (cmTarget* target = gg->FindTarget(currentTarget)) {
        if (target->GetType() == cmStateEnums::UTILITY) {
          status.SetError("given custom target \"" + currentTarget +
                          "\" which may not be exported.");
          return false;
        }
      } else {
        std::ostringstream e;
        e << "given target \"" << currentTarget
          << "\" which is not built by this project.";
        status.SetError(e.str());
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
    status.SetError("EXPORT or TARGETS specifier missing.");
    return false;
  }

  // if cmExportBuildFileGenerator is already defined for the file
  // and APPEND is not specified, if CMP0103 is OLD ignore previous definition
  // else raise an error
  if (gg->GetExportedTargetsFile(fname) != nullptr) {
    switch (mf.GetPolicyStatus(cmPolicies::CMP0103)) {
      case cmPolicies::WARN:
        mf.IssueMessage(
          MessageType::AUTHOR_WARNING,
          cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0103), '\n',
                   "export() command already specified for the file\n  ",
                   arguments.Filename, "\nDid you miss 'APPEND' keyword?"));
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        break;
      default:
        status.SetError(cmStrCat("command already specified for the file\n  ",
                                 arguments.Filename,
                                 "\nDid you miss 'APPEND' keyword?"));
        return false;
    }
  }

  // Setup export file generation.
  std::unique_ptr<cmExportBuildFileGenerator> ebfg = nullptr;
  if (android) {
    ebfg = cm::make_unique<cmExportBuildAndroidMKGenerator>();
  } else {
    ebfg = cm::make_unique<cmExportBuildFileGenerator>();
  }
  ebfg->SetExportFile(fname.c_str());
  ebfg->SetNamespace(arguments.Namespace);
  ebfg->SetCxxModuleDirectory(arguments.CxxModulesDirectory);
  ebfg->SetAppendMode(arguments.Append);
  if (exportSet != nullptr) {
    ebfg->SetExportSet(exportSet);
  } else {
    ebfg->SetTargets(targets);
  }
  ebfg->SetExportOld(arguments.ExportOld);

  // Compute the set of configurations exported.
  std::vector<std::string> configurationTypes =
    mf.GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  for (std::string const& ct : configurationTypes) {
    ebfg->AddConfiguration(ct);
  }
  if (exportSet != nullptr) {
    gg->AddBuildExportExportSet(ebfg.get());
  } else {
    gg->AddBuildExportSet(ebfg.get());
  }
  mf.AddExportBuildFileGenerator(std::move(ebfg));

  return true;
}

static bool HandlePackage(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
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
      status.SetError(e.str());
      return false;
    }
  }

  // Verify the package name.
  if (package.empty()) {
    status.SetError("PACKAGE must be given a package name.");
    return false;
  }
  const char* packageExpr = "^[A-Za-z0-9_.-]+$";
  cmsys::RegularExpression packageRegex(packageExpr);
  if (!packageRegex.find(package)) {
    std::ostringstream e;
    e << "PACKAGE given invalid package name \"" << package << "\".  "
      << "Package names must match \"" << packageExpr << "\".";
    status.SetError(e.str());
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // CMP0090 decides both the default and what variable changes it.
  switch (mf.GetPolicyStatus(cmPolicies::CMP0090)) {
    case cmPolicies::WARN:
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      // Default is to export, but can be disabled.
      if (mf.IsOn("CMAKE_EXPORT_NO_PACKAGE_REGISTRY")) {
        return true;
      }
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      // Default is to not export, but can be enabled.
      if (!mf.IsOn("CMAKE_EXPORT_PACKAGE_REGISTRY")) {
        return true;
      }
      break;
  }

  // We store the current build directory in the registry as a value
  // named by a hash of its own content.  This is deterministic and is
  // unique with high probability.
  const std::string& outDir = mf.GetCurrentBinaryDirectory();
  cmCryptoHash hasher(cmCryptoHash::AlgoMD5);
  std::string hash = hasher.HashString(outDir);
  StorePackageRegistry(mf, package, outDir.c_str(), hash.c_str());

  return true;
}

#if defined(_WIN32) && !defined(__CYGWIN__)

static void ReportRegistryError(cmMakefile& mf, std::string const& msg,
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
  mf.IssueMessage(MessageType::WARNING, e.str());
}

static void StorePackageRegistry(cmMakefile& mf, std::string const& package,
                                 const char* content, const char* hash)
{
  std::string key = cmStrCat("Software\\Kitware\\CMake\\Packages\\", package);
  HKEY hKey;
  LONG err =
    RegCreateKeyExW(HKEY_CURRENT_USER, cmsys::Encoding::ToWide(key).c_str(), 0,
                    0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0, &hKey, 0);
  if (err != ERROR_SUCCESS) {
    ReportRegistryError(mf, "Cannot create/open registry key", key, err);
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
    ReportRegistryError(mf, msg.str(), key, err);
    return;
  }
}
#else
static void StorePackageRegistry(cmMakefile& mf, std::string const& package,
                                 const char* content, const char* hash)
{
#  if defined(__HAIKU__)
  char dir[B_PATH_NAME_LENGTH];
  if (find_directory(B_USER_SETTINGS_DIRECTORY, -1, false, dir, sizeof(dir)) !=
      B_OK) {
    return;
  }
  std::string fname = cmStrCat(dir, "/cmake/packages/", package);
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
      mf.IssueMessage(MessageType::WARNING,
                      cmStrCat("Cannot create package registry file:\n"
                               "  ",
                               fname, '\n',
                               cmSystemTools::GetLastSystemError(), '\n'));
    }
  }
}
#endif
