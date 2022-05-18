/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallCommandArguments.h"

#include <algorithm>
#include <utility>

#include <cmext/string_view>

#include "cmRange.h"
#include "cmSystemTools.h"

// Table of valid permissions.
const char* cmInstallCommandArguments::PermissionsTable[] = {
  "OWNER_READ",    "OWNER_WRITE",   "OWNER_EXECUTE", "GROUP_READ",
  "GROUP_WRITE",   "GROUP_EXECUTE", "WORLD_READ",    "WORLD_WRITE",
  "WORLD_EXECUTE", "SETUID",        "SETGID",        nullptr
};

const std::string cmInstallCommandArguments::EmptyString;

cmInstallCommandArguments::cmInstallCommandArguments(
  std::string defaultComponent)
  : DefaultComponentName(std::move(defaultComponent))
{
  this->Bind("DESTINATION"_s, this->Destination);
  this->Bind("COMPONENT"_s, this->Component);
  this->Bind("NAMELINK_COMPONENT"_s, this->NamelinkComponent);
  this->Bind("EXCLUDE_FROM_ALL"_s, this->ExcludeFromAll);
  this->Bind("RENAME"_s, this->Rename);
  this->Bind("PERMISSIONS"_s, this->Permissions);
  this->Bind("CONFIGURATIONS"_s, this->Configurations);
  this->Bind("OPTIONAL"_s, this->Optional);
  this->Bind("NAMELINK_ONLY"_s, this->NamelinkOnly);
  this->Bind("NAMELINK_SKIP"_s, this->NamelinkSkip);
  this->Bind("TYPE"_s, this->Type);
}

const std::string& cmInstallCommandArguments::GetDestination() const
{
  if (!this->DestinationString.empty()) {
    return this->DestinationString;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetDestination();
  }
  return EmptyString;
}

const std::string& cmInstallCommandArguments::GetComponent() const
{
  if (!this->Component.empty()) {
    return this->Component;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetComponent();
  }
  if (!this->DefaultComponentName.empty()) {
    return this->DefaultComponentName;
  }
  static std::string unspecifiedComponent = "Unspecified";
  return unspecifiedComponent;
}

const std::string& cmInstallCommandArguments::GetNamelinkComponent() const
{
  if (!this->NamelinkComponent.empty()) {
    return this->NamelinkComponent;
  }
  return this->GetComponent();
}

const std::string& cmInstallCommandArguments::GetRename() const
{
  if (!this->Rename.empty()) {
    return this->Rename;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetRename();
  }
  return EmptyString;
}

const std::string& cmInstallCommandArguments::GetPermissions() const
{
  if (!this->PermissionsString.empty()) {
    return this->PermissionsString;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetPermissions();
  }
  return EmptyString;
}

bool cmInstallCommandArguments::GetOptional() const
{
  if (this->Optional) {
    return true;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetOptional();
  }
  return false;
}

bool cmInstallCommandArguments::GetExcludeFromAll() const
{
  if (this->ExcludeFromAll) {
    return true;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetExcludeFromAll();
  }
  return false;
}

bool cmInstallCommandArguments::GetNamelinkOnly() const
{
  if (this->NamelinkOnly) {
    return true;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetNamelinkOnly();
  }
  return false;
}

bool cmInstallCommandArguments::GetNamelinkSkip() const
{
  if (this->NamelinkSkip) {
    return true;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetNamelinkSkip();
  }
  return false;
}

bool cmInstallCommandArguments::HasNamelinkComponent() const
{
  if (!this->NamelinkComponent.empty()) {
    return true;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->HasNamelinkComponent();
  }
  return false;
}

const std::string& cmInstallCommandArguments::GetType() const
{
  return this->Type;
}

const std::string& cmInstallCommandArguments::GetDefaultComponent() const
{
  return this->DefaultComponentName;
}

const std::vector<std::string>& cmInstallCommandArguments::GetConfigurations()
  const
{
  if (!this->Configurations.empty()) {
    return this->Configurations;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetConfigurations();
  }
  return this->Configurations;
}

bool cmInstallCommandArguments::Finalize()
{
  if (!this->CheckPermissions()) {
    return false;
  }
  this->DestinationString = this->Destination;
  cmSystemTools::ConvertToUnixSlashes(this->DestinationString);
  return true;
}

bool cmInstallCommandArguments::CheckPermissions()
{
  this->PermissionsString.clear();
  return std::all_of(this->Permissions.begin(), this->Permissions.end(),
                     [this](std::string const& perm) -> bool {
                       return cmInstallCommandArguments::CheckPermissions(
                         perm, this->PermissionsString);
                     });
}

bool cmInstallCommandArguments::CheckPermissions(
  const std::string& onePermission, std::string& permissions)
{
  // Check the permission against the table.
  for (const char** valid = cmInstallCommandArguments::PermissionsTable;
       *valid; ++valid) {
    if (onePermission == *valid) {
      // This is a valid permission.
      permissions += " ";
      permissions += onePermission;
      return true;
    }
  }
  // This is not a valid permission.
  return false;
}

cmInstallCommandIncludesArgument::cmInstallCommandIncludesArgument() = default;

const std::vector<std::string>&
cmInstallCommandIncludesArgument::GetIncludeDirs() const
{
  return this->IncludeDirs;
}

void cmInstallCommandIncludesArgument::Parse(
  const std::vector<std::string>* args, std::vector<std::string>*)
{
  if (args->empty()) {
    return;
  }
  for (std::string dir : cmMakeRange(*args).advance(1)) {
    cmSystemTools::ConvertToUnixSlashes(dir);
    this->IncludeDirs.push_back(std::move(dir));
  }
}

cmInstallCommandFileSetArguments::cmInstallCommandFileSetArguments(
  std::string defaultComponent)
  : cmInstallCommandArguments(std::move(defaultComponent))
{
  this->Bind("FILE_SET"_s, this->FileSet);
}

void cmInstallCommandFileSetArguments::Parse(
  std::vector<std::string> args, std::vector<std::string>* unconsumedArgs)
{
  args.insert(args.begin(), "FILE_SET");
  this->cmInstallCommandArguments::Parse(args, unconsumedArgs);
}
