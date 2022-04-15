/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackIFWInstaller.h"

#include <cstddef>
#include <sstream>
#include <utility>

#include "cmCPackIFWCommon.h"
#include "cmCPackIFWGenerator.h"
#include "cmCPackIFWPackage.h"
#include "cmCPackIFWRepository.h"
#include "cmCPackLog.h" // IWYU pragma: keep
#include "cmGeneratedFileStream.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmXMLParser.h"
#include "cmXMLWriter.h"

cmCPackIFWInstaller::cmCPackIFWInstaller() = default;

void cmCPackIFWInstaller::printSkippedOptionWarning(
  const std::string& optionName, const std::string& optionValue)
{
  cmCPackIFWLogger(
    WARNING,
    "Option "
      << optionName << " contains the value \"" << optionValue
      << "\" but will be skipped because the specified file does not exist."
      << std::endl);
}

void cmCPackIFWInstaller::ConfigureFromOptions()
{
  // Name;
  if (cmValue optIFW_PACKAGE_NAME =
        this->GetOption("CPACK_IFW_PACKAGE_NAME")) {
    this->Name = *optIFW_PACKAGE_NAME;
  } else if (cmValue optPACKAGE_NAME = this->GetOption("CPACK_PACKAGE_NAME")) {
    this->Name = *optPACKAGE_NAME;
  } else {
    this->Name = "Your package";
  }

  // Title;
  if (cmValue optIFW_PACKAGE_TITLE =
        this->GetOption("CPACK_IFW_PACKAGE_TITLE")) {
    this->Title = *optIFW_PACKAGE_TITLE;
  } else if (cmValue optPACKAGE_DESCRIPTION_SUMMARY =
               this->GetOption("CPACK_PACKAGE_DESCRIPTION_SUMMARY")) {
    this->Title = *optPACKAGE_DESCRIPTION_SUMMARY;
  } else {
    this->Title = "Your package description";
  }

  // Version;
  if (cmValue option = this->GetOption("CPACK_PACKAGE_VERSION")) {
    this->Version = *option;
  } else {
    this->Version = "1.0.0";
  }

  // Publisher
  if (cmValue optIFW_PACKAGE_PUBLISHER =
        this->GetOption("CPACK_IFW_PACKAGE_PUBLISHER")) {
    this->Publisher = *optIFW_PACKAGE_PUBLISHER;
  } else if (cmValue optPACKAGE_VENDOR =
               this->GetOption("CPACK_PACKAGE_VENDOR")) {
    this->Publisher = *optPACKAGE_VENDOR;
  }

  // ProductUrl
  if (cmValue option = this->GetOption("CPACK_IFW_PRODUCT_URL")) {
    this->ProductUrl = *option;
  }

  // ApplicationIcon
  if (cmValue option = this->GetOption("CPACK_IFW_PACKAGE_ICON")) {
    if (cmSystemTools::FileExists(option)) {
      this->InstallerApplicationIcon = *option;
    } else {
      this->printSkippedOptionWarning("CPACK_IFW_PACKAGE_ICON", option);
    }
  }

  // WindowIcon
  if (cmValue option = this->GetOption("CPACK_IFW_PACKAGE_WINDOW_ICON")) {
    if (cmSystemTools::FileExists(option)) {
      this->InstallerWindowIcon = *option;
    } else {
      this->printSkippedOptionWarning("CPACK_IFW_PACKAGE_WINDOW_ICON", option);
    }
  }

  // RemoveTargetDir
  if (this->IsSetToOff("CPACK_IFW_PACKAGE_REMOVE_TARGET_DIR")) {
    this->RemoveTargetDir = "false";
  } else if (this->IsOn("CPACK_IFW_PACKAGE_REMOVE_TARGET_DIR")) {
    this->RemoveTargetDir = "true";
  } else {
    this->RemoveTargetDir.clear();
  }

  // Logo
  if (cmValue option = this->GetOption("CPACK_IFW_PACKAGE_LOGO")) {
    if (cmSystemTools::FileExists(option)) {
      this->Logo = *option;
    } else {
      this->printSkippedOptionWarning("CPACK_IFW_PACKAGE_LOGO", option);
    }
  }

  // Watermark
  if (cmValue option = this->GetOption("CPACK_IFW_PACKAGE_WATERMARK")) {
    if (cmSystemTools::FileExists(option)) {
      this->Watermark = *option;
    } else {
      this->printSkippedOptionWarning("CPACK_IFW_PACKAGE_WATERMARK", option);
    }
  }

  // Banner
  if (cmValue option = this->GetOption("CPACK_IFW_PACKAGE_BANNER")) {
    if (cmSystemTools::FileExists(option)) {
      this->Banner = *option;
    } else {
      this->printSkippedOptionWarning("CPACK_IFW_PACKAGE_BANNER", option);
    }
  }

  // Background
  if (cmValue option = this->GetOption("CPACK_IFW_PACKAGE_BACKGROUND")) {
    if (cmSystemTools::FileExists(option)) {
      this->Background = *option;
    } else {
      this->printSkippedOptionWarning("CPACK_IFW_PACKAGE_BACKGROUND", option);
    }
  }

  // WizardStyle
  if (cmValue option = this->GetOption("CPACK_IFW_PACKAGE_WIZARD_STYLE")) {
    // Setting the user value in any case
    this->WizardStyle = *option;
    // Check known values
    if (this->WizardStyle != "Modern" && this->WizardStyle != "Aero" &&
        this->WizardStyle != "Mac" && this->WizardStyle != "Classic") {
      cmCPackIFWLogger(
        WARNING,
        "Option CPACK_IFW_PACKAGE_WIZARD_STYLE has unknown value \""
          << option << "\". Expected values are: Modern, Aero, Mac, Classic."
          << std::endl);
    }
  }

  // StyleSheet
  if (cmValue option = this->GetOption("CPACK_IFW_PACKAGE_STYLE_SHEET")) {
    if (cmSystemTools::FileExists(option)) {
      this->StyleSheet = *option;
    } else {
      this->printSkippedOptionWarning("CPACK_IFW_PACKAGE_STYLE_SHEET", option);
    }
  }

  // WizardDefaultWidth
  if (cmValue option =
        this->GetOption("CPACK_IFW_PACKAGE_WIZARD_DEFAULT_WIDTH")) {
    this->WizardDefaultWidth = *option;
  }

  // WizardDefaultHeight
  if (cmValue option =
        this->GetOption("CPACK_IFW_PACKAGE_WIZARD_DEFAULT_HEIGHT")) {
    this->WizardDefaultHeight = *option;
  }

  // WizardShowPageList
  if (cmValue option =
        this->GetOption("CPACK_IFW_PACKAGE_WIZARD_SHOW_PAGE_LIST")) {
    if (!this->IsVersionLess("4.0")) {
      if (this->IsSetToOff("CPACK_IFW_PACKAGE_WIZARD_SHOW_PAGE_LIST")) {
        this->WizardShowPageList = "false";
      } else if (this->IsOn("CPACK_IFW_PACKAGE_WIZARD_SHOW_PAGE_LIST")) {
        this->WizardShowPageList = "true";
      } else {
        this->WizardShowPageList.clear();
      }
    } else {
      std::string currentVersionMsg;
      if (this->Generator) {
        currentVersionMsg =
          "QtIFW version " + this->Generator->FrameworkVersion;
      } else {
        currentVersionMsg = "an older QtIFW version";
      }
      cmCPackIFWLogger(
        WARNING,
        "Option CPACK_IFW_PACKAGE_WIZARD_SHOW_PAGE_LIST is set to \""
          << option
          << "\", but it is only supported with QtIFW version 4.0 or later. "
             "It is being ignored because you are using "
          << currentVersionMsg << std::endl);
    }
  }

  // TitleColor
  if (cmValue option = this->GetOption("CPACK_IFW_PACKAGE_TITLE_COLOR")) {
    this->TitleColor = *option;
  }

  // Start menu
  if (cmValue optIFW_START_MENU_DIR =
        this->GetOption("CPACK_IFW_PACKAGE_START_MENU_DIRECTORY")) {
    this->StartMenuDir = *optIFW_START_MENU_DIR;
  } else {
    this->StartMenuDir = this->Name;
  }

  // Default target directory for installation
  if (cmValue optIFW_TARGET_DIRECTORY =
        this->GetOption("CPACK_IFW_TARGET_DIRECTORY")) {
    this->TargetDir = *optIFW_TARGET_DIRECTORY;
  } else if (cmValue optPACKAGE_INSTALL_DIRECTORY =
               this->GetOption("CPACK_PACKAGE_INSTALL_DIRECTORY")) {
    this->TargetDir =
      cmStrCat("@ApplicationsDir@/", optPACKAGE_INSTALL_DIRECTORY);
  } else {
    this->TargetDir = "@RootDir@/usr/local";
  }

  // Default target directory for installation with administrator rights
  if (cmValue option = this->GetOption("CPACK_IFW_ADMIN_TARGET_DIRECTORY")) {
    this->AdminTargetDir = *option;
  }

  // Maintenance tool
  if (cmValue optIFW_MAINTENANCE_TOOL =
        this->GetOption("CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_NAME")) {
    this->MaintenanceToolName = *optIFW_MAINTENANCE_TOOL;
  }

  // Maintenance tool ini file
  if (cmValue optIFW_MAINTENANCE_TOOL_INI =
        this->GetOption("CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_INI_FILE")) {
    this->MaintenanceToolIniFile = *optIFW_MAINTENANCE_TOOL_INI;
  }

  // Allow non-ASCII characters
  if (this->GetOption("CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS")) {
    if (this->IsOn("CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS")) {
      this->AllowNonAsciiCharacters = "true";
    } else {
      this->AllowNonAsciiCharacters = "false";
    }
  }

  // DisableCommandLineInterface
  if (this->GetOption("CPACK_IFW_PACKAGE_DISABLE_COMMAND_LINE_INTERFACE")) {
    if (this->IsOn("CPACK_IFW_PACKAGE_DISABLE_COMMAND_LINE_INTERFACE")) {
      this->DisableCommandLineInterface = "true";
    } else if (this->IsSetToOff(
                 "CPACK_IFW_PACKAGE_DISABLE_COMMAND_LINE_INTERFACE")) {
      this->DisableCommandLineInterface = "false";
    }
  }

  // Space in path
  if (this->GetOption("CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH")) {
    if (this->IsOn("CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH")) {
      this->AllowSpaceInPath = "true";
    } else {
      this->AllowSpaceInPath = "false";
    }
  }

  // Control script
  if (cmValue optIFW_CONTROL_SCRIPT =
        this->GetOption("CPACK_IFW_PACKAGE_CONTROL_SCRIPT")) {
    if (!cmSystemTools::FileExists(optIFW_CONTROL_SCRIPT)) {
      this->printSkippedOptionWarning("CPACK_IFW_PACKAGE_CONTROL_SCRIPT",
                                      optIFW_CONTROL_SCRIPT);
    } else {
      this->ControlScript = *optIFW_CONTROL_SCRIPT;
    }
  }

  // Resources
  if (cmValue optIFW_PACKAGE_RESOURCES =
        this->GetOption("CPACK_IFW_PACKAGE_RESOURCES")) {
    this->Resources.clear();
    cmExpandList(optIFW_PACKAGE_RESOURCES, this->Resources);
    for (const auto& file : this->Resources) {
      if (!cmSystemTools::FileExists(file)) {
        // The warning will say skipped, but there will later be a hard error
        // when the binarycreator tool tries to read the missing file.
        this->printSkippedOptionWarning("CPACK_IFW_PACKAGE_RESOURCES", file);
      }
    }
  }

  // ProductImages
  if (cmValue productImages =
        this->GetOption("CPACK_IFW_PACKAGE_PRODUCT_IMAGES")) {
    this->ProductImages.clear();
    cmExpandList(productImages, this->ProductImages);
    for (const auto& file : this->ProductImages) {
      if (!cmSystemTools::FileExists(file)) {
        // The warning will say skipped, but there will later be a hard error
        // when the binarycreator tool tries to read the missing file.
        this->printSkippedOptionWarning("CPACK_IFW_PACKAGE_PRODUCT_IMAGES",
                                        file);
      }
    }
  }

  // Run program, run program arguments, and run program description
  if (cmValue program = this->GetOption("CPACK_IFW_PACKAGE_RUN_PROGRAM")) {
    this->RunProgram = *program;
  }
  if (cmValue arguments =
        this->GetOption("CPACK_IFW_PACKAGE_RUN_PROGRAM_ARGUMENTS")) {
    this->RunProgramArguments.clear();
    cmExpandList(arguments, this->RunProgramArguments);
  }
  if (cmValue description =
        this->GetOption("CPACK_IFW_PACKAGE_RUN_PROGRAM_DESCRIPTION")) {
    this->RunProgramDescription = *description;
  }

#ifdef __APPLE__
  // Code signing identity for signing the generated app bundle
  if (cmValue id = this->GetOption("CPACK_IFW_PACKAGE_SIGNING_IDENTITY")) {
    this->SigningIdentity = *id;
  }
#endif
}

/** \class cmCPackIFWResourcesParser
 * \brief Helper class that parse resources form .qrc (Qt)
 */
class cmCPackIFWResourcesParser : public cmXMLParser
{
public:
  explicit cmCPackIFWResourcesParser(cmCPackIFWInstaller* i)
    : installer(i)
  {
    this->path = i->Directory + "/resources";
  }

  bool ParseResource(size_t r)
  {
    this->hasFiles = false;
    this->hasErrors = false;

    this->basePath =
      cmSystemTools::GetFilenamePath(this->installer->Resources[r]);

    this->ParseFile(this->installer->Resources[r].data());

    return this->hasFiles && !this->hasErrors;
  }

  cmCPackIFWInstaller* installer;
  bool file = false;
  bool hasFiles = false;
  bool hasErrors = false;
  std::string path, basePath;

protected:
  void StartElement(const std::string& name, const char** /*atts*/) override
  {
    this->file = name == "file";
    if (this->file) {
      this->hasFiles = true;
    }
  }

  void CharacterDataHandler(const char* data, int length) override
  {
    if (this->file) {
      std::string content(data, data + length);
      content = cmTrimWhitespace(content);
      std::string source = this->basePath + "/" + content;
      std::string destination = this->path + "/" + content;
      if (!cmSystemTools::CopyFileIfDifferent(source, destination)) {
        this->hasErrors = true;
      }
    }
  }

  void EndElement(const std::string& /*name*/) override {}
};

void cmCPackIFWInstaller::GenerateInstallerFile()
{
  // Lazy directory initialization
  if (this->Directory.empty() && this->Generator) {
    this->Directory = this->Generator->toplevel;
  }

  // Output stream
  cmGeneratedFileStream fout(this->Directory + "/config/config.xml");
  cmXMLWriter xout(fout);

  xout.StartDocument();

  this->WriteGeneratedByToStrim(xout);

  xout.StartElement("Installer");

  xout.Element("Name", this->Name);
  xout.Element("Version", this->Version);
  xout.Element("Title", this->Title);

  if (!this->Publisher.empty()) {
    xout.Element("Publisher", this->Publisher);
  }

  if (!this->ProductUrl.empty()) {
    xout.Element("ProductUrl", this->ProductUrl);
  }

  // Logo
  if (!this->Logo.empty()) {
    std::string srcName = cmSystemTools::GetFilenameName(this->Logo);
    std::string suffix = cmSystemTools::GetFilenameLastExtension(srcName);
    std::string name = "cm_logo" + suffix;
    std::string path = this->Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(this->Logo, path);
    xout.Element("Logo", name);
  }

  // Banner
  if (!this->Banner.empty()) {
    std::string name = cmSystemTools::GetFilenameName(this->Banner);
    std::string path = this->Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(this->Banner, path);
    xout.Element("Banner", name);
  }

  // Watermark
  if (!this->Watermark.empty()) {
    std::string name = cmSystemTools::GetFilenameName(this->Watermark);
    std::string path = this->Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(this->Watermark, path);
    xout.Element("Watermark", name);
  }

  // Background
  if (!this->Background.empty()) {
    std::string name = cmSystemTools::GetFilenameName(this->Background);
    std::string path = this->Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(this->Background, path);
    xout.Element("Background", name);
  }

  // Attributes introduced in QtIFW 1.4.0
  if (!this->IsVersionLess("1.4")) {
    // ApplicationIcon
    if (!this->InstallerApplicationIcon.empty()) {
      std::string srcName =
        cmSystemTools::GetFilenameName(this->InstallerApplicationIcon);
      std::string suffix = cmSystemTools::GetFilenameLastExtension(srcName);
      std::string name = "cm_appicon" + suffix;
      std::string path = this->Directory + "/config/" + name;
      cmsys::SystemTools::CopyFileIfDifferent(this->InstallerApplicationIcon,
                                              path);
      // The actual file is looked up by attaching a '.icns' (macOS),
      // '.ico' (Windows). No functionality on Unix.
      name = cmSystemTools::GetFilenameWithoutExtension(name);
      xout.Element("InstallerApplicationIcon", name);
    }

    // WindowIcon
    if (!this->InstallerWindowIcon.empty()) {
      std::string srcName =
        cmSystemTools::GetFilenameName(this->InstallerWindowIcon);
      std::string suffix = cmSystemTools::GetFilenameLastExtension(srcName);
      std::string name = "cm_winicon" + suffix;
      std::string path = this->Directory + "/config/" + name;
      cmsys::SystemTools::CopyFileIfDifferent(this->InstallerWindowIcon, path);
      xout.Element("InstallerWindowIcon", name);
    }
  }

  // Attributes introduced in QtIFW 2.0.0
  if (!this->IsVersionLess("2.0")) {
    // WizardDefaultWidth
    if (!this->WizardDefaultWidth.empty()) {
      xout.Element("WizardDefaultWidth", this->WizardDefaultWidth);
    }

    // WizardDefaultHeight
    if (!this->WizardDefaultHeight.empty()) {
      xout.Element("WizardDefaultHeight", this->WizardDefaultHeight);
    }

    // Start menu directory
    if (!this->StartMenuDir.empty()) {
      xout.Element("StartMenuDir", this->StartMenuDir);
    }

    // Maintenance tool
    if (!this->MaintenanceToolName.empty()) {
      xout.Element("MaintenanceToolName", this->MaintenanceToolName);
    }

    // Maintenance tool ini file
    if (!this->MaintenanceToolIniFile.empty()) {
      xout.Element("MaintenanceToolIniFile", this->MaintenanceToolIniFile);
    }

    if (!this->AllowNonAsciiCharacters.empty()) {
      xout.Element("AllowNonAsciiCharacters", this->AllowNonAsciiCharacters);
    }
    if (!this->AllowSpaceInPath.empty()) {
      xout.Element("AllowSpaceInPath", this->AllowSpaceInPath);
    }

    // Control script (copy to config dir)
    if (!this->ControlScript.empty()) {
      std::string name = cmSystemTools::GetFilenameName(this->ControlScript);
      std::string path = this->Directory + "/config/" + name;
      cmsys::SystemTools::CopyFileIfDifferent(this->ControlScript, path);
      xout.Element("ControlScript", name);
    }
  } else {
    // CPack IFW default policy
    xout.Comment("CPack IFW default policy for QtIFW less 2.0");
    xout.Element("AllowNonAsciiCharacters", "true");
    xout.Element("AllowSpaceInPath", "true");
  }

  // Target dir
  if (!this->TargetDir.empty()) {
    xout.Element("TargetDir", this->TargetDir);
  }

  // Admin target dir
  if (!this->AdminTargetDir.empty()) {
    xout.Element("AdminTargetDir", this->AdminTargetDir);
  }

  // Remote repositories
  if (!this->RemoteRepositories.empty()) {
    xout.StartElement("RemoteRepositories");
    for (cmCPackIFWRepository* r : this->RemoteRepositories) {
      r->WriteRepositoryConfig(xout);
    }
    xout.EndElement();
  }

  // Attributes introduced in QtIFW 3.0.0
  if (!this->IsVersionLess("3.0")) {
    // WizardStyle
    if (!this->WizardStyle.empty()) {
      xout.Element("WizardStyle", this->WizardStyle);
    }

    // Stylesheet (copy to config dir)
    if (!this->StyleSheet.empty()) {
      std::string name = cmSystemTools::GetFilenameName(this->StyleSheet);
      std::string path = this->Directory + "/config/" + name;
      cmsys::SystemTools::CopyFileIfDifferent(this->StyleSheet, path);
      xout.Element("StyleSheet", name);
    }

    // TitleColor
    if (!this->TitleColor.empty()) {
      xout.Element("TitleColor", this->TitleColor);
    }
  }

  // Attributes introduced in QtIFW 4.0.0
  if (!this->IsVersionLess("4.0")) {
    // WizardShowPageList
    if (!this->WizardShowPageList.empty()) {
      xout.Element("WizardShowPageList", this->WizardShowPageList);
    }

    // DisableCommandLineInterface
    if (!this->DisableCommandLineInterface.empty()) {
      xout.Element("DisableCommandLineInterface",
                   this->DisableCommandLineInterface);
    }

    // RunProgram
    if (!this->RunProgram.empty()) {
      xout.Element("RunProgram", this->RunProgram);
    }

    // RunProgramArguments
    if (!this->RunProgramArguments.empty()) {
      xout.StartElement("RunProgramArguments");
      for (const auto& arg : this->RunProgramArguments) {
        xout.Element("Argument", arg);
      }
      xout.EndElement();
    }

    // RunProgramDescription
    if (!this->RunProgramDescription.empty()) {
      xout.Element("RunProgramDescription", this->RunProgramDescription);
    }
  }

  if (!this->RemoveTargetDir.empty()) {
    xout.Element("RemoveTargetDir", this->RemoveTargetDir);
  }

  // Product images (copy to config dir)
  if (!this->IsVersionLess("4.0") && !this->ProductImages.empty()) {
    xout.StartElement("ProductImages");
    for (auto const& srcImg : this->ProductImages) {
      std::string name = cmSystemTools::GetFilenameName(srcImg);
      std::string dstImg = this->Directory + "/config/" + name;
      cmsys::SystemTools::CopyFileIfDifferent(srcImg, dstImg);
      xout.Element("Image", name);
    }
    xout.EndElement();
  }

  // Resources (copy to resources dir)
  if (!this->Resources.empty()) {
    std::vector<std::string> resources;
    cmCPackIFWResourcesParser parser(this);
    for (size_t i = 0; i < this->Resources.size(); i++) {
      if (parser.ParseResource(i)) {
        std::string name = cmSystemTools::GetFilenameName(this->Resources[i]);
        std::string path = this->Directory + "/resources/" + name;
        cmsys::SystemTools::CopyFileIfDifferent(this->Resources[i], path);
        resources.push_back(std::move(name));
      } else {
        cmCPackIFWLogger(WARNING,
                         "Can't copy resources from \""
                           << this->Resources[i]
                           << "\". Resource will be skipped." << std::endl);
      }
    }
    this->Resources = resources;
  }

  xout.EndElement();
  xout.EndDocument();
}

void cmCPackIFWInstaller::GeneratePackageFiles()
{
  if (this->Packages.empty() || this->Generator->IsOnePackage()) {
    // Generate default package
    cmCPackIFWPackage package;
    package.Generator = this->Generator;
    package.Installer = this;
    // Check package group
    if (cmValue option = this->GetOption("CPACK_IFW_PACKAGE_GROUP")) {
      package.ConfigureFromGroup(option);
      std::string forcedOption = "CPACK_IFW_COMPONENT_GROUP_" +
        cmsys::SystemTools::UpperCase(option) + "_FORCED_INSTALLATION";
      if (!this->GetOption(forcedOption)) {
        package.ForcedInstallation = "true";
      }
    } else {
      package.ConfigureFromOptions();
    }
    package.GeneratePackageFile();
    return;
  }

  // Generate packages meta information
  for (auto& p : this->Packages) {
    cmCPackIFWPackage* package = p.second;
    package->GeneratePackageFile();
  }
}
