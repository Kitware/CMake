/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestEmptyBinaryDirectoryCommand.h"

#include "cmCTestScriptHandler.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"

bool cmCTestEmptyBinaryDirectoryCommand::InitialPass(
  std::vector<std::string> const& args, cmExecutionStatus& status)
{
  if (args.size() != 1) {
    this->SetError("called with incorrect number of arguments");
    return false;
  }

  std::string err;
  if (!cmCTestScriptHandler::EmptyBinaryDirectory(args[0], err)) {
    status.GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Did not remove the binary directory:\n ", args[0],
               "\nbecause:\n ", err));
    return true;
  }

  return true;
}
