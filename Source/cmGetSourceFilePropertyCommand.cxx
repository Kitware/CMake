/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGetSourceFilePropertyCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSetPropertyCommand.h"
#include "cmSourceFile.h"
#include "cmValue.h"

bool cmGetSourceFilePropertyCommand(std::vector<std::string> const& args,
                                    cmExecutionStatus& status)
{
  std::vector<std::string>::size_type args_size = args.size();
  if (args_size != 3 && args_size != 5) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  std::vector<std::string> source_file_directories;
  std::vector<std::string> source_file_target_directories;
  bool source_file_directory_option_enabled = false;
  bool source_file_target_option_enabled = false;

  int property_arg_index = 2;
  if (args[2] == "DIRECTORY" && args_size == 5) {
    property_arg_index = 4;
    source_file_directory_option_enabled = true;
    source_file_directories.push_back(args[3]);
  } else if (args[2] == "TARGET_DIRECTORY" && args_size == 5) {
    property_arg_index = 4;
    source_file_target_option_enabled = true;
    source_file_target_directories.push_back(args[3]);
  }

  std::vector<cmMakefile*> source_file_directory_makefiles;
  bool file_scopes_handled =
    SetPropertyCommand::HandleAndValidateSourceFileDirectoryScopes(
      status, source_file_directory_option_enabled,
      source_file_target_option_enabled, source_file_directories,
      source_file_target_directories, source_file_directory_makefiles);
  if (!file_scopes_handled) {
    return false;
  }

  std::string const& var = args[0];
  bool source_file_paths_should_be_absolute =
    source_file_directory_option_enabled || source_file_target_option_enabled;
  std::string const file =
    SetPropertyCommand::MakeSourceFilePathAbsoluteIfNeeded(
      status, args[1], source_file_paths_should_be_absolute);
  cmMakefile& mf = *source_file_directory_makefiles[0];
  cmSourceFile* sf = mf.GetSource(file);

  // for the location we must create a source file first
  if (!sf && args[property_arg_index] == "LOCATION") {
    sf = mf.CreateSource(file);
  }

  if (sf) {
    cmValue prop = nullptr;
    if (!args[property_arg_index].empty()) {
      prop = sf->GetPropertyForUser(args[property_arg_index]);
    }
    if (prop) {
      // Set the value on the original Makefile scope, not the scope of the
      // requested directory.
      status.GetMakefile().AddDefinition(var, *prop);
      return true;
    }
  }

  status.GetMakefile().AddDefinition(var, "NOTFOUND");
  return true;
}
