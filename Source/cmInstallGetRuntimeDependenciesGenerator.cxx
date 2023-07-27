/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallGetRuntimeDependenciesGenerator.h"

#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmGeneratorExpression.h"
#include "cmInstallRuntimeDependencySet.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmScriptGenerator.h"
#include "cmStringAlgorithms.h"

namespace {
template <typename T, typename F>
void WriteMultiArgument(std::ostream& os, const cm::string_view& keyword,
                        const std::vector<T>& list,
                        cmScriptGeneratorIndent indent, F transform)
{
  bool first = true;
  for (auto const& item : list) {
    cm::optional<std::string> result = transform(item);
    if (result) {
      if (first) {
        os << indent << "  " << keyword << "\n";
        first = false;
      }
      os << indent << "    " << *result << "\n";
    }
  }
}

void WriteFilesArgument(
  std::ostream& os, const cm::string_view& keyword,
  const std::vector<std::unique_ptr<cmInstallRuntimeDependencySet::Item>>&
    items,
  const std::string& config, cmScriptGeneratorIndent indent)
{
  WriteMultiArgument(
    os, keyword, items, indent,
    [config](const std::unique_ptr<cmInstallRuntimeDependencySet::Item>& i)
      -> std::string { return cmStrCat('"', i->GetItemPath(config), '"'); });
}

void WriteGenexEvaluatorArgument(std::ostream& os,
                                 const cm::string_view& keyword,
                                 const std::vector<std::string>& genexes,
                                 const std::string& config,
                                 cmLocalGenerator* lg,
                                 cmScriptGeneratorIndent indent)
{
  WriteMultiArgument(
    os, keyword, genexes, indent,
    [config, lg](const std::string& genex) -> cm::optional<std::string> {
      std::string result = cmGeneratorExpression::Evaluate(genex, lg, config);
      if (result.empty()) {
        return cm::nullopt;
      }
      return cmOutputConverter::EscapeForCMake(result);
    });
}
}

cmInstallGetRuntimeDependenciesGenerator::
  cmInstallGetRuntimeDependenciesGenerator(
    cmInstallRuntimeDependencySet* runtimeDependencySet,
    std::vector<std::string> directories,
    std::vector<std::string> preIncludeRegexes,
    std::vector<std::string> preExcludeRegexes,
    std::vector<std::string> postIncludeRegexes,
    std::vector<std::string> postExcludeRegexes,
    std::vector<std::string> postIncludeFiles,
    std::vector<std::string> postExcludeFiles, std::string libraryComponent,
    std::string frameworkComponent, bool noInstallRPath, const char* depsVar,
    const char* rpathPrefix, std::vector<std::string> const& configurations,
    MessageLevel message, bool exclude_from_all, cmListFileBacktrace backtrace)
  : cmInstallGenerator("", configurations, "", message, exclude_from_all,
                       false, std::move(backtrace))
  , RuntimeDependencySet(runtimeDependencySet)
  , Directories(std::move(directories))
  , PreIncludeRegexes(std::move(preIncludeRegexes))
  , PreExcludeRegexes(std::move(preExcludeRegexes))
  , PostIncludeRegexes(std::move(postIncludeRegexes))
  , PostExcludeRegexes(std::move(postExcludeRegexes))
  , PostIncludeFiles(std::move(postIncludeFiles))
  , PostExcludeFiles(std::move(postExcludeFiles))
  , LibraryComponent(std::move(libraryComponent))
  , FrameworkComponent(std::move(frameworkComponent))
  , NoInstallRPath(noInstallRPath)
  , DepsVar(depsVar)
  , RPathPrefix(rpathPrefix)
{
  this->ActionsPerConfig = true;
}

bool cmInstallGetRuntimeDependenciesGenerator::Compute(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;
  return true;
}

void cmInstallGetRuntimeDependenciesGenerator::GenerateScript(std::ostream& os)
{
  // Track indentation.
  Indent indent;

  // Begin this block of installation.
  os << indent << "if(";
  if (this->FrameworkComponent.empty() ||
      this->FrameworkComponent == this->LibraryComponent) {
    os << this->CreateComponentTest(this->LibraryComponent,
                                    this->ExcludeFromAll);
  } else {
    os << this->CreateComponentTest(this->LibraryComponent, true) << " OR "
       << this->CreateComponentTest(this->FrameworkComponent,
                                    this->ExcludeFromAll);
  }
  os << ")\n";

  // Generate the script possibly with per-configuration code.
  this->GenerateScriptConfigs(os, indent.Next());

  // End this block of installation.
  os << indent << "endif()\n\n";
}

void cmInstallGetRuntimeDependenciesGenerator::GenerateScriptForConfig(
  std::ostream& os, const std::string& config, Indent indent)
{
  std::string installNameTool =
    this->LocalGenerator->GetMakefile()->GetSafeDefinition(
      "CMAKE_INSTALL_NAME_TOOL");

  os << indent << "file(GET_RUNTIME_DEPENDENCIES\n"
     << indent << "  RESOLVED_DEPENDENCIES_VAR " << this->DepsVar << '\n';
  WriteFilesArgument(os, "EXECUTABLES"_s,
                     this->RuntimeDependencySet->GetExecutables(), config,
                     indent);
  WriteFilesArgument(os, "LIBRARIES"_s,
                     this->RuntimeDependencySet->GetLibraries(), config,
                     indent);
  WriteFilesArgument(os, "MODULES"_s, this->RuntimeDependencySet->GetModules(),
                     config, indent);
  if (this->RuntimeDependencySet->GetBundleExecutable()) {
    os << indent << "  BUNDLE_EXECUTABLE \""
       << this->RuntimeDependencySet->GetBundleExecutable()->GetItemPath(
            config)
       << "\"\n";
  }
  WriteGenexEvaluatorArgument(os, "DIRECTORIES"_s, this->Directories, config,
                              this->LocalGenerator, indent);
  WriteGenexEvaluatorArgument(os, "PRE_INCLUDE_REGEXES"_s,
                              this->PreIncludeRegexes, config,
                              this->LocalGenerator, indent);
  WriteGenexEvaluatorArgument(os, "PRE_EXCLUDE_REGEXES"_s,
                              this->PreExcludeRegexes, config,
                              this->LocalGenerator, indent);
  WriteGenexEvaluatorArgument(os, "POST_INCLUDE_REGEXES"_s,
                              this->PostIncludeRegexes, config,
                              this->LocalGenerator, indent);
  WriteGenexEvaluatorArgument(os, "POST_EXCLUDE_REGEXES"_s,
                              this->PostExcludeRegexes, config,
                              this->LocalGenerator, indent);
  WriteGenexEvaluatorArgument(os, "POST_INCLUDE_FILES"_s,
                              this->PostIncludeFiles, config,
                              this->LocalGenerator, indent);
  WriteGenexEvaluatorArgument(os, "POST_EXCLUDE_FILES"_s,
                              this->PostExcludeFiles, config,
                              this->LocalGenerator, indent);

  std::set<std::string> postExcludeFiles;
  auto const addPostExclude =
    [config, &postExcludeFiles, this](
      const std::vector<std::unique_ptr<cmInstallRuntimeDependencySet::Item>>&
        tgts) {
      for (auto const& item : tgts) {
        item->AddPostExcludeFiles(config, postExcludeFiles,
                                  this->RuntimeDependencySet);
      }
    };
  addPostExclude(this->RuntimeDependencySet->GetExecutables());
  addPostExclude(this->RuntimeDependencySet->GetLibraries());
  addPostExclude(this->RuntimeDependencySet->GetModules());
  bool first = true;
  for (auto const& file : postExcludeFiles) {
    if (first) {
      os << indent << "  POST_EXCLUDE_FILES_STRICT\n";
      first = false;
    }
    os << indent << "    \"" << file << "\"\n";
  }

  if (!installNameTool.empty() && !this->NoInstallRPath) {
    os << indent << "  RPATH_PREFIX " << this->RPathPrefix << '\n';
  }
  os << indent << "  )\n";
}
