/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmTargetSourcesCommand.h"

#include <sstream>
#include <utility>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmFileSet.h"
#include "cmFileSetMetadata.h"
#include "cmGeneratorExpression.h"
#include "cmList.h"
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
  ArgumentParser::MaybeEmpty<std::vector<std::string>> BaseDirs;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> Files;
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
                              std::vector<std::string> const& content,
                              bool prepend, bool system) override
  {
    this->cmTargetPropCommandBase::HandleInterfaceContent(
      tgt,
      this->ConvertToAbsoluteContent(tgt, content, IsInterface::Yes,
                                     CheckCMP0076::Yes),
      prepend, system);
  }

private:
  void HandleMissingTarget(std::string const& name) override
  {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Cannot specify sources for target \"", name,
               "\" which is not built by this project."));
  }

  bool HandleDirectContent(cmTarget* tgt,
                           std::vector<std::string> const& content,
                           bool /*prepend*/, bool /*system*/) override
  {
    tgt->AppendProperty("SOURCES",
                        this->Join(this->ConvertToAbsoluteContent(
                          tgt, content, IsInterface::No, CheckCMP0076::Yes)),
                        this->Makefile->GetBacktrace());
    return true; // Successfully handled.
  }

  bool PopulateTargetProperties(std::string const& scope,
                                std::vector<std::string> const& content,
                                bool prepend, bool system) override
  {
    if (!content.empty() && content.front() == "FILE_SET"_s) {
      return this->HandleFileSetMode(scope, content);
    }
    return this->cmTargetPropCommandBase::PopulateTargetProperties(
      scope, content, prepend, system);
  }

  std::string Join(std::vector<std::string> const& content) override
  {
    return cmList::to_string(content);
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
    cmTarget* tgt, std::vector<std::string> const& content,
    IsInterface isInterfaceContent, CheckCMP0076 checkCmp0076);

  bool HandleFileSetMode(std::string const& scope,
                         std::vector<std::string> const& content);
  bool HandleOneFileSet(std::string const& scope,
                        std::vector<std::string> const& content);
};

std::vector<std::string> TargetSourcesImpl::ConvertToAbsoluteContent(
  cmTarget* tgt, std::vector<std::string> const& content,
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
  std::string const& scope, std::vector<std::string> const& content)
{
  auto args = FileSetsArgsParser.Parse(content, /*unparsedArguments=*/nullptr);

  for (auto& argList : args.FileSets) {
    argList.emplace(argList.begin(), "FILE_SET"_s);
    if (!this->HandleOneFileSet(scope, argList)) {
      return false;
    }
  }

  return true;
}

bool TargetSourcesImpl::HandleOneFileSet(
  std::string const& scope, std::vector<std::string> const& content)
{
  std::vector<std::string> unparsed;
  auto args = FileSetArgsParser.Parse(content, &unparsed);

  if (!unparsed.empty()) {
    this->SetError(
      cmStrCat("Unrecognized keyword: \"", unparsed.front(), '"'));
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

  if (!args.Type.empty() && !cm::FileSetMetadata::IsKnownType(args.Type)) {
    this->SetError(
      cmStrCat("File set TYPE may only be \"",
               cmJoin(cm::FileSetMetadata::GetKnownTypes(), "\", \""), '"'));
    return false;
  }
  if (args.Type.empty() && args.FileSet[0] >= 'A' && args.FileSet[0] <= 'Z' &&
      !cm::FileSetMetadata::IsKnownType(args.FileSet)) {
    this->SetError(
      cmStrCat("FILE_SET names starting with a capital letter are reserved "
               "for built-in file sets and may only be \"",
               cmJoin(cm::FileSetMetadata::GetKnownTypes(), "\", \""), '"'));
    return false;
  }
  if (!args.Type.empty() && args.FileSet[0] >= 'A' && args.FileSet[0] <= 'Z' &&
      args.Type != args.FileSet) {
    this->SetError(cmStrCat("FILE_SET name starting with a capital letter "
                            "must match the TYPE name \"",
                            args.Type, '"'));
    return false;
  }

  bool const isDefault = args.Type == args.FileSet ||
    (args.Type.empty() && cm::FileSetMetadata::IsKnownType(args.FileSet));

  if (!isDefault && !cm::FileSetMetadata::IsValidName(args.FileSet)) {
    this->SetError("Non-default file set name must contain only letters, "
                   "numbers, and underscores, and must not start with a "
                   "capital letter or underscore");
    return false;
  }

  std::string type = isDefault ? args.FileSet : args.Type;
  cm::FileSetMetadata::Visibility visibility =
    cm::FileSetMetadata::VisibilityFromName(scope, this->Makefile);

  auto fileSet =
    this->Target->GetOrCreateFileSet(args.FileSet, type, visibility);
  if (fileSet.second) {
    if (type.empty()) {
      this->SetError("Must specify a TYPE when creating file set");
      return false;
    }

    if (cm::FileSetMetadata::VisibilityIsForSelf(visibility) &&
        this->Target->GetType() == cmStateEnums::INTERFACE_LIBRARY &&
        !this->Target->IsImported()) {
      if (type == cm::FileSetMetadata::CXX_MODULES) {
        this->SetError(
          cmStrCat(R"(File set TYPE ")", cm::FileSetMetadata::CXX_MODULES,
                   R"(" may not have "PUBLIC" )"
                   R"(or "PRIVATE" visibility on INTERFACE libraries.)"));
        return false;
      }
    }

    // FIXME(https://wg21.link/P3470): This condition can go
    // away when interface-only module units are a thing.
    if (cm::FileSetMetadata::VisibilityIsForInterface(visibility) &&
        !cm::FileSetMetadata::VisibilityIsForSelf(visibility) &&
        !this->Target->IsImported()) {
      if (type == cm::FileSetMetadata::CXX_MODULES) {
        this->SetError(cmStrCat(R"(File set TYPE ")",
                                cm::FileSetMetadata::CXX_MODULES,
                                R"(" may not have "INTERFACE" visibility)"));
        return false;
      }
    }

    if (args.BaseDirs.empty()) {
      args.BaseDirs.emplace_back(this->Makefile->GetCurrentSourceDirectory());
    }
  } else {
    type = fileSet.first->GetType();
    if (!args.Type.empty() && args.Type != type) {
      this->SetError(cmStrCat(
        "Type \"", args.Type, "\" for file set \"", fileSet.first->GetName(),
        "\" does not match original type \"", type, '"'));
      return false;
    }

    if (visibility != fileSet.first->GetVisibility()) {
      this->SetError(cmStrCat("Scope ", scope, " for file set \"",
                              args.FileSet,
                              "\" does not match original scope ",
                              cm::FileSetMetadata::VisibilityToName(
                                fileSet.first->GetVisibility())));
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
    if (type == cm::FileSetMetadata::HEADERS) {
      for (auto const& dir : cmList{ baseDirectories }) {
        auto interfaceDirectoriesGenex =
          cmStrCat("$<BUILD_INTERFACE:", dir, '>');
        if (cm::FileSetMetadata::VisibilityIsForSelf(visibility)) {
          this->Target->AppendProperty("INCLUDE_DIRECTORIES",
                                       interfaceDirectoriesGenex,
                                       this->Makefile->GetBacktrace());
        }
        if (cm::FileSetMetadata::VisibilityIsForInterface(visibility)) {
          this->Target->AppendProperty("INTERFACE_INCLUDE_DIRECTORIES",
                                       interfaceDirectoriesGenex,
                                       this->Makefile->GetBacktrace());
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
