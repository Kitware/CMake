/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackIFWInstaller.h"

#include <cmConfigure.h>
#include <sstream>
#include <stddef.h>
#include <utility>

#include "CPack/cmCPackGenerator.h"
#include "CPack/cmCPackLog.h"
#include "cmCPackIFWGenerator.h"
#include "cmCPackIFWPackage.h"
#include "cmCPackIFWRepository.h"
#include "cmGeneratedFileStream.h"
#include "cmSystemTools.h"
#include "cmXMLParser.h"
#include "cmXMLWriter.h"

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

cmCPackIFWInstaller::cmCPackIFWInstaller()
  : Generator(CM_NULLPTR)
{
}

const char* cmCPackIFWInstaller::GetOption(const std::string& op) const
{
  return Generator ? Generator->GetOption(op) : CM_NULLPTR;
}

bool cmCPackIFWInstaller::IsOn(const std::string& op) const
{
  return Generator ? Generator->IsOn(op) : false;
}

bool cmCPackIFWInstaller::IsVersionLess(const char* version)
{
  return Generator ? Generator->IsVersionLess(version) : false;
}

bool cmCPackIFWInstaller::IsVersionGreater(const char* version)
{
  return Generator ? Generator->IsVersionGreater(version) : false;
}

bool cmCPackIFWInstaller::IsVersionEqual(const char* version)
{
  return Generator ? Generator->IsVersionEqual(version) : false;
}

void cmCPackIFWInstaller::printSkippedOptionWarning(
  const std::string& optionName, const std::string& optionValue)
{
  cmCPackLogger(
    cmCPackLog::LOG_WARNING, "Option "
      << optionName << " is set to \"" << optionValue
      << "\" but will be skipped because the specified file does not exist."
      << std::endl);
}

void cmCPackIFWInstaller::ConfigureFromOptions()
{
  // Name;
  if (const char* optIFW_PACKAGE_NAME =
        this->GetOption("CPACK_IFW_PACKAGE_NAME")) {
    Name = optIFW_PACKAGE_NAME;
  } else if (const char* optPACKAGE_NAME =
               this->GetOption("CPACK_PACKAGE_NAME")) {
    Name = optPACKAGE_NAME;
  } else {
    Name = "Your package";
  }

  // Title;
  if (const char* optIFW_PACKAGE_TITLE =
        GetOption("CPACK_IFW_PACKAGE_TITLE")) {
    Title = optIFW_PACKAGE_TITLE;
  } else if (const char* optPACKAGE_DESCRIPTION_SUMMARY =
               GetOption("CPACK_PACKAGE_DESCRIPTION_SUMMARY")) {
    Title = optPACKAGE_DESCRIPTION_SUMMARY;
  } else {
    Title = "Your package description";
  }

  // Version;
  if (const char* option = GetOption("CPACK_PACKAGE_VERSION")) {
    Version = option;
  } else {
    Version = "1.0.0";
  }

  // Publisher
  if (const char* optIFW_PACKAGE_PUBLISHER =
        GetOption("CPACK_IFW_PACKAGE_PUBLISHER")) {
    Publisher = optIFW_PACKAGE_PUBLISHER;
  } else if (const char* optPACKAGE_VENDOR =
               GetOption("CPACK_PACKAGE_VENDOR")) {
    Publisher = optPACKAGE_VENDOR;
  }

  // ProductUrl
  if (const char* option = GetOption("CPACK_IFW_PRODUCT_URL")) {
    ProductUrl = option;
  }

  // ApplicationIcon
  if (const char* option = GetOption("CPACK_IFW_PACKAGE_ICON")) {
    if (cmSystemTools::FileExists(option)) {
      InstallerApplicationIcon = option;
    } else {
      printSkippedOptionWarning("CPACK_IFW_PACKAGE_ICON", option);
    }
  }

  // WindowIcon
  if (const char* option = GetOption("CPACK_IFW_PACKAGE_WINDOW_ICON")) {
    if (cmSystemTools::FileExists(option)) {
      InstallerWindowIcon = option;
    } else {
      printSkippedOptionWarning("CPACK_IFW_PACKAGE_WINDOW_ICON", option);
    }
  }

  // Logo
  if (const char* option = GetOption("CPACK_IFW_PACKAGE_LOGO")) {
    if (cmSystemTools::FileExists(option)) {
      Logo = option;
    } else {
      printSkippedOptionWarning("CPACK_IFW_PACKAGE_LOGO", option);
    }
  }

  // Watermark
  if (const char* option = GetOption("CPACK_IFW_PACKAGE_WATERMARK")) {
    if (cmSystemTools::FileExists(option)) {
      Watermark = option;
    } else {
      printSkippedOptionWarning("CPACK_IFW_PACKAGE_WATERMARK", option);
    }
  }

  // Banner
  if (const char* option = GetOption("CPACK_IFW_PACKAGE_BANNER")) {
    if (cmSystemTools::FileExists(option)) {
      Banner = option;
    } else {
      printSkippedOptionWarning("CPACK_IFW_PACKAGE_BANNER", option);
    }
  }

  // Background
  if (const char* option = GetOption("CPACK_IFW_PACKAGE_BACKGROUND")) {
    if (cmSystemTools::FileExists(option)) {
      Background = option;
    } else {
      printSkippedOptionWarning("CPACK_IFW_PACKAGE_BACKGROUND", option);
    }
  }

  // WizardStyle
  if (const char* option = GetOption("CPACK_IFW_PACKAGE_WIZARD_STYLE")) {
    if (WizardStyle.compare("Modern") == 0 &&
        WizardStyle.compare("Aero") == 0 && WizardStyle.compare("Mac") == 0 &&
        WizardStyle.compare("Classic") == 0) {
      cmCPackLogger(
        cmCPackLog::LOG_WARNING,
        "Option CPACK_IFW_PACKAGE_WIZARD_STYLE has unknown value \""
          << option << "\". Expected values are: Modern, Aero, Mac, Classic."
          << std::endl);
    }

    WizardStyle = option;
  }

  // WizardDefaultWidth
  if (const char* option =
        GetOption("CPACK_IFW_PACKAGE_WIZARD_DEFAULT_WIDTH")) {
    WizardDefaultWidth = option;
  }

  // WizardDefaultHeight
  if (const char* option =
        GetOption("CPACK_IFW_PACKAGE_WIZARD_DEFAULT_HEIGHT")) {
    WizardDefaultHeight = option;
  }

  // TitleColor
  if (const char* option = GetOption("CPACK_IFW_PACKAGE_TITLE_COLOR")) {
    TitleColor = option;
  }

  // Start menu
  if (const char* optIFW_START_MENU_DIR =
        this->GetOption("CPACK_IFW_PACKAGE_START_MENU_DIRECTORY")) {
    StartMenuDir = optIFW_START_MENU_DIR;
  } else {
    StartMenuDir = Name;
  }

  // Default target directory for installation
  if (const char* optIFW_TARGET_DIRECTORY =
        GetOption("CPACK_IFW_TARGET_DIRECTORY")) {
    TargetDir = optIFW_TARGET_DIRECTORY;
  } else if (const char* optPACKAGE_INSTALL_DIRECTORY =
               GetOption("CPACK_PACKAGE_INSTALL_DIRECTORY")) {
    TargetDir = "@ApplicationsDir@/";
    TargetDir += optPACKAGE_INSTALL_DIRECTORY;
  } else {
    TargetDir = "@RootDir@/usr/local";
  }

  // Default target directory for installation with administrator rights
  if (const char* option = GetOption("CPACK_IFW_ADMIN_TARGET_DIRECTORY")) {
    AdminTargetDir = option;
  }

  // Maintenance tool
  if (const char* optIFW_MAINTENANCE_TOOL =
        this->GetOption("CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_NAME")) {
    MaintenanceToolName = optIFW_MAINTENANCE_TOOL;
  }

  // Maintenance tool ini file
  if (const char* optIFW_MAINTENANCE_TOOL_INI =
        this->GetOption("CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_INI_FILE")) {
    MaintenanceToolIniFile = optIFW_MAINTENANCE_TOOL_INI;
  }

  // Allow non-ASCII characters
  if (this->GetOption("CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS")) {
    if (IsOn("CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS")) {
      AllowNonAsciiCharacters = "true";
    } else {
      AllowNonAsciiCharacters = "false";
    }
  }

  // Space in path
  if (this->GetOption("CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH")) {
    if (IsOn("CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH")) {
      AllowSpaceInPath = "true";
    } else {
      AllowSpaceInPath = "false";
    }
  }

  // Control script
  if (const char* optIFW_CONTROL_SCRIPT =
        this->GetOption("CPACK_IFW_PACKAGE_CONTROL_SCRIPT")) {
    ControlScript = optIFW_CONTROL_SCRIPT;
  }

  // Resources
  if (const char* optIFW_PACKAGE_RESOURCES =
        this->GetOption("CPACK_IFW_PACKAGE_RESOURCES")) {
    Resources.clear();
    cmSystemTools::ExpandListArgument(optIFW_PACKAGE_RESOURCES, Resources);
  }
}

/** \class cmCPackIFWResourcesParser
 * \brief Helper class that parse resources form .qrc (Qt)
 */
class cmCPackIFWResourcesParser : public cmXMLParser
{
public:
  cmCPackIFWResourcesParser(cmCPackIFWInstaller* i)
    : installer(i)
    , file(false)
  {
    path = i->Directory + "/resources";
  }

  bool ParseResource(size_t r)
  {
    hasFiles = false;
    hasErrors = false;

    basePath = cmSystemTools::GetFilenamePath(installer->Resources[r].data());

    ParseFile(installer->Resources[r].data());

    return hasFiles && !hasErrors;
  }

  cmCPackIFWInstaller* installer;
  bool file, hasFiles, hasErrors;
  std::string path, basePath;

protected:
  void StartElement(const std::string& name, const char** /*atts*/) CM_OVERRIDE
  {
    file = name == "file";
    if (file) {
      hasFiles = true;
    }
  }

  void CharacterDataHandler(const char* data, int length) CM_OVERRIDE
  {
    if (file) {
      std::string content(data, data + length);
      content = cmSystemTools::TrimWhitespace(content);
      std::string source = basePath + "/" + content;
      std::string destination = path + "/" + content;
      if (!cmSystemTools::CopyFileIfDifferent(source.data(),
                                              destination.data())) {
        hasErrors = true;
      }
    }
  }

  void EndElement(const std::string& /*name*/) CM_OVERRIDE {}
};

void cmCPackIFWInstaller::GenerateInstallerFile()
{
  // Lazy directory initialization
  if (Directory.empty() && Generator) {
    Directory = Generator->toplevel;
  }

  // Output stream
  cmGeneratedFileStream fout((Directory + "/config/config.xml").data());
  cmXMLWriter xout(fout);

  xout.StartDocument();

  WriteGeneratedByToStrim(xout);

  xout.StartElement("Installer");

  xout.Element("Name", Name);
  xout.Element("Version", Version);
  xout.Element("Title", Title);

  if (!Publisher.empty()) {
    xout.Element("Publisher", Publisher);
  }

  if (!ProductUrl.empty()) {
    xout.Element("ProductUrl", ProductUrl);
  }

  // ApplicationIcon
  if (!InstallerApplicationIcon.empty()) {
    std::string name =
      cmSystemTools::GetFilenameName(InstallerApplicationIcon);
    std::string path = Directory + "/config/" + name;
    name = cmSystemTools::GetFilenameWithoutExtension(name);
    cmsys::SystemTools::CopyFileIfDifferent(InstallerApplicationIcon.data(),
                                            path.data());
    xout.Element("InstallerApplicationIcon", name);
  }

  // WindowIcon
  if (!InstallerWindowIcon.empty()) {
    std::string name = cmSystemTools::GetFilenameName(InstallerWindowIcon);
    std::string path = Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(InstallerWindowIcon.data(),
                                            path.data());
    xout.Element("InstallerWindowIcon", name);
  }

  // Logo
  if (!Logo.empty()) {
    std::string name = cmSystemTools::GetFilenameName(Logo);
    std::string path = Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(Logo.data(), path.data());
    xout.Element("Logo", name);
  }

  // Banner
  if (!Banner.empty()) {
    std::string name = cmSystemTools::GetFilenameName(Banner);
    std::string path = Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(Banner.data(), path.data());
    xout.Element("Banner", name);
  }

  // Watermark
  if (!Watermark.empty()) {
    std::string name = cmSystemTools::GetFilenameName(Watermark);
    std::string path = Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(Watermark.data(), path.data());
    xout.Element("Watermark", name);
  }

  // Background
  if (!Background.empty()) {
    std::string name = cmSystemTools::GetFilenameName(Background);
    std::string path = Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(Background.data(), path.data());
    xout.Element("Background", name);
  }

  // WizardStyle
  if (!WizardStyle.empty()) {
    xout.Element("WizardStyle", WizardStyle);
  }

  // WizardDefaultWidth
  if (!WizardDefaultWidth.empty()) {
    xout.Element("WizardDefaultWidth", WizardDefaultWidth);
  }

  // WizardDefaultHeight
  if (!WizardDefaultHeight.empty()) {
    xout.Element("WizardDefaultHeight", WizardDefaultHeight);
  }

  // TitleColor
  if (!TitleColor.empty()) {
    xout.Element("TitleColor", TitleColor);
  }

  // Start menu
  if (!IsVersionLess("2.0")) {
    xout.Element("StartMenuDir", StartMenuDir);
  }

  // Target dir
  if (!TargetDir.empty()) {
    xout.Element("TargetDir", TargetDir);
  }

  // Admin target dir
  if (!AdminTargetDir.empty()) {
    xout.Element("AdminTargetDir", AdminTargetDir);
  }

  // Remote repositories
  if (!RemoteRepositories.empty()) {
    xout.StartElement("RemoteRepositories");
    for (RepositoriesVector::iterator rit = RemoteRepositories.begin();
         rit != RemoteRepositories.end(); ++rit) {
      (*rit)->WriteRepositoryConfig(xout);
    }
    xout.EndElement();
  }

  // Maintenance tool
  if (!IsVersionLess("2.0") && !MaintenanceToolName.empty()) {
    xout.Element("MaintenanceToolName", MaintenanceToolName);
  }

  // Maintenance tool ini file
  if (!IsVersionLess("2.0") && !MaintenanceToolIniFile.empty()) {
    xout.Element("MaintenanceToolIniFile", MaintenanceToolIniFile);
  }

  // Different allows
  if (IsVersionLess("2.0")) {
    // CPack IFW default policy
    xout.Comment("CPack IFW default policy for QtIFW less 2.0");
    xout.Element("AllowNonAsciiCharacters", "true");
    xout.Element("AllowSpaceInPath", "true");
  } else {
    if (!AllowNonAsciiCharacters.empty()) {
      xout.Element("AllowNonAsciiCharacters", AllowNonAsciiCharacters);
    }
    if (!AllowSpaceInPath.empty()) {
      xout.Element("AllowSpaceInPath", AllowSpaceInPath);
    }
  }

  // Control script (copy to config dir)
  if (!IsVersionLess("2.0") && !ControlScript.empty()) {
    std::string name = cmSystemTools::GetFilenameName(ControlScript);
    std::string path = Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(ControlScript.data(), path.data());
    xout.Element("ControlScript", name);
  }

  // Resources (copy to resources dir)
  if (!Resources.empty()) {
    std::vector<std::string> resources;
    cmCPackIFWResourcesParser parser(this);
    for (size_t i = 0; i < Resources.size(); i++) {
      if (parser.ParseResource(i)) {
        std::string name = cmSystemTools::GetFilenameName(Resources[i]);
        std::string path = Directory + "/resources/" + name;
        cmsys::SystemTools::CopyFileIfDifferent(Resources[i].data(),
                                                path.data());
        resources.push_back(name);
      } else {
        cmCPackLogger(cmCPackLog::LOG_WARNING, "Can't copy resources from \""
                        << Resources[i] << "\". Resource will be skipped."
                        << std::endl);
      }
    }
    Resources = resources;
  }

  xout.EndElement();
  xout.EndDocument();
}

void cmCPackIFWInstaller::GeneratePackageFiles()
{
  if (Packages.empty() || Generator->IsOnePackage()) {
    // Generate default package
    cmCPackIFWPackage package;
    package.Generator = Generator;
    package.Installer = this;
    // Check package group
    if (const char* option = GetOption("CPACK_IFW_PACKAGE_GROUP")) {
      package.ConfigureFromGroup(option);
      std::string forcedOption = "CPACK_IFW_COMPONENT_GROUP_" +
        cmsys::SystemTools::UpperCase(option) + "_FORCED_INSTALLATION";
      if (!GetOption(forcedOption)) {
        package.ForcedInstallation = "true";
      }
    } else {
      package.ConfigureFromOptions();
    }
    package.GeneratePackageFile();
    return;
  }

  // Generate packages meta information
  for (PackagesMap::iterator pit = Packages.begin(); pit != Packages.end();
       ++pit) {
    cmCPackIFWPackage* package = pit->second;
    package->GeneratePackageFile();
  }
}

void cmCPackIFWInstaller::WriteGeneratedByToStrim(cmXMLWriter& xout)
{
  if (Generator) {
    Generator->WriteGeneratedByToStrim(xout);
  }
}
