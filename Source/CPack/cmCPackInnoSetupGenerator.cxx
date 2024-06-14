/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
file Copyright.txt or https://cmake.org/licensing for details. */

#include "cmCPackInnoSetupGenerator.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ostream>
#include <stack>
#include <utility>

#include "cmsys/RegularExpression.hxx"

#include "cmCPackComponentGroup.h"
#include "cmCPackLog.h"
#include "cmDuration.h"
#include "cmGeneratedFileStream.h"
#include "cmList.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmCPackInnoSetupGenerator::cmCPackInnoSetupGenerator() = default;
cmCPackInnoSetupGenerator::~cmCPackInnoSetupGenerator() = default;

bool cmCPackInnoSetupGenerator::CanGenerate()
{
  // Inno Setup is only available for Windows
#ifdef _WIN32
  return true;
#else
  return false;
#endif
}

int cmCPackInnoSetupGenerator::InitializeInternal()
{
  if (cmIsOn(GetOption("CPACK_INCLUDE_TOPLEVEL_DIRECTORY"))) {
    cmCPackLogger(cmCPackLog::LOG_WARNING,
                  "Inno Setup Generator cannot work with "
                  "CPACK_INCLUDE_TOPLEVEL_DIRECTORY set. "
                  "This option will be reset to 0 (for this generator only)."
                    << std::endl);
    SetOption("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", nullptr);
  }

  std::vector<std::string> path;

#ifdef _WIN32
  path.push_back("C:\\Program Files (x86)\\Inno Setup 5");
  path.push_back("C:\\Program Files (x86)\\Inno Setup 6");
#endif

  SetOptionIfNotSet("CPACK_INNOSETUP_EXECUTABLE", "ISCC");
  const std::string& isccPath = cmSystemTools::FindProgram(
    GetOption("CPACK_INNOSETUP_EXECUTABLE"), path, false);

  if (isccPath.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Cannot find Inno Setup compiler ISCC: "
                  "likely it is not installed, or not in your PATH"
                    << std::endl);

    return 0;
  }

  const std::string isccCmd = cmStrCat(QuotePath(isccPath), "/?");
  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Test Inno Setup version: " << isccCmd << std::endl);
  std::string output;
  cmSystemTools::RunSingleCommand(isccCmd, &output, &output, nullptr, nullptr,
                                  this->GeneratorVerbose, cmDuration::zero());
  cmsys::RegularExpression vRex("Inno Setup ([0-9]+)");
  if (!vRex.find(output)) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Problem checking Inno Setup version with command: "
                    << isccCmd << std::endl
                    << "Have you downloaded Inno Setup from "
                       "https://jrsoftware.org/isinfo.php?"
                    << std::endl);
    return 0;
  }

  const int isccVersion = atoi(vRex.match(1).c_str());
  const int minIsccVersion = 6;
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
                "Inno Setup Version: " << isccVersion << std::endl);

  if (isccVersion < minIsccVersion) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "CPack requires Inno Setup Version 6 or greater. "
                  "Inno Setup found on the system was: "
                    << isccVersion << std::endl);
    return 0;
  }

  SetOption("CPACK_INSTALLER_PROGRAM", isccPath);

  return this->Superclass::InitializeInternal();
}

int cmCPackInnoSetupGenerator::PackageFiles()
{
  // Includes
  if (IsSet("CPACK_INNOSETUP_EXTRA_SCRIPTS")) {
    const cmList extraScripts(GetOption("CPACK_INNOSETUP_EXTRA_SCRIPTS"));

    for (const std::string& i : extraScripts) {
      includeDirectives.emplace_back(cmStrCat(
        "#include ", QuotePath(cmSystemTools::CollapseFullPath(i, toplevel))));
    }
  }

  // [Languages] section
  SetOptionIfNotSet("CPACK_INNOSETUP_LANGUAGES", "english");
  const cmList languages(GetOption("CPACK_INNOSETUP_LANGUAGES"));
  for (std::string i : languages) {
    cmCPackInnoSetupKeyValuePairs params;

    params["Name"] = Quote(i);

    if (cmSystemTools::LowerCase(i) == "english") {
      params["MessagesFile"] = "\"compiler:Default.isl\"";
    } else {
      i[0] = static_cast<char>(std::toupper(i[0]));
      params["MessagesFile"] = cmStrCat("\"compiler:Languages\\", i, ".isl\"");
    }

    languageInstructions.push_back(ISKeyValueLine(params));
  }

  if (!Components.empty() && !ProcessComponents()) {
    return false;
  }

  if (!(ProcessSetupSection() && ProcessFiles())) {
    return false;
  }

  // [Code] section
  if (IsSet("CPACK_INNOSETUP_CODE_FILES")) {
    const cmList codeFiles(GetOption("CPACK_INNOSETUP_CODE_FILES"));

    for (const std::string& i : codeFiles) {
      codeIncludes.emplace_back(cmStrCat(
        "#include ", QuotePath(cmSystemTools::CollapseFullPath(i, toplevel))));
    }
  }

  return ConfigureISScript() && Compile();
}

bool cmCPackInnoSetupGenerator::ProcessSetupSection()
{
  if (!RequireOption("CPACK_PACKAGE_INSTALL_REGISTRY_KEY")) {
    return false;
  }
  setupDirectives["AppId"] = GetOption("CPACK_PACKAGE_INSTALL_REGISTRY_KEY");

  if (!RequireOption("CPACK_PACKAGE_NAME")) {
    return false;
  }
  setupDirectives["AppName"] = GetOption("CPACK_PACKAGE_NAME");
  setupDirectives["UninstallDisplayName"] = GetOption("CPACK_PACKAGE_NAME");

  if (!RequireOption("CPACK_PACKAGE_VERSION")) {
    return false;
  }
  setupDirectives["AppVersion"] = GetOption("CPACK_PACKAGE_VERSION");

  if (!RequireOption("CPACK_PACKAGE_VENDOR")) {
    return false;
  }
  setupDirectives["AppPublisher"] = GetOption("CPACK_PACKAGE_VENDOR");

  if (IsSet("CPACK_PACKAGE_HOMEPAGE_URL")) {
    setupDirectives["AppPublisherURL"] =
      GetOption("CPACK_PACKAGE_HOMEPAGE_URL");
    setupDirectives["AppSupportURL"] = GetOption("CPACK_PACKAGE_HOMEPAGE_URL");
    setupDirectives["AppUpdatesURL"] = GetOption("CPACK_PACKAGE_HOMEPAGE_URL");
  }

  SetOptionIfNotSet("CPACK_INNOSETUP_IGNORE_LICENSE_PAGE", "OFF");
  if (IsSet("CPACK_RESOURCE_FILE_LICENSE") &&
      !GetOption("CPACK_INNOSETUP_IGNORE_LICENSE_PAGE").IsOn()) {
    setupDirectives["LicenseFile"] = cmSystemTools::ConvertToWindowsOutputPath(
      GetOption("CPACK_RESOURCE_FILE_LICENSE"));
  }

  SetOptionIfNotSet("CPACK_INNOSETUP_IGNORE_README_PAGE", "ON");
  if (IsSet("CPACK_RESOURCE_FILE_README") &&
      !GetOption("CPACK_INNOSETUP_IGNORE_README_PAGE").IsOn()) {
    setupDirectives["InfoBeforeFile"] =
      cmSystemTools::ConvertToWindowsOutputPath(
        GetOption("CPACK_RESOURCE_FILE_README"));
  }

  SetOptionIfNotSet("CPACK_INNOSETUP_USE_MODERN_WIZARD", "OFF");
  if (GetOption("CPACK_INNOSETUP_USE_MODERN_WIZARD").IsOn()) {
    setupDirectives["WizardStyle"] = "modern";
  } else {
    setupDirectives["WizardStyle"] = "classic";
    setupDirectives["WizardSmallImageFile"] =
      "compiler:WizClassicSmallImage.bmp";
    setupDirectives["WizardImageFile"] = "compiler:WizClassicImage.bmp";
    setupDirectives["SetupIconFile"] = "compiler:SetupClassicIcon.ico";
  }

  if (IsSet("CPACK_INNOSETUP_ICON_FILE")) {
    setupDirectives["SetupIconFile"] =
      cmSystemTools::ConvertToWindowsOutputPath(
        GetOption("CPACK_INNOSETUP_ICON_FILE"));
  }

  if (IsSet("CPACK_PACKAGE_ICON")) {
    setupDirectives["WizardSmallImageFile"] =
      cmSystemTools::ConvertToWindowsOutputPath(
        GetOption("CPACK_PACKAGE_ICON"));
  }

  if (!RequireOption("CPACK_PACKAGE_INSTALL_DIRECTORY")) {
    return false;
  }
  SetOptionIfNotSet("CPACK_INNOSETUP_INSTALL_ROOT", "{autopf}");
  setupDirectives["DefaultDirName"] =
    cmSystemTools::ConvertToWindowsOutputPath(
      cmStrCat(GetOption("CPACK_INNOSETUP_INSTALL_ROOT"), '\\',
               GetOption("CPACK_PACKAGE_INSTALL_DIRECTORY")));

  SetOptionIfNotSet("CPACK_INNOSETUP_ALLOW_CUSTOM_DIRECTORY", "ON");
  if (GetOption("CPACK_INNOSETUP_ALLOW_CUSTOM_DIRECTORY").IsOff()) {
    setupDirectives["DisableDirPage"] = "yes";
  }

  SetOptionIfNotSet("CPACK_INNOSETUP_PROGRAM_MENU_FOLDER",
                    GetOption("CPACK_PACKAGE_NAME"));
  if (GetOption("CPACK_INNOSETUP_PROGRAM_MENU_FOLDER") == ".") {
    setupDirectives["DisableProgramGroupPage"] = "yes";
    toplevelProgramFolder = true;
  } else {
    setupDirectives["DefaultGroupName"] =
      GetOption("CPACK_INNOSETUP_PROGRAM_MENU_FOLDER");
    toplevelProgramFolder = false;
  }

  if (IsSet("CPACK_INNOSETUP_PASSWORD")) {
    setupDirectives["Password"] = GetOption("CPACK_INNOSETUP_PASSWORD");
    setupDirectives["Encryption"] = "yes";
  }

  /*
   * These directives can only be modified using the
   * CPACK_INNOSETUP_SETUP_<directive> variables
   */
  setupDirectives["ShowLanguageDialog"] = "auto";
  setupDirectives["AllowNoIcons"] = "yes";
  setupDirectives["Compression"] = "lzma";
  setupDirectives["SolidCompression"] = "yes";

  // Output file and directory
  if (!RequireOption("CPACK_PACKAGE_FILE_NAME")) {
    return false;
  }
  setupDirectives["OutputBaseFilename"] = GetOption("CPACK_PACKAGE_FILE_NAME");

  if (!RequireOption("CPACK_TOPLEVEL_DIRECTORY")) {
    return false;
  }
  setupDirectives["OutputDir"] = cmSystemTools::ConvertToWindowsOutputPath(
    GetOption("CPACK_TOPLEVEL_DIRECTORY"));

  setupDirectives["SourceDir"] =
    cmSystemTools::ConvertToWindowsOutputPath(toplevel);

  // Target architecture
  if (!RequireOption("CPACK_INNOSETUP_ARCHITECTURE")) {
    return false;
  }

  cmValue const architecture = GetOption("CPACK_INNOSETUP_ARCHITECTURE");
  if (architecture != "x86" && architecture != "x64" &&
      architecture != "arm64" && architecture != "ia64") {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "CPACK_INNOSETUP_ARCHITECTURE must be either x86, x64, "
                  "arm64 or ia64"
                    << std::endl);
    return false;
  }

  // The following directives must not be set to target x86
  if (architecture != "x86") {
    setupDirectives["ArchitecturesAllowed"] = architecture;
    setupDirectives["ArchitecturesInstallIn64BitMode"] = architecture;
  }

  /*
   * Handle custom directives (they have higher priority than other variables,
   * so they have to be processed after all other variables)
   */
  for (const std::string& i : GetOptions()) {
    if (cmHasPrefix(i, "CPACK_INNOSETUP_SETUP_")) {
      const std::string& directive =
        i.substr(cmStrLen("CPACK_INNOSETUP_SETUP_"));
      setupDirectives[directive] = GetOption(i);
    }
  }

  return true;
}

bool cmCPackInnoSetupGenerator::ProcessFiles()
{
  std::map<std::string, std::string> customFileInstructions;
  if (IsSet("CPACK_INNOSETUP_CUSTOM_INSTALL_INSTRUCTIONS")) {
    const cmList instructions(
      GetOption("CPACK_INNOSETUP_CUSTOM_INSTALL_INSTRUCTIONS"));
    if (instructions.size() % 2 != 0) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "CPACK_INNOSETUP_CUSTOM_INSTALL_INSTRUCTIONS should "
                    "contain pairs of <path> and <instruction>"
                      << std::endl);
      return false;
    }

    for (auto it = instructions.begin(); it != instructions.end(); ++it) {
      const std::string& key =
        QuotePath(cmSystemTools::CollapseFullPath(*it, toplevel));
      customFileInstructions[key] = *(++it);
    }
  }

  const std::string& iconsPrefix =
    toplevelProgramFolder ? "{autoprograms}\\" : "{group}\\";

  std::map<std::string, std::string> icons;
  if (IsSet("CPACK_PACKAGE_EXECUTABLES")) {
    const cmList executables(GetOption("CPACK_PACKAGE_EXECUTABLES"));
    if (executables.size() % 2 != 0) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "CPACK_PACKAGE_EXECUTABLES should should contain pairs of "
                    "<executable> and <text label>"
                      << std::endl);
      return false;
    }

    for (auto it = executables.begin(); it != executables.end(); ++it) {
      const std::string& key = *it;
      icons[key] = *(++it);
    }
  }

  std::vector<std::string> desktopIcons;
  if (IsSet("CPACK_CREATE_DESKTOP_LINKS")) {
    cmExpandList(GetOption("CPACK_CREATE_DESKTOP_LINKS"), desktopIcons);
  }

  std::vector<std::string> runExecutables;
  if (IsSet("CPACK_INNOSETUP_RUN_EXECUTABLES")) {
    cmExpandList(GetOption("CPACK_INNOSETUP_RUN_EXECUTABLES"), runExecutables);
  }

  for (const std::string& i : files) {
    cmCPackInnoSetupKeyValuePairs params;

    std::string toplevelDirectory;
    std::string outputDir;
    cmCPackComponent* component = nullptr;
    std::string componentParam;
    if (!Components.empty()) {
      const std::string& fileName = cmSystemTools::RelativePath(toplevel, i);
      const std::string::size_type pos = fileName.find('/');

      // Use the custom component install directory if we have one
      if (pos != std::string::npos) {
        const std::string& componentName = fileName.substr(0, pos);
        component = &Components[componentName];

        toplevelDirectory =
          cmSystemTools::CollapseFullPath(componentName, toplevel);
        outputDir = CustomComponentInstallDirectory(component);
        componentParam =
          CreateRecursiveComponentPath(component->Group, component->Name);

        if (component->IsHidden && component->IsDisabledByDefault) {
          continue;
        }

        if (component->IsHidden) {
          componentParam.clear();
        }
      } else {
        // Don't install component directories
        continue;
      }
    } else {
      toplevelDirectory = toplevel;
      outputDir = "{app}";
    }

    if (!componentParam.empty()) {
      params["Components"] = componentParam;
    }

    if (cmSystemTools::FileIsDirectory(i)) {
      // Custom instructions replace the automatic generated instructions
      if (customFileInstructions.count(QuotePath(i))) {
        dirInstructions.push_back(customFileInstructions[QuotePath(i)]);
      } else {
        std::string destDir = cmSystemTools::ConvertToWindowsOutputPath(
          cmStrCat(outputDir, '\\',
                   cmSystemTools::RelativePath(toplevelDirectory, i)));
        cmStripSuffixIfExists(destDir, '\\');

        params["Name"] = QuotePath(destDir);

        dirInstructions.push_back(ISKeyValueLine(params));
      }
    } else {
      // Custom instructions replace the automatic generated instructions
      if (customFileInstructions.count(QuotePath(i))) {
        fileInstructions.push_back(customFileInstructions[QuotePath(i)]);
      } else {
        std::string destDir = cmSystemTools::ConvertToWindowsOutputPath(
          cmStrCat(outputDir, '\\',
                   cmSystemTools::GetParentDirectory(
                     cmSystemTools::RelativePath(toplevelDirectory, i))));
        cmStripSuffixIfExists(destDir, '\\');

        params["DestDir"] = QuotePath(destDir);

        if (component != nullptr && component->IsDownloaded) {
          const std::string& archiveName =
            cmSystemTools::GetFilenameWithoutLastExtension(
              component->ArchiveFile);
          const std::string& relativePath =
            cmSystemTools::RelativePath(toplevelDirectory, i);

          params["Source"] =
            QuotePath(cmStrCat("{tmp}\\", archiveName, '\\', relativePath));
          params["ExternalSize"] =
            std::to_string(cmSystemTools::FileLength(i));
          params["Flags"] = "external ignoreversion";
          params["BeforeInstall"] =
            cmStrCat("CPackExtractFile('", archiveName, "', '",
                     cmRemoveQuotes(cmSystemTools::ConvertToWindowsOutputPath(
                       relativePath)),
                     "')");
        } else {
          params["Source"] = QuotePath(i);
          params["Flags"] = "ignoreversion";
        }

        fileInstructions.push_back(ISKeyValueLine(params));

        // Icon
        const std::string& name =
          cmSystemTools::GetFilenameWithoutLastExtension(i);
        const std::string& extension =
          cmSystemTools::GetFilenameLastExtension(i);
        if ((extension == ".exe" || extension == ".com") && // only .exe, .com
            icons.count(name)) {
          cmCPackInnoSetupKeyValuePairs iconParams;

          iconParams["Name"] = QuotePath(cmStrCat(iconsPrefix, icons[name]));
          iconParams["Filename"] =
            QuotePath(cmStrCat(destDir, '\\', name, extension));

          if (!componentParam.empty()) {
            iconParams["Components"] = componentParam;
          }

          iconInstructions.push_back(ISKeyValueLine(iconParams));

          // Desktop icon
          if (std::find(desktopIcons.begin(), desktopIcons.end(), name) !=
              desktopIcons.end()) {
            iconParams["Name"] =
              QuotePath(cmStrCat("{autodesktop}\\", icons[name]));
            iconParams["Tasks"] = "desktopicon";

            if (!componentParam.empty() &&
                std::find(desktopIconComponents.begin(),
                          desktopIconComponents.end(),
                          componentParam) == desktopIconComponents.end()) {
              desktopIconComponents.push_back(componentParam);
            }
            iconInstructions.push_back(ISKeyValueLine(iconParams));
          }

          // [Run] section
          if (std::find(runExecutables.begin(), runExecutables.end(), name) !=
              runExecutables.end()) {
            cmCPackInnoSetupKeyValuePairs runParams;

            runParams["Filename"] = iconParams["Filename"];
            runParams["Description"] = cmStrCat(
              "\"{cm:LaunchProgram,", PrepareForConstant(icons[name]), "}\"");
            runParams["Flags"] = "nowait postinstall skipifsilent";

            if (!componentParam.empty()) {
              runParams["Components"] = componentParam;
            }

            runInstructions.push_back(ISKeyValueLine(runParams));
          }
        }
      }
    }
  }

  // Additional icons
  static cmsys::RegularExpression urlRegex(
    "^(mailto:|(ftps?|https?|news)://).*$");

  if (IsSet("CPACK_INNOSETUP_MENU_LINKS")) {
    const cmList menuIcons(GetOption("CPACK_INNOSETUP_MENU_LINKS"));
    if (menuIcons.size() % 2 != 0) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "CPACK_INNOSETUP_MENU_LINKS should "
                    "contain pairs of <shortcut target> and <shortcut label>"
                      << std::endl);
      return false;
    }

    for (auto it = menuIcons.begin(); it != menuIcons.end(); ++it) {
      const std::string& target = *it;
      const std::string& label = *(++it);
      cmCPackInnoSetupKeyValuePairs params;

      params["Name"] = QuotePath(cmStrCat(iconsPrefix, label));
      if (urlRegex.find(target)) {
        params["Filename"] = Quote(target);
      } else {
        std::string dir = "{app}";
        std::string componentName;
        for (const auto& i : Components) {
          if (cmSystemTools::FileExists(cmSystemTools::CollapseFullPath(
                cmStrCat(i.second.Name, '\\', target), toplevel))) {
            dir = CustomComponentInstallDirectory(&i.second);
            componentName =
              CreateRecursiveComponentPath(i.second.Group, i.second.Name);

            if (i.second.IsHidden && i.second.IsDisabledByDefault) {
              goto continueOuterLoop;
            } else if (i.second.IsHidden) {
              componentName.clear();
            }

            break;
          }
        }

        params["Filename"] = QuotePath(cmStrCat(dir, '\\', target));

        if (!componentName.empty()) {
          params["Components"] = componentName;
        }
      }

      iconInstructions.push_back(ISKeyValueLine(params));
    continueOuterLoop:;
    }
  }

  SetOptionIfNotSet("CPACK_INNOSETUP_CREATE_UNINSTALL_LINK", "OFF");
  if (GetOption("CPACK_INNOSETUP_CREATE_UNINSTALL_LINK").IsOn()) {
    cmCPackInnoSetupKeyValuePairs params;

    params["Name"] = QuotePath(
      cmStrCat(iconsPrefix, "{cm:UninstallProgram,",
               PrepareForConstant(GetOption("CPACK_PACKAGE_NAME")), '}'));
    params["Filename"] = "\"{uninstallexe}\"";

    iconInstructions.push_back(ISKeyValueLine(params));
  }

  return true;
}

bool cmCPackInnoSetupGenerator::ProcessComponents()
{
  codeIncludes.emplace_back(
    "{ The following lines are required by CPack because "
    "this script uses components }");

  // Installation types
  std::vector<cmCPackInstallationType*> types(InstallationTypes.size());
  for (auto& i : InstallationTypes) {
    types[i.second.Index - 1] = &i.second;
  }

  std::vector<std::string> allTypes; // For required components
  for (cmCPackInstallationType* i : types) {
    cmCPackInnoSetupKeyValuePairs params;

    params["Name"] = Quote(i->Name);
    params["Description"] = Quote(i->DisplayName);

    allTypes.push_back(i->Name);
    typeInstructions.push_back(ISKeyValueLine(params));
  }

  // Inno Setup requires the additional "custom" type
  cmCPackInnoSetupKeyValuePairs customTypeParams;

  customTypeParams["Name"] = "\"custom\"";
  customTypeParams["Description"] =
    "\"{code:CPackGetCustomInstallationMessage}\"";
  customTypeParams["Flags"] = "iscustom";

  allTypes.emplace_back("custom");
  typeInstructions.push_back(ISKeyValueLine(customTypeParams));

  // Components
  std::vector<cmCPackComponent*> downloadedComponents;
  std::stack<cmCPackComponentGroup*> groups;
  for (auto& i : Components) {
    cmCPackInnoSetupKeyValuePairs params;
    cmCPackComponent* component = &i.second;

    if (component->IsHidden) {
      continue;
    }

    CreateRecursiveComponentGroups(component->Group);

    params["Name"] =
      Quote(CreateRecursiveComponentPath(component->Group, component->Name));
    params["Description"] = Quote(component->DisplayName);

    if (component->IsRequired) {
      params["Types"] = cmJoin(allTypes, " ");
      params["Flags"] = "fixed";
    } else if (!component->InstallationTypes.empty()) {
      std::vector<std::string> installationTypes;

      installationTypes.reserve(component->InstallationTypes.size());
      for (cmCPackInstallationType* j : component->InstallationTypes) {
        installationTypes.push_back(j->Name);
      }

      params["Types"] = cmJoin(installationTypes, " ");
    }

    componentInstructions.push_back(ISKeyValueLine(params));

    if (component->IsDownloaded) {
      downloadedComponents.push_back(component);

      if (component->ArchiveFile.empty()) {
        // Compute the name of the archive.
        if (!RequireOption("CPACK_TEMPORARY_DIRECTORY")) {
          return false;
        }

        std::string packagesDir =
          cmStrCat(GetOption("CPACK_TEMPORARY_DIRECTORY"), ".dummy");
        component->ArchiveFile =
          cmStrCat(cmSystemTools::GetFilenameWithoutLastExtension(packagesDir),
                   '-', component->Name, ".zip");
      } else if (!cmHasSuffix(component->ArchiveFile, ".zip")) {
        component->ArchiveFile = cmStrCat(component->ArchiveFile, ".zip");
      }
    }
  }

  // Downloaded components
  if (!downloadedComponents.empty()) {
    // Create the directory for the upload area
    cmValue userUploadDirectory = GetOption("CPACK_UPLOAD_DIRECTORY");
    std::string uploadDirectory;
    if (cmNonempty(userUploadDirectory)) {
      uploadDirectory = *userUploadDirectory;
    } else {
      if (!RequireOption("CPACK_PACKAGE_DIRECTORY")) {
        return false;
      }

      uploadDirectory =
        cmStrCat(GetOption("CPACK_PACKAGE_DIRECTORY"), "/CPackUploads");
    }

    if (!cmSystemTools::FileExists(uploadDirectory)) {
      if (!cmSystemTools::MakeDirectory(uploadDirectory)) {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
                      "Unable to create Inno Setup upload directory "
                        << uploadDirectory << std::endl);
        return false;
      }
    }

    if (!RequireOption("CPACK_DOWNLOAD_SITE")) {
      return false;
    }

    SetOptionIfNotSet("CPACK_INNOSETUP_VERIFY_DOWNLOADS", "ON");
    const bool verifyDownloads =
      GetOption("CPACK_INNOSETUP_VERIFY_DOWNLOADS").IsOn();

    const std::string& urlPrefix =
      cmHasSuffix(GetOption("CPACK_DOWNLOAD_SITE").GetCStr(), '/')
      ? GetOption("CPACK_DOWNLOAD_SITE")
      : cmStrCat(GetOption("CPACK_DOWNLOAD_SITE"), '/');

    std::vector<std::string> archiveUrls;
    std::vector<std::string> archiveFiles;
    std::vector<std::string> archiveHashes;
    std::vector<std::string> archiveComponents;
    for (cmCPackComponent* i : downloadedComponents) {
      std::string hash;
      if (!BuildDownloadedComponentArchive(
            i, uploadDirectory, (verifyDownloads ? &hash : nullptr))) {
        return false;
      }

      archiveUrls.push_back(Quote(cmStrCat(urlPrefix, i->ArchiveFile)));
      archiveFiles.push_back(
        Quote(cmSystemTools::GetFilenameWithoutLastExtension(i->ArchiveFile)));
      archiveHashes.push_back(Quote(hash));
      archiveComponents.push_back(
        Quote(CreateRecursiveComponentPath(i->Group, i->Name)));
    }

    SetOption("CPACK_INNOSETUP_DOWNLOAD_COUNT_INTERNAL",
              std::to_string(archiveFiles.size()));
    SetOption("CPACK_INNOSETUP_DOWNLOAD_URLS_INTERNAL",
              cmJoin(archiveUrls, ", "));
    SetOption("CPACK_INNOSETUP_DOWNLOAD_ARCHIVES_INTERNAL",
              cmJoin(archiveFiles, ", "));
    SetOption("CPACK_INNOSETUP_DOWNLOAD_HASHES_INTERNAL",
              cmJoin(archiveHashes, ", "));
    SetOption("CPACK_INNOSETUP_DOWNLOAD_COMPONENTS_INTERNAL",
              cmJoin(archiveComponents, ", "));

    static const std::string& downloadLines =
      "#define protected CPackDownloadCount "
      "@CPACK_INNOSETUP_DOWNLOAD_COUNT_INTERNAL@\n"
      "#dim protected CPackDownloadUrls[CPackDownloadCount] "
      "{@CPACK_INNOSETUP_DOWNLOAD_URLS_INTERNAL@}\n"
      "#dim protected CPackDownloadArchives[CPackDownloadCount] "
      "{@CPACK_INNOSETUP_DOWNLOAD_ARCHIVES_INTERNAL@}\n"
      "#dim protected CPackDownloadHashes[CPackDownloadCount] "
      "{@CPACK_INNOSETUP_DOWNLOAD_HASHES_INTERNAL@}\n"
      "#dim protected CPackDownloadComponents[CPackDownloadCount] "
      "{@CPACK_INNOSETUP_DOWNLOAD_COMPONENTS_INTERNAL@}";

    std::string output;
    if (!ConfigureString(downloadLines, output)) {
      return false;
    }
    codeIncludes.push_back(output);
  }

  // Add the required script
  const std::string& componentsScriptTemplate =
    FindTemplate("ISComponents.pas");
  if (componentsScriptTemplate.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Could not find additional Inno Setup script file."
                    << std::endl);
    return false;
  }

  codeIncludes.push_back("#include " + QuotePath(componentsScriptTemplate) +
                         "\n");

  return true;
}

bool cmCPackInnoSetupGenerator::ConfigureISScript()
{
  const std::string& isScriptTemplate = FindTemplate("ISScript.template.in");
  const std::string& isScriptFile =
    cmStrCat(GetOption("CPACK_TOPLEVEL_DIRECTORY"), "/ISScript.iss");

  if (isScriptTemplate.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Could not find Inno Setup installer template file."
                    << std::endl);
    return false;
  }

  // Create internal variables
  std::vector<std::string> setupSection;
  for (const auto& i : setupDirectives) {
    setupSection.emplace_back(cmStrCat(i.first, '=', TranslateBool(i.second)));
  }

  // Also create comments if the sections are empty
  const std::string& defaultMessage =
    "; CPack didn't find any entries for this section";

  if (IsSet("CPACK_CREATE_DESKTOP_LINKS") &&
      !GetOption("CPACK_CREATE_DESKTOP_LINKS").Get()->empty()) {
    cmCPackInnoSetupKeyValuePairs tasks;
    tasks["Name"] = "\"desktopicon\"";
    tasks["Description"] = "\"{cm:CreateDesktopIcon}\"";
    tasks["GroupDescription"] = "\"{cm:AdditionalIcons}\"";
    tasks["Flags"] = "unchecked";

    if (!desktopIconComponents.empty()) {
      tasks["Components"] = cmJoin(desktopIconComponents, " ");
    }

    SetOption("CPACK_INNOSETUP_TASKS_INTERNAL", ISKeyValueLine(tasks));
  } else {
    SetOption("CPACK_INNOSETUP_TASKS_INTERNAL", defaultMessage);
  }

  SetOption("CPACK_INNOSETUP_INCLUDES_INTERNAL",
            includeDirectives.empty() ? "; No extra script files specified"
                                      : cmJoin(includeDirectives, "\n"));
  SetOption("CPACK_INNOSETUP_SETUP_INTERNAL",
            setupSection.empty() ? defaultMessage
                                 : cmJoin(setupSection, "\n"));
  SetOption("CPACK_INNOSETUP_LANGUAGES_INTERNAL",
            languageInstructions.empty() ? defaultMessage
                                         : cmJoin(languageInstructions, "\n"));
  SetOption("CPACK_INNOSETUP_DIRS_INTERNAL",
            dirInstructions.empty() ? defaultMessage
                                    : cmJoin(dirInstructions, "\n"));
  SetOption("CPACK_INNOSETUP_FILES_INTERNAL",
            fileInstructions.empty() ? defaultMessage
                                     : cmJoin(fileInstructions, "\n"));
  SetOption("CPACK_INNOSETUP_TYPES_INTERNAL",
            typeInstructions.empty() ? defaultMessage
                                     : cmJoin(typeInstructions, "\n"));
  SetOption("CPACK_INNOSETUP_COMPONENTS_INTERNAL",
            componentInstructions.empty()
              ? defaultMessage
              : cmJoin(componentInstructions, "\n"));
  SetOption("CPACK_INNOSETUP_ICONS_INTERNAL",
            iconInstructions.empty() ? defaultMessage
                                     : cmJoin(iconInstructions, "\n"));
  SetOption("CPACK_INNOSETUP_RUN_INTERNAL",
            runInstructions.empty() ? defaultMessage
                                    : cmJoin(runInstructions, "\n"));
  SetOption("CPACK_INNOSETUP_CODE_INTERNAL",
            codeIncludes.empty() ? "{ No extra code files specified }"
                                 : cmJoin(codeIncludes, "\n"));

  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Configure file: " << isScriptTemplate << " to "
                                   << isScriptFile << std::endl);

  return ConfigureFile(isScriptTemplate, isScriptFile);
}

bool cmCPackInnoSetupGenerator::Compile()
{
  const std::string& isScriptFile =
    cmStrCat(GetOption("CPACK_TOPLEVEL_DIRECTORY"), "/ISScript.iss");
  const std::string& isccLogFile =
    cmStrCat(GetOption("CPACK_TOPLEVEL_DIRECTORY"), "/ISCCOutput.log");

  std::vector<std::string> isccArgs;

  // Custom defines
  for (const std::string& i : GetOptions()) {
    if (cmHasPrefix(i, "CPACK_INNOSETUP_DEFINE_")) {
      const std::string& name = i.substr(cmStrLen("CPACK_INNOSETUP_DEFINE_"));
      isccArgs.push_back(
        cmStrCat("\"/D", name, '=', TranslateBool(GetOption(i)), '"'));
    }
  }

  if (IsSet("CPACK_INNOSETUP_EXECUTABLE_ARGUMENTS")) {
    const cmList args(GetOption("CPACK_INNOSETUP_EXECUTABLE_ARGUMENTS"));

    isccArgs.insert(isccArgs.end(), args.begin(), args.end());
  }

  const std::string& isccCmd =
    cmStrCat(QuotePath(GetOption("CPACK_INSTALLER_PROGRAM")), ' ',
             cmJoin(isccArgs, " "), ' ', QuotePath(isScriptFile));

  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Execute: " << isccCmd << std::endl);

  std::string output;
  int retVal = 1;
  const bool res = cmSystemTools::RunSingleCommand(
    isccCmd, &output, &output, &retVal, nullptr, this->GeneratorVerbose,
    cmDuration::zero());

  if (!res || retVal) {
    cmGeneratedFileStream ofs(isccLogFile);
    ofs << "# Run command: " << isccCmd << std::endl
        << "# Output:" << std::endl
        << output << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Problem running ISCC. Please check "
                    << isccLogFile << " for errors." << std::endl);
    return false;
  }

  return true;
}

bool cmCPackInnoSetupGenerator::BuildDownloadedComponentArchive(
  cmCPackComponent* component, const std::string& uploadDirectory,
  std::string* hash)
{
  // Remove the old archive, if one exists
  const std::string& archiveFile =
    uploadDirectory + '/' + component->ArchiveFile;
  cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                "-   Building downloaded component archive: " << archiveFile
                                                              << std::endl);
  if (cmSystemTools::FileExists(archiveFile, true)) {
    if (!cmSystemTools::RemoveFile(archiveFile)) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Unable to remove archive file " << archiveFile
                                                     << std::endl);
      return false;
    }
  }

  // Find a ZIP program
  if (!IsSet("ZIP_EXECUTABLE")) {
    ReadListFile("Internal/CPack/CPackZIP.cmake");

    if (!IsSet("ZIP_EXECUTABLE")) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Unable to find ZIP program" << std::endl);
      return false;
    }
  }

  if (!RequireOption("CPACK_TOPLEVEL_DIRECTORY") ||
      !RequireOption("CPACK_TEMPORARY_DIRECTORY") ||
      !RequireOption("CPACK_ZIP_NEED_QUOTES") ||
      !RequireOption("CPACK_ZIP_COMMAND")) {
    return false;
  }

  // The directory where this component's files reside
  const std::string& dirName =
    cmStrCat(GetOption("CPACK_TEMPORARY_DIRECTORY"), '/', component->Name);

  // Build the list of files to go into this archive
  const std::string& zipListFileName =
    cmStrCat(GetOption("CPACK_TEMPORARY_DIRECTORY"), "/winZip.filelist");
  const bool needQuotesInFile = cmIsOn(GetOption("CPACK_ZIP_NEED_QUOTES"));
  { // the scope is needed for cmGeneratedFileStream
    cmGeneratedFileStream out(zipListFileName);
    for (const std::string& i : component->Files) {
      out << (needQuotesInFile ? Quote(i) : i) << std::endl;
    }
  }

  // Build the archive in the upload area
  std::string cmd = GetOption("CPACK_ZIP_COMMAND");
  cmsys::SystemTools::ReplaceString(cmd, "<ARCHIVE>", archiveFile.c_str());
  cmsys::SystemTools::ReplaceString(cmd, "<FILELIST>",
                                    zipListFileName.c_str());
  std::string output;
  int retVal = -1;
  const bool res = cmSystemTools::RunSingleCommand(
    cmd, &output, &output, &retVal, dirName.c_str(), this->GeneratorVerbose,
    cmDuration::zero());
  if (!res || retVal) {
    std::string tmpFile =
      cmStrCat(GetOption("CPACK_TOPLEVEL_DIRECTORY"), "/CompressZip.log");
    cmGeneratedFileStream ofs(tmpFile);
    ofs << "# Run command: " << cmd << std::endl
        << "# Output:" << std::endl
        << output << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Problem running zip command: " << cmd << std::endl
                                                  << "Please check " << tmpFile
                                                  << " for errors"
                                                  << std::endl);
    return false;
  }

  // Try to get the SHA256 hash of the archive file
  if (hash == nullptr) {
    return true;
  }

#ifdef _WIN32
  const std::string& hashCmd =
    cmStrCat("certutil -hashfile ", QuotePath(archiveFile), " SHA256");

  std::string hashOutput;
  int hashRetVal = -1;
  const bool hashRes = cmSystemTools::RunSingleCommand(
    hashCmd, &hashOutput, &hashOutput, &hashRetVal, nullptr,
    this->GeneratorVerbose, cmDuration::zero());
  if (!hashRes || hashRetVal) {
    cmCPackLogger(cmCPackLog::LOG_WARNING,
                  "Problem running certutil command: " << hashCmd
                                                       << std::endl);
  }
  *hash = cmTrimWhitespace(cmTokenize(hashOutput, "\n").at(1));

  if (hash->length() != 64) {
    cmCPackLogger(cmCPackLog::LOG_WARNING,
                  "Problem parsing certutil output of command: " << hashCmd
                                                                 << std::endl);
    hash->clear();
  }
#endif

  return true;
}

cmValue cmCPackInnoSetupGenerator::RequireOption(const std::string& key)
{
  cmValue value = GetOption(key);

  if (!value) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Required variable " << key << " not set" << std::endl);
  }

  return value;
}

std::string cmCPackInnoSetupGenerator::CustomComponentInstallDirectory(
  const cmCPackComponent* component)
{
  cmValue outputDir = GetOption(
    cmStrCat("CPACK_INNOSETUP_", component->Name, "_INSTALL_DIRECTORY"));
  if (outputDir) {
    std::string destDir = cmSystemTools::ConvertToWindowsOutputPath(outputDir);
    cmStripSuffixIfExists(destDir, '\\');

    /*
     * Add a dir instruction for the custom directory
     * (only once and not for Inno Setup constants ending with '}')
     */
    static std::vector<std::string> customDirectories;
    if (!cmHasSuffix(destDir, '}') &&
        std::find(customDirectories.begin(), customDirectories.end(),
                  component->Name) == customDirectories.end()) {
      cmCPackInnoSetupKeyValuePairs params;
      params["Name"] = QuotePath(destDir);
      params["Components"] =
        CreateRecursiveComponentPath(component->Group, component->Name);

      dirInstructions.push_back(ISKeyValueLine(params));
      customDirectories.push_back(component->Name);
    }
    return destDir;
  }

  return "{app}";
}

std::string cmCPackInnoSetupGenerator::TranslateBool(const std::string& value)
{
  if (value.empty()) {
    return value;
  }

  SetOptionIfNotSet("CPACK_INNOSETUP_USE_CMAKE_BOOL_FORMAT", "ON");
  if (GetOption("CPACK_INNOSETUP_USE_CMAKE_BOOL_FORMAT").IsOn()) {
    if (cmIsOn(value)) {
      return "yes";
    }
    if (cmIsOff(value)) {
      return "no";
    }
  }

  return value;
}

std::string cmCPackInnoSetupGenerator::ISKeyValueLine(
  const cmCPackInnoSetupKeyValuePairs& params)
{
  /*
   * To simplify readability of the generated code, the keys are sorted.
   * Unknown keys are ignored to avoid errors during compilation.
   */
  static const char* const availableKeys[] = {
    "Source",       "DestDir",          "Name",         "Filename",
    "Description",  "GroupDescription", "MessagesFile", "Types",
    "ExternalSize", "BeforeInstall",    "Flags",        "Components",
    "Tasks"
  };

  std::vector<std::string> keys;
  for (const char* i : availableKeys) {
    if (params.count(i)) {
      keys.emplace_back(cmStrCat(i, ": ", params.at(i)));
    }
  }

  return cmJoin(keys, "; ");
}

std::string cmCPackInnoSetupGenerator::CreateRecursiveComponentPath(
  cmCPackComponentGroup* group, const std::string& path)
{
  if (group == nullptr) {
    return path;
  }

  const std::string& newPath =
    path.empty() ? group->Name : cmStrCat(group->Name, '\\', path);
  return CreateRecursiveComponentPath(group->ParentGroup, newPath);
}

void cmCPackInnoSetupGenerator::CreateRecursiveComponentGroups(
  cmCPackComponentGroup* group)
{
  if (group == nullptr) {
    return;
  }

  CreateRecursiveComponentGroups(group->ParentGroup);

  static std::vector<cmCPackComponentGroup*> processedGroups;
  if (std::find(processedGroups.begin(), processedGroups.end(), group) ==
      processedGroups.end()) {
    processedGroups.push_back(group);

    cmCPackInnoSetupKeyValuePairs params;

    params["Name"] = Quote(CreateRecursiveComponentPath(group));
    params["Description"] = Quote(group->DisplayName);

    componentInstructions.push_back(ISKeyValueLine(params));
  }
}

std::string cmCPackInnoSetupGenerator::Quote(const std::string& string)
{
  if (cmHasPrefix(string, '"') && cmHasSuffix(string, '"')) {
    return Quote(string.substr(1, string.length() - 2));
  }

  // Double quote syntax
  std::string nString = string;
  cmSystemTools::ReplaceString(nString, "\"", "\"\"");
  return cmStrCat('"', nString, '"');
}

std::string cmCPackInnoSetupGenerator::QuotePath(const std::string& path)
{
  return Quote(cmSystemTools::ConvertToWindowsOutputPath(path));
}

std::string cmCPackInnoSetupGenerator::PrepareForConstant(
  const std::string& string)
{
  std::string nString = string;

  cmSystemTools::ReplaceString(nString, "%", "%25"); // First replacement!
  cmSystemTools::ReplaceString(nString, "\"", "%22");
  cmSystemTools::ReplaceString(nString, ",", "%2c");
  cmSystemTools::ReplaceString(nString, "|", "%7c");
  cmSystemTools::ReplaceString(nString, "}", "%7d");

  return nString;
}
