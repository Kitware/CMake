/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGetSourceFilePropertyCommand.h"

#include <cm/string_view>
#include <cmext/string_view>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSourceFilePropertyHelper.h"
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
  if (args[2] == "DIRECTORY"_s && args_size == 5) {
    property_arg_index = 4;
    source_file_directory_option_enabled = true;
    source_file_directories.push_back(args[3]);
  } else if (args[2] == "TARGET_DIRECTORY"_s && args_size == 5) {
    property_arg_index = 4;
    source_file_target_option_enabled = true;
    source_file_target_directories.push_back(args[3]);
  }

  std::string const& var = args[0];
  std::string const& sourceName = args[1];
  std::string const& propName = args[property_arg_index];

  cmValue prop;
  auto result = cmGetSourceFileProperty(
    sourceName, propName, status, source_file_directory_option_enabled,
    source_file_target_option_enabled, source_file_directories,
    source_file_target_directories, prop);

  if (result == cmGetSourceFilePropertyResult::Success) {
    if (prop) {
      status.GetMakefile().AddDefinition(var, *prop);
      return true;
    }
    status.GetMakefile().AddDefinition(var, "NOTFOUND");
    return true;
  }
  if (result == cmGetSourceFilePropertyResult::SourceNotFound ||
      result == cmGetSourceFilePropertyResult::Error) {
    status.GetMakefile().AddDefinition(var, "NOTFOUND");
    return true;
  }
  return false;
}
