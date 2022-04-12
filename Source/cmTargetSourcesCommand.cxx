/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetSourcesCommand.h"

#include <sstream>
#include <utility>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmFileSet.h"
#include "cmGeneratorExpression.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetPropCommandBase.h"

namespace {

struct FileSetArgs
{
  std::string Type;
  std::string FileSet;
  std::vector<std::string> BaseDirs;
  std::vector<std::string> Files;
};

auto const FileSetArgsParser = cmArgumentParser<FileSetArgs>()
                                 .Bind("TYPE"_s, &FileSetArgs::Type)
                                 .Bind("FILE_SET"_s, &FileSetArgs::FileSet)
                                 .Bind("BASE_DIRS"_s, &FileSetArgs::BaseDirs)
                                 .Bind("FILES"_s, &FileSetArgs::Files);

struct FileSetsArgs
{
  std::vector<std::vector<std::string>> FileSets;
};

auto const FileSetsArgsParser =
  cmArgumentParser<FileSetsArgs>().Bind("FILE_SET"_s, &FileSetsArgs::FileSets);

class TargetSourcesImpl : public cmTargetPropCommandBase
{
public:
  using cmTargetPropCommandBase::cmTargetPropCommandBase;

protected:
  void HandleInterfaceContent(cmTarget* tgt,
                              const std::vector<std::string>& content,
                              bool prepend, bool system) override
  {
    this->cmTargetPropCommandBase::HandleInterfaceContent(
      tgt,
      this->ConvertToAbsoluteContent(tgt, content, IsInterface::Yes,
                                     CheckCMP0076::Yes),
      prepend, system);
  }

private:
  void HandleMissingTarget(const std::string& name) override
  {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Cannot specify sources for target \"", name,
               "\" which is not built by this project."));
  }

  bool HandleDirectContent(cmTarget* tgt,
                           const std::vector<std::string>& content,
                           bool /*prepend*/, bool /*system*/) override
  {
    tgt->AppendProperty("SOURCES",
                        this->Join(this->ConvertToAbsoluteContent(
                          tgt, content, IsInterface::No, CheckCMP0076::Yes)));
    return true; // Successfully handled.
  }

  bool PopulateTargetProperties(const std::string& scope,
                                const std::vector<std::string>& content,
                                bool prepend, bool system) override
  {
    if (!content.empty() && content.front() == "FILE_SET"_s) {
      return this->HandleFileSetMode(scope, content);
    }
    return this->cmTargetPropCommandBase::PopulateTargetProperties(
      scope, content, prepend, system);
  }

  std::string Join(const std::vector<std::string>& content) override
  {
    return cmJoin(content, ";");
  }

  enum class IsInterface
  {
    Yes,
    No,
  };
  enum class CheckCMP0076
  {
    Yes,
    No,
  };
  std::vector<std::string> ConvertToAbsoluteContent(
    cmTarget* tgt, const std::vector<std::string>& content,
    IsInterface isInterfaceContent, CheckCMP0076 checkCmp0076);

  bool HandleFileSetMode(const std::string& scope,
                         const std::vector<std::string>& content);
  bool HandleOneFileSet(const std::string& scope,
                        const std::vector<std::string>& content);
};

std::vector<std::string> TargetSourcesImpl::ConvertToAbsoluteContent(
  cmTarget* tgt, const std::vector<std::string>& content,
  IsInterface isInterfaceContent, CheckCMP0076 checkCmp0076)
{
  // Skip conversion in case old behavior has been explicitly requested
  if (checkCmp0076 == CheckCMP0076::Yes &&
      this->Makefile->GetPolicyStatus(cmPolicies::CMP0076) ==
        cmPolicies::OLD) {
    return content;
  }

  bool changedPath = false;
  std::vector<std::string> absoluteContent;
  absoluteContent.reserve(content.size());
  for (std::string const& src : content) {
    std::string absoluteSrc;
    if (cmSystemTools::FileIsFullPath(src) ||
        cmGeneratorExpression::Find(src) == 0 ||
        (isInterfaceContent == IsInterface::No &&
         (this->Makefile->GetCurrentSourceDirectory() ==
          tgt->GetMakefile()->GetCurrentSourceDirectory()))) {
      absoluteSrc = src;
    } else {
      changedPath = true;
      absoluteSrc =
        cmStrCat(this->Makefile->GetCurrentSourceDirectory(), '/', src);
    }
    absoluteContent.push_back(absoluteSrc);
  }

  if (!changedPath) {
    return content;
  }

  bool issueMessage = true;
  bool useAbsoluteContent = false;
  std::ostringstream e;
  if (checkCmp0076 == CheckCMP0076::Yes) {
    switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0076)) {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0076) << "\n";
        break;
      case cmPolicies::OLD:
        issueMessage = false;
        break;
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0076));
        break;
      case cmPolicies::NEW: {
        issueMessage = false;
        useAbsoluteContent = true;
        break;
      }
    }
  } else {
    issueMessage = false;
    useAbsoluteContent = true;
  }

  if (issueMessage) {
    if (isInterfaceContent == IsInterface::Yes) {
      e << "An interface source of target \"" << tgt->GetName()
        << "\" has a relative path.";
    } else {
      e << "A private source from a directory other than that of target \""
        << tgt->GetName() << "\" has a relative path.";
    }
    this->Makefile->IssueMessage(MessageType::AUTHOR_WARNING, e.str());
  }

  return useAbsoluteContent ? absoluteContent : content;
}

bool TargetSourcesImpl::HandleFileSetMode(
  const std::string& scope, const std::vector<std::string>& content)
{
  auto args = FileSetsArgsParser.Parse(content);

  for (auto& argList : args.FileSets) {
    argList.emplace(argList.begin(), "FILE_SET"_s);
    if (!this->HandleOneFileSet(scope, argList)) {
      return false;
    }
  }

  return true;
}

bool TargetSourcesImpl::HandleOneFileSet(
  const std::string& scope, const std::vector<std::string>& content)
{
  std::vector<std::string> unparsed;
  auto args = FileSetArgsParser.Parse(content, &unparsed);

  if (!unparsed.empty()) {
    this->SetError(
      cmStrCat("Unrecognized keyword: \"", unparsed.front(), "\""));
    return false;
  }

  if (args.FileSet.empty()) {
    this->SetError("FILE_SET must not be empty");
    return false;
  }

  if (this->Target->GetType() == cmStateEnums::UTILITY) {
    this->SetError("FILE_SETs may not be added to custom targets");
    return false;
  }
  if (this->Target->IsFrameworkOnApple()) {
    this->SetError("FILE_SETs may not be added to FRAMEWORK targets");
    return false;
  }

  bool const isDefault = args.Type == args.FileSet ||
    (args.Type.empty() && args.FileSet[0] >= 'A' && args.FileSet[0] <= 'Z');
  std::string type = isDefault ? args.FileSet : args.Type;

  cmFileSetVisibility visibility =
    cmFileSetVisibilityFromName(scope, this->Makefile);

  auto fileSet =
    this->Target->GetOrCreateFileSet(args.FileSet, type, visibility);
  if (fileSet.second) {
    if (!isDefault) {
      if (!cmFileSet::IsValidName(args.FileSet)) {
        this->SetError("Non-default file set name must contain only letters, "
                       "numbers, and underscores, and must not start with a "
                       "capital letter or underscore");
        return false;
      }
    }
    if (type.empty()) {
      this->SetError("Must specify a TYPE when creating file set");
      return false;
    }
    if (type != "HEADERS"_s) {
      this->SetError("File set TYPE may only be \"HEADERS\"");
      return false;
    }

    if (args.BaseDirs.empty()) {
      args.BaseDirs.emplace_back(this->Makefile->GetCurrentSourceDirectory());
    }
  } else {
    type = fileSet.first->GetType();
    if (!args.Type.empty() && args.Type != type) {
      this->SetError(cmStrCat(
        "Type \"", args.Type, "\" for file set \"", fileSet.first->GetName(),
        "\" does not match original type \"", type, "\""));
      return false;
    }

    if (visibility != fileSet.first->GetVisibility()) {
      this->SetError(
        cmStrCat("Scope ", scope, " for file set \"", args.FileSet,
                 "\" does not match original scope ",
                 cmFileSetVisibilityToName(fileSet.first->GetVisibility())));
      return false;
    }
  }

  auto files = this->Join(this->ConvertToAbsoluteContent(
    this->Target, args.Files, IsInterface::Yes, CheckCMP0076::No));
  if (!files.empty()) {
    fileSet.first->AddFileEntry(
      BT<std::string>(files, this->Makefile->GetBacktrace()));
  }

  auto baseDirectories = this->Join(this->ConvertToAbsoluteContent(
    this->Target, args.BaseDirs, IsInterface::Yes, CheckCMP0076::No));
  if (!baseDirectories.empty()) {
    fileSet.first->AddDirectoryEntry(
      BT<std::string>(baseDirectories, this->Makefile->GetBacktrace()));
    if (type == "HEADERS"_s) {
      for (auto const& dir : cmExpandedList(baseDirectories)) {
        auto interfaceDirectoriesGenex =
          cmStrCat("$<BUILD_INTERFACE:", dir, ">");
        if (cmFileSetVisibilityIsForSelf(visibility)) {
          this->Target->AppendProperty("INCLUDE_DIRECTORIES",
                                       interfaceDirectoriesGenex);
        }
        if (cmFileSetVisibilityIsForInterface(visibility)) {
          this->Target->AppendProperty("INTERFACE_INCLUDE_DIRECTORIES",
                                       interfaceDirectoriesGenex);
        }
      }
    }
  }

  return true;
}

} // namespace

bool cmTargetSourcesCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  return TargetSourcesImpl(status).HandleArguments(args, "SOURCES");
}
