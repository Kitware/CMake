/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmWIXAccessControlList.h"

#include <cm/string_view>

#include "cmCPackGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmWIXAccessControlList::cmWIXAccessControlList(
  cmCPackLog* logger, cmInstalledFile const& installedFile,
  cmWIXSourceWriter& sourceWriter)
  : Logger(logger)
  , InstalledFile(installedFile)
  , SourceWriter(sourceWriter)
{
}

bool cmWIXAccessControlList::Apply()
{
  auto entries = this->InstalledFile.GetPropertyAsList("CPACK_WIX_ACL");

  for (auto const& entry : entries) {
    this->CreatePermissionElement(entry);
  }

  return true;
}

void cmWIXAccessControlList::CreatePermissionElement(std::string const& entry)
{
  std::string::size_type pos = entry.find('=');
  if (pos == std::string::npos) {
    this->ReportError(entry, "Did not find mandatory '='");
    return;
  }

  cm::string_view enview(entry);
  cm::string_view user_and_domain = enview.substr(0, pos);
  cm::string_view permission_string = enview.substr(pos + 1);

  pos = user_and_domain.find('@');
  cm::string_view user;
  cm::string_view domain;
  if (pos != std::string::npos) {
    user = user_and_domain.substr(0, pos);
    domain = user_and_domain.substr(pos + 1);
  } else {
    user = user_and_domain;
  }

  std::vector<std::string> permissions = cmTokenize(permission_string, ",");

  this->SourceWriter.BeginElement("Permission");
  this->SourceWriter.AddAttribute("User", std::string(user));
  if (!domain.empty()) {
    this->SourceWriter.AddAttribute("Domain", std::string(domain));
  }
  for (std::string const& permission : permissions) {
    this->EmitBooleanAttribute(entry, cmTrimWhitespace(permission));
  }
  this->SourceWriter.EndElement("Permission");
}

void cmWIXAccessControlList::ReportError(std::string const& entry,
                                         std::string const& message)
{
  cmCPackLogger(cmCPackLog::LOG_ERROR,
                "Failed processing ACL entry '" << entry << "': " << message
                                                << std::endl);
}

bool cmWIXAccessControlList::IsBooleanAttribute(std::string const& name)
{
  static const char* validAttributes[] = {
    /* clang-format needs this comment to break after the opening brace */
    "Append",
    "ChangePermission",
    "CreateChild",
    "CreateFile",
    "CreateLink",
    "CreateSubkeys",
    "Delete",
    "DeleteChild",
    "EnumerateSubkeys",
    "Execute",
    "FileAllRights",
    "GenericAll",
    "GenericExecute",
    "GenericRead",
    "GenericWrite",
    "Notify",
    "Read",
    "ReadAttributes",
    "ReadExtendedAttributes",
    "ReadPermission",
    "SpecificRightsAll",
    "Synchronize",
    "TakeOwnership",
    "Traverse",
    "Write",
    "WriteAttributes",
    "WriteExtendedAttributes",
    0
  };

  size_t i = 0;
  while (validAttributes[i]) {
    if (name == validAttributes[i++])
      return true;
  }

  return false;
}

void cmWIXAccessControlList::EmitBooleanAttribute(std::string const& entry,
                                                  std::string const& name)
{
  if (!this->IsBooleanAttribute(name)) {
    std::ostringstream message;
    message << "Unknown boolean attribute '" << name << "'";
    this->ReportError(entry, message.str());
  }

  this->SourceWriter.AddAttribute(name, "yes");
}
