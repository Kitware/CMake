/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackNSISGenerator.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <map>
#include <sstream>
#include <utility>

#include <cmext/algorithm>

#include "cmsys/Directory.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmCPackComponentGroup.h"
#include "cmCPackGenerator.h"
#include "cmCPackLog.h"
#include "cmDuration.h"
#include "cmGeneratedFileStream.h"
#include "cmList.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

/* NSIS uses different command line syntax on Windows and others */
#ifdef _WIN32
#  define NSIS_OPT "/"
#else
#  define NSIS_OPT "-"
#endif

cmCPackNSISGenerator::cmCPackNSISGenerator(bool nsis64)
{
  this->Nsis64 = nsis64;
}

cmCPackNSISGenerator::~cmCPackNSISGenerator() = default;

int cmCPackNSISGenerator::PackageFiles()
{
  // TODO: Fix nsis to force out file name

  std::string nsisInFileName = this->FindTemplate("NSIS.template.in");
  if (nsisInFileName.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "CPack error: Could not find NSIS installer template file."
                    << std::endl);
    return false;
  }
  std::string nsisInInstallOptions =
    this->FindTemplate("NSIS.InstallOptions.ini.in");
  if (nsisInInstallOptions.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "CPack error: Could not find NSIS installer options file."
                    << std::endl);
    return false;
  }

  std::string nsisFileName = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  std::string tmpFile = cmStrCat(nsisFileName, "/NSISOutput.log");
  std::string nsisInstallOptions = nsisFileName + "/NSIS.InstallOptions.ini";
  nsisFileName += "/project.nsi";
  std::ostringstream str;
  for (std::string const& file : this->files) {
    std::string outputDir = "$INSTDIR";
    std::string fileN = cmSystemTools::RelativePath(this->toplevel, file);
    if (!this->Components.empty()) {
      const std::string::size_type pos = fileN.find('/');

      // Use the custom component install directory if we have one
      if (pos != std::string::npos) {
        auto componentName = cm::string_view(fileN).substr(0, pos);
        outputDir = this->CustomComponentInstallDirectory(componentName);
      } else {
        outputDir = this->CustomComponentInstallDirectory(fileN);
      }

      // Strip off the component part of the path.
      fileN = fileN.substr(pos + 1);
    }
    std::replace(fileN.begin(), fileN.end(), '/', '\\');

    str << "  Delete \"" << outputDir << "\\" << fileN << "\"" << std::endl;
  }
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
                "Uninstall Files: " << str.str() << std::endl);
  this->SetOptionIfNotSet("CPACK_NSIS_DELETE_FILES", str.str());
  std::vector<std::string> dirs;
  this->GetListOfSubdirectories(this->toplevel.c_str(), dirs);
  std::ostringstream dstr;
  for (std::string const& dir : dirs) {
    std::string componentName;
    std::string fileN = cmSystemTools::RelativePath(this->toplevel, dir);
    if (fileN.empty()) {
      continue;
    }
    if (!this->Components.empty()) {
      // If this is a component installation, strip off the component
      // part of the path.
      std::string::size_type slash = fileN.find('/');
      if (slash != std::string::npos) {
        // If this is a component installation, determine which component it
        // is.
        componentName = fileN.substr(0, slash);

        // Strip off the component part of the path.
        fileN.erase(0, slash + 1);
      }
    }
    std::replace(fileN.begin(), fileN.end(), '/', '\\');

    const std::string componentOutputDir =
      this->CustomComponentInstallDirectory(componentName);

    dstr << "  RMDir \"" << componentOutputDir << "\\" << fileN << "\""
         << std::endl;
    if (!componentName.empty()) {
      this->Components[componentName].Directories.push_back(std::move(fileN));
    }
  }
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
                "Uninstall Dirs: " << dstr.str() << std::endl);
  this->SetOptionIfNotSet("CPACK_NSIS_DELETE_DIRECTORIES", dstr.str());

  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Configure file: " << nsisInFileName << " to " << nsisFileName
                                   << std::endl);
  if (this->IsSet("CPACK_NSIS_MUI_ICON") ||
      this->IsSet("CPACK_NSIS_MUI_UNIICON")) {
    std::string installerIconCode;
    if (this->IsSet("CPACK_NSIS_MUI_ICON")) {
      installerIconCode += cmStrCat(
        "!define MUI_ICON \"", this->GetOption("CPACK_NSIS_MUI_ICON"), "\"\n");
    }
    if (this->IsSet("CPACK_NSIS_MUI_UNIICON")) {
      installerIconCode +=
        cmStrCat("!define MUI_UNICON \"",
                 this->GetOption("CPACK_NSIS_MUI_UNIICON"), "\"\n");
    }
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_MUI_ICON_CODE",
                            installerIconCode.c_str());
  }
  std::string installerHeaderImage;
  if (this->IsSet("CPACK_NSIS_MUI_HEADERIMAGE")) {
    installerHeaderImage = *this->GetOption("CPACK_NSIS_MUI_HEADERIMAGE");
  } else if (this->IsSet("CPACK_PACKAGE_ICON")) {
    installerHeaderImage = *this->GetOption("CPACK_PACKAGE_ICON");
  }
  if (!installerHeaderImage.empty()) {
    std::string installerIconCode = cmStrCat(
      "!define MUI_HEADERIMAGE_BITMAP \"", installerHeaderImage, "\"\n");
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_ICON_CODE",
                            installerIconCode);
  }

  if (this->IsSet("CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP")) {
    std::string installerBitmapCode = cmStrCat(
      "!define MUI_WELCOMEFINISHPAGE_BITMAP \"",
      this->GetOption("CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP"), "\"\n");
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_MUI_WELCOMEFINISH_CODE",
                            installerBitmapCode);
  }

  if (this->IsSet("CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP")) {
    std::string installerBitmapCode = cmStrCat(
      "!define MUI_UNWELCOMEFINISHPAGE_BITMAP \"",
      this->GetOption("CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP"), "\"\n");
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_MUI_UNWELCOMEFINISH_CODE",
                            installerBitmapCode);
  }

  if (this->IsSet("CPACK_NSIS_MUI_FINISHPAGE_RUN")) {
    std::string installerRunCode =
      cmStrCat("!define MUI_FINISHPAGE_RUN \"$INSTDIR\\",
               this->GetOption("CPACK_NSIS_EXECUTABLES_DIRECTORY"), '\\',
               this->GetOption("CPACK_NSIS_MUI_FINISHPAGE_RUN"), "\"\n");
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_MUI_FINISHPAGE_RUN_CODE",
                            installerRunCode);
  }

  if (this->IsSet("CPACK_NSIS_WELCOME_TITLE")) {
    std::string welcomeTitleCode =
      cmStrCat("!define MUI_WELCOMEPAGE_TITLE \"",
               this->GetOption("CPACK_NSIS_WELCOME_TITLE"), "\"");
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_WELCOME_TITLE_CODE",
                            welcomeTitleCode);
  }

  if (this->IsSet("CPACK_NSIS_WELCOME_TITLE_3LINES")) {
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_WELCOME_TITLE_3LINES_CODE",
                            "!define MUI_WELCOMEPAGE_TITLE_3LINES");
  }

  if (this->IsSet("CPACK_NSIS_FINISH_TITLE")) {
    std::string finishTitleCode =
      cmStrCat("!define MUI_FINISHPAGE_TITLE \"",
               this->GetOption("CPACK_NSIS_FINISH_TITLE"), "\"");
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_FINISH_TITLE_CODE",
                            finishTitleCode);
  }

  if (this->IsSet("CPACK_NSIS_FINISH_TITLE_3LINES")) {
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_FINISH_TITLE_3LINES_CODE",
                            "!define MUI_FINISHPAGE_TITLE_3LINES");
  }

  if (this->IsSet("CPACK_NSIS_MANIFEST_DPI_AWARE")) {
    this->SetOptionIfNotSet("CPACK_NSIS_MANIFEST_DPI_AWARE_CODE",
                            "ManifestDPIAware true");
  }

  if (this->IsSet("CPACK_NSIS_BRANDING_TEXT")) {
    // Default position to LEFT
    std::string brandingTextPosition = "LEFT";
    if (this->IsSet("CPACK_NSIS_BRANDING_TEXT_TRIM_POSITION")) {
      std::string wantedPosition =
        this->GetOption("CPACK_NSIS_BRANDING_TEXT_TRIM_POSITION");
      if (!wantedPosition.empty()) {
        const std::set<std::string> possiblePositions{ "CENTER", "LEFT",
                                                       "RIGHT" };
        if (possiblePositions.find(wantedPosition) ==
            possiblePositions.end()) {
          cmCPackLogger(cmCPackLog::LOG_ERROR,
                        "Unsupported branding text trim position "
                          << wantedPosition << std::endl);
          return false;
        }
        brandingTextPosition = wantedPosition;
      }
    }
    std::string brandingTextCode =
      cmStrCat("BrandingText /TRIM", brandingTextPosition, " \"",
               this->GetOption("CPACK_NSIS_BRANDING_TEXT"), "\"\n");
    this->SetOptionIfNotSet("CPACK_NSIS_BRANDING_TEXT_CODE", brandingTextCode);
  }

  if (!this->IsSet("CPACK_NSIS_IGNORE_LICENSE_PAGE")) {
    std::string licenceCode =
      cmStrCat("!insertmacro MUI_PAGE_LICENSE \"",
               this->GetOption("CPACK_RESOURCE_FILE_LICENSE"), "\"\n");
    this->SetOptionIfNotSet("CPACK_NSIS_LICENSE_PAGE", licenceCode);
  }

  std::string nsisPreArguments;
  if (cmValue nsisArguments =
        this->GetOption("CPACK_NSIS_EXECUTABLE_PRE_ARGUMENTS")) {
    cmList expandedArguments{ nsisArguments };

    for (auto& arg : expandedArguments) {
      if (!cmHasPrefix(arg, NSIS_OPT)) {
        nsisPreArguments = cmStrCat(nsisPreArguments, NSIS_OPT);
      }
      nsisPreArguments = cmStrCat(nsisPreArguments, arg, ' ');
    }
  }

  std::string nsisPostArguments;
  if (cmValue nsisArguments =
        this->GetOption("CPACK_NSIS_EXECUTABLE_POST_ARGUMENTS")) {
    cmList expandedArguments{ nsisArguments };
    for (auto& arg : expandedArguments) {
      if (!cmHasPrefix(arg, NSIS_OPT)) {
        nsisPostArguments = cmStrCat(nsisPostArguments, NSIS_OPT);
      }
      nsisPostArguments = cmStrCat(nsisPostArguments, arg, ' ');
    }
  }

  // Setup all of the component sections
  if (this->Components.empty()) {
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLATION_TYPES", "");
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_MUI_COMPONENTS_DESC", "");
    this->SetOptionIfNotSet("CPACK_NSIS_PAGE_COMPONENTS", "");
    this->SetOptionIfNotSet("CPACK_NSIS_FULL_INSTALL",
                            R"(File /r "${INST_DIR}\*.*")");
    this->SetOptionIfNotSet("CPACK_NSIS_COMPONENT_SECTIONS", "");
    this->SetOptionIfNotSet("CPACK_NSIS_COMPONENT_SECTION_LIST", "");
    this->SetOptionIfNotSet("CPACK_NSIS_SECTION_SELECTED_VARS", "");
  } else {
    std::string componentCode;
    std::string sectionList;
    std::string selectedVarsList;
    std::string componentDescriptions;
    std::string groupDescriptions;
    std::string installTypesCode;
    std::string defines;
    std::ostringstream macrosOut;
    bool anyDownloadedComponents = false;

    // Create installation types. The order is significant, so we first fill
    // in a vector based on the indices, and print them in that order.
    std::vector<cmCPackInstallationType*> installTypes(
      this->InstallationTypes.size());
    for (auto& installType : this->InstallationTypes) {
      installTypes[installType.second.Index - 1] = &installType.second;
    }
    for (cmCPackInstallationType* installType : installTypes) {
      installTypesCode += "InstType \"";
      installTypesCode += installType->DisplayName;
      installTypesCode += "\"\n";
    }

    // Create installation groups first
    for (auto& group : this->ComponentGroups) {
      if (group.second.ParentGroup == nullptr) {
        componentCode +=
          this->CreateComponentGroupDescription(&group.second, macrosOut);
      }

      // Add the group description, if any.
      if (!group.second.Description.empty()) {
        groupDescriptions += "  !insertmacro MUI_DESCRIPTION_TEXT ${" +
          group.first + "} \"" +
          cmCPackNSISGenerator::TranslateNewlines(group.second.Description) +
          "\"\n";
      }
    }

    // Create the remaining components, which aren't associated with groups.
    for (auto& comp : this->Components) {
      if (comp.second.Files.empty()) {
        // NSIS cannot cope with components that have no files.
        continue;
      }

      anyDownloadedComponents =
        anyDownloadedComponents || comp.second.IsDownloaded;

      if (!comp.second.Group) {
        componentCode +=
          this->CreateComponentDescription(&comp.second, macrosOut);
      }

      // Add this component to the various section lists.
      sectionList += R"(  !insertmacro "${MacroName}" ")";
      sectionList += comp.first;
      sectionList += "\"\n";
      selectedVarsList += "Var " + comp.first + "_selected\n";
      selectedVarsList += "Var " + comp.first + "_was_installed\n";

      // Add the component description, if any.
      if (!comp.second.Description.empty()) {
        componentDescriptions += "  !insertmacro MUI_DESCRIPTION_TEXT ${" +
          comp.first + "} \"" +
          cmCPackNSISGenerator::TranslateNewlines(comp.second.Description) +
          "\"\n";
      }
    }

    componentCode += macrosOut.str();

    if (componentDescriptions.empty() && groupDescriptions.empty()) {
      // Turn off the "Description" box
      this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_MUI_COMPONENTS_DESC",
                              "!define MUI_COMPONENTSPAGE_NODESC");
    } else {
      componentDescriptions = "!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN\n" +
        componentDescriptions + groupDescriptions +
        "!insertmacro MUI_FUNCTION_DESCRIPTION_END\n";
      this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_MUI_COMPONENTS_DESC",
                              componentDescriptions);
    }

    if (anyDownloadedComponents) {
      defines += "!define CPACK_USES_DOWNLOAD\n";
      if (cmIsOn(this->GetOption("CPACK_ADD_REMOVE"))) {
        defines += "!define CPACK_NSIS_ADD_REMOVE\n";
      }
    }

    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLATION_TYPES", installTypesCode);
    this->SetOptionIfNotSet("CPACK_NSIS_PAGE_COMPONENTS",
                            "!insertmacro MUI_PAGE_COMPONENTS");
    this->SetOptionIfNotSet("CPACK_NSIS_FULL_INSTALL", "");
    this->SetOptionIfNotSet("CPACK_NSIS_COMPONENT_SECTIONS", componentCode);
    this->SetOptionIfNotSet("CPACK_NSIS_COMPONENT_SECTION_LIST", sectionList);
    this->SetOptionIfNotSet("CPACK_NSIS_SECTION_SELECTED_VARS",
                            selectedVarsList);
    this->SetOption("CPACK_NSIS_DEFINES", defines);
  }

  this->ConfigureFile(nsisInInstallOptions, nsisInstallOptions);
  this->ConfigureFile(nsisInFileName, nsisFileName);
  std::string nsisCmd =
    cmStrCat('"', this->GetOption("CPACK_INSTALLER_PROGRAM"), "\" ",
             nsisPreArguments, " \"", nsisFileName, '"');
  if (!nsisPostArguments.empty()) {
    nsisCmd = cmStrCat(nsisCmd, " ", nsisPostArguments);
  }
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Execute: " << nsisCmd << std::endl);
  std::string output;
  int retVal = 1;
  bool res = cmSystemTools::RunSingleCommand(
    nsisCmd, &output, &output, &retVal, nullptr, this->GeneratorVerbose,
    cmDuration::zero());
  if (!res || retVal) {
    cmGeneratedFileStream ofs(tmpFile);
    ofs << "# Run command: " << nsisCmd << std::endl
        << "# Output:" << std::endl
        << output << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Problem running NSIS command: " << nsisCmd << std::endl
                                                   << "Please check "
                                                   << tmpFile << " for errors"
                                                   << std::endl);
    return 0;
  }
  return 1;
}

int cmCPackNSISGenerator::InitializeInternal()
{
  if (cmIsOn(this->GetOption("CPACK_INCLUDE_TOPLEVEL_DIRECTORY"))) {
    cmCPackLogger(
      cmCPackLog::LOG_WARNING,
      "NSIS Generator cannot work with CPACK_INCLUDE_TOPLEVEL_DIRECTORY set. "
      "This option will be reset to 0 (for this generator only)."
        << std::endl);
    this->SetOption("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", nullptr);
  }

  cmCPackLogger(cmCPackLog::LOG_DEBUG,
                "cmCPackNSISGenerator::Initialize()" << std::endl);
  std::vector<std::string> path;
  std::string nsisPath;
  bool gotRegValue = false;

#ifdef _WIN32
  if (Nsis64) {
    if (!gotRegValue &&
        cmsys::SystemTools::ReadRegistryValue(
          "HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS\\Unicode", nsisPath,
          cmsys::SystemTools::KeyWOW64_64)) {
      gotRegValue = true;
    }
    if (!gotRegValue &&
        cmsys::SystemTools::ReadRegistryValue(
          "HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS", nsisPath,
          cmsys::SystemTools::KeyWOW64_64)) {
      gotRegValue = true;
    }
  }
  if (!gotRegValue &&
      cmsys::SystemTools::ReadRegistryValue(
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS\\Unicode", nsisPath,
        cmsys::SystemTools::KeyWOW64_32)) {
    gotRegValue = true;
  }
  if (!gotRegValue &&
      cmsys::SystemTools::ReadRegistryValue(
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS\\Unicode", nsisPath)) {
    gotRegValue = true;
  }
  if (!gotRegValue &&
      cmsys::SystemTools::ReadRegistryValue(
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS", nsisPath,
        cmsys::SystemTools::KeyWOW64_32)) {
    gotRegValue = true;
  }
  if (!gotRegValue &&
      cmsys::SystemTools::ReadRegistryValue(
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS", nsisPath)) {
    gotRegValue = true;
  }

  if (gotRegValue) {
    path.push_back(nsisPath);
  }
#endif

  this->SetOptionIfNotSet("CPACK_NSIS_EXECUTABLE", "makensis");
  nsisPath = cmSystemTools::FindProgram(
    *this->GetOption("CPACK_NSIS_EXECUTABLE"), path, false);

  if (nsisPath.empty()) {
    cmCPackLogger(
      cmCPackLog::LOG_ERROR,
      "Cannot find NSIS compiler makensis: likely it is not installed, "
      "or not in your PATH"
        << std::endl);

    if (!gotRegValue) {
      cmCPackLogger(
        cmCPackLog::LOG_ERROR,
        "Could not read NSIS registry value. This is usually caused by "
        "NSIS not being installed. Please install NSIS from "
        "http://nsis.sourceforge.net"
          << std::endl);
    }

    return 0;
  }

  std::string nsisCmd = "\"" + nsisPath + "\" " NSIS_OPT "VERSION";
  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Test NSIS version: " << nsisCmd << std::endl);
  std::string output;
  int retVal = 1;
  bool resS = cmSystemTools::RunSingleCommand(
    nsisCmd, &output, &output, &retVal, nullptr, this->GeneratorVerbose,
    cmDuration::zero());
  cmsys::RegularExpression versionRex("v([0-9]+.[0-9]+)");
  cmsys::RegularExpression versionRexCVS("v(.*)\\.cvs");
  if (!resS || retVal ||
      (!versionRex.find(output) && !versionRexCVS.find(output))) {
    cmValue topDir = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    std::string tmpFile = cmStrCat(topDir ? *topDir : ".", "/NSISOutput.log");
    cmGeneratedFileStream ofs(tmpFile);
    ofs << "# Run command: " << nsisCmd << std::endl
        << "# Output:" << std::endl
        << output << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Problem checking NSIS version with command: "
                    << nsisCmd << std::endl
                    << "Please check " << tmpFile << " for errors"
                    << std::endl);
    return 0;
  }
  if (versionRex.find(output)) {
    double nsisVersion = atof(versionRex.match(1).c_str());
    double minNSISVersion = 3.03;
    cmCPackLogger(cmCPackLog::LOG_DEBUG,
                  "NSIS Version: " << nsisVersion << std::endl);
    if (nsisVersion < minNSISVersion) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "CPack requires NSIS Version 3.03 or greater. "
                    "NSIS found on the system was: "
                      << nsisVersion << std::endl);
      return 0;
    }
  }
  if (versionRexCVS.find(output)) {
    // No version check for NSIS cvs build
    cmCPackLogger(cmCPackLog::LOG_DEBUG,
                  "NSIS Version: CVS " << versionRexCVS.match(1) << std::endl);
  }
  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM", nsisPath);
  this->SetOptionIfNotSet("CPACK_NSIS_EXECUTABLES_DIRECTORY", "bin");
  cmValue cpackPackageExecutables =
    this->GetOption("CPACK_PACKAGE_EXECUTABLES");
  cmValue cpackPackageDeskTopLinks =
    this->GetOption("CPACK_CREATE_DESKTOP_LINKS");
  cmValue cpackNsisExecutablesDirectory =
    this->GetOption("CPACK_NSIS_EXECUTABLES_DIRECTORY");
  cmList cpackPackageDesktopLinksList;
  if (cpackPackageDeskTopLinks) {
    cmCPackLogger(cmCPackLog::LOG_DEBUG,
                  "CPACK_CREATE_DESKTOP_LINKS: " << cpackPackageDeskTopLinks
                                                 << std::endl);

    cpackPackageDesktopLinksList.assign(cpackPackageDeskTopLinks);
    for (std::string const& cpdl : cpackPackageDesktopLinksList) {
      cmCPackLogger(cmCPackLog::LOG_DEBUG,
                    "CPACK_CREATE_DESKTOP_LINKS: " << cpdl << std::endl);
    }
  } else {
    cmCPackLogger(cmCPackLog::LOG_DEBUG,
                  "CPACK_CREATE_DESKTOP_LINKS: "
                    << "not set" << std::endl);
  }

  std::ostringstream str;
  std::ostringstream deleteStr;

  if (cpackPackageExecutables) {
    cmCPackLogger(cmCPackLog::LOG_DEBUG,
                  "The cpackPackageExecutables: " << cpackPackageExecutables
                                                  << "." << std::endl);
    cmList cpackPackageExecutablesList{ cpackPackageExecutables };
    if (cpackPackageExecutablesList.size() % 2 != 0) {
      cmCPackLogger(
        cmCPackLog::LOG_ERROR,
        "CPACK_PACKAGE_EXECUTABLES should contain pairs of <executable> and "
        "<icon name>."
          << std::endl);
      return 0;
    }
    cmList::iterator it;
    for (it = cpackPackageExecutablesList.begin();
         it != cpackPackageExecutablesList.end(); ++it) {
      std::string execName = *it;
      ++it;
      std::string linkName = *it;
      str << R"(  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\)" << linkName
          << R"(.lnk" "$INSTDIR\)" << cpackNsisExecutablesDirectory << "\\"
          << execName << ".exe\"" << std::endl;
      deleteStr << R"(  Delete "$SMPROGRAMS\$MUI_TEMP\)" << linkName
                << ".lnk\"" << std::endl;
      // see if CPACK_CREATE_DESKTOP_LINK_ExeName is on
      // if so add a desktop link
      if (cm::contains(cpackPackageDesktopLinksList, execName)) {
        str << "  StrCmp \"$INSTALL_DESKTOP\" \"1\" 0 +2\n";
        str << "    CreateShortCut \"$DESKTOP\\" << linkName
            << R"(.lnk" "$INSTDIR\)" << cpackNsisExecutablesDirectory << "\\"
            << execName << ".exe\"" << std::endl;
        deleteStr << "  StrCmp \"$INSTALL_DESKTOP\" \"1\" 0 +2\n";
        deleteStr << "    Delete \"$DESKTOP\\" << linkName << ".lnk\""
                  << std::endl;
      }
    }
  }

  this->CreateMenuLinks(str, deleteStr);
  this->SetOptionIfNotSet("CPACK_NSIS_CREATE_ICONS", str.str());
  this->SetOptionIfNotSet("CPACK_NSIS_DELETE_ICONS", deleteStr.str());

  this->SetOptionIfNotSet("CPACK_NSIS_COMPRESSOR", "lzma");

  return this->Superclass::InitializeInternal();
}

void cmCPackNSISGenerator::CreateMenuLinks(std::ostream& str,
                                           std::ostream& deleteStr)
{
  cmValue cpackMenuLinks = this->GetOption("CPACK_NSIS_MENU_LINKS");
  if (!cpackMenuLinks) {
    return;
  }
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
                "The cpackMenuLinks: " << cpackMenuLinks << "." << std::endl);
  cmList cpackMenuLinksList{ cpackMenuLinks };
  if (cpackMenuLinksList.size() % 2 != 0) {
    cmCPackLogger(
      cmCPackLog::LOG_ERROR,
      "CPACK_NSIS_MENU_LINKS should contain pairs of <shortcut target> and "
      "<shortcut label>."
        << std::endl);
    return;
  }

  static cmsys::RegularExpression urlRegex(
    "^(mailto:|(ftps?|https?|news)://).*$");

  cmList::iterator it;
  for (it = cpackMenuLinksList.begin(); it != cpackMenuLinksList.end(); ++it) {
    std::string sourceName = *it;
    const bool url = urlRegex.find(sourceName);

    // Convert / to \ in filenames, but not in urls:
    //
    if (!url) {
      std::replace(sourceName.begin(), sourceName.end(), '/', '\\');
    }

    ++it;
    std::string linkName = *it;
    if (!url) {
      str << R"(  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\)" << linkName
          << R"(.lnk" "$INSTDIR\)" << sourceName << "\"" << std::endl;
      deleteStr << R"(  Delete "$SMPROGRAMS\$MUI_TEMP\)" << linkName
                << ".lnk\"" << std::endl;
    } else {
      str << R"(  WriteINIStr "$SMPROGRAMS\$STARTMENU_FOLDER\)" << linkName
          << R"(.url" "InternetShortcut" "URL" ")" << sourceName << "\""
          << std::endl;
      deleteStr << R"(  Delete "$SMPROGRAMS\$MUI_TEMP\)" << linkName
                << ".url\"" << std::endl;
    }
    // see if CPACK_CREATE_DESKTOP_LINK_ExeName is on
    // if so add a desktop link
    std::string desktop = cmStrCat("CPACK_CREATE_DESKTOP_LINK_", linkName);
    if (this->IsSet(desktop)) {
      str << "  StrCmp \"$INSTALL_DESKTOP\" \"1\" 0 +2\n";
      str << "    CreateShortCut \"$DESKTOP\\" << linkName
          << R"(.lnk" "$INSTDIR\)" << sourceName << "\"" << std::endl;
      deleteStr << "  StrCmp \"$INSTALL_DESKTOP\" \"1\" 0 +2\n";
      deleteStr << "    Delete \"$DESKTOP\\" << linkName << ".lnk\""
                << std::endl;
    }
  }
}

bool cmCPackNSISGenerator::GetListOfSubdirectories(
  const char* topdir, std::vector<std::string>& dirs)
{
  cmsys::Directory dir;
  dir.Load(topdir);
  for (unsigned long i = 0; i < dir.GetNumberOfFiles(); ++i) {
    const char* fileName = dir.GetFile(i);
    if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0) {
      std::string const fullPath =
        std::string(topdir).append("/").append(fileName);
      if (cmsys::SystemTools::FileIsDirectory(fullPath) &&
          !cmsys::SystemTools::FileIsSymlink(fullPath)) {
        if (!this->GetListOfSubdirectories(fullPath.c_str(), dirs)) {
          return false;
        }
      }
    }
  }
  dirs.emplace_back(topdir);
  return true;
}

enum cmCPackGenerator::CPackSetDestdirSupport
cmCPackNSISGenerator::SupportsSetDestdir() const
{
  return cmCPackGenerator::SETDESTDIR_SHOULD_NOT_BE_USED;
}

bool cmCPackNSISGenerator::SupportsAbsoluteDestination() const
{
  return false;
}

bool cmCPackNSISGenerator::SupportsComponentInstallation() const
{
  return true;
}

std::string cmCPackNSISGenerator::CreateComponentDescription(
  cmCPackComponent* component, std::ostream& macrosOut)
{
  // Basic description of the component
  std::string componentCode = "Section ";
  if (component->IsDisabledByDefault) {
    componentCode += "/o ";
  }
  componentCode += "\"";
  if (component->IsHidden) {
    componentCode += "-";
  }
  componentCode += component->DisplayName + "\" " + component->Name + "\n";
  if (component->IsRequired) {
    componentCode += "  SectionIn RO\n";
  } else if (!component->InstallationTypes.empty()) {
    std::ostringstream out;
    for (cmCPackInstallationType const* installType :
         component->InstallationTypes) {
      out << " " << installType->Index;
    }
    componentCode += "  SectionIn" + out.str() + "\n";
  }

  const std::string componentOutputDir =
    this->CustomComponentInstallDirectory(component->Name);
  componentCode += cmStrCat("  SetOutPath \"", componentOutputDir, "\"\n");

  // Create the actual installation commands
  if (component->IsDownloaded) {
    if (component->ArchiveFile.empty()) {
      // Compute the name of the archive.
      std::string packagesDir =
        cmStrCat(this->GetOption("CPACK_TEMPORARY_DIRECTORY"), ".dummy");
      std::ostringstream out;
      out << cmSystemTools::GetFilenameWithoutLastExtension(packagesDir) << "-"
          << component->Name << ".zip";
      component->ArchiveFile = out.str();
    }

    // Create the directory for the upload area
    cmValue userUploadDirectory = this->GetOption("CPACK_UPLOAD_DIRECTORY");
    std::string uploadDirectory;
    if (cmNonempty(userUploadDirectory)) {
      uploadDirectory = *userUploadDirectory;
    } else {
      uploadDirectory =
        cmStrCat(this->GetOption("CPACK_PACKAGE_DIRECTORY"), "/CPackUploads");
    }
    if (!cmSystemTools::FileExists(uploadDirectory)) {
      if (!cmSystemTools::MakeDirectory(uploadDirectory)) {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
                      "Unable to create NSIS upload directory "
                        << uploadDirectory << std::endl);
        return "";
      }
    }

    // Remove the old archive, if one exists
    std::string archiveFile = uploadDirectory + '/' + component->ArchiveFile;
    cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                  "-   Building downloaded component archive: " << archiveFile
                                                                << std::endl);
    if (cmSystemTools::FileExists(archiveFile, true)) {
      if (!cmSystemTools::RemoveFile(archiveFile)) {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
                      "Unable to remove archive file " << archiveFile
                                                       << std::endl);
        return "";
      }
    }

    // Find a ZIP program
    if (!this->IsSet("ZIP_EXECUTABLE")) {
      this->ReadListFile("Internal/CPack/CPackZIP.cmake");

      if (!this->IsSet("ZIP_EXECUTABLE")) {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
                      "Unable to find ZIP program" << std::endl);
        return "";
      }
    }

    // The directory where this component's files reside
    std::string dirName = cmStrCat(
      this->GetOption("CPACK_TEMPORARY_DIRECTORY"), '/', component->Name, '/');

    // Build the list of files to go into this archive, and determine the
    // size of the installed component.
    std::string zipListFileName = cmStrCat(
      this->GetOption("CPACK_TEMPORARY_DIRECTORY"), "/winZip.filelist");
    bool needQuotesInFile = cmIsOn(this->GetOption("CPACK_ZIP_NEED_QUOTES"));
    unsigned long totalSize = 0;
    { // the scope is needed for cmGeneratedFileStream
      cmGeneratedFileStream out(zipListFileName);
      for (std::string const& file : component->Files) {
        if (needQuotesInFile) {
          out << "\"";
        }
        out << file;
        if (needQuotesInFile) {
          out << "\"";
        }
        out << std::endl;

        totalSize += cmSystemTools::FileLength(dirName + file);
      }
    }

    // Build the archive in the upload area
    std::string cmd = this->GetOption("CPACK_ZIP_COMMAND");
    cmsys::SystemTools::ReplaceString(cmd, "<ARCHIVE>", archiveFile.c_str());
    cmsys::SystemTools::ReplaceString(cmd, "<FILELIST>",
                                      zipListFileName.c_str());
    std::string output;
    int retVal = -1;
    int res = cmSystemTools::RunSingleCommand(
      cmd, &output, &output, &retVal, dirName.c_str(),
      cmSystemTools::OUTPUT_NONE, cmDuration::zero());
    if (!res || retVal) {
      std::string tmpFile = cmStrCat(
        this->GetOption("CPACK_TOPLEVEL_DIRECTORY"), "/CompressZip.log");
      cmGeneratedFileStream ofs(tmpFile);
      ofs << "# Run command: " << cmd << std::endl
          << "# Output:" << std::endl
          << output << std::endl;
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Problem running zip command: " << cmd << std::endl
                                                    << "Please check "
                                                    << tmpFile << " for errors"
                                                    << std::endl);
      return "";
    }

    // Create the NSIS code to download this file on-the-fly.
    unsigned long totalSizeInKbytes = (totalSize + 512) / 1024;
    if (totalSizeInKbytes == 0) {
      totalSizeInKbytes = 1;
    }
    std::ostringstream out;
    /* clang-format off */
    out << "  AddSize " << totalSizeInKbytes << "\n"
        << "  Push \"" << component->ArchiveFile << "\"\n"
        << "  Call DownloadFile\n"
        << "  ZipDLL::extractall \"$INSTDIR\\"
        << component->ArchiveFile << "\" \"$INSTDIR\"\n"
        <<  "  Pop $2 ; error message\n"
                     "  StrCmp $2 \"success\" +2 0\n"
                     "  MessageBox MB_OK \"Failed to unzip $2\"\n"
                     "  Delete $INSTDIR\\$0\n";
    /* clang-format on */
    componentCode += out.str();
  } else {
    componentCode +=
      "  File /r \"${INST_DIR}\\" + component->Name + "\\*.*\"\n";
  }
  componentCode += "SectionEnd\n";

  // Macro used to remove the component
  macrosOut << "!macro Remove_${" << component->Name << "}\n";
  macrosOut << "  IntCmp $" << component->Name << "_was_installed 0 noremove_"
            << component->Name << "\n";
  std::string path;
  for (std::string const& pathIt : component->Files) {
    path = pathIt;
    std::replace(path.begin(), path.end(), '/', '\\');
    macrosOut << "  Delete \"" << componentOutputDir << "\\" << path << "\"\n";
  }
  for (std::string const& pathIt : component->Directories) {
    path = pathIt;
    std::replace(path.begin(), path.end(), '/', '\\');
    macrosOut << "  RMDir \"" << componentOutputDir << "\\" << path << "\"\n";
  }
  macrosOut << "  noremove_" << component->Name << ":\n";
  macrosOut << "!macroend\n";

  // Macro used to select each of the components that this component
  // depends on.
  std::set<cmCPackComponent*> visited;
  macrosOut << "!macro Select_" << component->Name << "_depends\n";
  macrosOut << this->CreateSelectionDependenciesDescription(component,
                                                            visited);
  macrosOut << "!macroend\n";

  // Macro used to deselect each of the components that depend on this
  // component.
  visited.clear();
  macrosOut << "!macro Deselect_required_by_" << component->Name << "\n";
  macrosOut << this->CreateDeselectionDependenciesDescription(component,
                                                              visited);
  macrosOut << "!macroend\n";
  return componentCode;
}

std::string cmCPackNSISGenerator::CreateSelectionDependenciesDescription(
  cmCPackComponent* component, std::set<cmCPackComponent*>& visited)
{
  // Don't visit a component twice
  if (visited.count(component)) {
    return {};
  }
  visited.insert(component);

  std::ostringstream out;
  for (cmCPackComponent* depend : component->Dependencies) {
    // Write NSIS code to select this dependency
    out << "  SectionGetFlags ${" << depend->Name << "} $0\n";
    out << "  IntOp $0 $0 | ${SF_SELECTED}\n";
    out << "  SectionSetFlags ${" << depend->Name << "} $0\n";
    out << "  IntOp $" << depend->Name << "_selected 0 + ${SF_SELECTED}\n";
    // Recurse
    out
      << this->CreateSelectionDependenciesDescription(depend, visited).c_str();
  }

  return out.str();
}

std::string cmCPackNSISGenerator::CreateDeselectionDependenciesDescription(
  cmCPackComponent* component, std::set<cmCPackComponent*>& visited)
{
  // Don't visit a component twice
  if (visited.count(component)) {
    return {};
  }
  visited.insert(component);

  std::ostringstream out;
  for (cmCPackComponent* depend : component->ReverseDependencies) {
    // Write NSIS code to deselect this dependency
    out << "  SectionGetFlags ${" << depend->Name << "} $0\n";
    out << "  IntOp $1 ${SF_SELECTED} ~\n";
    out << "  IntOp $0 $0 & $1\n";
    out << "  SectionSetFlags ${" << depend->Name << "} $0\n";
    out << "  IntOp $" << depend->Name << "_selected 0 + 0\n";

    // Recurse
    out << this->CreateDeselectionDependenciesDescription(depend, visited)
             .c_str();
  }

  return out.str();
}

std::string cmCPackNSISGenerator::CreateComponentGroupDescription(
  cmCPackComponentGroup* group, std::ostream& macrosOut)
{
  if (group->Components.empty() && group->Subgroups.empty()) {
    // Silently skip empty groups. NSIS doesn't support them.
    return {};
  }

  std::string code = "SectionGroup ";
  if (group->IsExpandedByDefault) {
    code += "/e ";
  }
  if (group->IsBold) {
    code += "\"!" + group->DisplayName + "\" " + group->Name + "\n";
  } else {
    code += "\"" + group->DisplayName + "\" " + group->Name + "\n";
  }

  for (cmCPackComponentGroup* g : group->Subgroups) {
    code += this->CreateComponentGroupDescription(g, macrosOut);
  }

  for (cmCPackComponent* comp : group->Components) {
    if (comp->Files.empty()) {
      continue;
    }

    code += this->CreateComponentDescription(comp, macrosOut);
  }
  code += "SectionGroupEnd\n";
  return code;
}

std::string cmCPackNSISGenerator::CustomComponentInstallDirectory(
  cm::string_view componentName)
{
  cmValue outputDir = this->GetOption(
    cmStrCat("CPACK_NSIS_", componentName, "_INSTALL_DIRECTORY"));
  return outputDir ? *outputDir : "$INSTDIR";
}

std::string cmCPackNSISGenerator::TranslateNewlines(std::string str)
{
  cmSystemTools::ReplaceString(str, "\n", "$\\r$\\n");
  return str;
}
