/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSetSourceFilesPropertiesCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"

static bool RunCommand(cmMakefile* mf,
                       std::vector<std::string>::const_iterator filebeg,
                       std::vector<std::string>::const_iterator fileend,
                       std::vector<std::string>::const_iterator propbeg,
                       std::vector<std::string>::const_iterator propend,
                       std::string& errors);

bool cmSetSourceFilesPropertiesCommand(std::vector<std::string> const& args,
                                       cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // break the arguments into source file names and properties
  int numFiles = 0;
  std::vector<std::string>::const_iterator j;
  j = args.begin();
  // old style allows for specifier before PROPERTIES keyword
  while (j != args.end() && *j != "ABSTRACT" && *j != "WRAP_EXCLUDE" &&
         *j != "GENERATED" && *j != "COMPILE_FLAGS" &&
         *j != "OBJECT_DEPENDS" && *j != "PROPERTIES") {
    numFiles++;
    ++j;
  }

  cmMakefile& mf = status.GetMakefile();

  // now call the worker function
  std::string errors;
  bool ret = RunCommand(&mf, args.begin(), args.begin() + numFiles,
                        args.begin() + numFiles, args.end(), errors);
  if (!ret) {
    status.SetError(errors);
  }
  return ret;
}

static bool RunCommand(cmMakefile* mf,
                       std::vector<std::string>::const_iterator filebeg,
                       std::vector<std::string>::const_iterator fileend,
                       std::vector<std::string>::const_iterator propbeg,
                       std::vector<std::string>::const_iterator propend,
                       std::string& errors)
{
  std::vector<std::string> propertyPairs;
  bool generated = false;
  std::vector<std::string>::const_iterator j;
  // build the property pairs
  for (j = propbeg; j != propend; ++j) {
    // old style allows for specifier before PROPERTIES keyword
    if (*j == "ABSTRACT") {
      propertyPairs.emplace_back("ABSTRACT");
      propertyPairs.emplace_back("1");
    } else if (*j == "WRAP_EXCLUDE") {
      propertyPairs.emplace_back("WRAP_EXCLUDE");
      propertyPairs.emplace_back("1");
    } else if (*j == "GENERATED") {
      generated = true;
      propertyPairs.emplace_back("GENERATED");
      propertyPairs.emplace_back("1");
    } else if (*j == "COMPILE_FLAGS") {
      propertyPairs.emplace_back("COMPILE_FLAGS");
      ++j;
      if (j == propend) {
        errors = "called with incorrect number of arguments "
                 "COMPILE_FLAGS with no flags";
        return false;
      }
      propertyPairs.push_back(*j);
    } else if (*j == "OBJECT_DEPENDS") {
      propertyPairs.emplace_back("OBJECT_DEPENDS");
      ++j;
      if (j == propend) {
        errors = "called with incorrect number of arguments "
                 "OBJECT_DEPENDS with no dependencies";
        return false;
      }
      propertyPairs.push_back(*j);
    } else if (*j == "PROPERTIES") {
      // now loop through the rest of the arguments, new style
      ++j;
      while (j != propend) {
        propertyPairs.push_back(*j);
        if (*j == "GENERATED") {
          ++j;
          if (j != propend && cmIsOn(*j)) {
            generated = true;
          }
        } else {
          ++j;
        }
        if (j == propend) {
          errors = "called with incorrect number of arguments.";
          return false;
        }
        propertyPairs.push_back(*j);
        ++j;
      }
      // break out of the loop because j is already == end
      break;
    } else {
      errors = "called with illegal arguments, maybe missing a "
               "PROPERTIES specifier?";
      return false;
    }
  }

  // now loop over all the files
  for (j = filebeg; j != fileend; ++j) {
    // get the source file
    cmSourceFile* sf = mf->GetOrCreateSource(*j, generated);
    if (sf) {
      // now loop through all the props and set them
      unsigned int k;
      for (k = 0; k < propertyPairs.size(); k = k + 2) {
        sf->SetProperty(propertyPairs[k], propertyPairs[k + 1].c_str());
      }
    }
  }
  return true;
}
