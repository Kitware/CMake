/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMakefileTargetGenerator.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <iterator>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cm_codecvt_Encoding.hxx"

#include "cmComputeLinkInformation.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandGenerator.h"
#include "cmFileSet.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLinkLineComputer.h" // IWYU pragma: keep
#include "cmList.h"
#include "cmLocalCommonGenerator.h"
#include "cmLocalGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmMakefileExecutableTargetGenerator.h"
#include "cmMakefileLibraryTargetGenerator.h"
#include "cmMakefileUtilityTargetGenerator.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmRulePlaceholderExpander.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocationKind.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

cmMakefileTargetGenerator::cmMakefileTargetGenerator(cmGeneratorTarget* target)
  : cmCommonTargetGenerator(target)
{
  this->CustomCommandDriver = OnBuild;
  this->LocalGenerator =
    static_cast<cmLocalUnixMakefileGenerator3*>(target->GetLocalGenerator());
  this->GlobalGenerator = static_cast<cmGlobalUnixMakefileGenerator3*>(
    this->LocalGenerator->GetGlobalGenerator());
  cmake* cm = this->GlobalGenerator->GetCMakeInstance();
  this->NoRuleMessages = false;
  if (cmValue ruleStatus =
        cm->GetState()->GetGlobalProperty("RULE_MESSAGES")) {
    this->NoRuleMessages = cmIsOff(*ruleStatus);
  }
  switch (this->GeneratorTarget->GetPolicyStatusCMP0113()) {
    case cmPolicies::WARN:
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      this->CMP0113New = false;
      break;
    case cmPolicies::NEW:
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      this->CMP0113New = true;
      break;
  }
  this->MacOSXContentGenerator =
    cm::make_unique<MacOSXContentGeneratorType>(this);
}

cmMakefileTargetGenerator::~cmMakefileTargetGenerator() = default;

std::unique_ptr<cmMakefileTargetGenerator> cmMakefileTargetGenerator::New(
  cmGeneratorTarget* tgt)
{
  std::unique_ptr<cmMakefileTargetGenerator> result;

  switch (tgt->GetType()) {
    case cmStateEnums::EXECUTABLE:
      result = cm::make_unique<cmMakefileExecutableTargetGenerator>(tgt);
      break;
    case cmStateEnums::STATIC_LIBRARY:
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY:
    case cmStateEnums::OBJECT_LIBRARY:
      result = cm::make_unique<cmMakefileLibraryTargetGenerator>(tgt);
      break;
    case cmStateEnums::INTERFACE_LIBRARY:
    case cmStateEnums::UTILITY:
      result = cm::make_unique<cmMakefileUtilityTargetGenerator>(tgt);
      break;
    default:
      return result;
      // break; /* unreachable */
  }
  return result;
}

std::string cmMakefileTargetGenerator::GetConfigName() const
{
  auto const& configNames = this->LocalGenerator->GetConfigNames();
  assert(configNames.size() == 1);
  return configNames.front();
}

void cmMakefileTargetGenerator::GetDeviceLinkFlags(
  std::string& linkFlags, const std::string& linkLanguage)
{
  cmGeneratorTarget::DeviceLinkSetter setter(*this->GetGeneratorTarget());

  std::vector<std::string> linkOpts;
  this->GeneratorTarget->GetLinkOptions(linkOpts, this->GetConfigName(),
                                        linkLanguage);
  this->LocalGenerator->SetLinkScriptShell(
    this->GlobalGenerator->GetUseLinkScript());
  // LINK_OPTIONS are escaped.
  this->LocalGenerator->AppendCompileOptions(linkFlags, linkOpts);
  this->LocalGenerator->SetLinkScriptShell(false);
}

void cmMakefileTargetGenerator::GetTargetLinkFlags(
  std::string& flags, const std::string& linkLanguage)
{
  this->LocalGenerator->AppendFlags(
    flags, this->GeneratorTarget->GetSafeProperty("LINK_FLAGS"));

  std::string linkFlagsConfig =
    cmStrCat("LINK_FLAGS_", cmSystemTools::UpperCase(this->GetConfigName()));
  this->LocalGenerator->AppendFlags(
    flags, this->GeneratorTarget->GetSafeProperty(linkFlagsConfig));

  std::vector<std::string> opts;
  this->GeneratorTarget->GetLinkOptions(opts, this->GetConfigName(),
                                        linkLanguage);
  this->LocalGenerator->SetLinkScriptShell(
    this->GlobalGenerator->GetUseLinkScript());
  // LINK_OPTIONS are escaped.
  this->LocalGenerator->AppendCompileOptions(flags, opts);
  this->LocalGenerator->SetLinkScriptShell(false);

  this->LocalGenerator->AppendPositionIndependentLinkerFlags(
    flags, this->GeneratorTarget, this->GetConfigName(), linkLanguage);
  this->LocalGenerator->AppendDependencyInfoLinkerFlags(
    flags, this->GeneratorTarget, this->GetConfigName(), linkLanguage);
}

void cmMakefileTargetGenerator::CreateRuleFile()
{
  // Create a directory for this target.
  this->TargetBuildDirectory =
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  this->TargetBuildDirectoryFull =
    this->LocalGenerator->ConvertToFullPath(this->TargetBuildDirectory);
  cmSystemTools::MakeDirectory(this->TargetBuildDirectoryFull);

  // Construct the rule file name.
  this->BuildFileName = cmStrCat(this->TargetBuildDirectory, "/build.make");
  this->BuildFileNameFull =
    cmStrCat(this->TargetBuildDirectoryFull, "/build.make");

  // Construct the rule file name.
  this->ProgressFileNameFull =
    cmStrCat(this->TargetBuildDirectoryFull, "/progress.make");

  // reset the progress count
  this->NumberOfProgressActions = 0;

  // Open the rule file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  this->BuildFileStream = cm::make_unique<cmGeneratedFileStream>(
    this->BuildFileNameFull, false,
    this->GlobalGenerator->GetMakefileEncoding());
  if (!this->BuildFileStream) {
    return;
  }
  this->BuildFileStream->SetCopyIfDifferent(true);
  this->LocalGenerator->WriteDisclaimer(*this->BuildFileStream);
  if (this->GlobalGenerator->AllowDeleteOnError()) {
    std::vector<std::string> no_depends;
    std::vector<std::string> no_commands;
    this->LocalGenerator->WriteMakeRule(
      *this->BuildFileStream, "Delete rule output on recipe failure.",
      ".DELETE_ON_ERROR", no_depends, no_commands, false);
  }
  this->LocalGenerator->WriteSpecialTargetsTop(*this->BuildFileStream);
}

void cmMakefileTargetGenerator::WriteTargetBuildRules()
{
  this->GeneratorTarget->CheckCxxModuleStatus(this->GetConfigName());

  if (this->GeneratorTarget->HaveCxx20ModuleSources()) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("The \"", this->GeneratorTarget->GetName(),
               "\" target contains C++ module sources which are not supported "
               "by the generator"));
  }

  // -- Write the custom commands for this target

  // Evaluates generator expressions and expands prop_value
  auto evaluatedFiles = [this](const std::string& prop_value) -> cmList {
    cmList files{ cmGeneratorExpression::Evaluate(
      prop_value, this->LocalGenerator, this->GetConfigName(),
      this->GeneratorTarget) };
    return files;
  };

  // Look for additional files registered for cleaning in this directory.
  if (cmValue prop_value =
        this->Makefile->GetProperty("ADDITIONAL_MAKE_CLEAN_FILES")) {
    auto const files = evaluatedFiles(*prop_value);
    this->CleanFiles.insert(files.begin(), files.end());
  }

  // Look for additional files registered for cleaning in this target.
  if (cmValue prop_value =
        this->GeneratorTarget->GetProperty("ADDITIONAL_CLEAN_FILES")) {
    auto const files = evaluatedFiles(*prop_value);
    // For relative path support
    std::string const& binaryDir =
      this->LocalGenerator->GetCurrentBinaryDirectory();
    for (std::string const& cfl : files) {
      this->CleanFiles.insert(cmSystemTools::CollapseFullPath(cfl, binaryDir));
    }
  }

  // Look for ISPC extra object files generated by this target
  auto ispcAdditionalObjs =
    this->GeneratorTarget->GetGeneratedISPCObjects(this->GetConfigName());
  for (std::string const& ispcObj : ispcAdditionalObjs) {
    this->CleanFiles.insert(
      this->LocalGenerator->MaybeRelativeToCurBinDir(ispcObj));
  }

  // add custom commands to the clean rules?
  bool clean = cmIsOff(this->Makefile->GetProperty("CLEAN_NO_CUSTOM"));

  // First generate the object rule files.  Save a list of all object
  // files for this target.
  std::vector<cmSourceFile const*> customCommands;
  this->GeneratorTarget->GetCustomCommands(customCommands,
                                           this->GetConfigName());
  for (cmSourceFile const* sf : customCommands) {
    if (this->CMP0113New &&
        !this->LocalGenerator->GetCommandsVisited(this->GeneratorTarget)
           .insert(sf)
           .second) {
      continue;
    }
    cmCustomCommandGenerator ccg(*sf->GetCustomCommand(),
                                 this->GetConfigName(), this->LocalGenerator);
    this->GenerateCustomRuleFile(ccg);
    if (clean) {
      const std::vector<std::string>& outputs = ccg.GetOutputs();
      for (std::string const& output : outputs) {
        this->CleanFiles.insert(
          this->LocalGenerator->MaybeRelativeToCurBinDir(output));
      }
      const std::vector<std::string>& byproducts = ccg.GetByproducts();
      for (std::string const& byproduct : byproducts) {
        this->CleanFiles.insert(
          this->LocalGenerator->MaybeRelativeToCurBinDir(byproduct));
      }
    }
  }

  // Add byproducts from build events to the clean rules
  if (clean) {
    std::vector<cmCustomCommand> buildEventCommands =
      this->GeneratorTarget->GetPreBuildCommands();

    cm::append(buildEventCommands,
               this->GeneratorTarget->GetPreLinkCommands());
    cm::append(buildEventCommands,
               this->GeneratorTarget->GetPostBuildCommands());

    for (const auto& be : buildEventCommands) {
      cmCustomCommandGenerator beg(be, this->GetConfigName(),
                                   this->LocalGenerator);
      const std::vector<std::string>& byproducts = beg.GetByproducts();
      for (std::string const& byproduct : byproducts) {
        this->CleanFiles.insert(
          this->LocalGenerator->MaybeRelativeToCurBinDir(byproduct));
      }
    }
  }
  std::vector<cmSourceFile const*> headerSources;
  this->GeneratorTarget->GetHeaderSources(headerSources,
                                          this->GetConfigName());
  this->OSXBundleGenerator->GenerateMacOSXContentStatements(
    headerSources, this->MacOSXContentGenerator.get(), this->GetConfigName());
  std::vector<cmSourceFile const*> extraSources;
  this->GeneratorTarget->GetExtraSources(extraSources, this->GetConfigName());
  this->OSXBundleGenerator->GenerateMacOSXContentStatements(
    extraSources, this->MacOSXContentGenerator.get(), this->GetConfigName());
  cmValue pchExtension = this->Makefile->GetDefinition("CMAKE_PCH_EXTENSION");
  std::vector<cmSourceFile const*> externalObjects;
  this->GeneratorTarget->GetExternalObjects(externalObjects,
                                            this->GetConfigName());
  for (cmSourceFile const* sf : externalObjects) {
    auto const& objectFileName = sf->GetFullPath();
    if (!cmHasSuffix(objectFileName, pchExtension)) {
      this->ExternalObjects.push_back(objectFileName);
    }
  }

  std::map<std::string, std::string> file_set_map;

  auto const* tgt = this->GeneratorTarget->Target;
  for (auto const& name : tgt->GetAllFileSetNames()) {
    auto const* file_set = tgt->GetFileSet(name);
    if (!file_set) {
      this->Makefile->IssueMessage(
        MessageType::INTERNAL_ERROR,
        cmStrCat("Target \"", tgt->GetName(),
                 "\" is tracked to have file set \"", name,
                 "\", but it was not found."));
      continue;
    }

    auto fileEntries = file_set->CompileFileEntries();
    auto directoryEntries = file_set->CompileDirectoryEntries();
    auto directories = file_set->EvaluateDirectoryEntries(
      directoryEntries, this->LocalGenerator, this->GetConfigName(),
      this->GeneratorTarget);

    std::map<std::string, std::vector<std::string>> files;
    for (auto const& entry : fileEntries) {
      file_set->EvaluateFileEntry(directories, files, entry,
                                  this->LocalGenerator, this->GetConfigName(),
                                  this->GeneratorTarget);
    }

    for (auto const& it : files) {
      for (auto const& filename : it.second) {
        file_set_map[filename] = file_set->GetType();
      }
    }
  }

  std::vector<cmSourceFile const*> objectSources;
  this->GeneratorTarget->GetObjectSources(objectSources,
                                          this->GetConfigName());

  // validate that all languages requested are enabled.
  std::set<std::string> requiredLangs;
  if (this->HaveRequiredLanguages(objectSources, requiredLangs)) {
    for (cmSourceFile const* sf : objectSources) {
      // Generate this object file's rule file.
      this->WriteObjectRuleFiles(*sf);
    }
  }

  for (cmSourceFile const* sf : objectSources) {
    auto const& path = sf->GetFullPath();
    auto const it = file_set_map.find(path);
    if (it != file_set_map.end()) {
      auto const& file_set_type = it->second;
      if (file_set_type == "CXX_MODULES"_s) {
        if (sf->GetLanguage() != "CXX"_s) {
          this->Makefile->IssueMessage(
            MessageType::FATAL_ERROR,
            cmStrCat(
              "Target \"", tgt->GetName(), "\" contains the source\n  ", path,
              "\nin a file set of type \"", file_set_type,
              R"(" but the source is not classified as a "CXX" source.)"));
        }
      }
    }
  }
}

void cmMakefileTargetGenerator::WriteCommonCodeRules()
{
  const char* root = (this->Makefile->IsOn("CMAKE_MAKE_INCLUDE_FROM_ROOT")
                        ? "$(CMAKE_BINARY_DIR)/"
                        : "");

  // Include the dependencies for the target.
  std::string dependFileNameFull =
    cmStrCat(this->TargetBuildDirectoryFull, "/depend.make");
  *this->BuildFileStream
    << "# Include any dependencies generated for this target.\n"
    << this->GlobalGenerator->IncludeDirective << " " << root
    << cmSystemTools::ConvertToOutputPath(
         this->LocalGenerator->MaybeRelativeToTopBinDir(dependFileNameFull))
    << "\n";

  // Scan any custom commands to check if DEPFILE option is specified
  bool ccGenerateDeps = false;
  std::vector<cmSourceFile const*> customCommands;
  this->GeneratorTarget->GetCustomCommands(customCommands,
                                           this->GetConfigName());
  for (cmSourceFile const* sf : customCommands) {
    if (!sf->GetCustomCommand()->GetDepfile().empty()) {
      ccGenerateDeps = true;
      break;
    }
  }

  std::string depsUseCompiler = "CMAKE_DEPENDS_USE_COMPILER";
  bool compilerGenerateDeps =
    this->GlobalGenerator->SupportsCompilerDependencies() &&
    (!this->Makefile->IsDefinitionSet(depsUseCompiler) ||
     this->Makefile->IsOn(depsUseCompiler));
  bool linkerGenerateDeps =
    this->GeneratorTarget->HasLinkDependencyFile(this->GetConfigName());

  if (compilerGenerateDeps || linkerGenerateDeps || ccGenerateDeps) {
    std::string const compilerDependFile =
      cmStrCat(this->TargetBuildDirectoryFull, "/compiler_depend.make");
    *this->BuildFileStream << "# Include any dependencies generated by the "
                              "compiler for this target.\n"
                           << this->GlobalGenerator->IncludeDirective << " "
                           << root
                           << cmSystemTools::ConvertToOutputPath(
                                this->LocalGenerator->MaybeRelativeToTopBinDir(
                                  compilerDependFile))
                           << "\n\n";

    // Write an empty dependency file.
    cmGeneratedFileStream depFileStream(
      compilerDependFile, false, this->GlobalGenerator->GetMakefileEncoding());
    depFileStream << "# Empty compiler generated dependencies file for "
                  << this->GeneratorTarget->GetName() << ".\n"
                  << "# This may be replaced when dependencies are built.\n";
    // remove internal dependency file
    cmSystemTools::RemoveFile(
      cmStrCat(this->TargetBuildDirectoryFull, "/compiler_depend.internal"));

    std::string compilerDependTimestamp =
      cmStrCat(this->TargetBuildDirectoryFull, "/compiler_depend.ts");
    if (!cmSystemTools::FileExists(compilerDependTimestamp)) {
      // Write a dependency timestamp file.
      cmGeneratedFileStream timestampFileStream(
        compilerDependTimestamp, false,
        this->GlobalGenerator->GetMakefileEncoding());
      timestampFileStream
        << "# CMAKE generated file: DO NOT EDIT!\n"
        << "# Timestamp file for compiler generated dependencies "
           "management for "
        << this->GeneratorTarget->GetName() << ".\n";
    }
  }

  if (compilerGenerateDeps) {
    // deactivate no longer needed legacy dependency files
    // Write an empty dependency file.
    cmGeneratedFileStream legacyDepFileStream(
      dependFileNameFull, false, this->GlobalGenerator->GetMakefileEncoding());
    legacyDepFileStream
      << "# Empty dependencies file for " << this->GeneratorTarget->GetName()
      << ".\n"
      << "# This may be replaced when dependencies are built.\n";
    // remove internal dependency file
    cmSystemTools::RemoveFile(
      cmStrCat(this->TargetBuildDirectoryFull, "/depend.internal"));
  } else {
    // make sure the depend file exists
    if (!cmSystemTools::FileExists(dependFileNameFull)) {
      // Write an empty dependency file.
      cmGeneratedFileStream depFileStream(
        dependFileNameFull, false,
        this->GlobalGenerator->GetMakefileEncoding());
      depFileStream << "# Empty dependencies file for "
                    << this->GeneratorTarget->GetName() << ".\n"
                    << "# This may be replaced when dependencies are built.\n";
    }
  }

  if (!this->NoRuleMessages) {
    // Include the progress variables for the target.
    *this->BuildFileStream
      << "# Include the progress variables for this target.\n"
      << this->GlobalGenerator->IncludeDirective << " " << root
      << cmSystemTools::ConvertToOutputPath(
           this->LocalGenerator->MaybeRelativeToTopBinDir(
             this->ProgressFileNameFull))
      << "\n\n";
  }

  // Open the flags file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  this->FlagFileNameFull =
    cmStrCat(this->TargetBuildDirectoryFull, "/flags.make");
  this->FlagFileStream = cm::make_unique<cmGeneratedFileStream>(
    this->FlagFileNameFull, false,
    this->GlobalGenerator->GetMakefileEncoding());
  if (!this->FlagFileStream) {
    return;
  }
  this->FlagFileStream->SetCopyIfDifferent(true);
  this->LocalGenerator->WriteDisclaimer(*this->FlagFileStream);

  // Include the flags for the target.
  *this->BuildFileStream
    << "# Include the compile flags for this target's objects.\n"
    << this->GlobalGenerator->IncludeDirective << " " << root
    << cmSystemTools::ConvertToOutputPath(
         this->LocalGenerator->MaybeRelativeToTopBinDir(
           this->FlagFileNameFull))
    << "\n\n";
}

void cmMakefileTargetGenerator::WriteTargetLanguageFlags()
{
  // write language flags for target
  std::set<std::string> languages;
  this->GeneratorTarget->GetLanguages(
    languages, this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE"));
  // put the compiler in the rules.make file so that if it changes
  // things rebuild
  for (std::string const& language : languages) {
    std::string compiler = cmStrCat("CMAKE_", language, "_COMPILER");
    *this->FlagFileStream << "# compile " << language << " with "
                          << this->Makefile->GetSafeDefinition(compiler)
                          << "\n";
  }

  bool const escapeOctothorpe = this->GlobalGenerator->CanEscapeOctothorpe();

  for (std::string const& language : languages) {
    std::string defines = this->GetDefines(language, this->GetConfigName());
    std::string includes = this->GetIncludes(language, this->GetConfigName());
    if (escapeOctothorpe) {
      // Escape comment characters so they do not terminate assignment.
      cmSystemTools::ReplaceString(defines, "#", "\\#");
      cmSystemTools::ReplaceString(includes, "#", "\\#");
    }
    *this->FlagFileStream << language << "_DEFINES = " << defines << "\n\n";
    *this->FlagFileStream << language << "_INCLUDES = " << includes << "\n\n";

    std::vector<std::string> architectures =
      this->GeneratorTarget->GetAppleArchs(this->GetConfigName(), language);
    architectures.emplace_back();

    for (const std::string& arch : architectures) {
      std::string flags =
        this->GetFlags(language, this->GetConfigName(), arch);
      if (escapeOctothorpe) {
        cmSystemTools::ReplaceString(flags, "#", "\\#");
      }
      *this->FlagFileStream << language << "_FLAGS" << arch << " = " << flags
                            << "\n\n";
    }
  }
}

void cmMakefileTargetGenerator::MacOSXContentGeneratorType::operator()(
  cmSourceFile const& source, const char* pkgloc, const std::string& config)
{
  // Skip OS X content when not building a Framework or Bundle.
  if (!this->Generator->GetGeneratorTarget()->IsBundleOnApple()) {
    return;
  }

  std::string macdir =
    this->Generator->OSXBundleGenerator->InitMacOSXContentDirectory(pkgloc,
                                                                    config);

  // Get the input file location.
  std::string const& input = source.GetFullPath();

  // Get the output file location.
  std::string output =
    cmStrCat(macdir, '/', cmSystemTools::GetFilenameName(input));
  this->Generator->CleanFiles.insert(
    this->Generator->LocalGenerator->MaybeRelativeToCurBinDir(output));
  output = this->Generator->LocalGenerator->MaybeRelativeToTopBinDir(output);

  // Create a rule to copy the content into the bundle.
  std::vector<std::string> depends;
  std::vector<std::string> commands;
  depends.push_back(input);
  std::string copyEcho = cmStrCat("Copying OS X content ", output);
  this->Generator->LocalGenerator->AppendEcho(
    commands, copyEcho, cmLocalUnixMakefileGenerator3::EchoBuild);
  std::string copyCommand =
    cmStrCat("$(CMAKE_COMMAND) -E copy ",
             this->Generator->LocalGenerator->ConvertToOutputFormat(
               input, cmOutputConverter::SHELL),
             ' ',
             this->Generator->LocalGenerator->ConvertToOutputFormat(
               output, cmOutputConverter::SHELL));
  commands.push_back(std::move(copyCommand));
  this->Generator->LocalGenerator->WriteMakeRule(
    *this->Generator->BuildFileStream, nullptr, output, depends, commands,
    false);
  this->Generator->ExtraFiles.insert(output);
}

void cmMakefileTargetGenerator::WriteObjectRuleFiles(
  cmSourceFile const& source)
{
  // Identify the language of the source file.
  const std::string& lang = source.GetLanguage();
  if (lang.empty()) {
    // don't know anything about this file so skip it
    return;
  }

  // Use compiler to generate dependencies, if supported.
  bool const compilerGenerateDeps =
    this->GlobalGenerator->SupportsCompilerDependencies() &&
    cmIsOn(this->Makefile->GetDefinition(
      cmStrCat("CMAKE_", lang, "_DEPENDS_USE_COMPILER")));
  auto const scanner = compilerGenerateDeps ? cmDependencyScannerKind::Compiler
                                            : cmDependencyScannerKind::CMake;

  // Get the full path name of the object file.
  std::string const& objectName =
    this->GeneratorTarget->GetObjectName(&source);
  std::string const obj =
    cmStrCat(this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget),
             '/', objectName);

  // Avoid generating duplicate rules.
  if (this->ObjectFiles.find(obj) == this->ObjectFiles.end()) {
    this->ObjectFiles.insert(obj);
  } else {
    std::ostringstream err;
    err << "Warning: Source file \"" << source.GetFullPath()
        << "\" is listed multiple times for target \""
        << this->GeneratorTarget->GetName() << "\".";
    cmSystemTools::Message(err.str(), "Warning");
    return;
  }

  // Create the directory containing the object file.  This may be a
  // subdirectory under the target's directory.
  {
    std::string const dir = cmSystemTools::GetFilenamePath(obj);
    cmSystemTools::MakeDirectory(this->LocalGenerator->ConvertToFullPath(dir));
  }

  // Save this in the target's list of object files.
  this->Objects.push_back(obj);
  this->CleanFiles.insert(obj);

  std::vector<std::string> depends;

  // The object file should be checked for dependency integrity.
  std::string objFullPath =
    cmStrCat(this->LocalGenerator->GetCurrentBinaryDirectory(), '/', obj);
  objFullPath = cmSystemTools::CollapseFullPath(objFullPath);
  std::string const srcFullPath =
    cmSystemTools::CollapseFullPath(source.GetFullPath());
  this->LocalGenerator->AddImplicitDepends(this->GeneratorTarget, lang,
                                           objFullPath, srcFullPath, scanner);

  this->LocalGenerator->AppendRuleDepend(depends,
                                         this->FlagFileNameFull.c_str());
  this->LocalGenerator->AppendRuleDepends(depends,
                                          this->FlagFileDepends[lang]);

  // generate the depend scanning rule
  this->WriteObjectDependRules(source, depends);

  std::string const config = this->GetConfigName();
  std::string const configUpper = cmSystemTools::UpperCase(config);

  // Add precompile headers dependencies
  std::vector<std::string> architectures =
    this->GeneratorTarget->GetAppleArchs(config, lang);
  if (architectures.empty()) {
    architectures.emplace_back();
  }

  std::string filterArch;
  std::unordered_map<std::string, std::string> pchSources;
  for (const std::string& arch : architectures) {
    const std::string pchSource =
      this->GeneratorTarget->GetPchSource(config, lang, arch);
    if (pchSource == source.GetFullPath()) {
      filterArch = arch;
    }
    if (!pchSource.empty()) {
      pchSources.insert(std::make_pair(pchSource, arch));
    }
  }

  if (!pchSources.empty() && !source.GetProperty("SKIP_PRECOMPILE_HEADERS")) {
    for (const std::string& arch : architectures) {
      std::string const& pchHeader =
        this->GeneratorTarget->GetPchHeader(config, lang, arch);
      depends.push_back(pchHeader);
      if (pchSources.find(source.GetFullPath()) == pchSources.end()) {
        depends.push_back(
          this->GeneratorTarget->GetPchFile(config, lang, arch));
      }
      this->LocalGenerator->AddImplicitDepends(
        this->GeneratorTarget, lang, objFullPath, pchHeader, scanner);
    }
  }

  if (lang != "ISPC") {
    auto const& headers =
      this->GeneratorTarget->GetGeneratedISPCHeaders(config);
    if (!headers.empty()) {
      depends.insert(depends.end(), headers.begin(), headers.end());
    }
  }

  std::string relativeObj =
    cmStrCat(this->LocalGenerator->GetHomeRelativeOutputPath(), obj);
  // Write the build rule.

  // Build the set of compiler flags.
  std::string flags;

  // Explicitly add the explicit language flag before any other flag
  // so user flags can override it.
  this->GeneratorTarget->AddExplicitLanguageFlags(flags, source);

  // Add language-specific flags.
  std::string const langFlags =
    cmStrCat("$(", lang, "_FLAGS", filterArch, ")");
  this->LocalGenerator->AppendFlags(flags, langFlags);

  cmGeneratorExpressionInterpreter genexInterpreter(
    this->LocalGenerator, config, this->GeneratorTarget, lang);

  // Add Fortran format flags.
  if (lang == "Fortran") {
    this->AppendFortranFormatFlags(flags, source);
    this->AppendFortranPreprocessFlags(flags, source);
  }

  std::string ispcHeaderRelative;
  std::string ispcHeaderForShell;
  if (lang == "ISPC") {
    std::string ispcSource =
      cmSystemTools::GetFilenameWithoutLastExtension(objectName);
    ispcSource = cmSystemTools::GetFilenameWithoutLastExtension(ispcSource);

    cmValue const ispcSuffixProp =
      this->GeneratorTarget->GetProperty("ISPC_HEADER_SUFFIX");
    assert(ispcSuffixProp);

    std::string directory = this->GeneratorTarget->GetObjectDirectory(config);
    if (cmValue prop =
          this->GeneratorTarget->GetProperty("ISPC_HEADER_DIRECTORY")) {
      directory =
        cmStrCat(this->LocalGenerator->GetBinaryDirectory(), '/', *prop);
    }
    ispcHeaderRelative = cmStrCat(directory, '/', ispcSource, *ispcSuffixProp);
    ispcHeaderForShell = this->LocalGenerator->ConvertToOutputFormat(
      ispcHeaderRelative, cmOutputConverter::SHELL);
  }

  // Add flags from source file properties.
  const std::string COMPILE_FLAGS("COMPILE_FLAGS");
  if (cmValue cflags = source.GetProperty(COMPILE_FLAGS)) {
    const std::string& evaluatedFlags =
      genexInterpreter.Evaluate(*cflags, COMPILE_FLAGS);
    this->LocalGenerator->AppendFlags(flags, evaluatedFlags);
    *this->FlagFileStream << "# Custom flags: " << relativeObj
                          << "_FLAGS = " << evaluatedFlags << "\n"
                          << "\n";
  }

  const std::string COMPILE_OPTIONS("COMPILE_OPTIONS");
  if (cmValue coptions = source.GetProperty(COMPILE_OPTIONS)) {
    const std::string& evaluatedOptions =
      genexInterpreter.Evaluate(*coptions, COMPILE_OPTIONS);
    this->LocalGenerator->AppendCompileOptions(flags, evaluatedOptions);
    *this->FlagFileStream << "# Custom options: " << relativeObj
                          << "_OPTIONS = " << evaluatedOptions << "\n"
                          << "\n";
  }

  // Add precompile headers compile options.
  if (!pchSources.empty() && !source.GetProperty("SKIP_PRECOMPILE_HEADERS")) {
    std::string pchOptions;
    auto const pchIt = pchSources.find(source.GetFullPath());
    if (pchIt != pchSources.end()) {
      pchOptions = this->GeneratorTarget->GetPchCreateCompileOptions(
        config, lang, pchIt->second);
    } else {
      pchOptions =
        this->GeneratorTarget->GetPchUseCompileOptions(config, lang);
    }

    const std::string& evaluatedFlags =
      genexInterpreter.Evaluate(pchOptions, COMPILE_OPTIONS);

    this->LocalGenerator->AppendCompileOptions(flags, evaluatedFlags);
    *this->FlagFileStream << "# PCH options: " << relativeObj
                          << "_OPTIONS = " << evaluatedFlags << "\n"
                          << "\n";
  }

  // Add include directories from source file properties.
  std::vector<std::string> includes;

  const std::string INCLUDE_DIRECTORIES("INCLUDE_DIRECTORIES");
  if (cmValue cincludes = source.GetProperty(INCLUDE_DIRECTORIES)) {
    const std::string& evaluatedIncludes =
      genexInterpreter.Evaluate(*cincludes, INCLUDE_DIRECTORIES);
    this->LocalGenerator->AppendIncludeDirectories(includes, evaluatedIncludes,
                                                   source);
    *this->FlagFileStream << "# Custom include directories: " << relativeObj
                          << "_INCLUDE_DIRECTORIES = " << evaluatedIncludes
                          << "\n"
                          << "\n";
  }

  // Add language-specific defines.
  std::set<std::string> defines;

  // Add source-specific preprocessor definitions.
  const std::string COMPILE_DEFINITIONS("COMPILE_DEFINITIONS");
  if (cmValue compile_defs = source.GetProperty(COMPILE_DEFINITIONS)) {
    const std::string& evaluatedDefs =
      genexInterpreter.Evaluate(*compile_defs, COMPILE_DEFINITIONS);
    this->LocalGenerator->AppendDefines(defines, evaluatedDefs);
    *this->FlagFileStream << "# Custom defines: " << relativeObj
                          << "_DEFINES = " << evaluatedDefs << "\n"
                          << "\n";
  }
  std::string const defPropName =
    cmStrCat("COMPILE_DEFINITIONS_", configUpper);
  if (cmValue config_compile_defs = source.GetProperty(defPropName)) {
    const std::string& evaluatedDefs =
      genexInterpreter.Evaluate(*config_compile_defs, COMPILE_DEFINITIONS);
    this->LocalGenerator->AppendDefines(defines, evaluatedDefs);
    *this->FlagFileStream << "# Custom defines: " << relativeObj << "_DEFINES_"
                          << configUpper << " = " << evaluatedDefs << "\n"
                          << "\n";
  }

  // Get the output paths for source and object files.
  std::string const sourceFile = this->LocalGenerator->ConvertToOutputFormat(
    source.GetFullPath(), cmOutputConverter::SHELL);

  // Construct the build message.
  std::vector<std::string> no_depends;
  std::vector<std::string> commands;

  // add in a progress call if needed
  this->NumberOfProgressActions++;

  if (!this->NoRuleMessages) {
    cmLocalUnixMakefileGenerator3::EchoProgress progress;
    this->MakeEchoProgress(progress);
    std::string buildEcho =
      cmStrCat("Building ", lang, " object ", relativeObj);
    this->LocalGenerator->AppendEcho(commands, buildEcho,
                                     cmLocalUnixMakefileGenerator3::EchoBuild,
                                     &progress);
  }

  std::string targetOutPathReal;
  std::string targetOutPathPDB;
  std::string targetOutPathCompilePDB;
  {
    std::string targetFullPathReal;
    std::string targetFullPathPDB;
    std::string targetFullPathCompilePDB =
      this->ComputeTargetCompilePDB(this->GetConfigName());
    if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE ||
        this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY ||
        this->GeneratorTarget->GetType() == cmStateEnums::SHARED_LIBRARY ||
        this->GeneratorTarget->GetType() == cmStateEnums::MODULE_LIBRARY) {
      targetFullPathReal = this->GeneratorTarget->GetFullPath(
        this->GetConfigName(), cmStateEnums::RuntimeBinaryArtifact, true);
      targetFullPathPDB = cmStrCat(
        this->GeneratorTarget->GetPDBDirectory(this->GetConfigName()), '/',
        this->GeneratorTarget->GetPDBName(this->GetConfigName()));
    }

    targetOutPathReal = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPathReal),
      cmOutputConverter::SHELL);
    targetOutPathPDB = this->LocalGenerator->ConvertToOutputFormat(
      targetFullPathPDB, cmOutputConverter::SHELL);
    targetOutPathCompilePDB = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPathCompilePDB),
      cmOutputConverter::SHELL);

    if (this->LocalGenerator->IsMinGWMake() &&
        cmHasLiteralSuffix(targetOutPathCompilePDB, "\\")) {
      // mingw32-make incorrectly interprets 'a\ b c' as 'a b' and 'c'
      // (but 'a\ b "c"' as 'a\', 'b', and 'c'!).  Workaround this by
      // avoiding a trailing backslash in the argument.
      targetOutPathCompilePDB.back() = '/';
    }

    std::string const compilePdbOutputPath =
      this->GeneratorTarget->GetCompilePDBDirectory(this->GetConfigName());
    cmSystemTools::MakeDirectory(compilePdbOutputPath);
  }
  cmRulePlaceholderExpander::RuleVariables vars;
  vars.CMTargetName = this->GeneratorTarget->GetName().c_str();
  vars.CMTargetType =
    cmState::GetTargetTypeName(this->GeneratorTarget->GetType()).c_str();
  vars.Language = lang.c_str();
  vars.Target = targetOutPathReal.c_str();
  vars.TargetPDB = targetOutPathPDB.c_str();
  vars.TargetCompilePDB = targetOutPathCompilePDB.c_str();
  vars.Source = sourceFile.c_str();
  std::string const shellObj =
    this->LocalGenerator->ConvertToOutputFormat(obj, cmOutputConverter::SHELL);
  vars.Object = shellObj.c_str();
  std::string objectDir = this->GeneratorTarget->GetSupportDirectory();
  objectDir = this->LocalGenerator->ConvertToOutputFormat(
    this->LocalGenerator->MaybeRelativeToCurBinDir(objectDir),
    cmOutputConverter::SHELL);
  vars.ObjectDir = objectDir.c_str();
  std::string objectFileDir = cmSystemTools::GetFilenamePath(obj);
  objectFileDir = this->LocalGenerator->ConvertToOutputFormat(
    this->LocalGenerator->MaybeRelativeToCurBinDir(objectFileDir),
    cmOutputConverter::SHELL);
  vars.ObjectFileDir = objectFileDir.c_str();
  vars.Flags = flags.c_str();
  vars.ISPCHeader = ispcHeaderForShell.c_str();

  std::string definesString = cmStrCat("$(", lang, "_DEFINES)");

  this->LocalGenerator->JoinDefines(defines, definesString, lang);

  vars.Defines = definesString.c_str();

  std::string includesString = this->LocalGenerator->GetIncludeFlags(
    includes, this->GeneratorTarget, lang, config);
  this->LocalGenerator->AppendFlags(includesString,
                                    "$(" + lang + "_INCLUDES)");
  vars.Includes = includesString.c_str();

  std::string dependencyTarget;
  std::string shellDependencyFile;
  std::string dependencyTimestamp;
  if (compilerGenerateDeps) {
    dependencyTarget = this->LocalGenerator->EscapeForShell(
      this->LocalGenerator->ConvertToMakefilePath(
        this->LocalGenerator->MaybeRelativeToTopBinDir(relativeObj)));
    vars.DependencyTarget = dependencyTarget.c_str();

    auto depFile = cmStrCat(obj, ".d");
    shellDependencyFile = this->LocalGenerator->ConvertToOutputFormat(
      depFile, cmOutputConverter::SHELL);
    vars.DependencyFile = shellDependencyFile.c_str();
    this->CleanFiles.insert(depFile);

    dependencyTimestamp = this->LocalGenerator->MaybeRelativeToTopBinDir(
      cmStrCat(this->TargetBuildDirectoryFull, "/compiler_depend.ts"));
  }

  // At the moment, it is assumed that C, C++, Fortran, and CUDA have both
  // assembly and preprocessor capabilities. The same is true for the
  // ability to export compile commands
  bool const lang_has_preprocessor =
    ((lang == "C") || (lang == "CXX") || (lang == "OBJC") ||
     (lang == "OBJCXX") || (lang == "Fortran") || (lang == "CUDA") ||
     lang == "ISPC" || lang == "HIP" || lang == "ASM");
  bool const lang_has_assembly = lang_has_preprocessor;
  bool const lang_can_export_cmds = lang_has_preprocessor;

  auto rulePlaceholderExpander =
    this->LocalGenerator->CreateRulePlaceholderExpander();

  // Construct the compile rules.
  {
    std::string cudaCompileMode;
    if (lang == "CUDA") {
      if (this->GeneratorTarget->GetPropertyAsBool(
            "CUDA_SEPARABLE_COMPILATION")) {
        const std::string& rdcFlag =
          this->Makefile->GetRequiredDefinition("_CMAKE_CUDA_RDC_FLAG");
        cudaCompileMode = cmStrCat(cudaCompileMode, rdcFlag, " ");
      }

      static std::array<cm::string_view, 4> const compileModes{
        { "PTX"_s, "CUBIN"_s, "FATBIN"_s, "OPTIX"_s }
      };
      bool useNormalCompileMode = true;
      for (cm::string_view mode : compileModes) {
        auto propName = cmStrCat("CUDA_", mode, "_COMPILATION");
        auto defName = cmStrCat("_CMAKE_CUDA_", mode, "_FLAG");
        if (this->GeneratorTarget->GetPropertyAsBool(propName)) {
          const std::string& flag =
            this->Makefile->GetRequiredDefinition(defName);
          cudaCompileMode = cmStrCat(cudaCompileMode, flag);
          useNormalCompileMode = false;
          break;
        }
      }
      if (useNormalCompileMode) {
        const std::string& wholeFlag =
          this->Makefile->GetRequiredDefinition("_CMAKE_CUDA_WHOLE_FLAG");
        cudaCompileMode = cmStrCat(cudaCompileMode, wholeFlag);
      }
      vars.CudaCompileMode = cudaCompileMode.c_str();
    }

    cmList compileCommands;
    const std::string& compileRule = this->Makefile->GetRequiredDefinition(
      "CMAKE_" + lang + "_COMPILE_OBJECT");
    compileCommands.assign(compileRule);

    if (this->GeneratorTarget->GetPropertyAsBool("EXPORT_COMPILE_COMMANDS") &&
        lang_can_export_cmds && compileCommands.size() == 1) {
      std::string compileCommand = compileCommands[0];

      // no launcher for CMAKE_EXPORT_COMPILE_COMMANDS
      rulePlaceholderExpander->ExpandRuleVariables(this->LocalGenerator,
                                                   compileCommand, vars);
      std::string const workingDirectory =
        this->LocalGenerator->GetCurrentBinaryDirectory();
      std::string::size_type lfPos = compileCommand.find(langFlags);
      if (lfPos != std::string::npos) {
        compileCommand.replace(lfPos, langFlags.size(),
                               this->GetFlags(lang, this->GetConfigName()));
      }
      std::string const langDefines = std::string("$(") + lang + "_DEFINES)";
      std::string::size_type const ldPos = compileCommand.find(langDefines);
      if (ldPos != std::string::npos) {
        compileCommand.replace(ldPos, langDefines.size(),
                               this->GetDefines(lang, this->GetConfigName()));
      }
      std::string const langIncludes = std::string("$(") + lang + "_INCLUDES)";
      std::string::size_type const liPos = compileCommand.find(langIncludes);
      if (liPos != std::string::npos) {
        compileCommand.replace(liPos, langIncludes.size(),
                               this->GetIncludes(lang, this->GetConfigName()));
      }

      cmValue const eliminate[] = {
        this->Makefile->GetDefinition("CMAKE_START_TEMP_FILE"),
        this->Makefile->GetDefinition("CMAKE_END_TEMP_FILE")
      };
      for (cmValue const& el : eliminate) {
        if (el) {
          cmSystemTools::ReplaceString(compileCommand, *el, "");
        }
      }

      this->GlobalGenerator->AddCXXCompileCommand(
        source.GetFullPath(), workingDirectory, compileCommand, relativeObj);
    }

    // See if we need to use a compiler launcher like ccache or distcc
    std::string compilerLauncher;
    if (!compileCommands.empty()) {
      compilerLauncher = GetCompilerLauncher(lang, config);
    }

    cmValue const skipCodeCheck = source.GetProperty("SKIP_LINTING");
    if (!skipCodeCheck.IsOn()) {
      std::string const codeCheck = this->GenerateCodeCheckRules(
        source, compilerLauncher, "$(CMAKE_COMMAND)", config, nullptr);
      if (!codeCheck.empty()) {
        compileCommands.front().insert(0, codeCheck);
      }
    }

    // If compiler launcher was specified and not consumed above, it
    // goes to the beginning of the command line.
    if (!compileCommands.empty() && !compilerLauncher.empty()) {
      cmList args{ compilerLauncher, cmList::EmptyElements::Yes };
      if (!args.empty()) {
        args[0] = this->LocalGenerator->ConvertToOutputFormat(
          args[0], cmOutputConverter::SHELL);
        for (std::string& i : cmMakeRange(args.begin() + 1, args.end())) {
          i = this->LocalGenerator->EscapeForShell(i);
        }
      }
      compileCommands.front().insert(0, args.join(" ") + " ");
    }

    std::string launcher;
    {
      std::string val = this->LocalGenerator->GetRuleLauncher(
        this->GeneratorTarget, "RULE_LAUNCH_COMPILE",
        this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE"));
      if (cmNonempty(val)) {
        launcher = cmStrCat(val, ' ');
      }
    }

    std::string flagsWithDeps(flags);

    if (compilerGenerateDeps) {
      // Injects dependency computation
      auto depFlags = this->Makefile->GetSafeDefinition(
        cmStrCat("CMAKE_DEPFILE_FLAGS_", lang));

      if (!depFlags.empty()) {
        // Add dependency flags
        rulePlaceholderExpander->ExpandRuleVariables(this->LocalGenerator,
                                                     depFlags, vars);
        flagsWithDeps.append(1, ' ');
        flagsWithDeps.append(depFlags);
      }
      vars.Flags = flagsWithDeps.c_str();

      const auto& extraCommands = this->Makefile->GetSafeDefinition(
        cmStrCat("CMAKE_", lang, "_DEPENDS_EXTRA_COMMANDS"));
      if (!extraCommands.empty()) {
        compileCommands.append(extraCommands);
      }

      const auto& depFormat = this->Makefile->GetRequiredDefinition(
        cmStrCat("CMAKE_", lang, "_DEPFILE_FORMAT"));

      if (depFormat == "msvc"_s) {
        // compiler must be launched through a wrapper to pick-up dependencies
        std::string depFilter =
          "$(CMAKE_COMMAND) -E cmake_cl_compile_depends ";
        depFilter += cmStrCat("--dep-file=", shellDependencyFile);
        depFilter +=
          cmStrCat(" --working-dir=",
                   this->LocalGenerator->ConvertToOutputFormat(
                     this->LocalGenerator->GetCurrentBinaryDirectory(),
                     cmOutputConverter::SHELL));
        const auto& prefix = this->Makefile->GetSafeDefinition(
          cmStrCat("CMAKE_", lang, "_CL_SHOWINCLUDES_PREFIX"));
        depFilter += cmStrCat(" --filter-prefix=",
                              this->LocalGenerator->ConvertToOutputFormat(
                                prefix, cmOutputConverter::SHELL));
        depFilter += " -- ";
        compileCommands.front().insert(0, depFilter);
      }
    }

    // Expand placeholders in the commands.
    for (std::string& compileCommand : compileCommands) {
      compileCommand = cmStrCat(launcher, compileCommand);
      rulePlaceholderExpander->ExpandRuleVariables(this->LocalGenerator,
                                                   compileCommand, vars);
    }

    // Change the command working directory to the local build tree.
    this->LocalGenerator->CreateCDCommand(
      compileCommands, this->LocalGenerator->GetCurrentBinaryDirectory(),
      this->LocalGenerator->GetBinaryDirectory());
    cm::append(commands, compileCommands);
  }

  // Check for extra outputs created by the compilation.
  cmList outputs;
  outputs.emplace_back(relativeObj);
  if (cmValue extra_outputs_str = source.GetProperty("OBJECT_OUTPUTS")) {
    std::string evaluated_outputs = cmGeneratorExpression::Evaluate(
      *extra_outputs_str, this->LocalGenerator, config);

    if (!evaluated_outputs.empty()) {
      // Register these as extra files to clean.
      outputs.append(evaluated_outputs);
    }
  }
  if (!ispcHeaderRelative.empty()) {
    // can't move ispcHeader as vars is using it
    outputs.emplace_back(ispcHeaderRelative);
  }

  if (outputs.size() > 1) {
    this->CleanFiles.insert(outputs.begin() + 1, outputs.end());
  }

  if (compilerGenerateDeps) {
    depends.push_back(dependencyTimestamp);
  }

  // Write the rule.
  this->WriteMakeRule(*this->BuildFileStream, nullptr, outputs, depends,
                      commands);

  if (compilerGenerateDeps) {
    // set back flags without dependency generation
    vars.Flags = flags.c_str();
  }

  bool do_preprocess_rules = lang_has_preprocessor &&
    this->LocalGenerator->GetCreatePreprocessedSourceRules();
  bool do_assembly_rules =
    lang_has_assembly && this->LocalGenerator->GetCreateAssemblySourceRules();
  if (do_preprocess_rules || do_assembly_rules) {
    std::vector<std::string> force_depends;
    force_depends.emplace_back("cmake_force");
    std::string::size_type dot_pos = relativeObj.rfind('.');
    std::string relativeObjBase = relativeObj.substr(0, dot_pos);
    dot_pos = obj.rfind('.');
    std::string objBase = obj.substr(0, dot_pos);

    if (do_preprocess_rules) {
      commands.clear();
      std::string const relativeObjI = relativeObjBase + ".i";
      std::string const objI = objBase + ".i";

      std::string preprocessEcho =
        cmStrCat("Preprocessing ", lang, " source to ", objI);
      this->LocalGenerator->AppendEcho(
        commands, preprocessEcho, cmLocalUnixMakefileGenerator3::EchoBuild);

      std::string preprocessRuleVar =
        cmStrCat("CMAKE_", lang, "_CREATE_PREPROCESSED_SOURCE");
      if (cmValue preprocessRule =
            this->Makefile->GetDefinition(preprocessRuleVar)) {
        cmList preprocessCommands{ *preprocessRule };

        std::string shellObjI = this->LocalGenerator->ConvertToOutputFormat(
          objI, cmOutputConverter::SHELL);
        vars.PreprocessedSource = shellObjI.c_str();

        // Expand placeholders in the commands.
        for (std::string& preprocessCommand : preprocessCommands) {
          // no launcher for preprocessor commands
          rulePlaceholderExpander->ExpandRuleVariables(
            this->LocalGenerator, preprocessCommand, vars);
        }

        this->LocalGenerator->CreateCDCommand(
          preprocessCommands,
          this->LocalGenerator->GetCurrentBinaryDirectory(),
          this->LocalGenerator->GetBinaryDirectory());
        cm::append(commands, preprocessCommands);
      } else {
        std::string cmd =
          cmStrCat("$(CMAKE_COMMAND) -E cmake_unimplemented_variable ",
                   preprocessRuleVar);
        commands.push_back(std::move(cmd));
      }

      this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, nullptr,
                                          relativeObjI, force_depends,
                                          commands, false);
    }

    if (do_assembly_rules) {
      commands.clear();
      std::string relativeObjS = relativeObjBase + ".s";
      std::string objS = objBase + ".s";

      std::string assemblyEcho =
        cmStrCat("Compiling ", lang, " source to assembly ", objS);
      this->LocalGenerator->AppendEcho(
        commands, assemblyEcho, cmLocalUnixMakefileGenerator3::EchoBuild);

      std::string assemblyRuleVar =
        cmStrCat("CMAKE_", lang, "_CREATE_ASSEMBLY_SOURCE");
      if (cmValue assemblyRule =
            this->Makefile->GetDefinition(assemblyRuleVar)) {
        cmList assemblyCommands{ *assemblyRule };

        std::string shellObjS = this->LocalGenerator->ConvertToOutputFormat(
          objS, cmOutputConverter::SHELL);
        vars.AssemblySource = shellObjS.c_str();

        // Expand placeholders in the commands.
        for (std::string& assemblyCommand : assemblyCommands) {
          // no launcher for assembly commands
          rulePlaceholderExpander->ExpandRuleVariables(this->LocalGenerator,
                                                       assemblyCommand, vars);
        }

        this->LocalGenerator->CreateCDCommand(
          assemblyCommands, this->LocalGenerator->GetCurrentBinaryDirectory(),
          this->LocalGenerator->GetBinaryDirectory());
        cm::append(commands, assemblyCommands);
      } else {
        std::string cmd =
          cmStrCat("$(CMAKE_COMMAND) -E cmake_unimplemented_variable ",
                   assemblyRuleVar);
        commands.push_back(std::move(cmd));
      }

      this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, nullptr,
                                          relativeObjS, force_depends,
                                          commands, false);
    }
  }
}

void cmMakefileTargetGenerator::WriteTargetCleanRules()
{
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // Construct the clean target name.
  std::string const cleanTarget = cmStrCat(
    this->LocalGenerator->GetRelativeTargetDirectory(this->GeneratorTarget),
    "/clean");

  // Construct the clean command.
  this->LocalGenerator->AppendCleanCommand(commands, this->CleanFiles,
                                           this->GeneratorTarget);
  this->LocalGenerator->CreateCDCommand(
    commands, this->LocalGenerator->GetCurrentBinaryDirectory(),
    this->LocalGenerator->GetBinaryDirectory());

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, nullptr,
                                      cleanTarget, depends, commands, true);
}

bool cmMakefileTargetGenerator::WriteMakeRule(
  std::ostream& os, const char* comment,
  const std::vector<std::string>& outputs,
  const std::vector<std::string>& depends,
  const std::vector<std::string>& commands, bool in_help)
{
  bool symbolic = false;
  if (outputs.empty()) {
    return symbolic;
  }

  // Check whether we need to bother checking for a symbolic output.
  bool const need_symbolic = this->GlobalGenerator->GetNeedSymbolicMark();

  // Check whether the first output is marked as symbolic.
  if (need_symbolic) {
    if (cmSourceFile* sf = this->Makefile->GetSource(outputs[0])) {
      symbolic = sf->GetPropertyAsBool("SYMBOLIC");
    }
  }

  // We always attach the actual commands to the first output.
  this->LocalGenerator->WriteMakeRule(os, comment, outputs[0], depends,
                                      commands, symbolic, in_help);

  // For single outputs, we are done.
  if (outputs.size() == 1) {
    return symbolic;
  }

  // For multiple outputs, make the extra ones depend on the first one.
  std::vector<std::string> const output_depends(1, outputs[0]);
  for (std::string const& output : cmMakeRange(outputs).advance(1)) {
    // Touch the extra output so "make" knows that it was updated,
    // but only if the output was actually created.
    std::string const out = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeRelativeToTopBinDir(output),
      cmOutputConverter::SHELL);
    std::vector<std::string> output_commands;

    bool o_symbolic = false;
    if (need_symbolic) {
      if (cmSourceFile const* sf = this->Makefile->GetSource(output)) {
        o_symbolic = sf->GetPropertyAsBool("SYMBOLIC");
      }
    }
    symbolic = symbolic && o_symbolic;

    if (!o_symbolic) {
      output_commands.push_back("@$(CMAKE_COMMAND) -E touch_nocreate " + out);
    }
    this->LocalGenerator->WriteMakeRule(os, nullptr, output, output_depends,
                                        output_commands, o_symbolic, in_help);

    if (!o_symbolic) {
      // At build time, remove the first output if this one does not exist
      // so that "make" will rerun the real commands that create this one.
      MultipleOutputPairsType::value_type p(output, outputs[0]);
      this->MultipleOutputPairs.insert(p);
    }
  }
  return symbolic;
}

void cmMakefileTargetGenerator::WriteTargetLinkDependRules()
{
  if (!this->GeneratorTarget->HasLinkDependencyFile(this->GetConfigName())) {
    return;
  }

  auto depFile = this->LocalGenerator->GetLinkDependencyFile(
    this->GeneratorTarget, this->GetConfigName());
  this->CleanFiles.insert(depFile);
  this->LocalGenerator->AddImplicitDepends(
    this->GeneratorTarget, "LINK",
    this->GeneratorTarget->GetFullPath(this->GetConfigName()), depFile,
    cmDependencyScannerKind::Compiler);
}
std::string cmMakefileTargetGenerator::GetClangTidyReplacementsFilePath(
  std::string const& directory, cmSourceFile const& source,
  std::string const& config) const
{
  (void)config;
  auto const& objectName = this->GeneratorTarget->GetObjectName(&source);
  auto fixesFile = cmSystemTools::CollapseFullPath(cmStrCat(
    directory, '/',
    this->GeneratorTarget->GetLocalGenerator()->MaybeRelativeToTopBinDir(
      cmStrCat(this->GeneratorTarget->GetLocalGenerator()
                 ->GetCurrentBinaryDirectory(),
               '/',
               this->GeneratorTarget->GetLocalGenerator()->GetTargetDirectory(
                 this->GeneratorTarget),
               '/', objectName, ".yaml"))));
  return fixesFile;
}

void cmMakefileTargetGenerator::WriteTargetDependRules()
{
  // must write the targets depend info file
  std::string dir =
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  this->InfoFileNameFull = cmStrCat(dir, "/DependInfo.cmake");
  this->InfoFileNameFull =
    this->LocalGenerator->ConvertToFullPath(this->InfoFileNameFull);
  this->InfoFileStream =
    cm::make_unique<cmGeneratedFileStream>(this->InfoFileNameFull);
  if (!this->InfoFileStream) {
    return;
  }
  this->InfoFileStream->SetCopyIfDifferent(true);
  this->LocalGenerator->WriteDependLanguageInfo(*this->InfoFileStream,
                                                this->GeneratorTarget);

  // Store multiple output pairs in the depend info file.
  if (!this->MultipleOutputPairs.empty()) {
    /* clang-format off */
    *this->InfoFileStream
      << "\n"
      << "# Pairs of files generated by the same build rule.\n"
      << "set(CMAKE_MULTIPLE_OUTPUT_PAIRS\n";
    /* clang-format on */
    for (auto const& pi : this->MultipleOutputPairs) {
      *this->InfoFileStream
        << "  " << cmOutputConverter::EscapeForCMake(pi.first) << " "
        << cmOutputConverter::EscapeForCMake(pi.second) << "\n";
    }
    *this->InfoFileStream << "  )\n\n";
  }

  // Store list of targets linked directly or transitively.
  {
    /* clang-format off */
  *this->InfoFileStream
    << "\n"
       "# Targets to which this target links which contain Fortran sources.\n"
       "set(CMAKE_Fortran_TARGET_LINKED_INFO_FILES\n";
    /* clang-format on */
    std::vector<std::string> const dirs =
      this->GetLinkedTargetDirectories("Fortran", this->GetConfigName());
    for (std::string const& d : dirs) {
      *this->InfoFileStream << "  \"" << d << "/DependInfo.cmake\"\n";
    }
    *this->InfoFileStream << "  )\n";
  }

  std::string const& working_dir =
    this->LocalGenerator->GetCurrentBinaryDirectory();

  /* clang-format off */
  *this->InfoFileStream
    << "\n"
    << "# Fortran module output directory.\n"
    << "set(CMAKE_Fortran_TARGET_MODULE_DIR \""
    << this->GeneratorTarget->GetFortranModuleDirectory(working_dir)
    << "\")\n";

  if (this->GeneratorTarget->IsFortranBuildingInstrinsicModules()) {
    *this->InfoFileStream
      << "\n"
      << "# Fortran compiler is building intrinsic modules.\n"
      << "set(CMAKE_Fortran_TARGET_BUILDING_INSTRINSIC_MODULES ON) \n";
  }
  /* clang-format on */

  // and now write the rule to use it
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // Construct the name of the dependency generation target.
  std::string const depTarget = cmStrCat(
    this->LocalGenerator->GetRelativeTargetDirectory(this->GeneratorTarget),
    "/depend");

  // Add a command to call CMake to scan dependencies.  CMake will
  // touch the corresponding depends file after scanning dependencies.
  std::ostringstream depCmd;
// TODO: Account for source file properties and directory-level
// definitions when scanning for dependencies.
#if !defined(_WIN32) || defined(__CYGWIN__)
  // This platform supports symlinks, so cmSystemTools will translate
  // paths.  Make sure PWD is set to the original name of the home
  // output directory to help cmSystemTools to create the same
  // translation table for the dependency scanning process.
  depCmd << "cd "
         << (this->LocalGenerator->ConvertToOutputFormat(
              this->LocalGenerator->GetBinaryDirectory(),
              cmOutputConverter::SHELL))
         << " && ";
#endif
  // Generate a call this signature:
  //
  //   cmake -E cmake_depends <generator>
  //                          <home-src-dir> <start-src-dir>
  //                          <home-out-dir> <start-out-dir>
  //                          <dep-info> --color=$(COLOR)
  //
  // This gives the dependency scanner enough information to recreate
  // the state of our local generator sufficiently for its needs.
  depCmd << "$(CMAKE_COMMAND) -E cmake_depends \""
         << this->GlobalGenerator->GetName() << "\" "
         << this->LocalGenerator->ConvertToOutputFormat(
              this->LocalGenerator->GetSourceDirectory(),
              cmOutputConverter::SHELL)
         << " "
         << this->LocalGenerator->ConvertToOutputFormat(
              this->LocalGenerator->GetCurrentSourceDirectory(),
              cmOutputConverter::SHELL)
         << " "
         << this->LocalGenerator->ConvertToOutputFormat(
              this->LocalGenerator->GetBinaryDirectory(),
              cmOutputConverter::SHELL)
         << " "
         << this->LocalGenerator->ConvertToOutputFormat(
              this->LocalGenerator->GetCurrentBinaryDirectory(),
              cmOutputConverter::SHELL)
         << " "
         << this->LocalGenerator->ConvertToOutputFormat(
              cmSystemTools::CollapseFullPath(this->InfoFileNameFull),
              cmOutputConverter::SHELL);
  if (this->LocalGenerator->GetColorMakefile()) {
    depCmd << " \"--color=$(COLOR)\"";
  }
  commands.push_back(depCmd.str());

  // Make sure all custom command outputs in this target are built.
  if (this->CustomCommandDriver == OnDepends) {
    this->DriveCustomCommands(depends);
  }

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, nullptr,
                                      depTarget, depends, commands, true);
}

void cmMakefileTargetGenerator::DriveCustomCommands(
  std::vector<std::string>& depends)
{
  // Depend on all custom command outputs.
  cm::append(depends, this->CustomCommandOutputs);
}

void cmMakefileTargetGenerator::WriteObjectDependRules(
  cmSourceFile const& source, std::vector<std::string>& depends)
{
  // Create the list of dependencies known at cmake time.  These are
  // shared between the object file and dependency scanning rule.
  depends.push_back(source.GetFullPath());
  if (cmValue objectDeps = source.GetProperty("OBJECT_DEPENDS")) {
    cmExpandList(*objectDeps, depends);
  }
}

void cmMakefileTargetGenerator::WriteDeviceLinkRule(
  std::vector<std::string>& commands, const std::string& output)
{
  std::string architecturesStr =
    this->GeneratorTarget->GetSafeProperty("CUDA_ARCHITECTURES");

  if (cmIsOff(architecturesStr)) {
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                 "CUDA_SEPARABLE_COMPILATION on Clang "
                                 "requires CUDA_ARCHITECTURES to be set.");
    return;
  }

  cmLocalUnixMakefileGenerator3* localGen{ this->LocalGenerator };
  cmList architectures{ architecturesStr };
  std::string const& relPath = localGen->GetHomeRelativeOutputPath();

  // Ensure there are no duplicates.
  const std::vector<std::string> linkDeps = [&]() -> std::vector<std::string> {
    std::vector<std::string> deps;
    this->AppendTargetDepends(deps, true);
    this->GeneratorTarget->GetLinkDepends(deps, this->GetConfigName(), "CUDA");

    for (std::string const& obj : this->Objects) {
      deps.emplace_back(cmStrCat(relPath, obj));
    }

    std::unordered_set<std::string> const depsSet(deps.begin(), deps.end());
    deps.clear();
    std::copy(depsSet.begin(), depsSet.end(), std::back_inserter(deps));
    return deps;
  }();

  const std::string objectDir = this->GeneratorTarget->ObjectDirectory;
  const std::string relObjectDir =
    localGen->MaybeRelativeToCurBinDir(objectDir);

  // Construct a list of files associated with this executable that
  // may need to be cleaned.
  std::vector<std::string> cleanFiles;
  cleanFiles.push_back(localGen->MaybeRelativeToCurBinDir(output));

  std::string profiles;
  std::vector<std::string> fatbinaryDepends;
  std::string const registerFile =
    cmStrCat(objectDir, "cmake_cuda_register.h");

  // Link device code for each architecture.
  for (const std::string& architectureKind : architectures) {
    std::string registerFileCmd;

    // The generated register file contains macros that when expanded
    // register the device routines. Because the routines are the same for
    // all architectures the register file will be the same too. Thus
    // generate it only on the first invocation to reduce overhead.
    if (fatbinaryDepends.empty()) {
      std::string const registerFileRel =
        cmStrCat(relPath, relObjectDir, "cmake_cuda_register.h");
      registerFileCmd =
        cmStrCat(" --register-link-binaries=", registerFileRel);
      cleanFiles.push_back(registerFileRel);
    }

    // Clang always generates real code, so strip the specifier.
    const std::string architecture =
      architectureKind.substr(0, architectureKind.find('-'));
    const std::string cubin =
      cmStrCat(objectDir, "sm_", architecture, ".cubin");

    profiles += cmStrCat(" -im=profile=sm_", architecture, ",file=", cubin);
    fatbinaryDepends.emplace_back(cubin);

    std::string command = cmStrCat(
      this->Makefile->GetRequiredDefinition("CMAKE_CUDA_DEVICE_LINKER"),
      " -arch=sm_", architecture, registerFileCmd, " -o=$@ ",
      cmJoin(linkDeps, " "));

    localGen->WriteMakeRule(*this->BuildFileStream, nullptr, cubin, linkDeps,
                            { command }, false);
  }

  // Combine all architectures into a single fatbinary.
  const std::string fatbinaryCommand =
    cmStrCat(this->Makefile->GetRequiredDefinition("CMAKE_CUDA_FATBINARY"),
             " -64 -cmdline=--compile-only -compress-all -link "
             "--embedded-fatbin=$@",
             profiles);
  const std::string fatbinaryOutput =
    cmStrCat(objectDir, "cmake_cuda_fatbin.h");
  const std::string fatbinaryOutputRel =
    cmStrCat(relPath, relObjectDir, "cmake_cuda_fatbin.h");

  localGen->WriteMakeRule(*this->BuildFileStream, nullptr, fatbinaryOutputRel,
                          fatbinaryDepends, { fatbinaryCommand }, false);

  // Compile the stub that registers the kernels and contains the
  // fatbinaries.
  cmRulePlaceholderExpander::RuleVariables vars;
  vars.CMTargetName = this->GetGeneratorTarget()->GetName().c_str();
  vars.CMTargetType =
    cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType()).c_str();

  vars.Language = "CUDA";
  vars.Object = output.c_str();
  vars.Fatbinary = fatbinaryOutput.c_str();
  vars.RegisterFile = registerFile.c_str();

  std::string linkFlags;
  this->GetDeviceLinkFlags(linkFlags, "CUDA");
  vars.LinkFlags = linkFlags.c_str();

  std::string const flags = this->GetFlags("CUDA", this->GetConfigName());
  vars.Flags = flags.c_str();

  std::string compileCmd = this->GetLinkRule("CMAKE_CUDA_DEVICE_LINK_COMPILE");
  auto rulePlaceholderExpander = localGen->CreateRulePlaceholderExpander();
  rulePlaceholderExpander->ExpandRuleVariables(localGen, compileCmd, vars);

  commands.emplace_back(compileCmd);
  localGen->WriteMakeRule(*this->BuildFileStream, nullptr, output,
                          { fatbinaryOutputRel }, commands, false);

  // Clean all the possible executable names and symlinks.
  this->CleanFiles.insert(cleanFiles.begin(), cleanFiles.end());
}

void cmMakefileTargetGenerator::GenerateCustomRuleFile(
  cmCustomCommandGenerator const& ccg)
{
  // Collect the commands.
  std::vector<std::string> commands;
  std::string comment = this->LocalGenerator->ConstructComment(ccg);
  if (!comment.empty()) {
    // add in a progress call if needed
    this->NumberOfProgressActions++;
    if (!this->NoRuleMessages) {
      cmLocalUnixMakefileGenerator3::EchoProgress progress;
      this->MakeEchoProgress(progress);
      this->LocalGenerator->AppendEcho(
        commands, comment, cmLocalUnixMakefileGenerator3::EchoGenerate,
        &progress);
    }
  }

  // Now append the actual user-specified commands.
  std::ostringstream content;
  this->LocalGenerator->AppendCustomCommand(
    commands, ccg, this->GeneratorTarget,
    this->LocalGenerator->GetBinaryDirectory(), false, &content);

  // Collect the dependencies.
  std::vector<std::string> depends;
  this->LocalGenerator->AppendCustomDepend(depends, ccg);

  if (!ccg.GetCC().GetDepfile().empty()) {
    // Add dependency over timestamp file for dependencies management
    auto dependTimestamp = cmSystemTools::ConvertToOutputPath(
      this->LocalGenerator->MaybeRelativeToTopBinDir(
        cmStrCat(this->TargetBuildDirectoryFull, "/compiler_depend.ts")));

    depends.push_back(dependTimestamp);
  }

  // Write the rule.
  const std::vector<std::string>& outputs = ccg.GetOutputs();
  bool const symbolic = this->WriteMakeRule(*this->BuildFileStream, nullptr,
                                            outputs, depends, commands);

  // Symbolic inputs are not expected to exist, so add dummy rules.
  if (this->CMP0113New && !depends.empty()) {
    std::vector<std::string> no_depends;
    std::vector<std::string> no_commands;
    for (std::string const& dep : depends) {
      if (cmSourceFile* dsf =
            this->Makefile->GetSource(dep, cmSourceFileLocationKind::Known)) {
        if (dsf->GetPropertyAsBool("SYMBOLIC")) {
          this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, nullptr,
                                              dep, no_depends, no_commands,
                                              true);
        }
      }
    }
  }

  // If the rule has changed make sure the output is rebuilt.
  if (!symbolic) {
    this->GlobalGenerator->AddRuleHash(ccg.GetOutputs(), content.str());
  }

  // Setup implicit dependency scanning.
  for (auto const& idi : ccg.GetCC().GetImplicitDepends()) {
    std::string objFullPath = cmSystemTools::CollapseFullPath(
      outputs[0], this->LocalGenerator->GetCurrentBinaryDirectory());
    std::string srcFullPath = cmSystemTools::CollapseFullPath(
      idi.second, this->LocalGenerator->GetCurrentBinaryDirectory());
    this->LocalGenerator->AddImplicitDepends(this->GeneratorTarget, idi.first,
                                             objFullPath, srcFullPath);
  }

  // Setup implicit depend for depfile if any
  if (!ccg.GetCC().GetDepfile().empty()) {
    std::string objFullPath = cmSystemTools::CollapseFullPath(
      outputs[0], this->LocalGenerator->GetCurrentBinaryDirectory());
    this->LocalGenerator->AddImplicitDepends(
      this->GeneratorTarget, "CUSTOM", objFullPath, ccg.GetFullDepfile(),
      cmDependencyScannerKind::Compiler);
  }

  this->CustomCommandOutputs.insert(outputs.begin(), outputs.end());
}

void cmMakefileTargetGenerator::MakeEchoProgress(
  cmLocalUnixMakefileGenerator3::EchoProgress& progress) const
{
  progress.Dir =
    cmStrCat(this->LocalGenerator->GetBinaryDirectory(), "/CMakeFiles");
  std::ostringstream progressArg;
  progressArg << "$(CMAKE_PROGRESS_" << this->NumberOfProgressActions << ")";
  progress.Arg = progressArg.str();
}

void cmMakefileTargetGenerator::WriteObjectsVariable(
  std::string& variableName, std::string& variableNameExternal,
  bool useWatcomQuote)
{
  // Write a make variable assignment that lists all objects for the
  // target.
  variableName = this->LocalGenerator->CreateMakeVariable(
    this->GeneratorTarget->GetName(), "_OBJECTS");
  *this->BuildFileStream << "# Object files for target "
                         << this->GeneratorTarget->GetName() << "\n"
                         << variableName << " =";
  std::string object;
  const auto& lineContinue = this->GlobalGenerator->LineContinueDirective;

  cmValue pchExtension = this->Makefile->GetDefinition("CMAKE_PCH_EXTENSION");

  for (std::string const& obj : this->Objects) {
    if (cmHasSuffix(obj, pchExtension)) {
      continue;
    }
    *this->BuildFileStream << " " << lineContinue;
    *this->BuildFileStream
      << cmLocalUnixMakefileGenerator3::ConvertToQuotedOutputPath(
           obj, useWatcomQuote);
  }
  *this->BuildFileStream << "\n";

  // Write a make variable assignment that lists all external objects
  // for the target.
  variableNameExternal = this->LocalGenerator->CreateMakeVariable(
    this->GeneratorTarget->GetName(), "_EXTERNAL_OBJECTS");
  /* clang-format off */
  *this->BuildFileStream
    << "\n"
    << "# External object files for target "
    << this->GeneratorTarget->GetName() << "\n"
    << variableNameExternal << " =";
  /* clang-format on */
  for (std::string const& obj : this->ExternalObjects) {
    object = this->LocalGenerator->MaybeRelativeToCurBinDir(obj);
    *this->BuildFileStream << " " << lineContinue;
    *this->BuildFileStream
      << cmLocalUnixMakefileGenerator3::ConvertToQuotedOutputPath(
           obj, useWatcomQuote);
  }
  *this->BuildFileStream << "\n"
                         << "\n";
}

class cmMakefileTargetGeneratorObjectStrings
{
public:
  cmMakefileTargetGeneratorObjectStrings(std::vector<std::string>& strings,
                                         cmOutputConverter* outputConverter,
                                         bool useWatcomQuote,
                                         cmStateDirectory const& stateDir,
                                         std::string::size_type limit)
    : Strings(strings)
    , OutputConverter(outputConverter)
    , UseWatcomQuote(useWatcomQuote)
    , StateDir(stateDir)
    , LengthLimit(limit)
  {
    this->Space = "";
  }
  void Feed(std::string const& obj)
  {
    // Construct the name of the next object.
    this->NextObject = this->OutputConverter->ConvertToOutputFormat(
      this->OutputConverter->MaybeRelativeToCurBinDir(obj),
      cmOutputConverter::RESPONSE, this->UseWatcomQuote);

    // Roll over to next string if the limit will be exceeded.
    if (this->LengthLimit != std::string::npos &&
        (this->CurrentString.length() + 1 + this->NextObject.length() >
         this->LengthLimit)) {
      this->Strings.push_back(this->CurrentString);
      this->CurrentString.clear();
      this->Space = "";
    }

    // Separate from previous object.
    this->CurrentString += this->Space;
    this->Space = " ";

    // Append this object.
    this->CurrentString += this->NextObject;
  }
  void Done() { this->Strings.push_back(this->CurrentString); }

private:
  std::vector<std::string>& Strings;
  cmOutputConverter* OutputConverter;
  bool UseWatcomQuote;
  cmStateDirectory StateDir;
  std::string::size_type LengthLimit;
  std::string CurrentString;
  std::string NextObject;
  const char* Space;
};

void cmMakefileTargetGenerator::WriteObjectsStrings(
  std::vector<std::string>& objStrings, bool useWatcomQuote,
  std::string::size_type limit)
{
  cmValue pchExtension = this->Makefile->GetDefinition("CMAKE_PCH_EXTENSION");

  cmMakefileTargetGeneratorObjectStrings helper(
    objStrings, this->LocalGenerator, useWatcomQuote,
    this->LocalGenerator->GetStateSnapshot().GetDirectory(), limit);
  for (std::string const& obj : this->Objects) {
    if (cmHasSuffix(obj, pchExtension)) {
      continue;
    }
    helper.Feed(obj);
  }
  for (std::string const& obj : this->ExternalObjects) {
    helper.Feed(obj);
  }
  auto ispcAdditionalObjs =
    this->GeneratorTarget->GetGeneratedISPCObjects(this->GetConfigName());
  for (std::string const& obj : ispcAdditionalObjs) {
    helper.Feed(obj);
  }
  helper.Done();
}

void cmMakefileTargetGenerator::WriteTargetDriverRule(
  const std::string& main_output, bool relink)
{
  // Compute the name of the driver target.
  std::string dir =
    this->LocalGenerator->GetRelativeTargetDirectory(this->GeneratorTarget);
  std::string buildTargetRuleName =
    cmStrCat(dir, relink ? "/preinstall" : "/build");
  buildTargetRuleName =
    this->LocalGenerator->MaybeRelativeToTopBinDir(buildTargetRuleName);

  // Build the list of target outputs to drive.
  std::vector<std::string> depends;
  depends.push_back(main_output);

  const char* comment = nullptr;
  if (relink) {
    // Setup the comment for the preinstall driver.
    comment = "Rule to relink during preinstall.";
  } else {
    // Setup the comment for the main build driver.
    comment = "Rule to build all files generated by this target.";

    // Make sure all custom command outputs in this target are built.
    if (this->CustomCommandDriver == OnBuild) {
      this->DriveCustomCommands(depends);
    }

    // Make sure the extra files are built.
    cm::append(depends, this->ExtraFiles);
  }

  // Write the driver rule.
  std::vector<std::string> no_commands;
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, comment,
                                      buildTargetRuleName, depends,
                                      no_commands, true);
}

void cmMakefileTargetGenerator::AppendTargetDepends(
  std::vector<std::string>& depends, bool ignoreType)
{
  // Static libraries never depend on anything for linking.
  if (this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY &&
      !ignoreType) {
    return;
  }

  const std::string& cfg = this->GetConfigName();

  if (this->GeneratorTarget->HasLinkDependencyFile(cfg)) {
    depends.push_back(
      cmStrCat(this->TargetBuildDirectoryFull, "/compiler_depend.ts"));
  }

  // Loop over all library dependencies.
  if (cmComputeLinkInformation* cli =
        this->GeneratorTarget->GetLinkInformation(cfg)) {
    cm::append(depends, cli->GetDepends());
  }
}

void cmMakefileTargetGenerator::AppendObjectDepends(
  std::vector<std::string>& depends)
{
  // Add dependencies on the compiled object files.
  std::string const& relPath =
    this->LocalGenerator->GetHomeRelativeOutputPath();
  for (std::string const& obj : this->Objects) {
    std::string objTarget = cmStrCat(relPath, obj);
    depends.push_back(std::move(objTarget));
  }

  // Add dependencies on the external object files.
  cm::append(depends, this->ExternalObjects);

  // Add a dependency on the rule file itself.
  this->LocalGenerator->AppendRuleDepend(depends,
                                         this->BuildFileNameFull.c_str());
}

void cmMakefileTargetGenerator::AppendLinkDepends(
  std::vector<std::string>& depends, const std::string& linkLanguage)
{
  this->AppendObjectDepends(depends);

  // Add dependencies on targets that must be built first.
  this->AppendTargetDepends(depends);

  // Add a dependency on the link definitions file, if any.
  if (cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
        this->GeneratorTarget->GetModuleDefinitionInfo(
          this->GetConfigName())) {
    for (cmSourceFile const* src : mdi->Sources) {
      depends.push_back(src->GetFullPath());
    }
  }

  // Add a dependency on user-specified manifest files, if any.
  std::vector<cmSourceFile const*> manifest_srcs;
  this->GeneratorTarget->GetManifests(manifest_srcs, this->GetConfigName());
  for (cmSourceFile const* manifest_src : manifest_srcs) {
    depends.push_back(manifest_src->GetFullPath());
  }

  // Add user-specified dependencies.
  this->GeneratorTarget->GetLinkDepends(depends, this->GetConfigName(),
                                        linkLanguage);
}

std::string cmMakefileTargetGenerator::GetLinkRule(
  const std::string& linkRuleVar)
{
  std::string linkRule = this->Makefile->GetRequiredDefinition(linkRuleVar);
  if (this->GeneratorTarget->HasImplibGNUtoMS(this->GetConfigName())) {
    std::string ruleVar =
      cmStrCat("CMAKE_",
               this->GeneratorTarget->GetLinkerLanguage(this->GetConfigName()),
               "_GNUtoMS_RULE");
    if (cmValue rule = this->Makefile->GetDefinition(ruleVar)) {
      linkRule += *rule;
    }
  }
  return linkRule;
}

void cmMakefileTargetGenerator::CloseFileStreams()
{
  this->BuildFileStream.reset();
  this->InfoFileStream.reset();
  this->FlagFileStream.reset();
}

void cmMakefileTargetGenerator::CreateLinkScript(
  const char* name, std::vector<std::string> const& link_commands,
  std::vector<std::string>& makefile_commands,
  std::vector<std::string>& makefile_depends)
{
  // Create the link script file.
  std::string linkScriptName =
    cmStrCat(this->TargetBuildDirectoryFull, '/', name);
  cmGeneratedFileStream linkScriptStream(linkScriptName);
  linkScriptStream.SetCopyIfDifferent(true);
  for (std::string const& link_command : link_commands) {
    // Do not write out empty commands or commands beginning in the
    // shell no-op ":".
    if (!link_command.empty() && link_command[0] != ':') {
      linkScriptStream << link_command << "\n";
    }
  }

  // Create the makefile command to invoke the link script.
  std::string link_command =
    cmStrCat("$(CMAKE_COMMAND) -E cmake_link_script ",
             this->LocalGenerator->ConvertToOutputFormat(
               this->LocalGenerator->MaybeRelativeToCurBinDir(linkScriptName),
               cmOutputConverter::SHELL),
             " --verbose=$(VERBOSE)");
  makefile_commands.push_back(std::move(link_command));
  makefile_depends.push_back(std::move(linkScriptName));
}

bool cmMakefileTargetGenerator::CheckUseResponseFileForObjects(
  std::string const& l) const
{
  // Check for an explicit setting one way or the other.
  std::string const responseVar =
    "CMAKE_" + l + "_USE_RESPONSE_FILE_FOR_OBJECTS";
  if (cmValue val = this->Makefile->GetDefinition(responseVar)) {
    if (!val->empty()) {
      return cmIsOn(val);
    }
  }

  // Check for a system limit.
  if (size_t const limit = cmSystemTools::CalculateCommandLineLengthLimit()) {
    // Compute the total length of our list of object files with room
    // for argument separation and quoting.  This does not convert paths
    // relative to CMAKE_CURRENT_BINARY_DIR like the final list will be, so
    // the actual list will likely be much shorter than this.  However, in
    // the worst case all objects will remain as absolute paths.
    size_t length = 0;
    for (std::string const& obj : this->Objects) {
      length += obj.size() + 3;
    }
    for (std::string const& ext_obj : this->ExternalObjects) {
      length += ext_obj.size() + 3;
    }

    // We need to guarantee room for both objects and libraries, so
    // if the objects take up more than half then use a response file
    // for them.
    if (length > (limit / 2)) {
      return true;
    }
  }

  // We do not need a response file for objects.
  return false;
}

bool cmMakefileTargetGenerator::CheckUseResponseFileForLibraries(
  std::string const& l) const
{
  // Check for an explicit setting one way or the other.
  std::string const responseVar =
    "CMAKE_" + l + "_USE_RESPONSE_FILE_FOR_LIBRARIES";
  if (cmValue val = this->Makefile->GetDefinition(responseVar)) {
    if (!val->empty()) {
      return cmIsOn(val);
    }
  }

  // We do not need a response file for libraries.
  return false;
}

std::string cmMakefileTargetGenerator::CreateResponseFile(
  const std::string& name, std::string const& options,
  std::vector<std::string>& makefile_depends, std::string const& language)
{
  // FIXME: Find a better way to determine the response file encoding,
  // perhaps using tool-specific platform information variables.
  // For now, use the makefile encoding as a heuristic.
  codecvt_Encoding responseEncoding =
    this->GlobalGenerator->GetMakefileEncoding();
  // Non-MSVC tooling doesn't understand BOM encoded files.
  if (responseEncoding == codecvt_Encoding::UTF8_WITH_BOM &&
      (language == "CUDA" || !this->Makefile->IsOn("MSVC"))) {
    responseEncoding = codecvt_Encoding::UTF8;
  }

  // Create the response file.
  std::string responseFileNameFull =
    cmStrCat(this->TargetBuildDirectoryFull, '/', name);
  cmGeneratedFileStream responseStream(responseFileNameFull, false,
                                       responseEncoding);
  responseStream.SetCopyIfDifferent(true);
  responseStream << options << "\n";

  // Add a dependency so the target will rebuild when the set of
  // objects changes.
  makefile_depends.push_back(std::move(responseFileNameFull));

  // Construct the name to be used on the command line.
  std::string responseFileName =
    cmStrCat(this->TargetBuildDirectory, '/', name);
  return responseFileName;
}

std::unique_ptr<cmLinkLineComputer>
cmMakefileTargetGenerator::CreateLinkLineComputer(
  cmOutputConverter* outputConverter, cmStateDirectory const& stateDir)
{
  if (this->Makefile->IsOn("MSVC60")) {
    return this->GlobalGenerator->CreateMSVC60LinkLineComputer(outputConverter,
                                                               stateDir);
  }
  return this->GlobalGenerator->CreateLinkLineComputer(outputConverter,
                                                       stateDir);
}

void cmMakefileTargetGenerator::CreateLinkLibs(
  cmLinkLineComputer* linkLineComputer, std::string& linkLibs,
  bool useResponseFile, std::vector<std::string>& makefile_depends,
  std::string const& linkLanguage, ResponseFlagFor responseMode)
{
  std::string frameworkPath;
  std::string linkPath;
  cmComputeLinkInformation* pcli =
    this->GeneratorTarget->GetLinkInformation(this->GetConfigName());
  this->LocalGenerator->OutputLinkLibraries(pcli, linkLineComputer, linkLibs,
                                            frameworkPath, linkPath);
  linkLibs = frameworkPath + linkPath + linkLibs;

  if (useResponseFile &&
      linkLibs.find_first_not_of(' ') != std::string::npos) {
    // Lookup the response file reference flag.
    std::string responseFlag = this->GetResponseFlag(responseMode);

    // Create this response file.
    std::string responseFileName =
      (responseMode == Link) ? "linkLibs.rsp" : "deviceLinkLibs.rsp";
    std::string responseLang = (responseMode == Link) ? linkLanguage : "CUDA";
    std::string link_rsp = this->CreateResponseFile(
      responseFileName, linkLibs, makefile_depends, responseLang);

    // Reference the response file.
    linkLibs = cmStrCat(responseFlag,
                        this->LocalGenerator->ConvertToOutputFormat(
                          link_rsp, cmOutputConverter::SHELL));
  }
}

void cmMakefileTargetGenerator::CreateObjectLists(
  bool useLinkScript, bool useArchiveRules, bool useResponseFile,
  std::string& buildObjs, std::vector<std::string>& makefile_depends,
  bool useWatcomQuote, std::string const& linkLanguage,
  ResponseFlagFor responseMode)
{
  std::string variableName;
  std::string variableNameExternal;
  this->WriteObjectsVariable(variableName, variableNameExternal,
                             useWatcomQuote);
  if (useResponseFile) {
    // MSVC response files cannot exceed 128K.
    std::string::size_type constexpr responseFileLimit = 131000;

    // Construct the individual object list strings.
    std::vector<std::string> object_strings;
    this->WriteObjectsStrings(object_strings, useWatcomQuote,
                              responseFileLimit);

    // Lookup the response file reference flag.
    std::string responseFlag = this->GetResponseFlag(responseMode);

    // Write a response file for each string.
    const char* sep = "";
    for (unsigned int i = 0; i < object_strings.size(); ++i) {
      // Number the response files.
      std::string responseFileName =
        (responseMode == Link) ? "objects" : "deviceObjects";
      responseFileName += std::to_string(i + 1);
      responseFileName += ".rsp";

      // Create this response file.
      std::string objects_rsp = this->CreateResponseFile(
        responseFileName, object_strings[i], makefile_depends, linkLanguage);

      // Separate from previous response file references.
      buildObjs += sep;
      sep = " ";

      // Reference the response file.
      buildObjs += responseFlag;
      buildObjs += this->LocalGenerator->ConvertToOutputFormat(
        objects_rsp, cmOutputConverter::SHELL);
    }
  } else if (useLinkScript) {
    if (!useArchiveRules) {
      std::vector<std::string> objStrings;
      this->WriteObjectsStrings(objStrings, useWatcomQuote);
      buildObjs = objStrings[0];
    }
  } else {
    buildObjs =
      cmStrCat("$(", variableName, ") $(", variableNameExternal, ')');
  }
}

void cmMakefileTargetGenerator::AddIncludeFlags(std::string& flags,
                                                const std::string& lang,
                                                const std::string& /*config*/)
{
  std::string responseVar =
    cmStrCat("CMAKE_", lang, "_USE_RESPONSE_FILE_FOR_INCLUDES");
  bool useResponseFile = this->Makefile->IsOn(responseVar);

  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes, this->GeneratorTarget,
                                              lang, this->GetConfigName());

  std::string includeFlags = this->LocalGenerator->GetIncludeFlags(
    includes, this->GeneratorTarget, lang, this->GetConfigName(),
    useResponseFile);
  if (includeFlags.empty()) {
    return;
  }

  if (useResponseFile) {
    std::string const responseFlagVar =
      "CMAKE_" + lang + "_RESPONSE_FILE_FLAG";
    std::string responseFlag =
      this->Makefile->GetSafeDefinition(responseFlagVar);
    if (responseFlag.empty()) {
      responseFlag = "@";
    }
    std::string name = cmStrCat("includes_", lang, ".rsp");
    std::string arg = std::move(responseFlag) +
      this->CreateResponseFile(name, includeFlags, this->FlagFileDepends[lang],
                               lang);
    this->LocalGenerator->AppendFlags(flags, arg);
  } else {
    this->LocalGenerator->AppendFlags(flags, includeFlags);
  }
}

void cmMakefileTargetGenerator::GenDefFile(
  std::vector<std::string>& real_link_commands)
{
  cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
    this->GeneratorTarget->GetModuleDefinitionInfo(this->GetConfigName());
  if (!mdi || !mdi->DefFileGenerated) {
    return;
  }
  std::string cmd = cmSystemTools::GetCMakeCommand();
  cmd = cmStrCat(
    this->LocalGenerator->ConvertToOutputFormat(cmd, cmOutputConverter::SHELL),
    " -E __create_def ",
    this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeRelativeToCurBinDir(mdi->DefFile),
      cmOutputConverter::SHELL),
    ' ');
  std::string objlist_file = mdi->DefFile + ".objs";
  cmd += this->LocalGenerator->ConvertToOutputFormat(
    this->LocalGenerator->MaybeRelativeToCurBinDir(objlist_file),
    cmOutputConverter::SHELL);
  cmValue nm_executable = this->Makefile->GetDefinition("CMAKE_NM");
  if (cmNonempty(nm_executable)) {
    cmd += " --nm=";
    cmd += this->LocalCommonGenerator->ConvertToOutputFormat(
      *nm_executable, cmOutputConverter::SHELL);
  }
  real_link_commands.insert(real_link_commands.begin(), cmd);
  // create a list of obj files for the -E __create_def to read
  cmGeneratedFileStream fout(objlist_file);

  if (mdi->WindowsExportAllSymbols) {
    for (std::string const& obj : this->Objects) {
      if (cmHasLiteralSuffix(obj, ".obj")) {
        fout << obj << "\n";
      }
    }
    for (std::string const& obj : this->ExternalObjects) {
      fout << obj << "\n";
    }
  }

  for (cmSourceFile const* src : mdi->Sources) {
    fout << src->GetFullPath() << "\n";
  }
}

std::string cmMakefileTargetGenerator::GetResponseFlag(
  ResponseFlagFor mode) const
{
  std::string responseFlag = "@";
  std::string responseFlagVar;

  auto const lang =
    this->GeneratorTarget->GetLinkerLanguage(this->GetConfigName());
  if (mode == cmMakefileTargetGenerator::ResponseFlagFor::Link) {
    responseFlagVar = cmStrCat("CMAKE_", lang, "_RESPONSE_FILE_LINK_FLAG");
  } else if (mode == cmMakefileTargetGenerator::ResponseFlagFor::DeviceLink) {
    responseFlagVar = "CMAKE_CUDA_RESPONSE_FILE_DEVICE_LINK_FLAG";
  }

  if (cmValue p = this->Makefile->GetDefinition(responseFlagVar)) {
    responseFlag = *p;
  }
  return responseFlag;
}
