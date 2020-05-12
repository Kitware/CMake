/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSetSourceFilesPropertiesCommand.h"

#include <algorithm>
#include <iterator>

#include <cm/string_view>
#include <cmext/algorithm>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"

bool cmSetSourceFilesPropertiesCommand(std::vector<std::string> const& args,
                                       cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // break the arguments into source file names and properties
  // old style allows for specifier before PROPERTIES keyword
  static const cm::string_view propNames[] = {
    "ABSTRACT",      "GENERATED",      "WRAP_EXCLUDE",
    "COMPILE_FLAGS", "OBJECT_DEPENDS", "PROPERTIES"
  };
  auto propsBegin = std::find_first_of(
    args.begin(), args.end(), std::begin(propNames), std::end(propNames));

  std::vector<std::string> propertyPairs;
  // build the property pairs
  for (auto j = propsBegin; j != args.end(); ++j) {
    // consume old style options
    if (*j == "ABSTRACT" || *j == "GENERATED" || *j == "WRAP_EXCLUDE") {
      propertyPairs.emplace_back(*j);
      propertyPairs.emplace_back("1");
    } else if (*j == "COMPILE_FLAGS") {
      propertyPairs.emplace_back("COMPILE_FLAGS");
      ++j;
      if (j == args.end()) {
        status.SetError("called with incorrect number of arguments "
                        "COMPILE_FLAGS with no flags");
        return false;
      }
      propertyPairs.push_back(*j);
    } else if (*j == "OBJECT_DEPENDS") {
      propertyPairs.emplace_back("OBJECT_DEPENDS");
      ++j;
      if (j == args.end()) {
        status.SetError("called with incorrect number of arguments "
                        "OBJECT_DEPENDS with no dependencies");
        return false;
      }
      propertyPairs.push_back(*j);
    } else if (*j == "PROPERTIES") {
      // PROPERTIES is followed by new style prop value pairs
      cmStringRange newStyleProps{ j + 1, args.end() };
      if (newStyleProps.size() % 2 != 0) {
        status.SetError("called with incorrect number of arguments.");
        return false;
      }
      // set newStyleProps as is.
      cm::append(propertyPairs, newStyleProps);
      // break out of the loop.
      break;
    } else {
      status.SetError("called with illegal arguments, maybe missing a "
                      "PROPERTIES specifier?");
      return false;
    }
  }

  // loop over all the files
  for (const std::string& sfname : cmStringRange{ args.begin(), propsBegin }) {
    // get the source file
    if (cmSourceFile* sf = status.GetMakefile().GetOrCreateSource(sfname)) {
      // loop through the props and set them
      for (auto k = propertyPairs.begin(); k != propertyPairs.end(); k += 2) {
        sf->SetProperty(*k, (k + 1)->c_str());
      }
    }
  }
  return true;
}
