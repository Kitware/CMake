/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetPrecompileHeadersCommand.h"

#include <utility>

#include "cmGeneratorExpression.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetPropCommandBase.h"

namespace {

std::vector<std::string> ConvertToAbsoluteContent(
  const std::vector<std::string>& content, std::string const& baseDir)
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
      absoluteSrc = cmStrCat(baseDir, '/', src);
    }
    absoluteContent.emplace_back(std::move(absoluteSrc));
  }
  return absoluteContent;
}

class TargetPrecompileHeadersImpl : public cmTargetPropCommandBase
{
public:
  using cmTargetPropCommandBase::cmTargetPropCommandBase;

private:
  bool HandleDirectContent(cmTarget* tgt,
                           const std::vector<std::string>& content,
                           bool /*prepend*/, bool /*system*/) override
  {
    std::string const& base = this->Makefile->GetCurrentSourceDirectory();
    tgt->AppendProperty("PRECOMPILE_HEADERS",
                        this->Join(ConvertToAbsoluteContent(content, base)));
    return true;
  }

  void HandleInterfaceContent(cmTarget* tgt,
                              const std::vector<std::string>& content,
                              bool prepend, bool system) override
  {
    std::string const& base = this->Makefile->GetCurrentSourceDirectory();
    cmTargetPropCommandBase::HandleInterfaceContent(
      tgt, ConvertToAbsoluteContent(content, base), prepend, system);
  }

  void HandleMissingTarget(const std::string& name) override
  {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Cannot specify precompile headers for target \"", name,
               "\" which is not built by this project."));
  }

  std::string Join(const std::vector<std::string>& content) override
  {
    return cmJoin(content, ";");
  }
};

} // namespace

bool cmTargetPrecompileHeadersCommand(std::vector<std::string> const& args,
                                      cmExecutionStatus& status)
{
  return TargetPrecompileHeadersImpl(status).HandleArguments(
    args, "PRECOMPILE_HEADERS",
    TargetPrecompileHeadersImpl::PROCESS_REUSE_FROM);
}
