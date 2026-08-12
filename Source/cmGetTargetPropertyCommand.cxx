/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGetTargetPropertyCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmTargetPropertyHelper.h"
#include "cmValue.h"

bool cmGetTargetPropertyCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  std::string const& var = args[0];
  std::string const& targetName = args[1];
  std::string const& propertyName = args[2];
  cmMakefile& mf = status.GetMakefile();

  cmValue propValue;
  auto result = cmGetTargetProperty(targetName, propertyName, mf, propValue);
  if (result == cmGetTargetPropertyResult::Success) {
    if (propValue) {
      mf.AddDefinition(var, *propValue);
      return true;
    }
    mf.AddDefinition(var, var + "-NOTFOUND");
    return true;
  }
  if (result == cmGetTargetPropertyResult::TargetNotFound) {
    mf.IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("get_target_property() called with non-existent target \"",
               targetName, "\"."));
  }
  return false;
}
