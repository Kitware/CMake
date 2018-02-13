/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGen.h"
#include "cmQtAutoGeneratorInitializer.h"

#include "cmAlgorithms.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmFilePathChecksum.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmProcessOutput.h"
#include "cmSourceFile.h"
#include "cmSourceGroup.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cm_sys_stat.h"
#include "cmake.h"
#include "cmsys/FStream.hxx"

#include <algorithm>
#include <array>
#include <deque>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

inline static const char* SafeString(const char* value)
{
  return (value != nullptr) ? value : "";
}

inline static std::string GetSafeProperty(cmGeneratorTarget const* target,
                                          const char* key)
{
  return std::string(SafeString(target->GetProperty(key)));
}

inline static std::string GetSafeProperty(cmSourceFile const* sf,
                                          const char* key)
{
  return std::string(SafeString(sf->GetProperty(key)));
}

static void AddDefinitionEscaped(cmMakefile* makefile, const char* key,
                                 std::string const& value)
{
  makefile->AddDefinition(key,
                          cmOutputConverter::EscapeForCMake(value).c_str());
}

static void AddDefinitionEscaped(cmMakefile* makefile, const char* key,
                                 const std::vector<std::string>& values)
{
  makefile->AddDefinition(
    key, cmOutputConverter::EscapeForCMake(cmJoin(values, ";")).c_str());
}

static void AddDefinitionEscaped(cmMakefile* makefile, const char* key,
                                 const std::set<std::string>& values)
{
  makefile->AddDefinition(
    key, cmOutputConverter::EscapeForCMake(cmJoin(values, ";")).c_str());
}

static void AddDefinitionEscaped(
  cmMakefile* makefile, const char* key,
  const std::vector<std::vector<std::string>>& lists)
{
  std::vector<std::string> seplist;
  for (const std::vector<std::string>& list : lists) {
    std::string blist = "{";
    blist += cmJoin(list, ";");
    blist += "}";
    seplist.push_back(std::move(blist));
  }
  makefile->AddDefinition(key, cmOutputConverter::EscapeForCMake(
                                 cmJoin(seplist, cmQtAutoGen::listSep))
                                 .c_str());
}

static bool AddToSourceGroup(cmMakefile* makefile, std::string const& fileName,
                             cmQtAutoGen::Generator genType)
{
  cmSourceGroup* sourceGroup = nullptr;
  // Acquire source group
  {
    std::string property;
    std::string groupName;
    {
      std::array<std::string, 2> props;
      // Use generator specific group name
      switch (genType) {
        case cmQtAutoGen::MOC:
          props[0] = "AUTOMOC_SOURCE_GROUP";
          break;
        case cmQtAutoGen::RCC:
          props[0] = "AUTORCC_SOURCE_GROUP";
          break;
        default:
          props[0] = "AUTOGEN_SOURCE_GROUP";
          break;
      }
      props[1] = "AUTOGEN_SOURCE_GROUP";
      for (std::string& prop : props) {
        const char* propName = makefile->GetState()->GetGlobalProperty(prop);
        if ((propName != nullptr) && (*propName != '\0')) {
          groupName = propName;
          property = std::move(prop);
          break;
        }
      }
    }
    // Generate a source group on demand
    if (!groupName.empty()) {
      sourceGroup = makefile->GetOrCreateSourceGroup(groupName);
      if (sourceGroup == nullptr) {
        std::ostringstream ost;
        ost << cmQtAutoGen::GeneratorNameUpper(genType);
        ost << ": " << property;
        ost << ": Could not find or create the source group ";
        ost << cmQtAutoGen::Quoted(groupName);
        cmSystemTools::Error(ost.str().c_str());
        return false;
      }
    }
  }
  if (sourceGroup != nullptr) {
    sourceGroup->AddGroupFile(fileName);
  }
  return true;
}

static void AddCleanFile(cmMakefile* makefile, std::string const& fileName)
{
  makefile->AppendProperty("ADDITIONAL_MAKE_CLEAN_FILES", fileName.c_str(),
                           false);
}

static std::string FileProjectRelativePath(cmMakefile* makefile,
                                           std::string const& fileName)
{
  std::string res;
  {
    std::string pSource = cmSystemTools::RelativePath(
      makefile->GetCurrentSourceDirectory(), fileName.c_str());
    std::string pBinary = cmSystemTools::RelativePath(
      makefile->GetCurrentBinaryDirectory(), fileName.c_str());
    if (pSource.size() < pBinary.size()) {
      res = std::move(pSource);
    } else if (pBinary.size() < fileName.size()) {
      res = std::move(pBinary);
    } else {
      res = fileName;
    }
  }
  return res;
}

/* @brief Tests if targetDepend is a STATIC_LIBRARY and if any of its
 * recursive STATIC_LIBRARY dependencies depends on targetOrigin
 * (STATIC_LIBRARY cycle).
 */
static bool StaticLibraryCycle(cmGeneratorTarget const* targetOrigin,
                               cmGeneratorTarget const* targetDepend,
                               std::string const& config)
{
  bool cycle = false;
  if ((targetOrigin->GetType() == cmStateEnums::STATIC_LIBRARY) &&
      (targetDepend->GetType() == cmStateEnums::STATIC_LIBRARY)) {
    std::set<cmGeneratorTarget const*> knownLibs;
    std::deque<cmGeneratorTarget const*> testLibs;

    // Insert initial static_library dependency
    knownLibs.insert(targetDepend);
    testLibs.push_back(targetDepend);

    while (!testLibs.empty()) {
      cmGeneratorTarget const* testTarget = testLibs.front();
      testLibs.pop_front();
      // Check if the test target is the origin target (cycle)
      if (testTarget == targetOrigin) {
        cycle = true;
        break;
      }
      // Collect all static_library dependencies from the test target
      cmLinkImplementationLibraries const* libs =
        testTarget->GetLinkImplementationLibraries(config);
      if (libs != nullptr) {
        for (cmLinkItem const& item : libs->Libraries) {
          cmGeneratorTarget const* depTarget = item.Target;
          if ((depTarget != nullptr) &&
              (depTarget->GetType() == cmStateEnums::STATIC_LIBRARY) &&
              knownLibs.insert(depTarget).second) {
            testLibs.push_back(depTarget);
          }
        }
      }
    }
  }
  return cycle;
}

cmQtAutoGeneratorInitializer::cmQtAutoGeneratorInitializer(
  cmGeneratorTarget* target, bool mocEnabled, bool uicEnabled, bool rccEnabled,
  std::string const& qtVersionMajor)
  : Target(target)
  , MocEnabled(mocEnabled)
  , UicEnabled(uicEnabled)
  , RccEnabled(rccEnabled)
  , QtVersionMajor(qtVersionMajor)
  , MultiConfig(cmQtAutoGen::WRAP)
{
  this->QtVersionMinor = cmQtAutoGeneratorInitializer::GetQtMinorVersion(
    target, this->QtVersionMajor);
}

void cmQtAutoGeneratorInitializer::InitCustomTargets()
{
  cmMakefile* makefile = this->Target->Target->GetMakefile();
  cmLocalGenerator* localGen = this->Target->GetLocalGenerator();
  cmGlobalGenerator* globalGen = localGen->GetGlobalGenerator();

  // Configurations
  this->ConfigDefault = makefile->GetConfigurations(this->ConfigsList);
  if (this->ConfigsList.empty()) {
    this->ConfigsList.push_back(this->ConfigDefault);
  }

  // Multi configuration
  {
    if (!globalGen->IsMultiConfig()) {
      this->MultiConfig = cmQtAutoGen::SINGLE;
    }

    // FIXME: Xcode does not support per-config sources, yet.
    //        (EXCLUDED_SOURCE_FILE_NAMES)
    // if (globalGen->GetName().find("Xcode") != std::string::npos) {
    //  return cmQtAutoGen::FULL;
    //}

    // FIXME: Visual Studio does not support per-config sources, yet.
    //        (EXCLUDED_SOURCE_FILE_NAMES)
    // if (globalGen->GetName().find("Visual Studio") != std::string::npos) {
    //  return cmQtAutoGen::FULL;
    //}
  }

  // Autogen target name
  this->AutogenTargetName = this->Target->GetName();
  this->AutogenTargetName += "_autogen";

  // Autogen directories
  {
    // Collapsed current binary directory
    std::string const cbd = cmSystemTools::CollapseFullPath(
      "", makefile->GetCurrentBinaryDirectory());

    // Autogen info dir
    this->DirInfo = cbd;
    this->DirInfo += makefile->GetCMakeInstance()->GetCMakeFilesDirectory();
    this->DirInfo += "/";
    this->DirInfo += this->AutogenTargetName;
    this->DirInfo += ".dir";
    cmSystemTools::ConvertToUnixSlashes(this->DirInfo);

    // Autogen build dir
    this->DirBuild = GetSafeProperty(this->Target, "AUTOGEN_BUILD_DIR");
    if (this->DirBuild.empty()) {
      this->DirBuild = cbd;
      this->DirBuild += "/";
      this->DirBuild += this->AutogenTargetName;
    }
    cmSystemTools::ConvertToUnixSlashes(this->DirBuild);

    // Working directory
    this->DirWork = cbd;
    cmSystemTools::ConvertToUnixSlashes(this->DirWork);
  }

  // Autogen files
  {
    this->AutogenInfoFile = this->DirInfo;
    this->AutogenInfoFile += "/AutogenInfo.cmake";

    this->AutogenSettingsFile = this->DirInfo;
    this->AutogenSettingsFile += "/AutogenOldSettings.cmake";
  }

  // Autogen target FOLDER property
  {
    const char* folder =
      makefile->GetState()->GetGlobalProperty("AUTOMOC_TARGETS_FOLDER");
    if (folder == nullptr) {
      folder =
        makefile->GetState()->GetGlobalProperty("AUTOGEN_TARGETS_FOLDER");
    }
    // Inherit FOLDER property from target (#13688)
    if (folder == nullptr) {
      folder = SafeString(this->Target->Target->GetProperty("FOLDER"));
    }
    if (folder != nullptr) {
      this->AutogenFolder = folder;
    }
  }

  std::set<std::string> autogenDependFiles;
  std::set<cmTarget*> autogenDependTargets;
  std::vector<std::string> autogenProvides;

  // Remove build directories on cleanup
  AddCleanFile(makefile, this->DirBuild);
  // Remove old settings on cleanup
  {
    std::string base = this->DirInfo;
    base += "/AutogenOldSettings";
    if (this->MultiConfig == cmQtAutoGen::SINGLE) {
      AddCleanFile(makefile, base.append(".cmake"));
    } else {
      for (std::string const& cfg : this->ConfigsList) {
        std::string filename = base;
        filename += "_";
        filename += cfg;
        filename += ".cmake";
        AddCleanFile(makefile, filename);
      }
    }
  }

  // Add moc compilation to generated files list
  if (this->MocEnabled) {
    std::string const mocsComp = this->DirBuild + "/mocs_compilation.cpp";
    auto files = this->AddGeneratedSource(mocsComp, cmQtAutoGen::MOC);
    for (std::string& file : files) {
      autogenProvides.push_back(std::move(file));
    }
  }

  // Add autogen includes directory to the origin target INCLUDE_DIRECTORIES
  if (this->MocEnabled || this->UicEnabled) {
    std::string includeDir = this->DirBuild + "/include";
    if (this->MultiConfig != cmQtAutoGen::SINGLE) {
      includeDir += "_$<CONFIG>";
    }
    this->Target->AddIncludeDirectory(includeDir, true);
  }

  // Acquire rcc executable and features
  if (this->RccEnabled) {
    {
      std::string err;
      if (this->QtVersionMajor == "5") {
        cmGeneratorTarget* tgt =
          localGen->FindGeneratorTargetToUse("Qt5::rcc");
        if (tgt != nullptr) {
          this->RccExecutable = SafeString(tgt->ImportedGetLocation(""));
        } else {
          err = "AUTORCC: Qt5::rcc target not found";
        }
      } else if (QtVersionMajor == "4") {
        cmGeneratorTarget* tgt =
          localGen->FindGeneratorTargetToUse("Qt4::rcc");
        if (tgt != nullptr) {
          this->RccExecutable = SafeString(tgt->ImportedGetLocation(""));
        } else {
          err = "AUTORCC: Qt4::rcc target not found";
        }
      } else {
        err = "The AUTORCC feature supports only Qt 4 and Qt 5";
      }
      if (!err.empty()) {
        err += " (";
        err += this->Target->GetName();
        err += ")";
        cmSystemTools::Error(err.c_str());
      }
    }
    // Detect if rcc supports (-)-list
    if (!this->RccExecutable.empty() && (this->QtVersionMajor == "5")) {
      std::vector<std::string> command;
      command.push_back(this->RccExecutable);
      command.push_back("--help");
      std::string rccStdOut;
      std::string rccStdErr;
      int retVal = 0;
      bool result = cmSystemTools::RunSingleCommand(
        command, &rccStdOut, &rccStdErr, &retVal, nullptr,
        cmSystemTools::OUTPUT_NONE, 0.0, cmProcessOutput::Auto);
      if (result && retVal == 0 &&
          rccStdOut.find("--list") != std::string::npos) {
        this->RccListOptions.push_back("--list");
      } else {
        this->RccListOptions.push_back("-list");
      }
    }
  }

  // Extract relevant source files
  std::vector<std::string> generatedSources;
  std::vector<std::string> generatedHeaders;
  {
    std::string const qrcExt = "qrc";
    std::vector<cmSourceFile*> srcFiles;
    this->Target->GetConfigCommonSourceFiles(srcFiles);
    for (cmSourceFile* sf : srcFiles) {
      if (sf->GetPropertyAsBool("SKIP_AUTOGEN")) {
        continue;
      }
      // sf->GetExtension() is only valid after sf->GetFullPath() ...
      std::string const& fPath = sf->GetFullPath();
      std::string const& ext = sf->GetExtension();
      // Register generated files that will be scanned by moc or uic
      if (this->MocEnabled || this->UicEnabled) {
        cmSystemTools::FileFormat const fileType =
          cmSystemTools::GetFileFormat(ext.c_str());
        if ((fileType == cmSystemTools::CXX_FILE_FORMAT) ||
            (fileType == cmSystemTools::HEADER_FILE_FORMAT)) {
          std::string const absPath = cmSystemTools::GetRealPath(fPath);
          if ((this->MocEnabled && !sf->GetPropertyAsBool("SKIP_AUTOMOC")) ||
              (this->UicEnabled && !sf->GetPropertyAsBool("SKIP_AUTOUIC"))) {
            // Register source
            const bool generated = sf->GetPropertyAsBool("GENERATED");
            if (fileType == cmSystemTools::HEADER_FILE_FORMAT) {
              if (generated) {
                generatedHeaders.push_back(absPath);
              } else {
                this->Headers.push_back(absPath);
              }
            } else {
              if (generated) {
                generatedSources.push_back(absPath);
              } else {
                this->Sources.push_back(absPath);
              }
            }
          }
        }
      }
      // Register rcc enabled files
      if (this->RccEnabled && (ext == qrcExt) &&
          !sf->GetPropertyAsBool("SKIP_AUTORCC")) {
        // Register qrc file
        {
          Qrc qrc;
          qrc.QrcFile = cmSystemTools::GetRealPath(fPath);
          qrc.QrcName =
            cmSystemTools::GetFilenameWithoutLastExtension(qrc.QrcFile);
          qrc.Generated = sf->GetPropertyAsBool("GENERATED");
          // RCC options
          {
            std::string const opts = GetSafeProperty(sf, "AUTORCC_OPTIONS");
            if (!opts.empty()) {
              cmSystemTools::ExpandListArgument(opts, qrc.Options);
            }
          }
          this->Qrcs.push_back(std::move(qrc));
        }
      }
    }
    // cmGeneratorTarget::GetConfigCommonSourceFiles computes the target's
    // sources meta data cache. Clear it so that OBJECT library targets that
    // are AUTOGEN initialized after this target get their added
    // mocs_compilation.cpp source acknowledged by this target.
    this->Target->ClearSourcesCache();
  }
  // Read skip files from makefile sources
  if (this->MocEnabled || this->UicEnabled) {
    const std::vector<cmSourceFile*>& allSources = makefile->GetSourceFiles();
    for (cmSourceFile* sf : allSources) {
      // sf->GetExtension() is only valid after sf->GetFullPath() ...
      std::string const& fPath = sf->GetFullPath();
      cmSystemTools::FileFormat const fileType =
        cmSystemTools::GetFileFormat(sf->GetExtension().c_str());
      if (!(fileType == cmSystemTools::CXX_FILE_FORMAT) &&
          !(fileType == cmSystemTools::HEADER_FILE_FORMAT)) {
        continue;
      }
      const bool skipAll = sf->GetPropertyAsBool("SKIP_AUTOGEN");
      const bool mocSkip =
        this->MocEnabled && (skipAll || sf->GetPropertyAsBool("SKIP_AUTOMOC"));
      const bool uicSkip =
        this->UicEnabled && (skipAll || sf->GetPropertyAsBool("SKIP_AUTOUIC"));
      if (mocSkip || uicSkip) {
        std::string const absFile = cmSystemTools::GetRealPath(fPath);
        if (mocSkip) {
          this->MocSkip.insert(absFile);
        }
        if (uicSkip) {
          this->UicSkip.insert(absFile);
        }
      }
    }
  }

  // Process GENERATED sources and headers
  if (!generatedSources.empty() || !generatedHeaders.empty()) {
    // Check status of policy CMP0071
    bool policyAccept = false;
    bool policyWarn = false;
    cmPolicies::PolicyStatus const CMP0071_status =
      makefile->GetPolicyStatus(cmPolicies::CMP0071);
    switch (CMP0071_status) {
      case cmPolicies::WARN:
        policyWarn = true;
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // Ignore GENERATED file
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::NEW:
        // Process GENERATED file
        policyAccept = true;
        break;
    }

    if (policyAccept) {
      // Accept GENERATED sources
      for (std::string const& absFile : generatedHeaders) {
        this->Headers.push_back(absFile);
        autogenDependFiles.insert(absFile);
      }
      for (std::string const& absFile : generatedSources) {
        this->Sources.push_back(absFile);
        autogenDependFiles.insert(absFile);
      }
    } else {
      if (policyWarn) {
        std::string msg;
        msg += cmPolicies::GetPolicyWarning(cmPolicies::CMP0071);
        msg += "\n";
        std::string tools;
        std::string property;
        if (this->MocEnabled && this->UicEnabled) {
          tools = "AUTOMOC and AUTOUIC";
          property = "SKIP_AUTOGEN";
        } else if (this->MocEnabled) {
          tools = "AUTOMOC";
          property = "SKIP_AUTOMOC";
        } else if (this->UicEnabled) {
          tools = "AUTOUIC";
          property = "SKIP_AUTOUIC";
        }
        msg += "For compatibility, CMake is excluding the GENERATED source "
               "file(s):\n";
        for (const std::string& absFile : generatedHeaders) {
          msg.append("  ").append(cmQtAutoGen::Quoted(absFile)).append("\n");
        }
        for (const std::string& absFile : generatedSources) {
          msg.append("  ").append(cmQtAutoGen::Quoted(absFile)).append("\n");
        }
        msg += "from processing by ";
        msg += tools;
        msg +=
          ". If any of the files should be processed, set CMP0071 to NEW. "
          "If any of the files should not be processed, "
          "explicitly exclude them by setting the source file property ";
        msg += property;
        msg += ":\n  set_property(SOURCE file.h PROPERTY ";
        msg += property;
        msg += " ON)\n";
        makefile->IssueMessage(cmake::AUTHOR_WARNING, msg);
      }
    }
    // Clear lists
    generatedSources.clear();
    generatedHeaders.clear();
  }
  // Sort headers and sources
  if (this->MocEnabled || this->UicEnabled) {
    std::sort(this->Headers.begin(), this->Headers.end());
    std::sort(this->Sources.begin(), this->Sources.end());
  }

  // Process qrc files
  if (!this->Qrcs.empty()) {
    const bool QtV5 = (this->QtVersionMajor == "5");
    // Target rcc options
    std::vector<std::string> optionsTarget;
    cmSystemTools::ExpandListArgument(
      GetSafeProperty(this->Target, "AUTORCC_OPTIONS"), optionsTarget);

    // Check if file name is unique
    for (Qrc& qrc : this->Qrcs) {
      qrc.Unique = true;
      for (Qrc const& qrc2 : this->Qrcs) {
        if ((&qrc != &qrc2) && (qrc.QrcName == qrc2.QrcName)) {
          qrc.Unique = false;
          break;
        }
      }
    }
    // Path checksum and file names
    {
      cmFilePathChecksum const fpathCheckSum(makefile);
      for (Qrc& qrc : this->Qrcs) {
        qrc.PathChecksum = fpathCheckSum.getPart(qrc.QrcFile);
        // RCC output file name
        {
          std::string rccFile = this->DirBuild + "/";
          rccFile += qrc.PathChecksum;
          rccFile += "/qrc_";
          rccFile += qrc.QrcName;
          rccFile += ".cpp";
          qrc.RccFile = std::move(rccFile);
        }
        {
          std::string base = this->DirInfo;
          base += "/RCC";
          base += qrc.QrcName;
          if (!qrc.Unique) {
            base += qrc.PathChecksum;
          }
          qrc.InfoFile = base;
          qrc.InfoFile += "Info.cmake";
          qrc.SettingsFile = base;
          qrc.SettingsFile += "Settings.cmake";
        }
      }
    }
    // RCC options
    for (Qrc& qrc : this->Qrcs) {
      // Target options
      std::vector<std::string> opts = optionsTarget;
      // Merge computed "-name XYZ" option
      {
        std::string name = qrc.QrcName;
        // Replace '-' with '_'. The former is not valid for symbol names.
        std::replace(name.begin(), name.end(), '-', '_');
        if (!qrc.Unique) {
          name += "_";
          name += qrc.PathChecksum;
        }
        std::vector<std::string> nameOpts;
        nameOpts.emplace_back("-name");
        nameOpts.emplace_back(std::move(name));
        cmQtAutoGen::RccMergeOptions(opts, nameOpts, QtV5);
      }
      // Merge file option
      cmQtAutoGen::RccMergeOptions(opts, qrc.Options, QtV5);
      qrc.Options = std::move(opts);
    }
    for (Qrc& qrc : this->Qrcs) {
      // Register file at target
      std::vector<std::string> const ccOutput =
        this->AddGeneratedSource(qrc.RccFile, cmQtAutoGen::RCC);

      cmCustomCommandLines commandLines;
      {
        cmCustomCommandLine currentLine;
        currentLine.push_back(cmSystemTools::GetCMakeCommand());
        currentLine.push_back("-E");
        currentLine.push_back("cmake_autorcc");
        currentLine.push_back(qrc.InfoFile);
        currentLine.push_back("$<CONFIGURATION>");
        commandLines.push_back(std::move(currentLine));
      }
      std::string ccComment = "Automatic RCC for ";
      ccComment += FileProjectRelativePath(makefile, qrc.QrcFile);

      if (qrc.Generated) {
        // Create custom rcc target
        std::string ccName;
        {
          ccName = this->Target->GetName();
          ccName += "_arcc_";
          ccName += qrc.QrcName;
          if (!qrc.Unique) {
            ccName += "_";
            ccName += qrc.PathChecksum;
          }
          std::vector<std::string> ccDepends;
          // Add the .qrc file to the custom target dependencies
          ccDepends.push_back(qrc.QrcFile);

          cmTarget* autoRccTarget = makefile->AddUtilityCommand(
            ccName, cmMakefile::TargetOrigin::Generator, true,
            this->DirWork.c_str(), ccOutput, ccDepends, commandLines, false,
            ccComment.c_str());
          // Create autogen generator target
          localGen->AddGeneratorTarget(
            new cmGeneratorTarget(autoRccTarget, localGen));

          // Set FOLDER property in autogen target
          if (!this->AutogenFolder.empty()) {
            autoRccTarget->SetProperty("FOLDER", this->AutogenFolder.c_str(), autoRccTarget->GetBacktrace());
          }
        }
        // Add autogen target to the origin target dependencies
        this->Target->Target->AddUtility(ccName, makefile);
      } else {
        // Create custom rcc command
        {
          std::vector<std::string> ccByproducts;
          std::vector<std::string> ccDepends;
          // Add the .qrc file to the custom command dependencies
          ccDepends.push_back(qrc.QrcFile);

          // Add the resource files to the dependencies
          {
            std::string error;
            if (cmQtAutoGen::RccListInputs(this->RccExecutable,
                                           this->RccListOptions, qrc.QrcFile,
                                           qrc.Resources, &error)) {
              for (std::string const& fileName : qrc.Resources) {
                // Add resource file to the custom command dependencies
                ccDepends.push_back(fileName);
              }
            } else {
              cmSystemTools::Error(error.c_str());
            }
          }
          makefile->AddCustomCommandToOutput(ccOutput, ccByproducts, ccDepends,
                                             /*main_dependency*/ std::string(),
                                             commandLines, ccComment.c_str(),
                                             this->DirWork.c_str());
        }
        // Reconfigure when .qrc file changes
        makefile->AddCMakeDependFile(qrc.QrcFile);
      }
    }
  }

  // Create _autogen target
  if (this->MocEnabled || this->UicEnabled) {
    // Add user defined autogen target dependencies
    {
      std::string const deps =
        GetSafeProperty(this->Target, "AUTOGEN_TARGET_DEPENDS");
      if (!deps.empty()) {
        std::vector<std::string> extraDeps;
        cmSystemTools::ExpandListArgument(deps, extraDeps);
        for (std::string const& depName : extraDeps) {
          // Allow target and file dependencies
          auto* depTarget = makefile->FindTargetToUse(depName);
          if (depTarget != nullptr) {
            autogenDependTargets.insert(depTarget);
          } else {
            autogenDependFiles.insert(depName);
          }
        }
      }
    }

    // Compose target comment
    std::string autogenComment;
    {
      std::string tools;
      if (this->MocEnabled) {
        tools += "MOC";
      }
      if (this->UicEnabled) {
        if (!tools.empty()) {
          tools += " and ";
        }
        tools += "UIC";
      }
      autogenComment = "Automatic ";
      autogenComment += tools;
      autogenComment += " for target ";
      autogenComment += this->Target->GetName();
    }

    // Compose command lines
    cmCustomCommandLines commandLines;
    {
      cmCustomCommandLine currentLine;
      currentLine.push_back(cmSystemTools::GetCMakeCommand());
      currentLine.push_back("-E");
      currentLine.push_back("cmake_autogen");
      currentLine.push_back(this->AutogenInfoFile);
      currentLine.push_back("$<CONFIGURATION>");
      commandLines.push_back(std::move(currentLine));
    }

    // Use PRE_BUILD on demand
    bool usePRE_BUILD = false;
    if (globalGen->GetName().find("Visual Studio") != std::string::npos) {
      // Under VS use a PRE_BUILD event instead of a separate target to
      // reduce the number of targets loaded into the IDE.
      // This also works around a VS 11 bug that may skip updating the target:
      //  https://connect.microsoft.com/VisualStudio/feedback/details/769495
      usePRE_BUILD = true;
    }
    // Disable PRE_BUILD in some cases
    if (usePRE_BUILD) {
      // Cannot use PRE_BUILD with file depends
      if (!autogenDependFiles.empty()) {
        usePRE_BUILD = false;
      }
    }
    // Create the autogen target/command
    if (usePRE_BUILD) {
      // Add additional autogen target dependencies to origin target
      for (cmTarget* depTarget : autogenDependTargets) {
        this->Target->Target->AddUtility(depTarget->GetName(), makefile);
      }

      // Add the pre-build command directly to bypass the OBJECT_LIBRARY
      // rejection in cmMakefile::AddCustomCommandToTarget because we know
      // PRE_BUILD will work for an OBJECT_LIBRARY in this specific case.
      //
      // PRE_BUILD does not support file dependencies!
      const std::vector<std::string> no_output;
      const std::vector<std::string> no_deps;
      cmCustomCommand cc(makefile, no_output, autogenProvides, no_deps,
                         commandLines, autogenComment.c_str(),
                         this->DirWork.c_str());
      cc.SetEscapeOldStyle(false);
      cc.SetEscapeAllowMakeVars(true);
      this->Target->Target->AddPreBuildCommand(cc);
    } else {

      // Convert file dependencies std::set to std::vector
      std::vector<std::string> autogenDepends(autogenDependFiles.begin(),
                                              autogenDependFiles.end());

      // Add link library target dependencies to the autogen target
      // dependencies
      for (std::string const& config : this->ConfigsList) {
        cmLinkImplementationLibraries const* libs =
          this->Target->GetLinkImplementationLibraries(config);
        if (libs != nullptr) {
          for (cmLinkItem const& item : libs->Libraries) {
            cmGeneratorTarget const* libTarget = item.Target;
            if ((libTarget != nullptr) &&
                !StaticLibraryCycle(this->Target, libTarget, config)) {
              std::string util;
              if (this->ConfigsList.size() > 1) {
                util += "$<$<CONFIG:";
                util += config;
                util += ">:";
              }
              util += libTarget->GetName();
              if (this->ConfigsList.size() > 1) {
                util += ">";
              }
              autogenDepends.push_back(util);
            }
          }
        }
      }

      // Create autogen target
      cmTarget* autogenTarget = makefile->AddUtilityCommand(
        this->AutogenTargetName, cmMakefile::TargetOrigin::Generator, true,
        this->DirWork.c_str(), /*byproducts=*/autogenProvides, autogenDepends,
        commandLines, false, autogenComment.c_str());
      // Create autogen generator target
      localGen->AddGeneratorTarget(
        new cmGeneratorTarget(autogenTarget, localGen));

      // Forward origin utilities to autogen target
      for (std::string const& depName : this->Target->Target->GetUtilities()) {
        autogenTarget->AddUtility(depName, makefile);
      }
      // Add additional autogen target dependencies to autogen target
      for (cmTarget* depTarget : autogenDependTargets) {
        autogenTarget->AddUtility(depTarget->GetName(), makefile);
      }

      // Set FOLDER property in autogen target
      if (!this->AutogenFolder.empty()) {
        autogenTarget->SetProperty("FOLDER", this->AutogenFolder.c_str(), autogenTarget->GetBacktrace());
      }

      // Add autogen target to the origin target dependencies
      this->Target->Target->AddUtility(this->AutogenTargetName, makefile);
    }
  }
}

void cmQtAutoGeneratorInitializer::SetupCustomTargets()
{
  cmMakefile* makefile = this->Target->Target->GetMakefile();

  // forget the variables added here afterwards again:
  cmMakefile::ScopePushPop varScope(makefile);
  static_cast<void>(varScope);

  // Configuration suffixes
  std::map<std::string, std::string> configSuffixes;
  for (std::string const& cfg : this->ConfigsList) {
    std::string& suffix = configSuffixes[cfg];
    suffix = "_";
    suffix += cfg;
  }

  // Basic setup
  AddDefinitionEscaped(makefile, "_multi_config",
                       cmQtAutoGen::MultiConfigName(this->MultiConfig));
  AddDefinitionEscaped(makefile, "_build_dir", this->DirBuild);

  if (this->MocEnabled || this->UicEnabled) {
    AddDefinitionEscaped(makefile, "_qt_version_major", this->QtVersionMajor);
    AddDefinitionEscaped(makefile, "_settings_file",
                         this->AutogenSettingsFile);
    AddDefinitionEscaped(makefile, "_sources", this->Sources);
    AddDefinitionEscaped(makefile, "_headers", this->Headers);

    if (this->MocEnabled) {
      this->SetupCustomTargetsMoc();
    }
    if (this->UicEnabled) {
      this->SetupCustomTargetsUic();
    }
  }
  if (this->RccEnabled) {
    AddDefinitionEscaped(makefile, "_qt_rcc_executable", this->RccExecutable);
    AddDefinitionEscaped(makefile, "_qt_rcc_list_options",
                         this->RccListOptions);
  }

  // Create info directory on demand
  if (!cmSystemTools::MakeDirectory(this->DirInfo)) {
    std::string emsg = ("Could not create directory: ");
    emsg += cmQtAutoGen::Quoted(this->DirInfo);
    cmSystemTools::Error(emsg.c_str());
  }

  auto ReOpenInfoFile = [](cmsys::ofstream& ofs,
                           std::string const& fileName) -> bool {
    // Ensure we have write permission
    mode_t perm = 0;
#if defined(_WIN32) && !defined(__CYGWIN__)
    mode_t mode_write = S_IWRITE;
#else
    mode_t mode_write = S_IWUSR;
#endif
    cmSystemTools::GetPermissions(fileName, perm);
    if (!(perm & mode_write)) {
      cmSystemTools::SetPermissions(fileName, perm | mode_write);
    }

    ofs.open(fileName.c_str(), std::ios::app);
    if (!ofs) {
      // File open error
      std::string error = "Internal CMake error when trying to open file: ";
      error += cmQtAutoGen::Quoted(fileName);
      error += " for writing.";
      cmSystemTools::Error(error.c_str());
    }
    return static_cast<bool>(ofs);
  };

  // Generate autogen target info file
  if (this->MocEnabled || this->UicEnabled) {
    {
      std::string infoFileIn = cmSystemTools::GetCMakeRoot();
      infoFileIn += "/Modules/AutogenInfo.cmake.in";
      makefile->ConfigureFile(
        infoFileIn.c_str(), this->AutogenInfoFile.c_str(), false, true, false);
    }

    // Append custom definitions to info file
    // --------------------------------------
    cmsys::ofstream ofs;
    if (ReOpenInfoFile(ofs, this->AutogenInfoFile)) {
      auto OfsWriteMap = [&ofs](
        const char* key, std::map<std::string, std::string> const& map) {
        for (auto const& item : map) {
          ofs << "set(" << key << "_" << item.first << " "
              << cmOutputConverter::EscapeForCMake(item.second) << ")\n";
        }
      };
      ofs << "# Configurations options\n";
      OfsWriteMap("AM_CONFIG_SUFFIX", configSuffixes);
      OfsWriteMap("AM_MOC_DEFINITIONS", this->ConfigMocDefines);
      OfsWriteMap("AM_MOC_INCLUDES", this->ConfigMocIncludes);
      OfsWriteMap("AM_UIC_TARGET_OPTIONS", this->ConfigUicOptions);
      // Settings files (only require for multi configuration generators)
      if (this->MultiConfig != cmQtAutoGen::SINGLE) {
        std::map<std::string, std::string> settingsFiles;
        for (std::string const& cfg : this->ConfigsList) {
          settingsFiles[cfg] = cmQtAutoGen::AppendFilenameSuffix(
            this->AutogenSettingsFile, "_" + cfg);
        }
        OfsWriteMap("AM_SETTINGS_FILE", settingsFiles);
      }
    }
  }

  // Generate auto RCC info files
  if (this->RccEnabled) {
    std::string infoFileIn = cmSystemTools::GetCMakeRoot();
    infoFileIn += "/Modules/AutoRccInfo.cmake.in";
    for (Qrc const& qrc : this->Qrcs) {
      // Configure info file
      makefile->ConfigureFile(infoFileIn.c_str(), qrc.InfoFile.c_str(), false,
                              true, false);

      // Append custom definitions to info file
      // --------------------------------------
      cmsys::ofstream ofs;
      if (ReOpenInfoFile(ofs, qrc.InfoFile)) {
        {
          ofs << "# Job\n";
          auto OfsWrite = [&ofs](const char* key, std::string const& value) {
            ofs << "set(" << key << " "
                << cmOutputConverter::EscapeForCMake(value) << ")\n";

          };
          OfsWrite("ARCC_SETTINGS_FILE", qrc.SettingsFile);
          OfsWrite("ARCC_SOURCE", qrc.QrcFile);
          OfsWrite("ARCC_OUTPUT", qrc.RccFile);
          OfsWrite("ARCC_OPTIONS", cmJoin(qrc.Options, ";"));
          OfsWrite("ARCC_INPUTS", cmJoin(qrc.Resources, ";"));
        }
        {
          ofs << "# Configurations options\n";
          auto OfsWriteMap = [&ofs](
            const char* key, std::map<std::string, std::string> const& map) {
            for (auto const& item : map) {
              ofs << "set(" << key << "_" << item.first << " "
                  << cmOutputConverter::EscapeForCMake(item.second) << ")\n";
            }
          };
          OfsWriteMap("ARCC_CONFIG_SUFFIX", configSuffixes);

          // Settings files (only require for multi configuration generators)
          if (this->MultiConfig != cmQtAutoGen::SINGLE) {
            std::map<std::string, std::string> settingsFiles;
            for (std::string const& cfg : this->ConfigsList) {
              settingsFiles[cfg] =
                cmQtAutoGen::AppendFilenameSuffix(qrc.SettingsFile, "_" + cfg);
            }
            OfsWriteMap("ARCC_SETTINGS_FILE", settingsFiles);
          }
        }
      } else {
        break;
      }
    }
  }
}

void cmQtAutoGeneratorInitializer::SetupCustomTargetsMoc()
{
  cmLocalGenerator* localGen = this->Target->GetLocalGenerator();
  cmMakefile* makefile = this->Target->Target->GetMakefile();

  AddDefinitionEscaped(makefile, "_moc_skip", this->MocSkip);
  AddDefinitionEscaped(makefile, "_moc_options",
                       GetSafeProperty(this->Target, "AUTOMOC_MOC_OPTIONS"));
  AddDefinitionEscaped(makefile, "_moc_relaxed_mode",
                       makefile->IsOn("CMAKE_AUTOMOC_RELAXED_MODE") ? "TRUE"
                                                                    : "FALSE");
  AddDefinitionEscaped(makefile, "_moc_macro_names",
                       GetSafeProperty(this->Target, "AUTOMOC_MACRO_NAMES"));
  AddDefinitionEscaped(
    makefile, "_moc_depend_filters",
    GetSafeProperty(this->Target, "AUTOMOC_DEPEND_FILTERS"));

  // Compiler predefines
  if (this->Target->GetPropertyAsBool("AUTOMOC_COMPILER_PREDEFINES") &&
      this->QtVersionGreaterOrEqual(5, 8)) {
    AddDefinitionEscaped(
      makefile, "_moc_predefs_cmd",
      makefile->GetSafeDefinition("CMAKE_CXX_COMPILER_PREDEFINES_COMMAND"));
  }
  // Moc includes and compile definitions
  {
    auto GetIncludeDirs = [this,
                           localGen](std::string const& cfg) -> std::string {
      // Get the include dirs for this target, without stripping the implicit
      // include dirs off, see
      // https://gitlab.kitware.com/cmake/cmake/issues/13667
      std::vector<std::string> includeDirs;
      localGen->GetIncludeDirectories(includeDirs, this->Target, "CXX", cfg,
                                      false);
      return cmJoin(includeDirs, ";");
    };
    auto GetCompileDefinitions =
      [this, localGen](std::string const& cfg) -> std::string {
      std::set<std::string> defines;
      localGen->AddCompileDefinitions(defines, this->Target, cfg, "CXX");
      return cmJoin(defines, ";");
    };

    // Default configuration settings
    std::string const includeDirs = GetIncludeDirs(this->ConfigDefault);
    std::string const compileDefs = GetCompileDefinitions(this->ConfigDefault);
    // Other configuration settings
    for (std::string const& cfg : this->ConfigsList) {
      {
        std::string const configIncludeDirs = GetIncludeDirs(cfg);
        if (configIncludeDirs != includeDirs) {
          this->ConfigMocIncludes[cfg] = configIncludeDirs;
        }
      }
      {
        std::string const configCompileDefs = GetCompileDefinitions(cfg);
        if (configCompileDefs != compileDefs) {
          this->ConfigMocDefines[cfg] = configCompileDefs;
        }
      }
    }
    AddDefinitionEscaped(makefile, "_moc_include_dirs", includeDirs);
    AddDefinitionEscaped(makefile, "_moc_compile_defs", compileDefs);
  }

  // Moc executable
  {
    std::string mocExec;
    std::string err;

    if (this->QtVersionMajor == "5") {
      cmGeneratorTarget* tgt = localGen->FindGeneratorTargetToUse("Qt5::moc");
      if (tgt != nullptr) {
        mocExec = SafeString(tgt->ImportedGetLocation(""));
      } else {
        err = "AUTOMOC: Qt5::moc target not found";
      }
    } else if (this->QtVersionMajor == "4") {
      cmGeneratorTarget* tgt = localGen->FindGeneratorTargetToUse("Qt4::moc");
      if (tgt != nullptr) {
        mocExec = SafeString(tgt->ImportedGetLocation(""));
      } else {
        err = "AUTOMOC: Qt4::moc target not found";
      }
    } else {
      err = "The AUTOMOC feature supports only Qt 4 and Qt 5";
    }

    if (err.empty()) {
      AddDefinitionEscaped(makefile, "_qt_moc_executable", mocExec);
    } else {
      err += " (";
      err += this->Target->GetName();
      err += ")";
      cmSystemTools::Error(err.c_str());
    }
  }
}

void cmQtAutoGeneratorInitializer::SetupCustomTargetsUic()
{
  cmMakefile* makefile = this->Target->Target->GetMakefile();

  // Uic search paths
  {
    std::vector<std::string> uicSearchPaths;
    {
      std::string const usp =
        GetSafeProperty(this->Target, "AUTOUIC_SEARCH_PATHS");
      if (!usp.empty()) {
        cmSystemTools::ExpandListArgument(usp, uicSearchPaths);
        std::string const srcDir = makefile->GetCurrentSourceDirectory();
        for (std::string& path : uicSearchPaths) {
          path = cmSystemTools::CollapseFullPath(path, srcDir);
        }
      }
    }
    AddDefinitionEscaped(makefile, "_uic_search_paths", uicSearchPaths);
  }
  // Uic target options
  {
    auto UicGetOpts = [this](std::string const& cfg) -> std::string {
      std::vector<std::string> opts;
      this->Target->GetAutoUicOptions(opts, cfg);
      return cmJoin(opts, ";");
    };

    // Default settings
    std::string const uicOpts = UicGetOpts(this->ConfigDefault);
    AddDefinitionEscaped(makefile, "_uic_target_options", uicOpts);

    // Configuration specific settings
    for (std::string const& cfg : this->ConfigsList) {
      std::string const configUicOpts = UicGetOpts(cfg);
      if (configUicOpts != uicOpts) {
        this->ConfigUicOptions[cfg] = configUicOpts;
      }
    }
  }
  // .ui files skip and options
  {
    std::vector<std::string> uiFileFiles;
    std::vector<std::vector<std::string>> uiFileOptions;
    {
      std::string const uiExt = "ui";
      for (cmSourceFile* sf : makefile->GetSourceFiles()) {
        // sf->GetExtension() is only valid after sf->GetFullPath() ...
        std::string const& fPath = sf->GetFullPath();
        if (sf->GetExtension() == uiExt) {
          std::string const absFile = cmSystemTools::GetRealPath(fPath);
          // Check if the .ui file should be skipped
          if (sf->GetPropertyAsBool("SKIP_AUTOUIC") ||
              sf->GetPropertyAsBool("SKIP_AUTOGEN")) {
            this->UicSkip.insert(absFile);
          }
          // Check if the .ui file has uic options
          std::string const uicOpts = GetSafeProperty(sf, "AUTOUIC_OPTIONS");
          if (!uicOpts.empty()) {
            // Check if file isn't skipped
            if (this->UicSkip.count(absFile) == 0) {
              uiFileFiles.push_back(absFile);
              std::vector<std::string> optsVec;
              cmSystemTools::ExpandListArgument(uicOpts, optsVec);
              uiFileOptions.push_back(std::move(optsVec));
            }
          }
        }
      }
    }
    AddDefinitionEscaped(makefile, "_qt_uic_options_files", uiFileFiles);
    AddDefinitionEscaped(makefile, "_qt_uic_options_options", uiFileOptions);
  }

  AddDefinitionEscaped(makefile, "_uic_skip", this->UicSkip);

  // Uic executable
  {
    std::string err;
    std::string uicExec;

    cmLocalGenerator* localGen = this->Target->GetLocalGenerator();
    if (this->QtVersionMajor == "5") {
      cmGeneratorTarget* tgt = localGen->FindGeneratorTargetToUse("Qt5::uic");
      if (tgt != nullptr) {
        uicExec = SafeString(tgt->ImportedGetLocation(""));
      } else {
        // Project does not use Qt5Widgets, but has AUTOUIC ON anyway
      }
    } else if (this->QtVersionMajor == "4") {
      cmGeneratorTarget* tgt = localGen->FindGeneratorTargetToUse("Qt4::uic");
      if (tgt != nullptr) {
        uicExec = SafeString(tgt->ImportedGetLocation(""));
      } else {
        err = "AUTOUIC: Qt4::uic target not found";
      }
    } else {
      err = "The AUTOUIC feature supports only Qt 4 and Qt 5";
    }

    if (err.empty()) {
      AddDefinitionEscaped(makefile, "_qt_uic_executable", uicExec);
    } else {
      err += " (";
      err += this->Target->GetName();
      err += ")";
      cmSystemTools::Error(err.c_str());
    }
  }
}

std::vector<std::string> cmQtAutoGeneratorInitializer::AddGeneratedSource(
  std::string const& filename, cmQtAutoGen::Generator genType)
{
  std::vector<std::string> genFiles;
  // Register source file in makefile and source group
  if (this->MultiConfig != cmQtAutoGen::FULL) {
    genFiles.push_back(filename);
  } else {
    for (std::string const& cfg : this->ConfigsList) {
      genFiles.push_back(
        cmQtAutoGen::AppendFilenameSuffix(filename, "_" + cfg));
    }
  }
  {
    cmMakefile* makefile = this->Target->Target->GetMakefile();
    for (std::string const& genFile : genFiles) {
      {
        cmSourceFile* gFile = makefile->GetOrCreateSource(genFile, true);
        gFile->SetProperty("GENERATED", "1");
        gFile->SetProperty("SKIP_AUTOGEN", "On");
      }
      AddToSourceGroup(makefile, genFile, genType);
    }
  }

  // Add source file to target
  if (this->MultiConfig != cmQtAutoGen::FULL) {
    this->Target->AddSource(filename);
  } else {
    for (std::string const& cfg : this->ConfigsList) {
      std::string src = "$<$<CONFIG:";
      src += cfg;
      src += ">:";
      src += cmQtAutoGen::AppendFilenameSuffix(filename, "_" + cfg);
      src += ">";
      this->Target->AddSource(src);
    }
  }

  return genFiles;
}

std::string cmQtAutoGeneratorInitializer::GetQtMajorVersion(
  cmGeneratorTarget const* target)
{
  cmMakefile* makefile = target->Target->GetMakefile();
  std::string qtMajor = makefile->GetSafeDefinition("QT_VERSION_MAJOR");
  if (qtMajor.empty()) {
    qtMajor = makefile->GetSafeDefinition("Qt5Core_VERSION_MAJOR");
  }
  const char* targetQtVersion =
    target->GetLinkInterfaceDependentStringProperty("QT_MAJOR_VERSION", "");
  if (targetQtVersion != nullptr) {
    qtMajor = targetQtVersion;
  }
  return qtMajor;
}

std::string cmQtAutoGeneratorInitializer::GetQtMinorVersion(
  cmGeneratorTarget const* target, std::string const& qtVersionMajor)
{
  cmMakefile* makefile = target->Target->GetMakefile();
  std::string qtMinor;
  if (qtVersionMajor == "5") {
    qtMinor = makefile->GetSafeDefinition("Qt5Core_VERSION_MINOR");
  }
  if (qtMinor.empty()) {
    qtMinor = makefile->GetSafeDefinition("QT_VERSION_MINOR");
  }

  const char* targetQtVersion =
    target->GetLinkInterfaceDependentStringProperty("QT_MINOR_VERSION", "");
  if (targetQtVersion != nullptr) {
    qtMinor = targetQtVersion;
  }
  return qtMinor;
}

bool cmQtAutoGeneratorInitializer::QtVersionGreaterOrEqual(
  unsigned long requestMajor, unsigned long requestMinor) const
{
  unsigned long majorUL(0);
  unsigned long minorUL(0);
  if (cmSystemTools::StringToULong(this->QtVersionMajor.c_str(), &majorUL) &&
      cmSystemTools::StringToULong(this->QtVersionMinor.c_str(), &minorUL)) {
    return (majorUL > requestMajor) ||
      (majorUL == requestMajor && minorUL >= requestMinor);
  }
  return false;
}
