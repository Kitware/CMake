/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGetTargetPropertyCommand.h"

#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
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
  std::string prop;
  bool prop_exists = false;
  cmMakefile& mf = status.GetMakefile();

  if (cmTarget* tgt = mf.FindTargetToUse(targetName)) {
    if (args[2] == "ALIASED_TARGET" || args[2] == "ALIAS_GLOBAL") {
      if (mf.IsAlias(targetName)) {
        prop_exists = true;
        if (args[2] == "ALIASED_TARGET") {

          prop = tgt->GetName();
        }
        if (args[2] == "ALIAS_GLOBAL") {
          prop =
            mf.GetGlobalGenerator()->IsAlias(targetName) ? "TRUE" : "FALSE";
        }
      }
    } else if (!args[2].empty()) {
      cmValue prop_cstr = nullptr;
      prop_cstr = tgt->GetComputedProperty(args[2], mf);
      if (!prop_cstr) {
        prop_cstr = tgt->GetProperty(args[2]);
      }
      if (prop_cstr) {
        prop = *prop_cstr;
        prop_exists = true;
      }
    }
  } else {
    mf.IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("get_target_property() called with non-existent target \"",
               targetName, "\"."));
    return false;
  }
  if (prop_exists) {
    mf.AddDefinition(var, prop);
    return true;
  }
  mf.AddDefinition(var, var + "-NOTFOUND");
  return true;
}
