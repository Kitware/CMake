/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmNinjaTargetGenerator.h"

#include "cm_jsoncpp_value.h"
#include "cm_jsoncpp_writer.h"
#include <algorithm>
#include <assert.h>
#include <iterator>
#include <map>
#include <memory> // IWYU pragma: keep
#include <ostream>
#include <utility>

#include "cmAlgorithms.h"
#include "cmComputeLinkInformation.h"
#include "cmCustomCommandGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmLocalGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmMakefile.h"
#include "cmNinjaNormalTargetGenerator.h"
#include "cmNinjaUtilityTargetGenerator.h"
#include "cmOutputConverter.h"
#include "cmRange.h"
#include "cmRulePlaceholderExpander.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmake.h"

std::unique_ptr<cmNinjaTargetGenerator> cmNinjaTargetGenerator::New(
  cmGeneratorTarget* target)
{
  switch (target->GetType()) {
    case cmStateEnums::EXECUTABLE:
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::STATIC_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY:
    case cmStateEnums::OBJECT_LIBRARY:
      return cm::make_unique<cmNinjaNormalTargetGenerator>(target);

    case cmStateEnums::UTILITY:
    case cmStateEnums::GLOBAL_TARGET:
      return cm::make_unique<cmNinjaUtilityTargetGenerator>(target);

    default:
      return std::unique_ptr<cmNinjaTargetGenerator>();
  }
}

cmNinjaTargetGenerator::cmNinjaTargetGenerator(cmGeneratorTarget* target)
  : cmCommonTargetGenerator(target)
  , MacOSXContentGenerator(nullptr)
  , OSXBundleGenerator(nullptr)
  , LocalGenerator(
      static_cast<cmLocalNinjaGenerator*>(target->GetLocalGenerator()))
{
  MacOSXContentGenerator = cm::make_unique<MacOSXContentGeneratorType>(this);
}

cmNinjaTargetGenerator::~cmNinjaTargetGenerator() = default;

cmGeneratedFileStream& cmNinjaTargetGenerator::GetBuildFileStream() const
{
  return *this->GetGlobalGenerator()->GetBuildFileStream();
}

cmGeneratedFileStream& cmNinjaTargetGenerator::GetRulesFileStream() const
{
  return *this->GetGlobalGenerator()->GetRulesFileStream();
}

cmGlobalNinjaGenerator* cmNinjaTargetGenerator::GetGlobalGenerator() const
{
  return this->LocalGenerator->GetGlobalNinjaGenerator();
}

std::string cmNinjaTargetGenerator::LanguageCompilerRule(
  const std::string& lang) const
{
  return lang + "_COMPILER__" +
    cmGlobalNinjaGenerator::EncodeRuleName(this->GeneratorTarget->GetName());
}

std::string cmNinjaTargetGenerator::LanguagePreprocessRule(
  std::string const& lang) const
{
  return lang + "_PREPROCESS__" +
    cmGlobalNinjaGenerator::EncodeRuleName(this->GeneratorTarget->GetName());
}

bool cmNinjaTargetGenerator::NeedExplicitPreprocessing(
  std::string const& lang) const
{
  return lang == "Fortran";
}

bool cmNinjaTargetGenerator::UsePreprocessedSource(
  std::string const& lang) const
{
  return lang == "Fortran";
}

std::string cmNinjaTargetGenerator::LanguageDyndepRule(
  const std::string& lang) const
{
  return lang + "_DYNDEP__" +
    cmGlobalNinjaGenerator::EncodeRuleName(this->GeneratorTarget->GetName());
}

bool cmNinjaTargetGenerator::NeedDyndep(std::string const& lang) const
{
  return lang == "Fortran";
}

std::string cmNinjaTargetGenerator::OrderDependsTargetForTarget()
{
  return "cmake_object_order_depends_target_" + this->GetTargetName();
}

// TODO: Most of the code is picked up from
// void cmMakefileExecutableTargetGenerator::WriteExecutableRule(bool relink),
// void cmMakefileTargetGenerator::WriteTargetLanguageFlags()
// Refactor it.
std::string cmNinjaTargetGenerator::ComputeFlagsForObject(
  cmSourceFile const* source, const std::string& language)
{
  std::string flags = this->GetFlags(language);

  // Add Fortran format flags.
  if (language == "Fortran") {
    this->AppendFortranFormatFlags(flags, *source);
  }

  // Add source file specific flags.
  cmGeneratorExpressionInterpreter genexInterpreter(
    this->LocalGenerator, this->LocalGenerator->GetConfigName(),
    this->GeneratorTarget, language);

  const std::string COMPILE_FLAGS("COMPILE_FLAGS");
  if (const char* cflags = source->GetProperty(COMPILE_FLAGS)) {
    this->LocalGenerator->AppendFlags(
      flags, genexInterpreter.Evaluate(cflags, COMPILE_FLAGS));
  }

  const std::string COMPILE_OPTIONS("COMPILE_OPTIONS");
  if (const char* coptions = source->GetProperty(COMPILE_OPTIONS)) {
    this->LocalGenerator->AppendCompileOptions(
      flags, genexInterpreter.Evaluate(coptions, COMPILE_OPTIONS));
  }

  return flags;
}

void cmNinjaTargetGenerator::AddIncludeFlags(std::string& languageFlags,
                                             std::string const& language)
{
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes, this->GeneratorTarget,
                                              language, this->GetConfigName());
  // Add include directory flags.
  std::string includeFlags = this->LocalGenerator->GetIncludeFlags(
    includes, this->GeneratorTarget, language,
    language == "RC", // full include paths for RC needed by cmcldeps
    false, this->GetConfigName());
  if (this->GetGlobalGenerator()->IsGCCOnWindows()) {
    std::replace(includeFlags.begin(), includeFlags.end(), '\\', '/');
  }

  this->LocalGenerator->AppendFlags(languageFlags, includeFlags);
}

bool cmNinjaTargetGenerator::NeedDepTypeMSVC(const std::string& lang) const
{
  std::string const& deptype =
    this->GetMakefile()->GetSafeDefinition("CMAKE_NINJA_DEPTYPE_" + lang);
  if (deptype == "msvc") {
    return true;
  }
  if (deptype == "intel") {
    // Ninja does not really define "intel", but we use it to switch based
    // on whether this environment supports "gcc" or "msvc" deptype.
    if (!this->GetGlobalGenerator()->SupportsMultilineDepfile()) {
      // This ninja version is too old to support the Intel depfile format.
      // Fall back to msvc deptype.
      return true;
    }
    if ((this->Makefile->GetHomeDirectory().find(' ') != std::string::npos) ||
        (this->Makefile->GetHomeOutputDirectory().find(' ') !=
         std::string::npos)) {
      // The Intel compiler does not properly escape spaces in a depfile.
      // Fall back to msvc deptype.
      return true;
    }
  }
  return false;
}

// TODO: Refactor with
// void cmMakefileTargetGenerator::WriteTargetLanguageFlags().
std::string cmNinjaTargetGenerator::ComputeDefines(cmSourceFile const* source,
                                                   const std::string& language)
{
  std::set<std::string> defines;
  const std::string config = this->LocalGenerator->GetConfigName();
  cmGeneratorExpressionInterpreter genexInterpreter(
    this->LocalGenerator, config, this->GeneratorTarget, language);

  const std::string COMPILE_DEFINITIONS("COMPILE_DEFINITIONS");
  if (const char* compile_defs = source->GetProperty(COMPILE_DEFINITIONS)) {
    this->LocalGenerator->AppendDefines(
      defines, genexInterpreter.Evaluate(compile_defs, COMPILE_DEFINITIONS));
  }

  std::string defPropName = "COMPILE_DEFINITIONS_";
  defPropName += cmSystemTools::UpperCase(config);
  if (const char* config_compile_defs = source->GetProperty(defPropName)) {
    this->LocalGenerator->AppendDefines(
      defines,
      genexInterpreter.Evaluate(config_compile_defs, COMPILE_DEFINITIONS));
  }

  std::string definesString = this->GetDefines(language);
  this->LocalGenerator->JoinDefines(defines, definesString, language);

  return definesString;
}

std::string cmNinjaTargetGenerator::ComputeIncludes(
  cmSourceFile const* source, const std::string& language)
{
  std::vector<std::string> includes;
  const std::string config = this->LocalGenerator->GetConfigName();
  cmGeneratorExpressionInterpreter genexInterpreter(
    this->LocalGenerator, config, this->GeneratorTarget, language);

  const std::string INCLUDE_DIRECTORIES("INCLUDE_DIRECTORIES");
  if (const char* cincludes = source->GetProperty(INCLUDE_DIRECTORIES)) {
    this->LocalGenerator->AppendIncludeDirectories(
      includes, genexInterpreter.Evaluate(cincludes, INCLUDE_DIRECTORIES),
      *source);
  }

  std::string includesString = this->LocalGenerator->GetIncludeFlags(
    includes, this->GeneratorTarget, language, true, false, config);
  this->LocalGenerator->AppendFlags(includesString,
                                    this->GetIncludes(language));

  return includesString;
}

cmNinjaDeps cmNinjaTargetGenerator::ComputeLinkDeps(
  const std::string& linkLanguage) const
{
  // Static libraries never depend on other targets for linking.
  if (this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY ||
      this->GeneratorTarget->GetType() == cmStateEnums::OBJECT_LIBRARY) {
    return cmNinjaDeps();
  }

  cmComputeLinkInformation* cli =
    this->GeneratorTarget->GetLinkInformation(this->GetConfigName());
  if (!cli) {
    return cmNinjaDeps();
  }

  const std::vector<std::string>& deps = cli->GetDepends();
  cmNinjaDeps result(deps.size());
  std::transform(deps.begin(), deps.end(), result.begin(), MapToNinjaPath());

  // Add a dependency on the link definitions file, if any.
  if (cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
        this->GeneratorTarget->GetModuleDefinitionInfo(
          this->GetConfigName())) {
    for (cmSourceFile const* src : mdi->Sources) {
      result.push_back(this->ConvertToNinjaPath(src->GetFullPath()));
    }
  }

  // Add a dependency on user-specified manifest files, if any.
  std::vector<cmSourceFile const*> manifest_srcs;
  this->GeneratorTarget->GetManifests(manifest_srcs, this->ConfigName);
  for (cmSourceFile const* manifest_src : manifest_srcs) {
    result.push_back(this->ConvertToNinjaPath(manifest_src->GetFullPath()));
  }

  // Add user-specified dependencies.
  std::vector<std::string> linkDeps;
  this->GeneratorTarget->GetLinkDepends(linkDeps, this->ConfigName,
                                        linkLanguage);
  std::transform(linkDeps.begin(), linkDeps.end(), std::back_inserter(result),
                 MapToNinjaPath());

  return result;
}

std::string cmNinjaTargetGenerator::GetSourceFilePath(
  cmSourceFile const* source) const
{
  return ConvertToNinjaPath(source->GetFullPath());
}

std::string cmNinjaTargetGenerator::GetObjectFilePath(
  cmSourceFile const* source) const
{
  std::string path = this->LocalGenerator->GetHomeRelativeOutputPath();
  if (!path.empty()) {
    path += "/";
  }
  std::string const& objectName = this->GeneratorTarget->GetObjectName(source);
  path += this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  path += "/";
  path += objectName;
  return path;
}

std::string cmNinjaTargetGenerator::GetPreprocessedFilePath(
  cmSourceFile const* source) const
{
  // Choose an extension to compile already-preprocessed source.
  std::string ppExt = source->GetExtension();
  if (cmHasLiteralPrefix(ppExt, "F")) {
    // Some Fortran compilers automatically enable preprocessing for
    // upper-case extensions.  Since the source is already preprocessed,
    // use a lower-case extension.
    ppExt = cmSystemTools::LowerCase(ppExt);
  }
  if (ppExt == "fpp") {
    // Some Fortran compilers automatically enable preprocessing for
    // the ".fpp" extension.  Since the source is already preprocessed,
    // use the ".f" extension.
    ppExt = "f";
  }

  // Take the object file name and replace the extension.
  std::string const& objName = this->GeneratorTarget->GetObjectName(source);
  std::string const& objExt =
    this->GetGlobalGenerator()->GetLanguageOutputExtension(*source);
  assert(objName.size() >= objExt.size());
  std::string const ppName =
    objName.substr(0, objName.size() - objExt.size()) + "-pp." + ppExt;

  std::string path = this->LocalGenerator->GetHomeRelativeOutputPath();
  if (!path.empty()) {
    path += "/";
  }
  path += this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  path += "/";
  path += ppName;
  return path;
}

std::string cmNinjaTargetGenerator::GetDyndepFilePath(
  std::string const& lang) const
{
  std::string path = this->LocalGenerator->GetHomeRelativeOutputPath();
  if (!path.empty()) {
    path += "/";
  }
  path += this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  path += "/";
  path += lang;
  path += ".dd";
  return path;
}

std::string cmNinjaTargetGenerator::GetTargetDependInfoPath(
  std::string const& lang) const
{
  std::string path = this->Makefile->GetCurrentBinaryDirectory();
  path += "/";
  path += this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  path += "/" + lang + "DependInfo.json";
  return path;
}

std::string cmNinjaTargetGenerator::GetTargetOutputDir() const
{
  std::string dir = this->GeneratorTarget->GetDirectory(this->GetConfigName());
  return ConvertToNinjaPath(dir);
}

std::string cmNinjaTargetGenerator::GetTargetFilePath(
  const std::string& name) const
{
  std::string path = this->GetTargetOutputDir();
  if (path.empty() || path == ".") {
    return name;
  }
  path += "/";
  path += name;
  return path;
}

std::string cmNinjaTargetGenerator::GetTargetName() const
{
  return this->GeneratorTarget->GetName();
}

bool cmNinjaTargetGenerator::SetMsvcTargetPdbVariable(cmNinjaVars& vars) const
{
  cmMakefile* mf = this->GetMakefile();
  if (mf->GetDefinition("MSVC_C_ARCHITECTURE_ID") ||
      mf->GetDefinition("MSVC_CXX_ARCHITECTURE_ID") ||
      mf->GetDefinition("MSVC_CUDA_ARCHITECTURE_ID")) {
    std::string pdbPath;
    std::string compilePdbPath = this->ComputeTargetCompilePDB();
    if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE ||
        this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY ||
        this->GeneratorTarget->GetType() == cmStateEnums::SHARED_LIBRARY ||
        this->GeneratorTarget->GetType() == cmStateEnums::MODULE_LIBRARY) {
      pdbPath = this->GeneratorTarget->GetPDBDirectory(this->GetConfigName());
      pdbPath += "/";
      pdbPath += this->GeneratorTarget->GetPDBName(this->GetConfigName());
    }

    vars["TARGET_PDB"] = this->GetLocalGenerator()->ConvertToOutputFormat(
      ConvertToNinjaPath(pdbPath), cmOutputConverter::SHELL);
    vars["TARGET_COMPILE_PDB"] =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        ConvertToNinjaPath(compilePdbPath), cmOutputConverter::SHELL);

    EnsureParentDirectoryExists(pdbPath);
    EnsureParentDirectoryExists(compilePdbPath);
    return true;
  }
  return false;
}

void cmNinjaTargetGenerator::WriteLanguageRules(const std::string& language)
{
#ifdef NINJA_GEN_VERBOSE_FILES
  this->GetRulesFileStream() << "# Rules for language " << language << "\n\n";
#endif
  this->WriteCompileRule(language);
}

void cmNinjaTargetGenerator::WriteCompileRule(const std::string& lang)
{
  cmRulePlaceholderExpander::RuleVariables vars;
  vars.CMTargetName = this->GetGeneratorTarget()->GetName().c_str();
  vars.CMTargetType =
    cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType());
  vars.Language = lang.c_str();
  vars.Source = "$in";
  vars.Object = "$out";
  vars.Defines = "$DEFINES";
  vars.Includes = "$INCLUDES";
  vars.TargetPDB = "$TARGET_PDB";
  vars.TargetCompilePDB = "$TARGET_COMPILE_PDB";
  vars.ObjectDir = "$OBJECT_DIR";
  vars.ObjectFileDir = "$OBJECT_FILE_DIR";

  // For some cases we do an explicit preprocessor invocation.
  bool const explicitPP = this->NeedExplicitPreprocessing(lang);
  bool const needDyndep = this->NeedDyndep(lang);

  cmMakefile* mf = this->GetMakefile();

  std::string flags = "$FLAGS";

  std::string responseFlag;
  bool const lang_supports_response = lang != "RC";
  if (lang_supports_response && this->ForceResponseFile()) {
    std::string const responseFlagVar =
      "CMAKE_" + lang + "_RESPONSE_FILE_FLAG";
    responseFlag = this->Makefile->GetSafeDefinition(responseFlagVar);
    if (responseFlag.empty() && lang != "CUDA") {
      responseFlag = "@";
    }
  }

  std::unique_ptr<cmRulePlaceholderExpander> rulePlaceholderExpander(
    this->GetLocalGenerator()->CreateRulePlaceholderExpander());

  std::string const tdi = this->GetLocalGenerator()->ConvertToOutputFormat(
    ConvertToNinjaPath(this->GetTargetDependInfoPath(lang)),
    cmLocalGenerator::SHELL);

  std::string launcher;
  const char* val = this->GetLocalGenerator()->GetRuleLauncher(
    this->GetGeneratorTarget(), "RULE_LAUNCH_COMPILE");
  if (val && *val) {
    launcher = val;
    launcher += " ";
  }

  std::string const cmakeCmd =
    this->GetLocalGenerator()->ConvertToOutputFormat(
      cmSystemTools::GetCMakeCommand(), cmLocalGenerator::SHELL);

  if (explicitPP) {
    cmNinjaRule rule(this->LanguagePreprocessRule(lang));
    // Explicit preprocessing always uses a depfile.
    rule.DepType = ""; // no deps= for multiple outputs
    rule.DepFile = "$DEP_FILE";

    cmRulePlaceholderExpander::RuleVariables ppVars;
    ppVars.CMTargetName = vars.CMTargetName;
    ppVars.CMTargetType = vars.CMTargetType;
    ppVars.Language = vars.Language;
    ppVars.Object = "$out"; // for RULE_LAUNCH_COMPILE
    ppVars.PreprocessedSource = "$out";
    ppVars.DependencyFile = rule.DepFile.c_str();

    // Preprocessing uses the original source,
    // compilation uses preprocessed output.
    ppVars.Source = vars.Source;
    vars.Source = "$in";

    // Preprocessing and compilation use the same flags.
    std::string ppFlags = flags;

    // Move preprocessor definitions to the preprocessor rule.
    ppVars.Defines = vars.Defines;
    vars.Defines = "";

    // Copy include directories to the preprocessor rule.  The Fortran
    // compilation rule still needs them for the INCLUDE directive.
    ppVars.Includes = vars.Includes;

    // If using a response file, move defines, includes, and flags into it.
    if (!responseFlag.empty()) {
      rule.RspFile = "$RSP_FILE";
      rule.RspContent = " ";
      rule.RspContent += ppVars.Defines;
      rule.RspContent += " ";
      rule.RspContent += ppVars.Includes;
      rule.RspContent += " ";
      rule.RspContent += ppFlags;
      ppFlags = responseFlag + rule.RspFile;
      ppVars.Defines = "";
      ppVars.Includes = "";
    }

    ppVars.Flags = ppFlags.c_str();

    // Rule for preprocessing source file.
    std::vector<std::string> ppCmds;
    {
      // Lookup the explicit preprocessing rule.
      std::string ppVar = "CMAKE_" + lang;
      ppVar += "_PREPROCESS_SOURCE";
      cmSystemTools::ExpandListArgument(
        this->GetMakefile()->GetRequiredDefinition(ppVar), ppCmds);
    }

    for (std::string& i : ppCmds) {
      i = launcher + i;
      rulePlaceholderExpander->ExpandRuleVariables(this->GetLocalGenerator(),
                                                   i, ppVars);
    }

    // Run CMake dependency scanner on preprocessed output.
    {
      std::string ccmd = cmakeCmd;
      ccmd += " -E cmake_ninja_depends --tdi=";
      ccmd += tdi;
      ccmd += " --lang=";
      ccmd += lang;
      ccmd += " --pp=$out --dep=$DEP_FILE";
      if (needDyndep) {
        ccmd += " --obj=$OBJ_FILE --ddi=$DYNDEP_INTERMEDIATE_FILE";
      }
      ppCmds.emplace_back(std::move(ccmd));
    }
    rule.Command = this->GetLocalGenerator()->BuildCommandLine(ppCmds);

    // Write the rule for preprocessing file of the given language.
    rule.Comment = "Rule for preprocessing ";
    rule.Comment += lang;
    rule.Comment += " files.";
    rule.Description = "Building ";
    rule.Description += lang;
    rule.Description += " preprocessed $out";
    this->GetGlobalGenerator()->AddRule(rule);
  }

  if (needDyndep) {
    // Write the rule for ninja dyndep file generation.
    cmNinjaRule rule(this->LanguageDyndepRule(lang));
    // Command line length is almost always limited -> use response file for
    // dyndep rules
    rule.RspFile = "$out.rsp";
    rule.RspContent = "$in";

    // Run CMake dependency scanner on the source file (using the preprocessed
    // source if that was performed).
    {
      std::vector<std::string> ddCmds;
      {
        std::string ccmd = cmakeCmd;
        ccmd += " -E cmake_ninja_dyndep --tdi=";
        ccmd += tdi;
        ccmd += " --lang=";
        ccmd += lang;
        ccmd += " --dd=$out ";
        ccmd += "@";
        ccmd += rule.RspFile;
        ddCmds.emplace_back(std::move(ccmd));
      }
      rule.Command = this->GetLocalGenerator()->BuildCommandLine(ddCmds);
    }
    rule.Comment = "Rule to generate ninja dyndep files for ";
    rule.Comment += lang;
    rule.Comment += ".";
    rule.Description = "Generating ";
    rule.Description += lang;
    rule.Description += " dyndep file $out";
    this->GetGlobalGenerator()->AddRule(rule);
  }

  cmNinjaRule rule(this->LanguageCompilerRule(lang));
  // If using a response file, move defines, includes, and flags into it.
  if (!responseFlag.empty()) {
    rule.RspFile = "$RSP_FILE";
    rule.RspContent = " ";
    rule.RspContent += vars.Defines;
    rule.RspContent += " ";
    rule.RspContent += vars.Includes;
    rule.RspContent += " ";
    rule.RspContent += flags;
    flags = responseFlag + rule.RspFile;
    vars.Defines = "";
    vars.Includes = "";
  }

  // Tell ninja dependency format so all deps can be loaded into a database
  std::string cldeps;
  if (explicitPP) {
    // The explicit preprocessing step will handle dependency scanning.
  } else if (this->NeedDepTypeMSVC(lang)) {
    rule.DepType = "msvc";
    rule.DepFile.clear();
    flags += " /showIncludes";
  } else if (mf->IsOn("CMAKE_NINJA_CMCLDEPS_" + lang)) {
    // For the MS resource compiler we need cmcldeps, but skip dependencies
    // for source-file try_compile cases because they are always fresh.
    if (!mf->GetIsSourceFileTryCompile()) {
      rule.DepType = "gcc";
      rule.DepFile = "$DEP_FILE";
      const std::string cl = mf->GetDefinition("CMAKE_C_COMPILER")
        ? mf->GetSafeDefinition("CMAKE_C_COMPILER")
        : mf->GetSafeDefinition("CMAKE_CXX_COMPILER");
      cldeps = "\"";
      cldeps += cmSystemTools::GetCMClDepsCommand();
      cldeps += "\" " + lang + " " + vars.Source + " $DEP_FILE $out \"";
      cldeps += mf->GetSafeDefinition("CMAKE_CL_SHOWINCLUDES_PREFIX");
      cldeps += "\" \"" + cl + "\" ";
    }
  } else {
    rule.DepType = "gcc";
    rule.DepFile = "$DEP_FILE";
    const std::string flagsName = "CMAKE_DEPFILE_FLAGS_" + lang;
    std::string depfileFlags = mf->GetSafeDefinition(flagsName);
    if (!depfileFlags.empty()) {
      cmSystemTools::ReplaceString(depfileFlags, "<DEPFILE>", "$DEP_FILE");
      cmSystemTools::ReplaceString(depfileFlags, "<OBJECT>", "$out");
      cmSystemTools::ReplaceString(depfileFlags, "<CMAKE_C_COMPILER>",
                                   mf->GetDefinition("CMAKE_C_COMPILER"));
      flags += " " + depfileFlags;
    }
  }

  vars.Flags = flags.c_str();
  vars.DependencyFile = rule.DepFile.c_str();

  // Rule for compiling object file.
  std::vector<std::string> compileCmds;
  if (lang == "CUDA") {
    std::string cmdVar;
    if (this->GeneratorTarget->GetPropertyAsBool(
          "CUDA_SEPARABLE_COMPILATION")) {
      cmdVar = "CMAKE_CUDA_COMPILE_SEPARABLE_COMPILATION";
    } else if (this->GeneratorTarget->GetPropertyAsBool(
                 "CUDA_PTX_COMPILATION")) {
      cmdVar = "CMAKE_CUDA_COMPILE_PTX_COMPILATION";
    } else {
      cmdVar = "CMAKE_CUDA_COMPILE_WHOLE_COMPILATION";
    }
    const std::string& compileCmd = mf->GetRequiredDefinition(cmdVar);
    cmSystemTools::ExpandListArgument(compileCmd, compileCmds);
  } else {
    const std::string cmdVar = "CMAKE_" + lang + "_COMPILE_OBJECT";
    const std::string& compileCmd = mf->GetRequiredDefinition(cmdVar);
    cmSystemTools::ExpandListArgument(compileCmd, compileCmds);
  }

  // See if we need to use a compiler launcher like ccache or distcc
  std::string compilerLauncher;
  if (!compileCmds.empty() &&
      (lang == "C" || lang == "CXX" || lang == "Fortran" || lang == "CUDA")) {
    std::string const clauncher_prop = lang + "_COMPILER_LAUNCHER";
    const char* clauncher = this->GeneratorTarget->GetProperty(clauncher_prop);
    if (clauncher && *clauncher) {
      compilerLauncher = clauncher;
    }
  }

  // Maybe insert an include-what-you-use runner.
  if (!compileCmds.empty() && (lang == "C" || lang == "CXX")) {
    std::string const iwyu_prop = lang + "_INCLUDE_WHAT_YOU_USE";
    const char* iwyu = this->GeneratorTarget->GetProperty(iwyu_prop);
    std::string const tidy_prop = lang + "_CLANG_TIDY";
    const char* tidy = this->GeneratorTarget->GetProperty(tidy_prop);
    std::string const cpplint_prop = lang + "_CPPLINT";
    const char* cpplint = this->GeneratorTarget->GetProperty(cpplint_prop);
    std::string const cppcheck_prop = lang + "_CPPCHECK";
    const char* cppcheck = this->GeneratorTarget->GetProperty(cppcheck_prop);
    if ((iwyu && *iwyu) || (tidy && *tidy) || (cpplint && *cpplint) ||
        (cppcheck && *cppcheck)) {
      std::string run_iwyu = cmakeCmd;
      run_iwyu += " -E __run_co_compile";
      if (!compilerLauncher.empty()) {
        // In __run_co_compile case the launcher command is supplied
        // via --launcher=<maybe-list> and consumed
        run_iwyu += " --launcher=";
        run_iwyu += this->LocalGenerator->EscapeForShell(compilerLauncher);
        compilerLauncher.clear();
      }
      if (iwyu && *iwyu) {
        run_iwyu += " --iwyu=";
        run_iwyu += this->GetLocalGenerator()->EscapeForShell(iwyu);
      }
      if (tidy && *tidy) {
        run_iwyu += " --tidy=";
        run_iwyu += this->GetLocalGenerator()->EscapeForShell(tidy);
      }
      if (cpplint && *cpplint) {
        run_iwyu += " --cpplint=";
        run_iwyu += this->GetLocalGenerator()->EscapeForShell(cpplint);
      }
      if (cppcheck && *cppcheck) {
        run_iwyu += " --cppcheck=";
        run_iwyu += this->GetLocalGenerator()->EscapeForShell(cppcheck);
      }
      if ((tidy && *tidy) || (cpplint && *cpplint) ||
          (cppcheck && *cppcheck)) {
        run_iwyu += " --source=$in";
      }
      run_iwyu += " -- ";
      compileCmds.front().insert(0, run_iwyu);
    }
  }

  // If compiler launcher was specified and not consumed above, it
  // goes to the beginning of the command line.
  if (!compileCmds.empty() && !compilerLauncher.empty()) {
    std::vector<std::string> args;
    cmSystemTools::ExpandListArgument(compilerLauncher, args, true);
    if (!args.empty()) {
      args[0] = this->LocalGenerator->ConvertToOutputFormat(
        args[0], cmOutputConverter::SHELL);
      for (std::string& i : cmMakeRange(args.begin() + 1, args.end())) {
        i = this->LocalGenerator->EscapeForShell(i);
      }
    }
    compileCmds.front().insert(0, cmJoin(args, " ") + " ");
  }

  if (!compileCmds.empty()) {
    compileCmds.front().insert(0, cldeps);
  }

  for (std::string& i : compileCmds) {
    i = launcher + i;
    rulePlaceholderExpander->ExpandRuleVariables(this->GetLocalGenerator(), i,
                                                 vars);
  }

  rule.Command = this->GetLocalGenerator()->BuildCommandLine(compileCmds);

  // Write the rule for compiling file of the given language.
  rule.Comment = "Rule for compiling ";
  rule.Comment += lang;
  rule.Comment += " files.";
  rule.Description = "Building ";
  rule.Description += lang;
  rule.Description += " object $out";
  this->GetGlobalGenerator()->AddRule(rule);
}

void cmNinjaTargetGenerator::WriteObjectBuildStatements()
{
  // Write comments.
  cmGlobalNinjaGenerator::WriteDivider(this->GetBuildFileStream());
  this->GetBuildFileStream()
    << "# Object build statements for "
    << cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType())
    << " target " << this->GetTargetName() << "\n\n";

  const std::string& config =
    this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
  {
    std::vector<cmSourceFile const*> customCommands;
    this->GeneratorTarget->GetCustomCommands(customCommands, config);
    for (cmSourceFile const* sf : customCommands) {
      cmCustomCommand const* cc = sf->GetCustomCommand();
      this->GetLocalGenerator()->AddCustomCommandTarget(
        cc, this->GetGeneratorTarget());
      // Record the custom commands for this target. The container is used
      // in WriteObjectBuildStatement when called in a loop below.
      this->CustomCommands.push_back(cc);
    }
  }
  {
    std::vector<cmSourceFile const*> headerSources;
    this->GeneratorTarget->GetHeaderSources(headerSources, config);
    this->OSXBundleGenerator->GenerateMacOSXContentStatements(
      headerSources, this->MacOSXContentGenerator.get());
  }
  {
    std::vector<cmSourceFile const*> extraSources;
    this->GeneratorTarget->GetExtraSources(extraSources, config);
    this->OSXBundleGenerator->GenerateMacOSXContentStatements(
      extraSources, this->MacOSXContentGenerator.get());
  }
  {
    std::vector<cmSourceFile const*> externalObjects;
    this->GeneratorTarget->GetExternalObjects(externalObjects, config);
    for (cmSourceFile const* sf : externalObjects) {
      this->Objects.push_back(this->GetSourceFilePath(sf));
    }
  }

  {
    cmNinjaBuild build("phony");
    build.Comment = "Order-only phony target for " + this->GetTargetName();
    build.Outputs.push_back(this->OrderDependsTargetForTarget());

    cmNinjaDeps& orderOnlyDeps = build.OrderOnlyDeps;
    this->GetLocalGenerator()->AppendTargetDepends(
      this->GeneratorTarget, orderOnlyDeps, DependOnTargetOrdering);

    // Add order-only dependencies on other files associated with the target.
    cmAppend(orderOnlyDeps, this->ExtraFiles);

    // Add order-only dependencies on custom command outputs.
    for (cmCustomCommand const* cc : this->CustomCommands) {
      cmCustomCommandGenerator ccg(*cc, this->GetConfigName(),
                                   this->GetLocalGenerator());
      const std::vector<std::string>& ccoutputs = ccg.GetOutputs();
      const std::vector<std::string>& ccbyproducts = ccg.GetByproducts();
      std::transform(ccoutputs.begin(), ccoutputs.end(),
                     std::back_inserter(orderOnlyDeps), MapToNinjaPath());
      std::transform(ccbyproducts.begin(), ccbyproducts.end(),
                     std::back_inserter(orderOnlyDeps), MapToNinjaPath());
    }

    std::sort(orderOnlyDeps.begin(), orderOnlyDeps.end());
    orderOnlyDeps.erase(
      std::unique(orderOnlyDeps.begin(), orderOnlyDeps.end()),
      orderOnlyDeps.end());

    // The phony target must depend on at least one input or ninja will explain
    // that "output ... of phony edge with no inputs doesn't exist" and
    // consider the phony output "dirty".
    if (orderOnlyDeps.empty()) {
      // Any path that always exists will work here.  It would be nice to
      // use just "." but that is not supported by Ninja < 1.7.
      std::string tgtDir;
      tgtDir += this->LocalGenerator->GetCurrentBinaryDirectory();
      tgtDir += "/";
      tgtDir +=
        this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
      orderOnlyDeps.push_back(this->ConvertToNinjaPath(tgtDir));
    }

    this->GetGlobalGenerator()->WriteBuild(this->GetBuildFileStream(), build);
  }

  {
    std::vector<cmSourceFile const*> objectSources;
    this->GeneratorTarget->GetObjectSources(objectSources, config);
    for (cmSourceFile const* sf : objectSources) {
      this->WriteObjectBuildStatement(sf);
    }
  }

  for (auto const& langDDIFiles : this->DDIFiles) {
    std::string const& language = langDDIFiles.first;
    cmNinjaDeps const& ddiFiles = langDDIFiles.second;

    cmNinjaBuild build(this->LanguageDyndepRule(language));
    build.Outputs.push_back(this->GetDyndepFilePath(language));
    build.ExplicitDeps = ddiFiles;

    this->WriteTargetDependInfo(language);

    // Make sure dyndep files for all our dependencies have already
    // been generated so that the '<LANG>Modules.json' files they
    // produced as side-effects are available for us to read.
    // Ideally we should depend on the '<LANG>Modules.json' files
    // from our dependencies directly, but we don't know which of
    // our dependencies produces them.  Fixing this will require
    // refactoring the Ninja generator to generate targets in
    // dependency order so that we can collect the needed information.
    this->GetLocalGenerator()->AppendTargetDepends(
      this->GeneratorTarget, build.OrderOnlyDeps, DependOnTargetArtifact);

    this->GetGlobalGenerator()->WriteBuild(this->GetBuildFileStream(), build);
  }

  this->GetBuildFileStream() << "\n";

  if (!this->SwiftOutputMap.empty()) {
    std::string const mapFilePath = this->ConvertToNinjaPath(
      this->GeneratorTarget->GetSupportDirectory() + "/output-file-map.json");
    std::string const targetSwiftDepsPath = [this]() -> std::string {
      cmGeneratorTarget const* target = this->GeneratorTarget;
      if (const char* name = target->GetProperty("Swift_DEPENDENCIES_FILE")) {
        return name;
      }
      return this->ConvertToNinjaPath(target->GetSupportDirectory() + "/" +
                                      target->GetName() + ".swiftdeps");
    }();

    // build the global target dependencies
    // https://github.com/apple/swift/blob/master/docs/Driver.md#output-file-maps
    Json::Value deps(Json::objectValue);
    deps["swift-dependencies"] = targetSwiftDepsPath;
    this->SwiftOutputMap[""] = deps;

    cmGeneratedFileStream output(mapFilePath);
    output << this->SwiftOutputMap;
  }
}

void cmNinjaTargetGenerator::WriteObjectBuildStatement(
  cmSourceFile const* source)
{
  std::string const language = source->GetLanguage();
  std::string const sourceFileName =
    language == "RC" ? source->GetFullPath() : this->GetSourceFilePath(source);
  std::string const objectDir =
    this->ConvertToNinjaPath(this->GeneratorTarget->GetSupportDirectory());
  std::string const objectFileName =
    this->ConvertToNinjaPath(this->GetObjectFilePath(source));
  std::string const objectFileDir =
    cmSystemTools::GetFilenamePath(objectFileName);

  std::string cmakeVarLang = "CMAKE_";
  cmakeVarLang += language;

  // build response file name
  std::string cmakeLinkVar = cmakeVarLang + "_RESPONSE_FILE_FLAG";

  const char* flag = GetMakefile()->GetDefinition(cmakeLinkVar);

  bool const lang_supports_response =
    !(language == "RC" || (language == "CUDA" && !flag));
  int const commandLineLengthLimit =
    ((lang_supports_response && this->ForceResponseFile())) ? -1 : 0;

  cmNinjaBuild objBuild(this->LanguageCompilerRule(language));
  cmNinjaVars& vars = objBuild.Variables;
  vars["FLAGS"] = this->ComputeFlagsForObject(source, language);
  vars["DEFINES"] = this->ComputeDefines(source, language);
  vars["INCLUDES"] = this->ComputeIncludes(source, language);

  if (!this->NeedDepTypeMSVC(language)) {
    bool replaceExt(false);
    if (!language.empty()) {
      std::string repVar = "CMAKE_";
      repVar += language;
      repVar += "_DEPFILE_EXTENSION_REPLACE";
      replaceExt = this->Makefile->IsOn(repVar);
    }
    if (!replaceExt) {
      // use original code
      vars["DEP_FILE"] = this->GetLocalGenerator()->ConvertToOutputFormat(
        objectFileName + ".d", cmOutputConverter::SHELL);
    } else {
      // Replace the original source file extension with the
      // depend file extension.
      std::string dependFileName =
        cmSystemTools::GetFilenameWithoutLastExtension(objectFileName) + ".d";
      vars["DEP_FILE"] = this->GetLocalGenerator()->ConvertToOutputFormat(
        objectFileDir + "/" + dependFileName, cmOutputConverter::SHELL);
    }
  }

  this->ExportObjectCompileCommand(
    language, sourceFileName, objectDir, objectFileName, objectFileDir,
    vars["FLAGS"], vars["DEFINES"], vars["INCLUDES"]);

  objBuild.Outputs.push_back(objectFileName);
  // Add this object to the list of object files.
  this->Objects.push_back(objectFileName);

  objBuild.ExplicitDeps.push_back(sourceFileName);

  if (const char* objectDeps = source->GetProperty("OBJECT_DEPENDS")) {
    std::vector<std::string> depList =
      cmSystemTools::ExpandedListArgument(objectDeps);
    for (std::string& odi : depList) {
      if (cmSystemTools::FileIsFullPath(odi)) {
        odi = cmSystemTools::CollapseFullPath(odi);
      }
    }
    std::transform(depList.begin(), depList.end(),
                   std::back_inserter(objBuild.ImplicitDeps),
                   MapToNinjaPath());
  }

  objBuild.OrderOnlyDeps.push_back(this->OrderDependsTargetForTarget());

  // If the source file is GENERATED and does not have a custom command
  // (either attached to this source file or another one), assume that one of
  // the target dependencies, OBJECT_DEPENDS or header file custom commands
  // will rebuild the file.
  if (source->GetIsGenerated() &&
      !source->GetPropertyAsBool("__CMAKE_GENERATED_BY_CMAKE") &&
      !source->GetCustomCommand() &&
      !this->GetGlobalGenerator()->HasCustomCommandOutput(sourceFileName)) {
    this->GetGlobalGenerator()->AddAssumedSourceDependencies(
      sourceFileName, objBuild.OrderOnlyDeps);
  }

  // For some cases we need to generate a ninja dyndep file.
  bool const needDyndep = this->NeedDyndep(language);

  // For some cases we do an explicit preprocessor invocation.
  bool const explicitPP = this->NeedExplicitPreprocessing(language);
  if (explicitPP) {
    cmNinjaBuild ppBuild(this->LanguagePreprocessRule(language));

    std::string const ppFileName =
      this->ConvertToNinjaPath(this->GetPreprocessedFilePath(source));
    ppBuild.Outputs.push_back(ppFileName);

    ppBuild.RspFile = ppFileName + ".rsp";

    bool const compilePP = this->UsePreprocessedSource(language);
    if (compilePP) {
      // Move compilation dependencies to the preprocessing build statement.
      std::swap(ppBuild.ExplicitDeps, objBuild.ExplicitDeps);
      std::swap(ppBuild.ImplicitDeps, objBuild.ImplicitDeps);
      std::swap(ppBuild.OrderOnlyDeps, objBuild.OrderOnlyDeps);
      std::swap(ppBuild.Variables["IN_ABS"], vars["IN_ABS"]);

      // The actual compilation will now use the preprocessed source.
      objBuild.ExplicitDeps.push_back(ppFileName);
    } else {
      // Copy compilation dependencies to the preprocessing build statement.
      ppBuild.ExplicitDeps = objBuild.ExplicitDeps;
      ppBuild.ImplicitDeps = objBuild.ImplicitDeps;
      ppBuild.OrderOnlyDeps = objBuild.OrderOnlyDeps;
      ppBuild.Variables["IN_ABS"] = vars["IN_ABS"];
    }

    // Preprocessing and compilation generally use the same flags.
    ppBuild.Variables["FLAGS"] = vars["FLAGS"];

    if (compilePP) {
      // In case compilation requires flags that are incompatible with
      // preprocessing, include them here.
      std::string const& postFlag = this->Makefile->GetSafeDefinition(
        "CMAKE_" + language + "_POSTPROCESS_FLAG");
      this->LocalGenerator->AppendFlags(vars["FLAGS"], postFlag);
    }

    if (compilePP) {
      // Move preprocessor definitions to the preprocessor build statement.
      std::swap(ppBuild.Variables["DEFINES"], vars["DEFINES"]);
    } else {
      // Copy preprocessor definitions to the preprocessor build statement.
      ppBuild.Variables["DEFINES"] = vars["DEFINES"];
    }

    // Copy include directories to the preprocessor build statement.  The
    // Fortran compilation build statement still needs them for the INCLUDE
    // directive.
    ppBuild.Variables["INCLUDES"] = vars["INCLUDES"];

    if (compilePP) {
      // Prepend source file's original directory as an include directory
      // so e.g. Fortran INCLUDE statements can look for files in it.
      std::vector<std::string> sourceDirectory;
      sourceDirectory.push_back(
        cmSystemTools::GetParentDirectory(source->GetFullPath()));

      std::string sourceDirectoryFlag = this->LocalGenerator->GetIncludeFlags(
        sourceDirectory, this->GeneratorTarget, language, false, false,
        this->GetConfigName());

      vars["INCLUDES"] = sourceDirectoryFlag + " " + vars["INCLUDES"];
    }

    // Explicit preprocessing always uses a depfile.
    ppBuild.Variables["DEP_FILE"] =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        objectFileName + ".pp.d", cmOutputConverter::SHELL);
    if (compilePP) {
      // The actual compilation does not need a depfile because it
      // depends on the already-preprocessed source.
      vars.erase("DEP_FILE");
    }

    if (needDyndep) {
      // Tell dependency scanner the object file that will result from
      // compiling the source.
      ppBuild.Variables["OBJ_FILE"] = objectFileName;

      // Tell dependency scanner where to store dyndep intermediate results.
      std::string const ddiFile = objectFileName + ".ddi";
      ppBuild.Variables["DYNDEP_INTERMEDIATE_FILE"] = ddiFile;
      ppBuild.ImplicitOuts.push_back(ddiFile);
      this->DDIFiles[language].push_back(ddiFile);
    }

    this->addPoolNinjaVariable("JOB_POOL_COMPILE", this->GetGeneratorTarget(),
                               ppBuild.Variables);

    this->GetGlobalGenerator()->WriteBuild(this->GetBuildFileStream(), ppBuild,
                                           commandLineLengthLimit);
  }
  if (needDyndep) {
    std::string const dyndep = this->GetDyndepFilePath(language);
    objBuild.OrderOnlyDeps.push_back(dyndep);
    vars["dyndep"] = dyndep;
  }

  EnsureParentDirectoryExists(objectFileName);

  vars["OBJECT_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    objectDir, cmOutputConverter::SHELL);
  vars["OBJECT_FILE_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    objectFileDir, cmOutputConverter::SHELL);

  this->addPoolNinjaVariable("JOB_POOL_COMPILE", this->GetGeneratorTarget(),
                             vars);

  this->SetMsvcTargetPdbVariable(vars);

  objBuild.RspFile = objectFileName + ".rsp";

  if (language == "Swift") {
    this->EmitSwiftDependencyInfo(source);
  } else {
    this->GetGlobalGenerator()->WriteBuild(this->GetBuildFileStream(),
                                           objBuild, commandLineLengthLimit);
  }

  if (const char* objectOutputs = source->GetProperty("OBJECT_OUTPUTS")) {
    cmNinjaBuild build("phony");
    build.Comment = "Additional output files.";
    build.Outputs = cmSystemTools::ExpandedListArgument(objectOutputs);
    std::transform(build.Outputs.begin(), build.Outputs.end(),
                   build.Outputs.begin(), MapToNinjaPath());
    build.ExplicitDeps = objBuild.Outputs;
    this->GetGlobalGenerator()->WriteBuild(this->GetBuildFileStream(), build);
  }
}

void cmNinjaTargetGenerator::WriteTargetDependInfo(std::string const& lang)
{
  Json::Value tdi(Json::objectValue);
  tdi["language"] = lang;
  tdi["compiler-id"] =
    this->Makefile->GetSafeDefinition("CMAKE_" + lang + "_COMPILER_ID");

  if (lang == "Fortran") {
    std::string mod_dir = this->GeneratorTarget->GetFortranModuleDirectory(
      this->Makefile->GetHomeOutputDirectory());
    if (mod_dir.empty()) {
      mod_dir = this->Makefile->GetCurrentBinaryDirectory();
    }
    tdi["module-dir"] = mod_dir;
    tdi["submodule-sep"] =
      this->Makefile->GetSafeDefinition("CMAKE_Fortran_SUBMODULE_SEP");
    tdi["submodule-ext"] =
      this->Makefile->GetSafeDefinition("CMAKE_Fortran_SUBMODULE_EXT");
  }

  tdi["dir-cur-bld"] = this->Makefile->GetCurrentBinaryDirectory();
  tdi["dir-cur-src"] = this->Makefile->GetCurrentSourceDirectory();
  tdi["dir-top-bld"] = this->Makefile->GetHomeOutputDirectory();
  tdi["dir-top-src"] = this->Makefile->GetHomeDirectory();

  Json::Value& tdi_include_dirs = tdi["include-dirs"] = Json::arrayValue;
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes, this->GeneratorTarget,
                                              lang, this->GetConfigName());
  for (std::string const& i : includes) {
    // Convert the include directories the same way we do for -I flags.
    // See upstream ninja issue 1251.
    tdi_include_dirs.append(this->ConvertToNinjaPath(i));
  }

  Json::Value& tdi_linked_target_dirs = tdi["linked-target-dirs"] =
    Json::arrayValue;
  for (std::string const& l : this->GetLinkedTargetDirectories()) {
    tdi_linked_target_dirs.append(l);
  }

  std::string const tdin = this->GetTargetDependInfoPath(lang);
  cmGeneratedFileStream tdif(tdin);
  tdif << tdi;
}

void cmNinjaTargetGenerator::EmitSwiftDependencyInfo(
  cmSourceFile const* source)
{
  std::string const sourceFilePath =
    this->ConvertToNinjaPath(this->GetSourceFilePath(source));
  std::string const objectFilePath =
    this->ConvertToNinjaPath(this->GetObjectFilePath(source));
  std::string const swiftDepsPath = [source, objectFilePath]() -> std::string {
    if (const char* name = source->GetProperty("Swift_DEPENDENCIES_FILE")) {
      return name;
    }
    return objectFilePath + ".swiftdeps";
  }();
  std::string const swiftDiaPath = [source, objectFilePath]() -> std::string {
    if (const char* name = source->GetProperty("Swift_DIAGNOSTICS_FILE")) {
      return name;
    }
    return objectFilePath + ".dia";
  }();
  std::string const makeDepsPath = [this, source]() -> std::string {
    cmLocalNinjaGenerator const* local = this->GetLocalGenerator();
    std::string const objectFileName =
      this->ConvertToNinjaPath(this->GetObjectFilePath(source));
    std::string const objectFileDir =
      cmSystemTools::GetFilenamePath(objectFileName);

    if (this->Makefile->IsOn("CMAKE_Swift_DEPFLE_EXTNSION_REPLACE")) {
      std::string dependFileName =
        cmSystemTools::GetFilenameWithoutLastExtension(objectFileName) + ".d";
      return local->ConvertToOutputFormat(objectFileDir + "/" + dependFileName,
                                          cmOutputConverter::SHELL);
    }
    return local->ConvertToOutputFormat(objectFileName + ".d",
                                        cmOutputConverter::SHELL);
  }();

  // build the source file mapping
  // https://github.com/apple/swift/blob/master/docs/Driver.md#output-file-maps
  Json::Value entry = Json::Value(Json::objectValue);
  entry["object"] = objectFilePath;
  entry["dependencies"] = makeDepsPath;
  entry["swift-dependencies"] = swiftDepsPath;
  entry["diagnostics"] = swiftDiaPath;
  SwiftOutputMap[sourceFilePath] = entry;
}

void cmNinjaTargetGenerator::ExportObjectCompileCommand(
  std::string const& language, std::string const& sourceFileName,
  std::string const& objectDir, std::string const& objectFileName,
  std::string const& objectFileDir, std::string const& flags,
  std::string const& defines, std::string const& includes)
{
  if (!this->Makefile->IsOn("CMAKE_EXPORT_COMPILE_COMMANDS")) {
    return;
  }

  cmRulePlaceholderExpander::RuleVariables compileObjectVars;
  compileObjectVars.Language = language.c_str();

  std::string escapedSourceFileName = sourceFileName;

  if (!cmSystemTools::FileIsFullPath(sourceFileName)) {
    escapedSourceFileName =
      cmSystemTools::CollapseFullPath(escapedSourceFileName,
                                      this->GetGlobalGenerator()
                                        ->GetCMakeInstance()
                                        ->GetHomeOutputDirectory());
  }

  escapedSourceFileName = this->LocalGenerator->ConvertToOutputFormat(
    escapedSourceFileName, cmOutputConverter::SHELL);

  compileObjectVars.Source = escapedSourceFileName.c_str();
  compileObjectVars.Object = objectFileName.c_str();
  compileObjectVars.ObjectDir = objectDir.c_str();
  compileObjectVars.ObjectFileDir = objectFileDir.c_str();
  compileObjectVars.Flags = flags.c_str();
  compileObjectVars.Defines = defines.c_str();
  compileObjectVars.Includes = includes.c_str();

  // Rule for compiling object file.
  std::vector<std::string> compileCmds;
  if (language == "CUDA") {
    std::string cmdVar;
    if (this->GeneratorTarget->GetPropertyAsBool(
          "CUDA_SEPARABLE_COMPILATION")) {
      cmdVar = "CMAKE_CUDA_COMPILE_SEPARABLE_COMPILATION";
    } else if (this->GeneratorTarget->GetPropertyAsBool(
                 "CUDA_PTX_COMPILATION")) {
      cmdVar = "CMAKE_CUDA_COMPILE_PTX_COMPILATION";
    } else {
      cmdVar = "CMAKE_CUDA_COMPILE_WHOLE_COMPILATION";
    }
    const std::string& compileCmd =
      this->GetMakefile()->GetRequiredDefinition(cmdVar);
    cmSystemTools::ExpandListArgument(compileCmd, compileCmds);
  } else {
    const std::string cmdVar = "CMAKE_" + language + "_COMPILE_OBJECT";
    const std::string& compileCmd =
      this->GetMakefile()->GetRequiredDefinition(cmdVar);
    cmSystemTools::ExpandListArgument(compileCmd, compileCmds);
  }

  std::unique_ptr<cmRulePlaceholderExpander> rulePlaceholderExpander(
    this->GetLocalGenerator()->CreateRulePlaceholderExpander());

  for (std::string& i : compileCmds) {
    // no launcher for CMAKE_EXPORT_COMPILE_COMMANDS
    rulePlaceholderExpander->ExpandRuleVariables(this->GetLocalGenerator(), i,
                                                 compileObjectVars);
  }

  std::string cmdLine =
    this->GetLocalGenerator()->BuildCommandLine(compileCmds);

  this->GetGlobalGenerator()->AddCXXCompileCommand(cmdLine, sourceFileName);
}

void cmNinjaTargetGenerator::AdditionalCleanFiles()
{
  if (const char* prop_value =
        this->GeneratorTarget->GetProperty("ADDITIONAL_CLEAN_FILES")) {
    cmLocalNinjaGenerator* lg = this->LocalGenerator;
    std::vector<std::string> cleanFiles;
    {
      cmGeneratorExpression ge;
      auto cge = ge.Parse(prop_value);
      cmSystemTools::ExpandListArgument(
        cge->Evaluate(lg,
                      this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE"),
                      false, this->GeneratorTarget, nullptr, nullptr),
        cleanFiles);
    }
    std::string const& binaryDir = lg->GetCurrentBinaryDirectory();
    cmGlobalNinjaGenerator* gg = lg->GetGlobalNinjaGenerator();
    for (std::string const& cleanFile : cleanFiles) {
      // Support relative paths
      gg->AddAdditionalCleanFile(
        cmSystemTools::CollapseFullPath(cleanFile, binaryDir));
    }
  }
}

void cmNinjaTargetGenerator::EnsureDirectoryExists(
  const std::string& path) const
{
  if (cmSystemTools::FileIsFullPath(path)) {
    cmSystemTools::MakeDirectory(path);
  } else {
    cmGlobalNinjaGenerator* gg = this->GetGlobalGenerator();
    std::string fullPath = gg->GetCMakeInstance()->GetHomeOutputDirectory();
    // Also ensures their is a trailing slash.
    gg->StripNinjaOutputPathPrefixAsSuffix(fullPath);
    fullPath += path;
    cmSystemTools::MakeDirectory(fullPath);
  }
}

void cmNinjaTargetGenerator::EnsureParentDirectoryExists(
  const std::string& path) const
{
  EnsureDirectoryExists(cmSystemTools::GetParentDirectory(path));
}

void cmNinjaTargetGenerator::MacOSXContentGeneratorType::operator()(
  cmSourceFile const& source, const char* pkgloc)
{
  // Skip OS X content when not building a Framework or Bundle.
  if (!this->Generator->GetGeneratorTarget()->IsBundleOnApple()) {
    return;
  }

  std::string macdir =
    this->Generator->OSXBundleGenerator->InitMacOSXContentDirectory(pkgloc);

  // Get the input file location.
  std::string input = source.GetFullPath();
  input = this->Generator->GetGlobalGenerator()->ConvertToNinjaPath(input);

  // Get the output file location.
  std::string output = macdir;
  output += "/";
  output += cmSystemTools::GetFilenameName(input);
  output = this->Generator->GetGlobalGenerator()->ConvertToNinjaPath(output);

  // Write a build statement to copy the content into the bundle.
  this->Generator->GetGlobalGenerator()->WriteMacOSXContentBuild(input,
                                                                 output);

  // Add as a dependency to the target so that it gets called.
  this->Generator->ExtraFiles.push_back(std::move(output));
}

void cmNinjaTargetGenerator::addPoolNinjaVariable(
  const std::string& pool_property, cmGeneratorTarget* target,
  cmNinjaVars& vars)
{
  const char* pool = target->GetProperty(pool_property);
  if (pool) {
    vars["pool"] = pool;
  }
}

bool cmNinjaTargetGenerator::ForceResponseFile()
{
  static std::string const forceRspFile = "CMAKE_NINJA_FORCE_RESPONSE_FILE";
  return (this->GetMakefile()->IsDefinitionSet(forceRspFile) ||
          cmSystemTools::HasEnv(forceRspFile));
}
