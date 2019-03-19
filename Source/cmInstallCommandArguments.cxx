/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallCommandArguments.h"

#include "cmRange.h"
#include "cmSystemTools.h"

#include <utility>

// Table of valid permissions.
const char* cmInstallCommandArguments::PermissionsTable[] = {
  "OWNER_READ",    "OWNER_WRITE",   "OWNER_EXECUTE", "GROUP_READ",
  "GROUP_WRITE",   "GROUP_EXECUTE", "WORLD_READ",    "WORLD_WRITE",
  "WORLD_EXECUTE", "SETUID",        "SETGID",        nullptr
};

const std::string cmInstallCommandArguments::EmptyString;

cmInstallCommandArguments::cmInstallCommandArguments(
  std::string defaultComponent)
  : Destination(&Parser, "DESTINATION", &ArgumentGroup)
  , Component(&Parser, "COMPONENT", &ArgumentGroup)
  , NamelinkComponent(&Parser, "NAMELINK_COMPONENT", &ArgumentGroup)
  , ExcludeFromAll(&Parser, "EXCLUDE_FROM_ALL", &ArgumentGroup)
  , Rename(&Parser, "RENAME", &ArgumentGroup)
  , Permissions(&Parser, "PERMISSIONS", &ArgumentGroup)
  , Configurations(&Parser, "CONFIGURATIONS", &ArgumentGroup)
  , Optional(&Parser, "OPTIONAL", &ArgumentGroup)
  , NamelinkOnly(&Parser, "NAMELINK_ONLY", &ArgumentGroup)
  , NamelinkSkip(&Parser, "NAMELINK_SKIP", &ArgumentGroup)
  , Type(&Parser, "TYPE", &ArgumentGroup)
  , GenericArguments(nullptr)
  , DefaultComponentName(std::move(defaultComponent))
{
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
  if (!this->Component.GetString().empty()) {
    return this->Component.GetString();
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
  if (!this->NamelinkComponent.GetString().empty()) {
    return this->NamelinkComponent.GetString();
  }
  return this->GetComponent();
}

const std::string& cmInstallCommandArguments::GetRename() const
{
  if (!this->Rename.GetString().empty()) {
    return this->Rename.GetString();
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
  if (this->Optional.IsEnabled()) {
    return true;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetOptional();
  }
  return false;
}

bool cmInstallCommandArguments::GetExcludeFromAll() const
{
  if (this->ExcludeFromAll.IsEnabled()) {
    return true;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetExcludeFromAll();
  }
  return false;
}

bool cmInstallCommandArguments::GetNamelinkOnly() const
{
  if (this->NamelinkOnly.IsEnabled()) {
    return true;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetNamelinkOnly();
  }
  return false;
}

bool cmInstallCommandArguments::GetNamelinkSkip() const
{
  if (this->NamelinkSkip.IsEnabled()) {
    return true;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetNamelinkSkip();
  }
  return false;
}

bool cmInstallCommandArguments::HasNamelinkComponent() const
{
  if (!this->NamelinkComponent.GetString().empty()) {
    return true;
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->HasNamelinkComponent();
  }
  return false;
}

const std::string& cmInstallCommandArguments::GetType() const
{
  return this->Type.GetString();
}

const std::vector<std::string>& cmInstallCommandArguments::GetConfigurations()
  const
{
  if (!this->Configurations.GetVector().empty()) {
    return this->Configurations.GetVector();
  }
  if (this->GenericArguments != nullptr) {
    return this->GenericArguments->GetConfigurations();
  }
  return this->Configurations.GetVector();
}

bool cmInstallCommandArguments::Finalize()
{
  if (!this->CheckPermissions()) {
    return false;
  }
  this->DestinationString = this->Destination.GetString();
  cmSystemTools::ConvertToUnixSlashes(this->DestinationString);
  return true;
}

void cmInstallCommandArguments::Parse(const std::vector<std::string>* args,
                                      std::vector<std::string>* unconsumedArgs)
{
  this->Parser.Parse(args, unconsumedArgs);
}

bool cmInstallCommandArguments::CheckPermissions()
{
  this->PermissionsString.clear();
  for (std::string const& perm : this->Permissions.GetVector()) {
    if (!cmInstallCommandArguments::CheckPermissions(
          perm, this->PermissionsString)) {
      return false;
    }
  }
  return true;
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
