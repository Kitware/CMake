/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackIFWRepository.h"

#include <cstddef>

#include "cmCPackIFWGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmXMLParser.h"
#include "cmXMLWriter.h"

cmCPackIFWRepository::cmCPackIFWRepository()
  : Update(cmCPackIFWRepository::None)
{
}

bool cmCPackIFWRepository::IsValid() const
{
  bool valid = true;

  switch (this->Update) {
    case cmCPackIFWRepository::None:
    case cmCPackIFWRepository::Add:
    case cmCPackIFWRepository::Remove:
      valid = !this->Url.empty();
      break;
    case cmCPackIFWRepository::Replace:
      valid = !this->OldUrl.empty() && !this->NewUrl.empty();
      break;
  }

  return valid;
}

bool cmCPackIFWRepository::ConfigureFromOptions()
{
  // Name;
  if (this->Name.empty()) {
    return false;
  }

  std::string prefix =
    "CPACK_IFW_REPOSITORY_" + cmsys::SystemTools::UpperCase(this->Name) + "_";

  // Update
  if (this->IsOn(prefix + "ADD")) {
    this->Update = cmCPackIFWRepository::Add;
  } else if (this->IsOn(prefix + "REMOVE")) {
    this->Update = cmCPackIFWRepository::Remove;
  } else if (this->IsOn(prefix + "REPLACE")) {
    this->Update = cmCPackIFWRepository::Replace;
  } else {
    this->Update = cmCPackIFWRepository::None;
  }

  // Url
  if (cmValue url = this->GetOption(prefix + "URL")) {
    this->Url = *url;
  } else {
    this->Url.clear();
  }

  // Old url
  if (cmValue oldUrl = this->GetOption(prefix + "OLD_URL")) {
    this->OldUrl = *oldUrl;
  } else {
    this->OldUrl.clear();
  }

  // New url
  if (cmValue newUrl = this->GetOption(prefix + "NEW_URL")) {
    this->NewUrl = *newUrl;
  } else {
    this->NewUrl.clear();
  }

  // Enabled
  if (this->IsOn(prefix + "DISABLED")) {
    this->Enabled = "0";
  } else {
    this->Enabled.clear();
  }

  // Username
  if (cmValue username = this->GetOption(prefix + "USERNAME")) {
    this->Username = *username;
  } else {
    this->Username.clear();
  }

  // Password
  if (cmValue password = this->GetOption(prefix + "PASSWORD")) {
    this->Password = *password;
  } else {
    this->Password.clear();
  }

  // DisplayName
  if (cmValue displayName = this->GetOption(prefix + "DISPLAY_NAME")) {
    this->DisplayName = *displayName;
  } else {
    this->DisplayName.clear();
  }

  return this->IsValid();
}

/** \class cmCPackeIFWUpdatesPatcher
 * \brief Helper class that parses and patch Updates.xml file (QtIFW)
 */
class cmCPackeIFWUpdatesPatcher : public cmXMLParser
{
public:
  cmCPackeIFWUpdatesPatcher(cmCPackIFWRepository* r, cmXMLWriter& x)
    : repository(r)
    , xout(x)
  {
  }

  cmCPackIFWRepository* repository;
  cmXMLWriter& xout;
  bool patched = false;

protected:
  void StartElement(const std::string& name, const char** atts) override
  {
    this->xout.StartElement(name);
    this->StartFragment(atts);
  }

  void StartFragment(const char** atts)
  {
    for (size_t i = 0; atts[i]; i += 2) {
      const char* key = atts[i];
      const char* value = atts[i + 1];
      this->xout.Attribute(key, value);
    }
  }

  void EndElement(const std::string& name) override
  {
    if (name == "Updates" && !this->patched) {
      this->repository->WriteRepositoryUpdates(this->xout);
      this->patched = true;
    }
    this->xout.EndElement();
    if (this->patched) {
      return;
    }
    if (name == "Checksum") {
      this->repository->WriteRepositoryUpdates(this->xout);
      this->patched = true;
    }
  }

  void CharacterDataHandler(const char* data, int length) override
  {
    std::string content(data, data + length);
    if (content.empty() || content == " " || content == "  " ||
        content == "\n") {
      return;
    }
    this->xout.Content(content);
  }
};

bool cmCPackIFWRepository::PatchUpdatesXml()
{
  // Lazy directory initialization
  if (this->Directory.empty() && this->Generator) {
    this->Directory = this->Generator->toplevel;
  }

  // Filenames
  std::string updatesXml = this->Directory + "/repository/Updates.xml";
  std::string updatesPatchXml =
    this->Directory + "/repository/UpdatesPatch.xml";

  // Output stream
  cmGeneratedFileStream fout(updatesPatchXml);
  cmXMLWriter xout(fout);

  xout.StartDocument();

  this->WriteGeneratedByToStrim(xout);

  // Patch
  {
    cmCPackeIFWUpdatesPatcher patcher(this, xout);
    patcher.ParseFile(updatesXml.data());
  }

  xout.EndDocument();

  fout.Close();

  return cmSystemTools::RenameFile(updatesPatchXml, updatesXml);
}

void cmCPackIFWRepository::WriteRepositoryConfig(cmXMLWriter& xout) const
{
  xout.StartElement("Repository");

  // Url
  xout.Element("Url", this->Url);
  // Enabled
  if (!this->Enabled.empty()) {
    xout.Element("Enabled", this->Enabled);
  }
  // Username
  if (!this->Username.empty()) {
    xout.Element("Username", this->Username);
  }
  // Password
  if (!this->Password.empty()) {
    xout.Element("Password", this->Password);
  }
  // DisplayName
  if (!this->DisplayName.empty()) {
    xout.Element("DisplayName", this->DisplayName);
  }

  xout.EndElement();
}

void cmCPackIFWRepository::WriteRepositoryUpdate(cmXMLWriter& xout) const
{
  xout.StartElement("Repository");

  switch (this->Update) {
    case cmCPackIFWRepository::None:
      break;
    case cmCPackIFWRepository::Add:
      xout.Attribute("action", "add");
      break;
    case cmCPackIFWRepository::Remove:
      xout.Attribute("action", "remove");
      break;
    case cmCPackIFWRepository::Replace:
      xout.Attribute("action", "replace");
      break;
  }

  // Url
  if (this->Update == cmCPackIFWRepository::Add ||
      this->Update == cmCPackIFWRepository::Remove) {
    xout.Attribute("url", this->Url);
  } else if (this->Update == cmCPackIFWRepository::Replace) {
    xout.Attribute("oldUrl", this->OldUrl);
    xout.Attribute("newUrl", this->NewUrl);
  }
  // Enabled
  if (!this->Enabled.empty()) {
    xout.Attribute("enabled", this->Enabled);
  }
  // Username
  if (!this->Username.empty()) {
    xout.Attribute("username", this->Username);
  }
  // Password
  if (!this->Password.empty()) {
    xout.Attribute("password", this->Password);
  }
  // DisplayName
  if (!this->DisplayName.empty()) {
    xout.Attribute("displayname", this->DisplayName);
  }

  xout.EndElement();
}

void cmCPackIFWRepository::WriteRepositoryUpdates(cmXMLWriter& xout)
{
  if (!this->RepositoryUpdate.empty()) {
    xout.StartElement("RepositoryUpdate");
    for (cmCPackIFWRepository* r : this->RepositoryUpdate) {
      r->WriteRepositoryUpdate(xout);
    }
    xout.EndElement();
  }
}
