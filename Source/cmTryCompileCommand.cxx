/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTryCompileCommand.h"

#include "cmCoreTryCompile.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmake.h"

bool cmTryCompileCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args.size() < 3) {
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  if (mf.GetCMakeInstance()->GetWorkingMode() == cmake::FIND_PACKAGE_MODE) {
    mf.IssueMessage(
      MessageType::FATAL_ERROR,
      "The try_compile() command is not supported in --find-package mode.");
    return false;
  }

  cmCoreTryCompile tc(&mf);
  tc.TryCompileCode(args, false);

  // if They specified clean then we clean up what we can
  if (tc.SrcFileSignature) {
    if (!mf.GetCMakeInstance()->GetDebugTryCompile()) {
      tc.CleanupFiles(tc.BinaryDirectory);
    }
  }
  return true;
}
