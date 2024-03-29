/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGetSourceFilePropertyCommand.h"

#include <functional>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmPolicies.h"
#include "cmSetPropertyCommand.h"
#include "cmSourceFile.h"
#include "cmValue.h"

namespace GetPropertyCommand {
bool GetSourceFilePropertyGENERATED(
  const std::string& name, cmMakefile& mf,
  const std::function<bool(bool)>& storeResult);
}

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
  std::string const& propName = args[property_arg_index];
  bool source_file_paths_should_be_absolute =
    source_file_directory_option_enabled || source_file_target_option_enabled;
  cmMakefile& directory_makefile = *source_file_directory_makefiles[0];

  // Special handling for GENERATED property.
  // Note: Only, if CMP0163 is set to NEW!
  if (propName == "GENERATED"_s) {
    auto& mf = status.GetMakefile();
    auto cmp0163 = directory_makefile.GetPolicyStatus(cmPolicies::CMP0163);
    bool const cmp0163new =
      cmp0163 != cmPolicies::OLD && cmp0163 != cmPolicies::WARN;
    if (cmp0163new) {
      return GetPropertyCommand::GetSourceFilePropertyGENERATED(
        args[1], mf, [&var, &mf](bool isGenerated) -> bool {
          // Set the value on the original Makefile scope, not the scope of the
          // requested directory.
          mf.AddDefinition(var, (isGenerated) ? cmValue("1") : cmValue("0"));
          return true;
        });
    }
  }

  // Get the source file.
  std::string const file =
    SetPropertyCommand::MakeSourceFilePathAbsoluteIfNeeded(
      status, args[1], source_file_paths_should_be_absolute);
  cmSourceFile* sf = directory_makefile.GetSource(file);

  // for the location we must create a source file first
  if (!sf && propName == "LOCATION"_s) {
    sf = directory_makefile.CreateSource(file);
  }

  if (sf) {
    cmValue prop = nullptr;
    if (!propName.empty()) {
      prop = sf->GetPropertyForUser(propName);
    }
    if (prop) {
      // Set the value on the original Makefile scope, not the scope of the
      // requested directory.
      status.GetMakefile().AddDefinition(var, *prop);
      return true;
    }
  }

  // Set the value on the original Makefile scope, not the scope of the
  // requested directory.
  status.GetMakefile().AddDefinition(var, "NOTFOUND");
  return true;
}
