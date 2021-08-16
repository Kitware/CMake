/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSetSourceFilesPropertiesCommand.h"

#include <algorithm>
#include <iterator>

#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSetPropertyCommand.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"

static bool RunCommandForScope(
  cmMakefile* mf, std::vector<std::string>::const_iterator file_begin,
  std::vector<std::string>::const_iterator file_end,
  std::vector<std::string>::const_iterator prop_begin,
  std::vector<std::string>::const_iterator prop_end, std::string& errors);

bool cmSetSourceFilesPropertiesCommand(std::vector<std::string> const& args,
                                       cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // break the arguments into source file names and properties
  // old style allows for specifier before PROPERTIES keyword
  static const cm::string_view prop_names[] = {
    "ABSTRACT",       "GENERATED",  "WRAP_EXCLUDE", "COMPILE_FLAGS",
    "OBJECT_DEPENDS", "PROPERTIES", "DIRECTORY",    "TARGET_DIRECTORY"
  };

  auto isAPropertyKeyword =
    [](const std::vector<std::string>::const_iterator& arg_it) {
      return std::any_of(
        std::begin(prop_names), std::end(prop_names),
        [&arg_it](cm::string_view prop_name) { return *arg_it == prop_name; });
    };

  auto options_begin = std::find_first_of(
    args.begin(), args.end(), std::begin(prop_names), std::end(prop_names));
  auto options_it = options_begin;

  // Handle directory options.
  std::vector<std::string> source_file_directories;
  std::vector<std::string> source_file_target_directories;
  bool source_file_directory_option_enabled = false;
  bool source_file_target_option_enabled = false;
  std::vector<cmMakefile*> source_file_directory_makefiles;

  enum Doing
  {
    DoingNone,
    DoingSourceDirectory,
    DoingSourceTargetDirectory
  };
  Doing doing = DoingNone;
  for (; options_it != args.end(); ++options_it) {
    if (*options_it == "DIRECTORY") {
      doing = DoingSourceDirectory;
      source_file_directory_option_enabled = true;
    } else if (*options_it == "TARGET_DIRECTORY") {
      doing = DoingSourceTargetDirectory;
      source_file_target_option_enabled = true;
    } else if (isAPropertyKeyword(options_it)) {
      break;
    } else if (doing == DoingSourceDirectory) {
      source_file_directories.push_back(*options_it);
    } else if (doing == DoingSourceTargetDirectory) {
      source_file_target_directories.push_back(*options_it);
    } else {
      status.SetError(
        cmStrCat("given invalid argument \"", *options_it, "\"."));
    }
  }

  const auto props_begin = options_it;

  bool file_scopes_handled =
    SetPropertyCommand::HandleAndValidateSourceFileDirectoryScopes(
      status, source_file_directory_option_enabled,
      source_file_target_option_enabled, source_file_directories,
      source_file_target_directories, source_file_directory_makefiles);
  if (!file_scopes_handled) {
    return false;
  }

  std::vector<std::string> files;
  bool source_file_paths_should_be_absolute =
    source_file_directory_option_enabled || source_file_target_option_enabled;
  SetPropertyCommand::MakeSourceFilePathsAbsoluteIfNeeded(
    status, files, args.begin(), options_begin,
    source_file_paths_should_be_absolute);

  // Now call the worker function for each directory scope represented by a
  // cmMakefile instance.
  std::string errors;
  for (auto* const mf : source_file_directory_makefiles) {
    bool ret = RunCommandForScope(mf, files.begin(), files.end(), props_begin,
                                  args.end(), errors);
    if (!ret) {
      status.SetError(errors);
      return ret;
    }
  }

  return true;
}

static bool RunCommandForScope(
  cmMakefile* mf, std::vector<std::string>::const_iterator file_begin,
  std::vector<std::string>::const_iterator file_end,
  std::vector<std::string>::const_iterator prop_begin,
  std::vector<std::string>::const_iterator prop_end, std::string& errors)
{
  std::vector<std::string> propertyPairs;
  // build the property pairs
  for (auto j = prop_begin; j != prop_end; ++j) {
    // consume old style options
    if (*j == "ABSTRACT" || *j == "GENERATED" || *j == "WRAP_EXCLUDE") {
      propertyPairs.emplace_back(*j);
      propertyPairs.emplace_back("1");
    } else if (*j == "COMPILE_FLAGS") {
      propertyPairs.emplace_back("COMPILE_FLAGS");
      ++j;
      if (j == prop_end) {
        errors = "called with incorrect number of arguments "
                 "COMPILE_FLAGS with no flags";
        return false;
      }
      propertyPairs.push_back(*j);
    } else if (*j == "OBJECT_DEPENDS") {
      propertyPairs.emplace_back("OBJECT_DEPENDS");
      ++j;
      if (j == prop_end) {
        errors = "called with incorrect number of arguments "
                 "OBJECT_DEPENDS with no dependencies";
        return false;
      }
      propertyPairs.push_back(*j);
    } else if (*j == "PROPERTIES") {
      // PROPERTIES is followed by new style prop value pairs
      cmStringRange newStyleProps{ j + 1, prop_end };
      if (newStyleProps.size() % 2 != 0) {
        errors = "called with incorrect number of arguments.";
        return false;
      }
      // set newStyleProps as is.
      cm::append(propertyPairs, newStyleProps);
      // break out of the loop.
      break;
    } else {
      errors = "called with illegal arguments, maybe missing a "
               "PROPERTIES specifier?";
      return false;
    }
  }

  // loop over all the files
  for (const std::string& sfname : cmStringRange{ file_begin, file_end }) {
    // get the source file
    if (cmSourceFile* sf = mf->GetOrCreateSource(sfname)) {
      // loop through the props and set them
      for (auto k = propertyPairs.begin(); k != propertyPairs.end(); k += 2) {
        // Special handling for GENERATED property?
        if (*k == "GENERATED"_s) {
          SetPropertyCommand::HandleAndValidateSourceFilePropertyGENERATED(
            sf, *(k + 1));
        } else {
          sf->SetProperty(*k, *(k + 1));
        }
      }
    }
  }
  return true;
}
