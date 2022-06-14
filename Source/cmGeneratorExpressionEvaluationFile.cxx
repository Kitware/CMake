/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGeneratorExpressionEvaluationFile.h"

#include <memory>
#include <sstream>
#include <utility>

#include "cmsys/FStream.hxx"

#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"

cmGeneratorExpressionEvaluationFile::cmGeneratorExpressionEvaluationFile(
  std::string input, std::string target,
  std::unique_ptr<cmCompiledGeneratorExpression> outputFileExpr,
  std::unique_ptr<cmCompiledGeneratorExpression> condition,
  bool inputIsContent, std::string newLineCharacter, mode_t permissions,
  cmPolicies::PolicyStatus policyStatusCMP0070)
  : Input(std::move(input))
  , Target(std::move(target))
  , OutputFileExpr(std::move(outputFileExpr))
  , Condition(std::move(condition))
  , InputIsContent(inputIsContent)
  , NewLineCharacter(std::move(newLineCharacter))
  , PolicyStatusCMP0070(policyStatusCMP0070)
  , Permissions(permissions)
{
}

void cmGeneratorExpressionEvaluationFile::Generate(
  cmLocalGenerator* lg, const std::string& config, const std::string& lang,
  cmCompiledGeneratorExpression* inputExpression,
  std::map<std::string, std::string>& outputFiles, mode_t perm)
{
  std::string rawCondition = this->Condition->GetInput();
  cmGeneratorTarget* target = lg->FindGeneratorTargetToUse(this->Target);
  if (!rawCondition.empty()) {
    std::string condResult =
      this->Condition->Evaluate(lg, config, target, nullptr, nullptr, lang);
    if (condResult == "0") {
      return;
    }
    if (condResult != "1") {
      std::ostringstream e;
      e << "Evaluation file condition \"" << rawCondition
        << "\" did "
           "not evaluate to valid content. Got \""
        << condResult << "\".";
      lg->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return;
    }
  }

  const std::string outputFileName =
    this->GetOutputFileName(lg, target, config, lang);
  const std::string& outputContent =
    inputExpression->Evaluate(lg, config, target, nullptr, nullptr, lang);

  auto it = outputFiles.find(outputFileName);

  if (it != outputFiles.end()) {
    if (it->second == outputContent) {
      return;
    }
    std::ostringstream e;
    e << "Evaluation file to be written multiple times with different "
         "content. "
         "This is generally caused by the content evaluating the "
         "configuration type, language, or location of object files:\n "
      << outputFileName;
    lg->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }

  lg->GetMakefile()->AddCMakeOutputFile(outputFileName);
  this->Files.push_back(outputFileName);
  outputFiles[outputFileName] = outputContent;

  bool openWithBinaryFlag = false;
  if (!this->NewLineCharacter.empty()) {
    openWithBinaryFlag = true;
  }
  cmGeneratedFileStream fout;
  fout.Open(outputFileName, false, openWithBinaryFlag);
  if (!fout) {
    lg->IssueMessage(MessageType::FATAL_ERROR,
                     "Could not open file for write in copy operation " +
                       outputFileName);
    return;
  }
  fout.SetCopyIfDifferent(true);
  std::istringstream iss(outputContent);
  std::string line;
  bool hasNewLine = false;
  while (cmSystemTools::GetLineFromStream(iss, line, &hasNewLine)) {
    fout << line;
    if (!this->NewLineCharacter.empty()) {
      fout << this->NewLineCharacter;
    } else if (hasNewLine) {
      // if new line character is not specified, the file will be opened in
      // text mode. So, "\n" will be translated to the correct newline
      // ending based on the platform.
      fout << "\n";
    }
  }
  if (fout.Close() && perm) {
    cmSystemTools::SetPermissions(outputFileName.c_str(), perm);
  }
}

void cmGeneratorExpressionEvaluationFile::CreateOutputFile(
  cmLocalGenerator* lg, std::string const& config)
{
  std::vector<std::string> enabledLanguages;
  cmGlobalGenerator* gg = lg->GetGlobalGenerator();
  cmGeneratorTarget* target = lg->FindGeneratorTargetToUse(this->Target);
  gg->GetEnabledLanguages(enabledLanguages);

  for (std::string const& le : enabledLanguages) {
    std::string const name = this->GetOutputFileName(lg, target, config, le);
    cmSourceFile* sf = lg->GetMakefile()->GetOrCreateGeneratedSource(name);

    // Tell the build system generators that there is no build rule
    // to generate the file.
    sf->SetProperty("__CMAKE_GENERATED_BY_CMAKE", "1");

    gg->SetFilenameTargetDepends(
      sf, this->OutputFileExpr->GetSourceSensitiveTargets());
  }
}

void cmGeneratorExpressionEvaluationFile::Generate(cmLocalGenerator* lg)
{
  std::string inputContent;
  if (this->InputIsContent) {
    inputContent = this->Input;
  } else {
    const std::string inputFileName = this->GetInputFileName(lg);
    lg->GetMakefile()->AddCMakeDependFile(inputFileName);
    if (!this->Permissions) {
      cmSystemTools::GetPermissions(inputFileName.c_str(), this->Permissions);
    }
    cmsys::ifstream fin(inputFileName.c_str());
    if (!fin) {
      std::ostringstream e;
      e << "Evaluation file \"" << inputFileName << "\" cannot be read.";
      lg->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return;
    }

    std::string line;
    std::string sep;
    while (cmSystemTools::GetLineFromStream(fin, line)) {
      inputContent += sep + line;
      sep = "\n";
    }
    inputContent += sep;
  }

  cmListFileBacktrace lfbt = this->OutputFileExpr->GetBacktrace();
  cmGeneratorExpression contentGE(lfbt);
  std::unique_ptr<cmCompiledGeneratorExpression> inputExpression =
    contentGE.Parse(inputContent);

  std::map<std::string, std::string> outputFiles;

  std::vector<std::string> allConfigs =
    lg->GetMakefile()->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  std::vector<std::string> enabledLanguages;
  cmGlobalGenerator* gg = lg->GetGlobalGenerator();
  gg->GetEnabledLanguages(enabledLanguages);

  for (std::string const& le : enabledLanguages) {
    for (std::string const& li : allConfigs) {
      this->Generate(lg, li, le, inputExpression.get(), outputFiles,
                     this->Permissions);
      if (cmSystemTools::GetFatalErrorOccurred()) {
        return;
      }
    }
  }
}

std::string cmGeneratorExpressionEvaluationFile::GetInputFileName(
  cmLocalGenerator* lg)
{
  std::string inputFileName = this->Input;

  if (cmSystemTools::FileIsFullPath(inputFileName)) {
    inputFileName = cmSystemTools::CollapseFullPath(inputFileName);
  } else {
    inputFileName = this->FixRelativePath(inputFileName, PathForInput, lg);
  }

  return inputFileName;
}

std::string cmGeneratorExpressionEvaluationFile::GetOutputFileName(
  cmLocalGenerator* lg, cmGeneratorTarget* target, const std::string& config,
  const std::string& lang)
{
  std::string outputFileName =
    this->OutputFileExpr->Evaluate(lg, config, target, nullptr, nullptr, lang);

  if (cmSystemTools::FileIsFullPath(outputFileName)) {
    outputFileName = cmSystemTools::CollapseFullPath(outputFileName);
  } else {
    outputFileName = this->FixRelativePath(outputFileName, PathForOutput, lg);
  }

  return outputFileName;
}

std::string cmGeneratorExpressionEvaluationFile::FixRelativePath(
  std::string const& relativePath, PathRole role, cmLocalGenerator* lg)
{
  std::string resultPath;
  switch (this->PolicyStatusCMP0070) {
    case cmPolicies::WARN: {
      std::string arg;
      switch (role) {
        case PathForInput:
          arg = "INPUT";
          break;
        case PathForOutput:
          arg = "OUTPUT";
          break;
      }
      std::ostringstream w;
      /* clang-format off */
      w <<
        cmPolicies::GetPolicyWarning(cmPolicies::CMP0070) << "\n"
        "file(GENERATE) given relative " << arg << " path:\n"
        "  " << relativePath << "\n"
        "This is not defined behavior unless CMP0070 is set to NEW.  "
        "For compatibility with older versions of CMake, the previous "
        "undefined behavior will be used."
        ;
      /* clang-format on */
      lg->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
    }
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      // OLD behavior is to use the relative path unchanged,
      // which ends up being used relative to the working dir.
      resultPath = relativePath;
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      // NEW behavior is to interpret the relative path with respect
      // to the current source or binary directory.
      switch (role) {
        case PathForInput:
          resultPath = cmSystemTools::CollapseFullPath(
            relativePath, lg->GetCurrentSourceDirectory());
          break;
        case PathForOutput:
          resultPath = cmSystemTools::CollapseFullPath(
            relativePath, lg->GetCurrentBinaryDirectory());
          break;
      }
      break;
  }
  return resultPath;
}
