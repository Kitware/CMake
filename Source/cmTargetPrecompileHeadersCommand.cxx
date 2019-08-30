/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetPrecompileHeadersCommand.h"

#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"

bool cmTargetPrecompileHeadersCommand::InitialPass(
  std::vector<std::string> const& args, cmExecutionStatus&)
{
  return this->HandleArguments(args, "PRECOMPILE_HEADERS", PROCESS_REUSE_FROM);
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
  tgt->AppendProperty("PRECOMPILE_HEADERS", this->Join(content).c_str());
  return true;
}
