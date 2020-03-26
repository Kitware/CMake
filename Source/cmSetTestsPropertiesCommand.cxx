/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSetTestsPropertiesCommand.h"

#include <iterator>

#include <cmext/algorithm>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmTest.h"

static bool SetOneTest(const std::string& tname,
                       std::vector<std::string>& propertyPairs, cmMakefile* mf,
                       std::string& errors);

bool cmSetTestsPropertiesCommand(std::vector<std::string> const& args,
                                 cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // first collect up the list of files
  std::vector<std::string> propertyPairs;
  int numFiles = 0;
  std::vector<std::string>::const_iterator j;
  for (j = args.begin(); j != args.end(); ++j) {
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
    status.SetError("called with illegal arguments, maybe "
                    "missing a PROPERTIES specifier?");
    return false;
  }

  // now loop over all the targets
  int i;
  for (i = 0; i < numFiles; ++i) {
    std::string errors;
    bool ret = SetOneTest(args[i], propertyPairs, &mf, errors);
    if (!ret) {
      status.SetError(errors);
      return ret;
    }
  }

  return true;
}

static bool SetOneTest(const std::string& tname,
                       std::vector<std::string>& propertyPairs, cmMakefile* mf,
                       std::string& errors)
{
  if (cmTest* test = mf->GetTest(tname)) {
    // now loop through all the props and set them
    unsigned int k;
    for (k = 0; k < propertyPairs.size(); k = k + 2) {
      if (!propertyPairs[k].empty()) {
        test->SetProperty(propertyPairs[k], propertyPairs[k + 1].c_str());
      }
    }
  } else {
    errors = cmStrCat("Can not find test to add properties to: ", tname);
    return false;
  }

  return true;
}
