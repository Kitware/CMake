/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSetTargetPropertiesCommand.h"

#include <algorithm>
#include <iterator>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"

bool cmSetTargetPropertiesCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // first identify the properties arguments
  auto propsIter = std::find(args.begin(), args.end(), "PROPERTIES");
  if (propsIter == args.end() || propsIter + 1 == args.end()) {
    status.SetError("called with illegal arguments, maybe missing a "
                    "PROPERTIES specifier?");
    return false;
  }

  if (std::distance(propsIter, args.end()) % 2 != 1) {
    status.SetError("called with incorrect number of arguments.");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // loop over all the targets
  for (const std::string& tname : cmStringRange{ args.begin(), propsIter }) {
    if (mf.IsAlias(tname)) {
      status.SetError("can not be used on an ALIAS target.");
      return false;
    }
    if (cmTarget* target = mf.FindTargetToUse(tname)) {
      // loop through all the props and set them
      for (auto k = propsIter + 1; k != args.end(); k += 2) {
        target->SetProperty(*k, *(k + 1));
        target->CheckProperty(*k, &mf);
      }
    } else {
      status.SetError(
        cmStrCat("Can not find target to add properties to: ", tname));
      return false;
    }
  }
  return true;
}
