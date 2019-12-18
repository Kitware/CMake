/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSetTargetPropertiesCommand.h"

#include <iterator>

#include <cmext/algorithm>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"

static bool SetOneTarget(const std::string& tname,
                         std::vector<std::string>& propertyPairs,
                         cmMakefile* mf);

bool cmSetTargetPropertiesCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // first collect up the list of files
  std::vector<std::string> propertyPairs;
  int numFiles = 0;
  for (auto j = args.begin(); j != args.end(); ++j) {
    if (*j == "PROPERTIES") {
      // now loop through the rest of the arguments, new style
      ++j;
      if (std::distance(j, args.end()) % 2 != 0) {
        status.SetError("called with incorrect number of arguments.");
        return false;
      }
      cm::append(propertyPairs, j, args.end());
      break;
    }
    numFiles++;
  }
  if (propertyPairs.empty()) {
    status.SetError("called with illegal arguments, maybe missing "
                    "a PROPERTIES specifier?");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // now loop over all the targets
  for (int i = 0; i < numFiles; ++i) {
    if (mf.IsAlias(args[i])) {
      status.SetError("can not be used on an ALIAS target.");
      return false;
    }
    bool ret = SetOneTarget(args[i], propertyPairs, &mf);
    if (!ret) {
      status.SetError(
        cmStrCat("Can not find target to add properties to: ", args[i]));
      return false;
    }
  }
  return true;
}

static bool SetOneTarget(const std::string& tname,
                         std::vector<std::string>& propertyPairs,
                         cmMakefile* mf)
{
  if (cmTarget* target = mf->FindTargetToUse(tname)) {
    // now loop through all the props and set them
    unsigned int k;
    for (k = 0; k < propertyPairs.size(); k = k + 2) {
      target->SetProperty(propertyPairs[k], propertyPairs[k + 1]);
      target->CheckProperty(propertyPairs[k], mf);
    }
  }
  // if file is not already in the makefile, then add it
  else {
    return false;
  }
  return true;
}
