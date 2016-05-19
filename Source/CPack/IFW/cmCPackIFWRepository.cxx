/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackIFWRepository.h"

#include "cmCPackIFWGenerator.h"

#include <CPack/cmCPackLog.h>

#include <cmGeneratedFileStream.h>
#include <cmXMLParser.h>
#include <cmXMLWriter.h>

#ifdef cmCPackLogger
#undef cmCPackLogger
#endif
#define cmCPackLogger(logType, msg)                                           \
  do {                                                                        \
    std::ostringstream cmCPackLog_msg;                                        \
    cmCPackLog_msg << msg;                                                    \
    if (Generator) {                                                          \
      Generator->Logger->Log(logType, __FILE__, __LINE__,                     \
                             cmCPackLog_msg.str().c_str());                   \
    }                                                                         \
  } while (0)

cmCPackIFWRepository::cmCPackIFWRepository()
  : Update(None)
  , Generator(0)
{
}

bool cmCPackIFWRepository::IsValid() const
{
  bool valid = true;

  switch (Update) {
    case None:
      valid = Url.empty() ? false : true;
      break;
    case Add:
      valid = Url.empty() ? false : true;
      break;
    case Remove:
      valid = Url.empty() ? false : true;
      break;
    case Replace:
      valid = (OldUrl.empty() || NewUrl.empty()) ? false : true;
      break;
  }

  return valid;
}

const char* cmCPackIFWRepository::GetOption(const std::string& op) const
{
  return Generator ? Generator->GetOption(op) : 0;
}

bool cmCPackIFWRepository::IsOn(const std::string& op) const
{
  return Generator ? Generator->IsOn(op) : false;
}

bool cmCPackIFWRepository::IsVersionLess(const char* version)
{
  return Generator ? Generator->IsVersionLess(version) : false;
}

bool cmCPackIFWRepository::IsVersionGreater(const char* version)
{
  return Generator ? Generator->IsVersionGreater(version) : false;
}

bool cmCPackIFWRepository::IsVersionEqual(const char* version)
{
  return Generator ? Generator->IsVersionEqual(version) : false;
}

bool cmCPackIFWRepository::ConfigureFromOptions()
{
  // Name;
  if (Name.empty())
    return false;

  std::string prefix =
    "CPACK_IFW_REPOSITORY_" + cmsys::SystemTools::UpperCase(Name) + "_";

  // Update
  if (IsOn(prefix + "ADD")) {
    Update = Add;
  } else if (IsOn(prefix + "REMOVE")) {
    Update = Remove;
  } else if (IsOn(prefix + "REPLACE")) {
    Update = Replace;
  } else {
    Update = None;
  }

  // Url
  if (const char* url = GetOption(prefix + "URL")) {
    Url = url;
  } else {
    Url = "";
  }

  // Old url
  if (const char* oldUrl = GetOption(prefix + "OLD_URL")) {
    OldUrl = oldUrl;
  } else {
    OldUrl = "";
  }

  // New url
  if (const char* newUrl = GetOption(prefix + "NEW_URL")) {
    NewUrl = newUrl;
  } else {
    NewUrl = "";
  }

  // Enabled
  if (IsOn(prefix + "DISABLED")) {
    Enabled = "0";
  } else {
    Enabled = "";
  }

  // Username
  if (const char* username = GetOption(prefix + "USERNAME")) {
    Username = username;
  } else {
    Username = "";
  }

  // Password
  if (const char* password = GetOption(prefix + "PASSWORD")) {
    Password = password;
  } else {
    Password = "";
  }

  // DisplayName
  if (const char* displayName = GetOption(prefix + "DISPLAY_NAME")) {
    DisplayName = displayName;
  } else {
    DisplayName = "";
  }

  return IsValid();
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
    , patched(false)
  {
  }

  cmCPackIFWRepository* repository;
  cmXMLWriter& xout;
  bool patched;

protected:
  virtual void StartElement(const std::string& name, const char** atts)
  {
    xout.StartElement(name);
    StartFragment(atts);
  }

  void StartFragment(const char** atts)
  {
    for (size_t i = 0; atts[i]; i += 2) {
      const char* key = atts[i];
      const char* value = atts[i + 1];
      xout.Attribute(key, value);
    }
  }

  virtual void EndElement(const std::string& name)
  {
    if (name == "Updates" && !patched) {
      repository->WriteRepositoryUpdates(xout);
      patched = true;
    }
    xout.EndElement();
    if (patched)
      return;
    if (name == "Checksum") {
      repository->WriteRepositoryUpdates(xout);
      patched = true;
    }
  }

  virtual void CharacterDataHandler(const char* data, int length)
  {
    std::string content(data, data + length);
    if (content == "" || content == " " || content == "  " || content == "\n")
      return;
    xout.Content(content);
  }
};

bool cmCPackIFWRepository::PatchUpdatesXml()
{
  // Lazy directory initialization
  if (Directory.empty() && Generator) {
    Directory = Generator->toplevel;
  }

  // Filenames
  std::string updatesXml = Directory + "/repository/Updates.xml";
  std::string updatesPatchXml = Directory + "/repository/UpdatesPatch.xml";

  // Output stream
  cmGeneratedFileStream fout(updatesPatchXml.data());
  cmXMLWriter xout(fout);

  xout.StartDocument();

  WriteGeneratedByToStrim(xout);

  // Patch
  {
    cmCPackeIFWUpdatesPatcher patcher(this, xout);
    patcher.ParseFile(updatesXml.data());
  }

  xout.EndDocument();

  fout.Close();

  if (!cmSystemTools::RenameFile(updatesPatchXml.data(), updatesXml.data())) {
    return false;
  }

  return true;
}

void cmCPackIFWRepository::WriteRepositoryConfig(cmXMLWriter& xout)
{
  xout.StartElement("Repository");

  // Url
  xout.Element("Url", Url);
  // Enabled
  if (!Enabled.empty()) {
    xout.Element("Enabled", Enabled);
  }
  // Username
  if (!Username.empty()) {
    xout.Element("Username", Username);
  }
  // Password
  if (!Password.empty()) {
    xout.Element("Password", Password);
  }
  // DisplayName
  if (!DisplayName.empty()) {
    xout.Element("DisplayName", DisplayName);
  }

  xout.EndElement();
}

void cmCPackIFWRepository::WriteRepositoryUpdate(cmXMLWriter& xout)
{
  xout.StartElement("Repository");

  switch (Update) {
    case None:
      break;
    case Add:
      xout.Attribute("action", "add");
      break;
    case Remove:
      xout.Attribute("action", "remove");
      break;
    case Replace:
      xout.Attribute("action", "replace");
      break;
  }

  // Url
  if (Update == Add || Update == Remove) {
    xout.Attribute("url", Url);
  } else if (Update == Replace) {
    xout.Attribute("oldurl", OldUrl);
    xout.Attribute("newurl", NewUrl);
  }
  // Enabled
  if (!Enabled.empty()) {
    xout.Attribute("enabled", Enabled);
  }
  // Username
  if (!Username.empty()) {
    xout.Attribute("username", Username);
  }
  // Password
  if (!Password.empty()) {
    xout.Attribute("password", Password);
  }
  // DisplayName
  if (!DisplayName.empty()) {
    xout.Attribute("displayname", DisplayName);
  }

  xout.EndElement();
}

void cmCPackIFWRepository::WriteRepositoryUpdates(cmXMLWriter& xout)
{
  if (!RepositoryUpdate.empty()) {
    xout.StartElement("RepositoryUpdate");
    for (RepositoriesVector::iterator rit = RepositoryUpdate.begin();
         rit != RepositoryUpdate.end(); ++rit) {
      (*rit)->WriteRepositoryUpdate(xout);
    }
    xout.EndElement();
  }
}

void cmCPackIFWRepository::WriteGeneratedByToStrim(cmXMLWriter& xout)
{
  if (Generator)
    Generator->WriteGeneratedByToStrim(xout);
}
