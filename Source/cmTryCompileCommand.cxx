/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTryCompileCommand.h"

#include "cmCoreTryCompile.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"
#include "cmake.h"

bool cmTryCompileCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();

  if (args.size() < 3) {
    mf.IssueMessage(
      MessageType::FATAL_ERROR,
      "The try_compile() command requires at least 3 arguments.");
    return false;
  }

  if (mf.GetCMakeInstance()->GetWorkingMode() == cmake::FIND_PACKAGE_MODE) {
    mf.IssueMessage(
      MessageType::FATAL_ERROR,
      "The try_compile() command is not supported in --find-package mode.");
    return false;
  }

  cmStateEnums::TargetType targetType = cmStateEnums::EXECUTABLE;
  cmValue tt = mf.GetDefinition("CMAKE_TRY_COMPILE_TARGET_TYPE");
  if (cmNonempty(tt)) {
    if (*tt == cmState::GetTargetTypeName(cmStateEnums::EXECUTABLE)) {
      targetType = cmStateEnums::EXECUTABLE;
    } else if (*tt ==
               cmState::GetTargetTypeName(cmStateEnums::STATIC_LIBRARY)) {
      targetType = cmStateEnums::STATIC_LIBRARY;
    } else {
      mf.IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Invalid value '", *tt,
                 "' for CMAKE_TRY_COMPILE_TARGET_TYPE.  Only '",
                 cmState::GetTargetTypeName(cmStateEnums::EXECUTABLE),
                 "' and '",
                 cmState::GetTargetTypeName(cmStateEnums::STATIC_LIBRARY),
                 "' are allowed."));
      return false;
    }
  }

  cmCoreTryCompile tc(&mf);
  cmCoreTryCompile::Arguments arguments =
    tc.ParseArgs(cmMakeRange(args), false);
  if (!arguments) {
    return true;
  }
  tc.TryCompileCode(arguments, targetType);

  // if They specified clean then we clean up what we can
  if (tc.SrcFileSignature) {
    if (!mf.GetCMakeInstance()->GetDebugTryCompile()) {
      tc.CleanupFiles(tc.BinaryDirectory);
    }
  }
  return true;
}
