/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetPrecompileHeadersCommand.h"

#include "cmGeneratorExpression.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

#include <utility>

bool cmTargetPrecompileHeadersCommand::InitialPass(
  std::vector<std::string> const& args, cmExecutionStatus&)
{
  return this->HandleArguments(args, "PRECOMPILE_HEADERS", PROCESS_REUSE_FROM);
}

void cmTargetPrecompileHeadersCommand::HandleInterfaceContent(
  cmTarget* tgt, const std::vector<std::string>& content, bool prepend,
  bool system)
{
  cmTargetPropCommandBase::HandleInterfaceContent(
    tgt, ConvertToAbsoluteContent(tgt, content, true), prepend, system);
}

void cmTargetPrecompileHeadersCommand::HandleMissingTarget(
  const std::string& name)
{
  const std::string e =
    cmStrCat("Cannot specify precompile headers for target \"", name,
             "\" which is not built by this project.");
  this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
}

std::string cmTargetPrecompileHeadersCommand::Join(
  const std::vector<std::string>& content)
{
  return cmJoin(content, ";");
}

bool cmTargetPrecompileHeadersCommand::HandleDirectContent(
  cmTarget* tgt, const std::vector<std::string>& content, bool, bool)
{
  tgt->AppendProperty(
    "PRECOMPILE_HEADERS",
    this->Join(ConvertToAbsoluteContent(tgt, content, false)).c_str());
  return true;
}

std::vector<std::string>
cmTargetPrecompileHeadersCommand::ConvertToAbsoluteContent(
  cmTarget* /*tgt*/, const std::vector<std::string>& content,
  bool /*isInterfaceContent*/)
{
  std::vector<std::string> absoluteContent;
  absoluteContent.reserve(content.size());
  for (std::string const& src : content) {
    std::string absoluteSrc;
    // Use '<foo.h>' and '"foo.h"' includes and absolute paths as-is.
    // Interpret relative paths with respect to the source directory.
    // If the path starts in a generator expression, assume it is absolute.
    if (cmHasLiteralPrefix(src, "<") || cmHasLiteralPrefix(src, "\"") ||
        cmSystemTools::FileIsFullPath(src) ||
        cmGeneratorExpression::Find(src) == 0) {
      absoluteSrc = src;
    } else {
      absoluteSrc =
        cmStrCat(this->Makefile->GetCurrentSourceDirectory(), '/', src);
    }
    absoluteContent.emplace_back(std::move(absoluteSrc));
  }
  return absoluteContent;
}
