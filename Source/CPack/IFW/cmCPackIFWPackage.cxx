/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackIFWPackage.h"

#include "CPack/cmCPackComponentGroup.h"
#include "CPack/cmCPackGenerator.h"
#include "CPack/cmCPackLog.h"
#include "cmCPackIFWGenerator.h"
#include "cmCPackIFWInstaller.h"
#include "cmGeneratedFileStream.h"
#include "cmSystemTools.h"
#include "cmTimestamp.h"
#include "cmXMLWriter.h"

#include <cmConfigure.h>
#include <map>
#include <sstream>
#include <stddef.h>

//----------------------------------------------------------------- Logger ---
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
  } while (false)

//---------------------------------------------------------- CompareStruct ---
cmCPackIFWPackage::CompareStruct::CompareStruct()
  : Type(CompareNone)
{
}

//------------------------------------------------------- DependenceStruct ---
cmCPackIFWPackage::DependenceStruct::DependenceStruct()
{
}

cmCPackIFWPackage::DependenceStruct::DependenceStruct(
  const std::string& dependence)
{
  // Search compare section
  size_t pos = std::string::npos;
  if ((pos = dependence.find("<=")) != std::string::npos) {
    Compare.Type = CompareLessOrEqual;
    Compare.Value = dependence.substr(pos + 2);
  } else if ((pos = dependence.find(">=")) != std::string::npos) {
    Compare.Type = CompareGreaterOrEqual;
    Compare.Value = dependence.substr(pos + 2);
  } else if ((pos = dependence.find('<')) != std::string::npos) {
    Compare.Type = CompareLess;
    Compare.Value = dependence.substr(pos + 1);
  } else if ((pos = dependence.find('=')) != std::string::npos) {
    Compare.Type = CompareEqual;
    Compare.Value = dependence.substr(pos + 1);
  } else if ((pos = dependence.find('>')) != std::string::npos) {
    Compare.Type = CompareGreater;
    Compare.Value = dependence.substr(pos + 1);
  } else if ((pos = dependence.find('-')) != std::string::npos) {
    Compare.Type = CompareNone;
    Compare.Value = dependence.substr(pos + 1);
  }
  size_t dashPos = dependence.find('-');
  if (dashPos != std::string::npos) {
    pos = dashPos;
  }
  Name = pos == std::string::npos ? dependence : dependence.substr(0, pos);
}

std::string cmCPackIFWPackage::DependenceStruct::NameWithCompare() const
{
  if (Compare.Type == CompareNone) {
    return Name;
  }

  std::string result = Name;

  if (Compare.Type != CompareNone || !Compare.Value.empty()) {
    result += "-";
  }

  if (Compare.Type == CompareLessOrEqual) {
    result += "<=";
  } else if (Compare.Type == CompareGreaterOrEqual) {
    result += ">=";
  } else if (Compare.Type == CompareLess) {
    result += "<";
  } else if (Compare.Type == CompareEqual) {
    result += "=";
  } else if (Compare.Type == CompareGreater) {
    result += ">";
  }

  result += Compare.Value;

  return result;
}

//------------------------------------------------------ cmCPackIFWPackage ---
cmCPackIFWPackage::cmCPackIFWPackage()
  : Generator(CM_NULLPTR)
  , Installer(CM_NULLPTR)
{
}

const char* cmCPackIFWPackage::GetOption(const std::string& op) const
{
  const char* option = Generator ? Generator->GetOption(op) : CM_NULLPTR;
  return option && *option ? option : CM_NULLPTR;
}

bool cmCPackIFWPackage::IsOn(const std::string& op) const
{
  return Generator ? Generator->IsOn(op) : false;
}

bool cmCPackIFWPackage::IsSetToOff(const std::string& op) const
{
  return Generator ? Generator->IsSetToOff(op) : false;
}

bool cmCPackIFWPackage::IsSetToEmpty(const std::string& op) const
{
  return Generator ? Generator->IsSetToEmpty(op) : false;
}

bool cmCPackIFWPackage::IsVersionLess(const char* version)
{
  return Generator ? Generator->IsVersionLess(version) : false;
}

bool cmCPackIFWPackage::IsVersionGreater(const char* version)
{
  return Generator ? Generator->IsVersionGreater(version) : false;
}

bool cmCPackIFWPackage::IsVersionEqual(const char* version)
{
  return Generator ? Generator->IsVersionEqual(version) : false;
}

std::string cmCPackIFWPackage::GetComponentName(cmCPackComponent* component)
{
  if (!component) {
    return "";
  }
  const char* option =
    GetOption("CPACK_IFW_COMPONENT_" +
              cmsys::SystemTools::UpperCase(component->Name) + "_NAME");
  return option ? option : component->Name;
}

void cmCPackIFWPackage::DefaultConfiguration()
{
  DisplayName = "";
  Description = "";
  Version = "";
  ReleaseDate = "";
  Script = "";
  Licenses.clear();
  UserInterfaces.clear();
  Translations.clear();
  SortingPriority = "";
  UpdateText = "";
  Default = "";
  Essential = "";
  Virtual = "";
  ForcedInstallation = "";
  RequiresAdminRights = "";
}

// Defaul configuration (all in one package)
int cmCPackIFWPackage::ConfigureFromOptions()
{
  // Restore defaul configuration
  DefaultConfiguration();

  // Name
  Name = Generator->GetRootPackageName();

  // Display name
  if (const char* option = this->GetOption("CPACK_PACKAGE_NAME")) {
    DisplayName = option;
  } else {
    DisplayName = "Your package";
  }

  // Description
  if (const char* option =
        this->GetOption("CPACK_PACKAGE_DESCRIPTION_SUMMARY")) {
    Description = option;
  } else {
    Description = "Your package description";
  }

  // Version
  if (const char* option = GetOption("CPACK_PACKAGE_VERSION")) {
    Version = option;
  } else {
    Version = "1.0.0";
  }

  ForcedInstallation = "true";

  return 1;
}

int cmCPackIFWPackage::ConfigureFromComponent(cmCPackComponent* component)
{
  if (!component) {
    return 0;
  }

  // Restore defaul configuration
  DefaultConfiguration();

  std::string prefix = "CPACK_IFW_COMPONENT_" +
    cmsys::SystemTools::UpperCase(component->Name) + "_";

  // Display name
  DisplayName = component->DisplayName;

  // Description
  Description = component->Description;

  // Version
  if (const char* optVERSION = GetOption(prefix + "VERSION")) {
    Version = optVERSION;
  } else if (const char* optPACKAGE_VERSION =
               GetOption("CPACK_PACKAGE_VERSION")) {
    Version = optPACKAGE_VERSION;
  } else {
    Version = "1.0.0";
  }

  // Script
  if (const char* option = GetOption(prefix + "SCRIPT")) {
    Script = option;
  }

  // User interfaces
  if (const char* option = this->GetOption(prefix + "USER_INTERFACES")) {
    UserInterfaces.clear();
    cmSystemTools::ExpandListArgument(option, UserInterfaces);
  }

  // CMake dependencies
  if (!component->Dependencies.empty()) {
    std::vector<cmCPackComponent*>::iterator dit;
    for (dit = component->Dependencies.begin();
         dit != component->Dependencies.end(); ++dit) {
      Dependencies.insert(Generator->ComponentPackages[*dit]);
    }
  }

  // Licenses
  if (const char* option = this->GetOption(prefix + "LICENSES")) {
    Licenses.clear();
    cmSystemTools::ExpandListArgument(option, Licenses);
    if (Licenses.size() % 2 != 0) {
      cmCPackLogger(
        cmCPackLog::LOG_WARNING, prefix
          << "LICENSES"
          << " should contain pairs of <display_name> and <file_path>."
          << std::endl);
      Licenses.clear();
    }
  }

  // Priority
  if (const char* option = this->GetOption(prefix + "PRIORITY")) {
    SortingPriority = option;
    cmCPackLogger(
      cmCPackLog::LOG_WARNING, "The \"PRIORITY\" option is set "
        << "for component \"" << component->Name << "\", but there option is "
        << "deprecated. Please use \"SORTING_PRIORITY\" option instead."
        << std::endl);
  }

  // Default
  Default = component->IsDisabledByDefault ? "false" : "true";

  // Essential
  if (this->IsOn(prefix + "ESSENTIAL")) {
    Essential = "true";
  }

  // Virtual
  Virtual = component->IsHidden ? "true" : "";

  // ForcedInstallation
  ForcedInstallation = component->IsRequired ? "true" : "false";

  return ConfigureFromPrefix(prefix);
}

int cmCPackIFWPackage::ConfigureFromGroup(cmCPackComponentGroup* group)
{
  if (!group) {
    return 0;
  }

  // Restore defaul configuration
  DefaultConfiguration();

  std::string prefix = "CPACK_IFW_COMPONENT_GROUP_" +
    cmsys::SystemTools::UpperCase(group->Name) + "_";

  DisplayName = group->DisplayName;
  Description = group->Description;

  // Version
  if (const char* optVERSION = GetOption(prefix + "VERSION")) {
    Version = optVERSION;
  } else if (const char* optPACKAGE_VERSION =
               GetOption("CPACK_PACKAGE_VERSION")) {
    Version = optPACKAGE_VERSION;
  } else {
    Version = "1.0.0";
  }

  // Script
  if (const char* option = GetOption(prefix + "SCRIPT")) {
    Script = option;
  }

  // User interfaces
  if (const char* option = this->GetOption(prefix + "USER_INTERFACES")) {
    UserInterfaces.clear();
    cmSystemTools::ExpandListArgument(option, UserInterfaces);
  }

  // Licenses
  if (const char* option = this->GetOption(prefix + "LICENSES")) {
    Licenses.clear();
    cmSystemTools::ExpandListArgument(option, Licenses);
    if (Licenses.size() % 2 != 0) {
      cmCPackLogger(
        cmCPackLog::LOG_WARNING, prefix
          << "LICENSES"
          << " should contain pairs of <display_name> and <file_path>."
          << std::endl);
      Licenses.clear();
    }
  }

  // Priority
  if (const char* option = this->GetOption(prefix + "PRIORITY")) {
    SortingPriority = option;
    cmCPackLogger(
      cmCPackLog::LOG_WARNING, "The \"PRIORITY\" option is set "
        << "for component group \"" << group->Name
        << "\", but there option is "
        << "deprecated. Please use \"SORTING_PRIORITY\" option instead."
        << std::endl);
  }

  return ConfigureFromPrefix(prefix);
}

int cmCPackIFWPackage::ConfigureFromGroup(const std::string& groupName)
{
  // Group configuration

  cmCPackComponentGroup group;
  std::string prefix =
    "CPACK_COMPONENT_GROUP_" + cmsys::SystemTools::UpperCase(groupName) + "_";

  if (const char* option = GetOption(prefix + "DISPLAY_NAME")) {
    group.DisplayName = option;
  } else {
    group.DisplayName = group.Name;
  }

  if (const char* option = GetOption(prefix + "DESCRIPTION")) {
    group.Description = option;
  }
  group.IsBold = IsOn(prefix + "BOLD_TITLE");
  group.IsExpandedByDefault = IsOn(prefix + "EXPANDED");

  // Package configuration

  group.Name = groupName;

  if (Generator) {
    Name = Generator->GetGroupPackageName(&group);
  } else {
    Name = group.Name;
  }

  return ConfigureFromGroup(&group);
}

// Common options for components and groups
int cmCPackIFWPackage::ConfigureFromPrefix(const std::string& prefix)
{
  // Temporary variable for full option name
  std::string option;

  // Display name
  option = prefix + "DISPLAY_NAME";
  if (IsSetToEmpty(option)) {
    DisplayName.clear();
  } else if (const char* value = GetOption(option)) {
    DisplayName = value;
  }

  // Description
  option = prefix + "DESCRIPTION";
  if (IsSetToEmpty(option)) {
    Description.clear();
  } else if (const char* value = GetOption(option)) {
    Description = value;
  }

  // Release date
  option = prefix + "RELEASE_DATE";
  if (IsSetToEmpty(option)) {
    ReleaseDate.clear();
  } else if (const char* value = GetOption(option)) {
    ReleaseDate = value;
  }

  // Sorting priority
  option = prefix + "SORTING_PRIORITY";
  if (IsSetToEmpty(option)) {
    SortingPriority.clear();
  } else if (const char* value = GetOption(option)) {
    SortingPriority = value;
  }

  // Update text
  option = prefix + "UPDATE_TEXT";
  if (IsSetToEmpty(option)) {
    UpdateText.clear();
  } else if (const char* value = GetOption(option)) {
    UpdateText = value;
  }

  // Translations
  option = prefix + "TRANSLATIONS";
  if (IsSetToEmpty(option)) {
    Translations.clear();
  } else if (const char* value = this->GetOption(option)) {
    Translations.clear();
    cmSystemTools::ExpandListArgument(value, Translations);
  }

  // QtIFW dependencies
  std::vector<std::string> deps;
  option = prefix + "DEPENDS";
  if (const char* value = this->GetOption(option)) {
    cmSystemTools::ExpandListArgument(value, deps);
  }
  option = prefix + "DEPENDENCIES";
  if (const char* value = this->GetOption(option)) {
    cmSystemTools::ExpandListArgument(value, deps);
  }
  for (std::vector<std::string>::iterator dit = deps.begin();
       dit != deps.end(); ++dit) {
    DependenceStruct dep(*dit);
    if (Generator->Packages.count(dep.Name)) {
      cmCPackIFWPackage& depPkg = Generator->Packages[dep.Name];
      dep.Name = depPkg.Name;
    }
    bool hasDep = Generator->DependentPackages.count(dep.Name) > 0;
    DependenceStruct& depRef = Generator->DependentPackages[dep.Name];
    if (!hasDep) {
      depRef = dep;
    }
    AlienDependencies.insert(&depRef);
  }

  // Automatic dependency on
  option = prefix + "AUTO_DEPEND_ON";
  if (IsSetToEmpty(option)) {
    AlienAutoDependOn.clear();
  } else if (const char* value = this->GetOption(option)) {
    std::vector<std::string> depsOn;
    cmSystemTools::ExpandListArgument(value, depsOn);
    for (std::vector<std::string>::iterator dit = depsOn.begin();
         dit != depsOn.end(); ++dit) {
      DependenceStruct dep(*dit);
      if (Generator->Packages.count(dep.Name)) {
        cmCPackIFWPackage& depPkg = Generator->Packages[dep.Name];
        dep.Name = depPkg.Name;
      }
      bool hasDep = Generator->DependentPackages.count(dep.Name) > 0;
      DependenceStruct& depRef = Generator->DependentPackages[dep.Name];
      if (!hasDep) {
        depRef = dep;
      }
      AlienAutoDependOn.insert(&depRef);
    }
  }

  // Visibility
  option = prefix + "VIRTUAL";
  if (IsSetToEmpty(option)) {
    Virtual.clear();
  } else if (IsOn(option)) {
    Virtual = "true";
  }

  // Default selection
  option = prefix + "DEFAULT";
  if (IsSetToEmpty(option)) {
    Default.clear();
  } else if (const char* value = GetOption(option)) {
    std::string lowerValue = cmsys::SystemTools::LowerCase(value);
    if (lowerValue.compare("true") == 0) {
      Default = "true";
    } else if (lowerValue.compare("false") == 0) {
      Default = "false";
    } else if (lowerValue.compare("script") == 0) {
      Default = "script";
    } else {
      Default = value;
    }
  }

  // Forsed installation
  option = prefix + "FORCED_INSTALLATION";
  if (IsSetToEmpty(option)) {
    ForcedInstallation.clear();
  } else if (IsOn(option)) {
    ForcedInstallation = "true";
  } else if (IsSetToOff(option)) {
    ForcedInstallation = "false";
  }

  // Requires admin rights
  option = prefix + "REQUIRES_ADMIN_RIGHTS";
  if (IsSetToEmpty(option)) {
    RequiresAdminRights.clear();
  } else if (IsOn(option)) {
    RequiresAdminRights = "true";
  } else if (IsSetToOff(option)) {
    RequiresAdminRights = "false";
  }

  return 1;
}

void cmCPackIFWPackage::GeneratePackageFile()
{
  // Lazy directory initialization
  if (Directory.empty()) {
    if (Installer) {
      Directory = Installer->Directory + "/packages/" + Name;
    } else if (Generator) {
      Directory = Generator->toplevel + "/packages/" + Name;
    }
  }

  // Output stream
  cmGeneratedFileStream fout((Directory + "/meta/package.xml").data());
  cmXMLWriter xout(fout);

  xout.StartDocument();

  WriteGeneratedByToStrim(xout);

  xout.StartElement("Package");

  xout.Element("DisplayName", DisplayName);
  xout.Element("Description", Description);

  // Update text
  if (!UpdateText.empty()) {
    xout.Element("UpdateText", UpdateText);
  }

  xout.Element("Name", Name);
  xout.Element("Version", Version);

  if (!ReleaseDate.empty()) {
    xout.Element("ReleaseDate", ReleaseDate);
  } else {
    xout.Element("ReleaseDate", cmTimestamp().CurrentTime("%Y-%m-%d", true));
  }

  // Script (copy to meta dir)
  if (!Script.empty()) {
    std::string name = cmSystemTools::GetFilenameName(Script);
    std::string path = Directory + "/meta/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(Script.data(), path.data());
    xout.Element("Script", name);
  }

  // User Interfaces (copy to meta dir)
  std::vector<std::string> userInterfaces = UserInterfaces;
  for (size_t i = 0; i < userInterfaces.size(); i++) {
    std::string name = cmSystemTools::GetFilenameName(userInterfaces[i]);
    std::string path = Directory + "/meta/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(userInterfaces[i].data(),
                                            path.data());
    userInterfaces[i] = name;
  }
  if (!userInterfaces.empty()) {
    xout.StartElement("UserInterfaces");
    for (size_t i = 0; i < userInterfaces.size(); i++) {
      xout.Element("UserInterface", userInterfaces[i]);
    }
    xout.EndElement();
  }

  // Translations (copy to meta dir)
  std::vector<std::string> translations = Translations;
  for (size_t i = 0; i < translations.size(); i++) {
    std::string name = cmSystemTools::GetFilenameName(translations[i]);
    std::string path = Directory + "/meta/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(translations[i].data(),
                                            path.data());
    translations[i] = name;
  }
  if (!translations.empty()) {
    xout.StartElement("Translations");
    for (size_t i = 0; i < translations.size(); i++) {
      xout.Element("Translation", translations[i]);
    }
    xout.EndElement();
  }

  // Dependencies
  std::set<DependenceStruct> compDepSet;
  for (std::set<DependenceStruct*>::iterator ait = AlienDependencies.begin();
       ait != AlienDependencies.end(); ++ait) {
    compDepSet.insert(*(*ait));
  }
  for (std::set<cmCPackIFWPackage*>::iterator it = Dependencies.begin();
       it != Dependencies.end(); ++it) {
    compDepSet.insert(DependenceStruct((*it)->Name));
  }
  // Write dependencies
  if (!compDepSet.empty()) {
    std::ostringstream dependencies;
    std::set<DependenceStruct>::iterator it = compDepSet.begin();
    dependencies << it->NameWithCompare();
    ++it;
    while (it != compDepSet.end()) {
      dependencies << "," << it->NameWithCompare();
      ++it;
    }
    xout.Element("Dependencies", dependencies.str());
  }

  // Automatic dependency on
  std::set<DependenceStruct> compAutoDepSet;
  for (std::set<DependenceStruct*>::iterator ait = AlienAutoDependOn.begin();
       ait != AlienAutoDependOn.end(); ++ait) {
    compAutoDepSet.insert(*(*ait));
  }
  // Write automatic dependency on
  if (!compAutoDepSet.empty()) {
    std::ostringstream dependencies;
    std::set<DependenceStruct>::iterator it = compAutoDepSet.begin();
    dependencies << it->NameWithCompare();
    ++it;
    while (it != compAutoDepSet.end()) {
      dependencies << "," << it->NameWithCompare();
      ++it;
    }
    xout.Element("AutoDependOn", dependencies.str());
  }

  // Licenses (copy to meta dir)
  std::vector<std::string> licenses = Licenses;
  for (size_t i = 1; i < licenses.size(); i += 2) {
    std::string name = cmSystemTools::GetFilenameName(licenses[i]);
    std::string path = Directory + "/meta/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(licenses[i].data(), path.data());
    licenses[i] = name;
  }
  if (!licenses.empty()) {
    xout.StartElement("Licenses");
    for (size_t i = 0; i < licenses.size(); i += 2) {
      xout.StartElement("License");
      xout.Attribute("name", licenses[i]);
      xout.Attribute("file", licenses[i + 1]);
      xout.EndElement();
    }
    xout.EndElement();
  }

  if (!ForcedInstallation.empty()) {
    xout.Element("ForcedInstallation", ForcedInstallation);
  }

  if (!RequiresAdminRights.empty()) {
    xout.Element("RequiresAdminRights", RequiresAdminRights);
  }

  if (!Virtual.empty()) {
    xout.Element("Virtual", Virtual);
  } else if (!Default.empty()) {
    xout.Element("Default", Default);
  }

  // Essential
  if (!Essential.empty()) {
    xout.Element("Essential", Essential);
  }

  // Priority
  if (!SortingPriority.empty()) {
    xout.Element("SortingPriority", SortingPriority);
  }

  xout.EndElement();
  xout.EndDocument();
}

void cmCPackIFWPackage::WriteGeneratedByToStrim(cmXMLWriter& xout)
{
  if (Generator) {
    Generator->WriteGeneratedByToStrim(xout);
  }
}
