/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmMakefile;
class cmExecutionStatus;
class cmSourceFile;

bool cmSetPropertyCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status);

namespace SetPropertyCommand {
bool HandleSourceFileDirectoryScopes(
  cmExecutionStatus& status, std::vector<std::string>& source_file_directories,
  std::vector<std::string>& source_file_target_directories,
  std::vector<cmMakefile*>& directory_makefiles);

bool HandleSourceFileDirectoryScopeValidation(
  cmExecutionStatus& status, bool source_file_directory_option_enabled,
  bool source_file_target_option_enabled,
  std::vector<std::string>& source_file_directories,
  std::vector<std::string>& source_file_target_directories);

bool HandleAndValidateSourceFileDirectoryScopes(
  cmExecutionStatus& status, bool source_directories_option_encountered,
  bool source_target_directories_option_encountered,
  std::vector<std::string>& source_directories,
  std::vector<std::string>& source_target_directories,
  std::vector<cmMakefile*>& source_file_directory_makefiles);

std::string MakeSourceFilePathAbsoluteIfNeeded(
  cmExecutionStatus& status, const std::string& source_file_path, bool needed);
void MakeSourceFilePathsAbsoluteIfNeeded(
  cmExecutionStatus& status,
  std::vector<std::string>& source_files_absolute_paths,
  std::vector<std::string>::const_iterator files_it_begin,
  std::vector<std::string>::const_iterator files_it_end, bool needed);

enum class PropertyOp
{
  Remove,
  Set,
  Append,
  AppendAsString
};

bool HandleAndValidateSourceFilePropertyGENERATED(
  cmSourceFile* sf, std::string const& propertyValue,
  PropertyOp op = PropertyOp::Set);
}
