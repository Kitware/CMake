/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackDragNDropGenerator.h"

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <map>

#include <CoreFoundation/CoreFoundation.h>
#include <cm3p/kwiml/abi.h>

#include "cmsys/Base64.h"
#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmCPackConfigure.h"
#include "cmCPackGenerator.h"
#include "cmCPackLog.h"
#include "cmDuration.h"
#include "cmGeneratedFileStream.h"
#include "cmList.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmXMLWriter.h"

#if HAVE_CoreServices
// For the old LocaleStringToLangAndRegionCodes() function, to convert
// to the old Script Manager RegionCode values needed for the 'LPic' data
// structure used for generating multi-lingual SLAs.
#  include <CoreServices/CoreServices.h>
#endif

static const uint16_t DefaultLpic[] = {
  /* clang-format off */
  0x0002, 0x0011, 0x0003, 0x0001, 0x0000, 0x0000, 0x0002, 0x0000,
  0x0008, 0x0003, 0x0000, 0x0001, 0x0004, 0x0000, 0x0004, 0x0005,
  0x0000, 0x000E, 0x0006, 0x0001, 0x0005, 0x0007, 0x0000, 0x0007,
  0x0008, 0x0000, 0x0047, 0x0009, 0x0000, 0x0034, 0x000A, 0x0001,
  0x0035, 0x000B, 0x0001, 0x0020, 0x000C, 0x0000, 0x0011, 0x000D,
  0x0000, 0x005B, 0x0004, 0x0000, 0x0033, 0x000F, 0x0001, 0x000C,
  0x0010, 0x0000, 0x000B, 0x000E, 0x0000
  /* clang-format on */
};

static const std::vector<std::string> DefaultMenu = {
  { "English", "Agree", "Disagree", "Print", "Save...",
    // NOLINTNEXTLINE(bugprone-suspicious-missing-comma)
    "You agree to the License Agreement terms when "
    "you click the \"Agree\" button.",
    "Software License Agreement",
    "This text cannot be saved.  "
    "This disk may be full or locked, or the file may be locked.",
    "Unable to print.  Make sure you have selected a printer." }
};

cmCPackDragNDropGenerator::cmCPackDragNDropGenerator()
  : singleLicense(false)
{
  // default to one package file for components
  this->componentPackageMethod = ONE_PACKAGE;
}

cmCPackDragNDropGenerator::~cmCPackDragNDropGenerator() = default;

int cmCPackDragNDropGenerator::InitializeInternal()
{
  // Starting with Xcode 4.3, look in "/Applications/Xcode.app" first:
  //
  std::vector<std::string> paths;
  paths.emplace_back("/Applications/Xcode.app/Contents/Developer/Tools");
  paths.emplace_back("/Developer/Tools");

  const std::string hdiutil_path =
    cmSystemTools::FindProgram("hdiutil", std::vector<std::string>(), false);
  if (hdiutil_path.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Cannot locate hdiutil command" << std::endl);
    return 0;
  }
  this->SetOptionIfNotSet("CPACK_COMMAND_HDIUTIL", hdiutil_path);

  const std::string setfile_path =
    cmSystemTools::FindProgram("SetFile", paths, false);
  if (setfile_path.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Cannot locate SetFile command" << std::endl);
    return 0;
  }
  this->SetOptionIfNotSet("CPACK_COMMAND_SETFILE", setfile_path);

  const std::string rez_path = cmSystemTools::FindProgram("Rez", paths, false);
  if (rez_path.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Cannot locate Rez command" << std::endl);
    return 0;
  }
  this->SetOptionIfNotSet("CPACK_COMMAND_REZ", rez_path);

  if (this->IsSet("CPACK_DMG_SLA_DIR")) {
    slaDirectory = this->GetOption("CPACK_DMG_SLA_DIR");
    if (!slaDirectory.empty() &&
        this->IsOn("CPACK_DMG_SLA_USE_RESOURCE_FILE_LICENSE") &&
        this->IsSet("CPACK_RESOURCE_FILE_LICENSE")) {
      std::string license_file =
        this->GetOption("CPACK_RESOURCE_FILE_LICENSE");
      if (!license_file.empty() &&
          (license_file.find("CPack.GenericLicense.txt") ==
           std::string::npos)) {
        cmCPackLogger(
          cmCPackLog::LOG_OUTPUT,
          "Both CPACK_DMG_SLA_DIR and CPACK_RESOURCE_FILE_LICENSE specified, "
          "using CPACK_RESOURCE_FILE_LICENSE as a license for all languages."
            << std::endl);
        singleLicense = true;
      }
    }
    if (!this->IsSet("CPACK_DMG_SLA_LANGUAGES")) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "CPACK_DMG_SLA_DIR set but no languages defined "
                    "(set CPACK_DMG_SLA_LANGUAGES)"
                      << std::endl);
      return 0;
    }
    if (!cmSystemTools::FileExists(slaDirectory, false)) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "CPACK_DMG_SLA_DIR does not exist" << std::endl);
      return 0;
    }

    cmList languages{ this->GetOption("CPACK_DMG_SLA_LANGUAGES") };
    if (languages.empty()) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "CPACK_DMG_SLA_LANGUAGES set but empty" << std::endl);
      return 0;
    }
    for (auto const& language : languages) {
      std::string license = slaDirectory + "/" + language + ".license.txt";
      std::string license_rtf = slaDirectory + "/" + language + ".license.rtf";
      if (!singleLicense) {
        if (!cmSystemTools::FileExists(license) &&
            !cmSystemTools::FileExists(license_rtf)) {
          cmCPackLogger(cmCPackLog::LOG_ERROR,
                        "Missing license file "
                          << language << ".license.txt"
                          << " / " << language << ".license.rtf" << std::endl);
          return 0;
        }
      }
      std::string menu = slaDirectory + "/" + language + ".menu.txt";
      if (!cmSystemTools::FileExists(menu)) {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
                      "Missing menu file " << language << ".menu.txt"
                                           << std::endl);
        return 0;
      }
    }
  }

  return this->Superclass::InitializeInternal();
}

const char* cmCPackDragNDropGenerator::GetOutputExtension()
{
  return ".dmg";
}

int cmCPackDragNDropGenerator::PackageFiles()
{
  // gather which directories to make dmg files for
  // multiple directories occur if packaging components or groups separately

  // monolith
  if (this->Components.empty()) {
    return this->CreateDMG(toplevel, packageFileNames[0]);
  }

  // component install
  std::vector<std::string> package_files;

  std::map<std::string, cmCPackComponent>::iterator compIt;
  for (compIt = this->Components.begin(); compIt != this->Components.end();
       ++compIt) {
    std::string name = GetComponentInstallDirNameSuffix(compIt->first);
    package_files.push_back(name);
  }
  std::sort(package_files.begin(), package_files.end());
  package_files.erase(std::unique(package_files.begin(), package_files.end()),
                      package_files.end());

  // loop to create dmg files
  packageFileNames.clear();
  for (auto const& package_file : package_files) {
    std::string full_package_name = std::string(toplevel) + std::string("/");
    if (package_file == "ALL_IN_ONE") {
      full_package_name += this->GetOption("CPACK_PACKAGE_FILE_NAME");
    } else {
      full_package_name += package_file;
    }
    full_package_name += std::string(GetOutputExtension());
    packageFileNames.push_back(full_package_name);

    std::string src_dir = cmStrCat(toplevel, '/', package_file);

    if (0 == this->CreateDMG(src_dir, full_package_name)) {
      return 0;
    }
  }
  return 1;
}

bool cmCPackDragNDropGenerator::CopyFile(std::ostringstream& source,
                                         std::ostringstream& target)
{
  if (!cmSystemTools::CopyFileIfDifferent(source.str(), target.str())) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error copying " << source.str() << " to " << target.str()
                                   << std::endl);

    return false;
  }

  return true;
}

bool cmCPackDragNDropGenerator::CreateEmptyFile(std::ostringstream& target,
                                                size_t size)
{
  cmsys::ofstream fout(target.str().c_str(), std::ios::out | std::ios::binary);
  if (!fout) {
    return false;
  }

  // Seek to desired size - 1 byte
  fout.seekp(size - 1, std::ios::beg);
  char byte = 0;
  // Write one byte to ensure file grows
  fout.write(&byte, 1);

  return true;
}

bool cmCPackDragNDropGenerator::RunCommand(std::ostringstream& command,
                                           std::string* output)
{
  int exit_code = 1;

  bool result = cmSystemTools::RunSingleCommand(
    command.str(), output, output, &exit_code, nullptr, this->GeneratorVerbose,
    cmDuration::zero());

  if (!result || exit_code) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error executing: " << command.str() << std::endl);

    return false;
  }

  return true;
}

int cmCPackDragNDropGenerator::CreateDMG(const std::string& src_dir,
                                         const std::string& output_file)
{
  // Get optional arguments ...
  cmValue cpack_package_icon = this->GetOption("CPACK_PACKAGE_ICON");

  const std::string cpack_dmg_volume_name =
    this->GetOption("CPACK_DMG_VOLUME_NAME")
    ? *this->GetOption("CPACK_DMG_VOLUME_NAME")
    : *this->GetOption("CPACK_PACKAGE_FILE_NAME");

  const std::string cpack_dmg_format = this->GetOption("CPACK_DMG_FORMAT")
    ? *this->GetOption("CPACK_DMG_FORMAT")
    : "UDZO";

  const std::string cpack_dmg_filesystem =
    this->GetOption("CPACK_DMG_FILESYSTEM")
    ? *this->GetOption("CPACK_DMG_FILESYSTEM")
    : "HFS+";

  // Get optional arguments ...
  std::string cpack_license_file;
  if (this->IsOn("CPACK_DMG_SLA_USE_RESOURCE_FILE_LICENSE")) {
    cpack_license_file = *this->GetOption("CPACK_RESOURCE_FILE_LICENSE");
  }

  cmValue cpack_dmg_background_image =
    this->GetOption("CPACK_DMG_BACKGROUND_IMAGE");

  cmValue cpack_dmg_ds_store = this->GetOption("CPACK_DMG_DS_STORE");

  cmValue cpack_dmg_languages = this->GetOption("CPACK_DMG_SLA_LANGUAGES");

  cmValue cpack_dmg_ds_store_setup_script =
    this->GetOption("CPACK_DMG_DS_STORE_SETUP_SCRIPT");

  const bool cpack_dmg_disable_applications_symlink =
    this->IsOn("CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK");

  // only put license on dmg if is user provided
  if (!cpack_license_file.empty() &&
      cpack_license_file.find("CPack.GenericLicense.txt") !=
        std::string::npos) {
    cpack_license_file = "";
  }

  // use sla_dir if both sla_dir and license_file are set
  if (!cpack_license_file.empty() && !slaDirectory.empty() && !singleLicense) {
    cpack_license_file = "";
  }

  // The staging directory contains everything that will end-up inside the
  // final disk image ...
  std::ostringstream staging;
  staging << src_dir;

  // Add a symlink to /Applications so users can drag-and-drop the bundle
  // into it unless this behavior was disabled
  if (!cpack_dmg_disable_applications_symlink) {
    std::ostringstream application_link;
    application_link << staging.str() << "/Applications";
    cmSystemTools::CreateSymlink("/Applications", application_link.str());
  }

  // Optionally add a custom volume icon ...
  if (!cpack_package_icon->empty()) {
    std::ostringstream package_icon_source;
    package_icon_source << cpack_package_icon;

    std::ostringstream package_icon_destination;
    package_icon_destination << staging.str() << "/.VolumeIcon.icns";

    if (!this->CopyFile(package_icon_source, package_icon_destination)) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error copying disk volume icon.  "
                    "Check the value of CPACK_PACKAGE_ICON."
                      << std::endl);

      return 0;
    }
  }

  // Optionally add a custom .DS_Store file
  // (e.g. for setting background/layout) ...
  if (!cpack_dmg_ds_store->empty()) {
    std::ostringstream package_settings_source;
    package_settings_source << cpack_dmg_ds_store;

    std::ostringstream package_settings_destination;
    package_settings_destination << staging.str() << "/.DS_Store";

    if (!this->CopyFile(package_settings_source,
                        package_settings_destination)) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error copying disk volume settings file.  "
                    "Check the value of CPACK_DMG_DS_STORE."
                      << std::endl);

      return 0;
    }
  }

  // Optionally add a custom background image ...
  // Make sure the background file type is the same as the custom image
  // and that the file is hidden so it doesn't show up.
  if (!cpack_dmg_background_image->empty()) {
    const std::string extension =
      cmSystemTools::GetFilenameLastExtension(cpack_dmg_background_image);
    std::ostringstream package_background_source;
    package_background_source << cpack_dmg_background_image;

    std::ostringstream package_background_destination;
    package_background_destination << staging.str()
                                   << "/.background/background" << extension;

    if (!this->CopyFile(package_background_source,
                        package_background_destination)) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error copying disk volume background image.  "
                    "Check the value of CPACK_DMG_BACKGROUND_IMAGE."
                      << std::endl);

      return 0;
    }
  }

  bool remount_image =
    !cpack_package_icon->empty() || !cpack_dmg_ds_store_setup_script->empty();

  std::string temp_image_format = "UDZO";

  // Create 1 MB dummy padding file in staging area when we need to remount
  // image, so we have enough space for storing changes ...
  if (remount_image) {
    std::ostringstream dummy_padding;
    dummy_padding << staging.str() << "/.dummy-padding-file";
    if (!this->CreateEmptyFile(dummy_padding, 1048576)) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error creating dummy padding file." << std::endl);

      return 0;
    }
    temp_image_format = "UDRW";
  }

  // Create a temporary read-write disk image ...
  std::string temp_image =
    cmStrCat(this->GetOption("CPACK_TOPLEVEL_DIRECTORY"), "/temp.dmg");

  std::string create_error;
  std::ostringstream temp_image_command;
  temp_image_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
  temp_image_command << " create";
  temp_image_command << " -ov";
  temp_image_command << " -srcfolder \"" << staging.str() << "\"";
  temp_image_command << " -volname \"" << cpack_dmg_volume_name << "\"";
  temp_image_command << " -fs \"" << cpack_dmg_filesystem << "\"";
  temp_image_command << " -format " << temp_image_format;
  temp_image_command << " \"" << temp_image << "\"";

  if (!this->RunCommand(temp_image_command, &create_error)) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error generating temporary disk image." << std::endl
                                                           << create_error
                                                           << std::endl);

    return 0;
  }

  if (remount_image) {
    // Store that we have a failure so that we always unmount the image
    // before we exit.
    bool had_error = false;

    std::ostringstream attach_command;
    attach_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
    attach_command << " attach";
    attach_command << " \"" << temp_image << "\"";

    std::string attach_output;
    if (!this->RunCommand(attach_command, &attach_output)) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error attaching temporary disk image." << std::endl);

      return 0;
    }

    cmsys::RegularExpression mountpoint_regex(".*(/Volumes/[^\n]+)\n.*");
    mountpoint_regex.find(attach_output.c_str());
    std::string const temp_mount = mountpoint_regex.match(1);
    std::string const temp_mount_name =
      temp_mount.substr(cmStrLen("/Volumes/"));

    // Remove dummy padding file so we have enough space on RW image ...
    std::ostringstream dummy_padding;
    dummy_padding << temp_mount << "/.dummy-padding-file";
    if (!cmSystemTools::RemoveFile(dummy_padding.str())) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error removing dummy padding file." << std::endl);

      had_error = true;
    }

    // Optionally set the custom icon flag for the image ...
    if (!had_error && !cpack_package_icon->empty()) {
      std::string error;
      std::ostringstream setfile_command;
      setfile_command << this->GetOption("CPACK_COMMAND_SETFILE");
      setfile_command << " -a C";
      setfile_command << " \"" << temp_mount << "\"";

      if (!this->RunCommand(setfile_command, &error)) {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
                      "Error assigning custom icon to temporary disk image."
                        << std::endl
                        << error << std::endl);

        had_error = true;
      }
    }

    // Optionally we can execute a custom apple script to generate
    // the .DS_Store for the volume folder ...
    if (!had_error && !cpack_dmg_ds_store_setup_script->empty()) {
      std::ostringstream setup_script_command;
      setup_script_command << "osascript"
                           << " \"" << cpack_dmg_ds_store_setup_script << "\""
                           << " \"" << temp_mount_name << "\"";
      std::string error;
      if (!this->RunCommand(setup_script_command, &error)) {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
                      "Error executing custom script on disk image."
                        << std::endl
                        << error << std::endl);

        had_error = true;
      }
    }

    std::ostringstream detach_command;
    detach_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
    detach_command << " detach";
    detach_command << " \"" << temp_mount << "\"";

    if (!this->RunCommand(detach_command)) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error detaching temporary disk image." << std::endl);

      return 0;
    }

    if (had_error) {
      return 0;
    }
  }

  // Create the final compressed read-only disk image ...
  std::ostringstream final_image_command;
  final_image_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
  final_image_command << " convert \"" << temp_image << "\"";
  final_image_command << " -format ";
  final_image_command << cpack_dmg_format;
  final_image_command << " -imagekey";
  final_image_command << " zlib-level=9";
  final_image_command << " -o \"" << output_file << "\"";

  std::string convert_error;

  if (!this->RunCommand(final_image_command, &convert_error)) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error compressing disk image." << std::endl
                                                  << convert_error
                                                  << std::endl);

    return 0;
  }

  if (!cpack_license_file.empty() || !slaDirectory.empty()) {
    // Use old hardcoded style if sla_dir is not set
    bool oldStyle = slaDirectory.empty();
    std::string sla_xml =
      cmStrCat(this->GetOption("CPACK_TOPLEVEL_DIRECTORY"), "/sla.xml");

    cmList languages;
    if (!oldStyle) {
      languages.assign(cpack_dmg_languages);
    }

    std::vector<uint16_t> header_data;
    if (oldStyle) {
      header_data = std::vector<uint16_t>(
        DefaultLpic,
        DefaultLpic + (sizeof(DefaultLpic) / sizeof(*DefaultLpic)));
    } else {
      /*
       * LPic Layout
       * (https://github.com/pypt/dmg-add-license/blob/master/main.c)
       * as far as I can tell (no official documentation seems to exist):
       * struct LPic {
       *  uint16_t default_language; // points to a resid, defaulting to 0,
       *                             // which is the first set language
       *  uint16_t length;
       *  struct {
       *    uint16_t language_code;
       *    uint16_t resid;
       *    uint16_t encoding; // Encoding from TextCommon.h,
       *                       // forcing MacRoman (0) for now. Might need to
       *                       // allow overwrite per license by user later
       *  } item[1];
       * }
       */

      header_data.push_back(0);
      header_data.push_back(languages.size());
      for (cmList::size_type i = 0; i < languages.size(); ++i) {
        CFStringRef language_cfstring = CFStringCreateWithCString(
          nullptr, languages[i].c_str(), kCFStringEncodingUTF8);
        CFStringRef iso_language =
          CFLocaleCreateCanonicalLanguageIdentifierFromString(
            nullptr, language_cfstring);
        if (!iso_language) {
          cmCPackLogger(cmCPackLog::LOG_ERROR,
                        languages[i] << " is not a recognized language"
                                     << std::endl);
        }
        char iso_language_cstr[65];
        CFStringGetCString(iso_language, iso_language_cstr,
                           sizeof(iso_language_cstr) - 1,
                           kCFStringEncodingMacRoman);
        LangCode lang = 0;
        RegionCode region = 0;
#if HAVE_CoreServices
        OSStatus err =
          LocaleStringToLangAndRegionCodes(iso_language_cstr, &lang, &region);
        if (err != noErr)
#endif
        {
          cmCPackLogger(cmCPackLog::LOG_ERROR,
                        "No language/region code available for "
                          << iso_language_cstr << std::endl);
          return 0;
        }
#if HAVE_CoreServices
        header_data.push_back(region);
        header_data.push_back(i);
        header_data.push_back(0);
#endif
      }
    }

    RezDoc rez;

    {
      RezDict lpic = { {}, 5000, {} };
      lpic.Data.reserve(header_data.size() * sizeof(header_data[0]));
      for (uint16_t x : header_data) {
        // LPic header is big-endian.
        char* d = reinterpret_cast<char*>(&x);
#if KWIML_ABI_ENDIAN_ID == KWIML_ABI_ENDIAN_ID_LITTLE
        lpic.Data.push_back(d[1]);
        lpic.Data.push_back(d[0]);
#else
        lpic.Data.push_back(d[0]);
        lpic.Data.push_back(d[1]);
#endif
      }
      rez.LPic.Entries.emplace_back(std::move(lpic));
    }

    bool have_write_license_error = false;
    std::string error;

    if (oldStyle) {
      if (!this->WriteLicense(rez, 0, "", cpack_license_file, &error)) {
        have_write_license_error = true;
      }
    } else {
      for (size_t i = 0; i < languages.size() && !have_write_license_error;
           ++i) {
        if (singleLicense) {
          if (!this->WriteLicense(rez, i + 5000, languages[i],
                                  cpack_license_file, &error)) {
            have_write_license_error = true;
          }
        } else {
          if (!this->WriteLicense(rez, i + 5000, languages[i], "", &error)) {
            have_write_license_error = true;
          }
        }
      }
    }

    if (have_write_license_error) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error writing license file to SLA." << std::endl
                                                         << error
                                                         << std::endl);
      return 0;
    }

    this->WriteRezXML(sla_xml, rez);

    // Create the final compressed read-only disk image ...
    std::ostringstream embed_sla_command;
    embed_sla_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
    embed_sla_command << " udifrez";
    embed_sla_command << " -xml";
    embed_sla_command << " \"" << sla_xml << "\"";
    embed_sla_command << " FIXME_WHY_IS_THIS_ARGUMENT_NEEDED";
    embed_sla_command << " \"" << output_file << "\"";
    std::string embed_error;
    if (!this->RunCommand(embed_sla_command, &embed_error)) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error compressing disk image." << std::endl
                                                    << embed_error
                                                    << std::endl);

      return 0;
    }
  }

  return 1;
}

bool cmCPackDragNDropGenerator::SupportsComponentInstallation() const
{
  return true;
}

std::string cmCPackDragNDropGenerator::GetComponentInstallDirNameSuffix(
  const std::string& componentName)
{
  // we want to group components together that go in the same dmg package
  std::string package_file_name = this->GetOption("CPACK_PACKAGE_FILE_NAME");

  // we have 3 mutually exclusive modes to work in
  // 1. all components in one package
  // 2. each group goes in its own package with left over
  //    components in their own package
  // 3. ignore groups - if grouping is defined, it is ignored
  //    and each component goes in its own package

  if (this->componentPackageMethod == ONE_PACKAGE) {
    return "ALL_IN_ONE";
  }

  if (this->componentPackageMethod == ONE_PACKAGE_PER_GROUP) {
    // We have to find the name of the COMPONENT GROUP
    // the current COMPONENT belongs to.
    std::string groupVar =
      "CPACK_COMPONENT_" + cmSystemTools::UpperCase(componentName) + "_GROUP";
    cmValue _groupName = this->GetOption(groupVar);
    if (_groupName) {
      std::string groupName = _groupName;

      groupName =
        GetComponentPackageFileName(package_file_name, groupName, true);
      return groupName;
    }
  }

  std::string componentFileName =
    "CPACK_DMG_" + cmSystemTools::UpperCase(componentName) + "_FILE_NAME";
  if (this->IsSet(componentFileName)) {
    return this->GetOption(componentFileName);
  }
  return GetComponentPackageFileName(package_file_name, componentName, false);
}

void cmCPackDragNDropGenerator::WriteRezXML(std::string const& file,
                                            RezDoc const& rez)
{
  cmGeneratedFileStream fxml(file);
  cmXMLWriter xml(fxml);
  xml.StartDocument();
  xml.StartElement("plist");
  xml.Attribute("version", "1.0");
  xml.StartElement("dict");
  this->WriteRezArray(xml, rez.LPic);
  this->WriteRezArray(xml, rez.Menu);
  this->WriteRezArray(xml, rez.Text);
  this->WriteRezArray(xml, rez.RTF);
  xml.EndElement(); // dict
  xml.EndElement(); // plist
  xml.EndDocument();
  fxml.Close();
}

void cmCPackDragNDropGenerator::WriteRezArray(cmXMLWriter& xml,
                                              RezArray const& array)
{
  if (array.Entries.empty()) {
    return;
  }
  xml.StartElement("key");
  xml.Content(array.Key);
  xml.EndElement(); // key
  xml.StartElement("array");
  for (RezDict const& dict : array.Entries) {
    this->WriteRezDict(xml, dict);
  }
  xml.EndElement(); // array
}

void cmCPackDragNDropGenerator::WriteRezDict(cmXMLWriter& xml,
                                             RezDict const& dict)
{
  std::vector<char> base64buf(dict.Data.size() * 3 / 2 + 5);
  size_t base64len =
    cmsysBase64_Encode(dict.Data.data(), dict.Data.size(),
                       reinterpret_cast<unsigned char*>(base64buf.data()), 0);
  std::string base64data(base64buf.data(), base64len);
  /* clang-format off */
  xml.StartElement("dict");
  xml.StartElement("key");    xml.Content("Attributes"); xml.EndElement();
  xml.StartElement("string"); xml.Content("0x0000");     xml.EndElement();
  xml.StartElement("key");    xml.Content("Data");       xml.EndElement();
  xml.StartElement("data");   xml.Content(base64data);   xml.EndElement();
  xml.StartElement("key");    xml.Content("ID");         xml.EndElement();
  xml.StartElement("string"); xml.Content(dict.ID);      xml.EndElement();
  xml.StartElement("key");    xml.Content("Name");       xml.EndElement();
  xml.StartElement("string"); xml.Content(dict.Name);    xml.EndElement();
  xml.EndElement(); // dict
  /* clang-format on */
}

bool cmCPackDragNDropGenerator::WriteLicense(RezDoc& rez, size_t licenseNumber,
                                             std::string licenseLanguage,
                                             const std::string& licenseFile,
                                             std::string* error)
{
  if (!licenseFile.empty() && !singleLicense) {
    licenseNumber = 5002;
    licenseLanguage = "English";
  }

  // License file
  RezArray* licenseArray = &rez.Text;
  std::string actual_license;
  if (!licenseFile.empty()) {
    if (cmHasLiteralSuffix(licenseFile, ".rtf")) {
      licenseArray = &rez.RTF;
    }
    actual_license = licenseFile;
  } else {
    std::string license_wo_ext =
      slaDirectory + "/" + licenseLanguage + ".license";
    if (cmSystemTools::FileExists(license_wo_ext + ".txt")) {
      actual_license = license_wo_ext + ".txt";
    } else {
      licenseArray = &rez.RTF;
      actual_license = license_wo_ext + ".rtf";
    }
  }

  // License body
  {
    RezDict license = { licenseLanguage, licenseNumber, {} };
    std::vector<std::string> lines;
    if (!this->ReadFile(actual_license, lines, error)) {
      return false;
    }
    this->EncodeLicense(license, lines);
    licenseArray->Entries.emplace_back(std::move(license));
  }

  // Menu body
  {
    RezDict menu = { licenseLanguage, licenseNumber, {} };
    if (!licenseFile.empty() && !singleLicense) {
      this->EncodeMenu(menu, DefaultMenu);
    } else {
      std::vector<std::string> lines;
      std::string actual_menu =
        slaDirectory + "/" + licenseLanguage + ".menu.txt";
      if (!this->ReadFile(actual_menu, lines, error)) {
        return false;
      }
      this->EncodeMenu(menu, lines);
    }
    rez.Menu.Entries.emplace_back(std::move(menu));
  }

  return true;
}

void cmCPackDragNDropGenerator::EncodeLicense(
  RezDict& dict, std::vector<std::string> const& lines)
{
  // License text uses CR newlines.
  for (std::string const& l : lines) {
    dict.Data.insert(dict.Data.end(), l.begin(), l.end());
    dict.Data.push_back('\r');
  }
  dict.Data.push_back('\r');
}

void cmCPackDragNDropGenerator::EncodeMenu(
  RezDict& dict, std::vector<std::string> const& lines)
{
  // Menu resources start with a big-endian uint16_t for number of lines:
  {
    uint16_t numLines = static_cast<uint16_t>(lines.size());
    char* d = reinterpret_cast<char*>(&numLines);
#if KWIML_ABI_ENDIAN_ID == KWIML_ABI_ENDIAN_ID_LITTLE
    dict.Data.push_back(d[1]);
    dict.Data.push_back(d[0]);
#else
    dict.Data.push_back(d[0]);
    dict.Data.push_back(d[1]);
#endif
  }
  // Each line starts with a uint8_t length, plus the bytes themselves:
  for (std::string const& l : lines) {
    dict.Data.push_back(static_cast<unsigned char>(l.length()));
    dict.Data.insert(dict.Data.end(), l.begin(), l.end());
  }
}

bool cmCPackDragNDropGenerator::ReadFile(std::string const& file,
                                         std::vector<std::string>& lines,
                                         std::string* error)
{
  cmsys::ifstream ifs(file);
  std::string line;
  while (std::getline(ifs, line)) {
    if (!this->BreakLongLine(line, lines, error)) {
      return false;
    }
  }
  return true;
}

bool cmCPackDragNDropGenerator::BreakLongLine(const std::string& line,
                                              std::vector<std::string>& lines,
                                              std::string* error)
{
  const size_t max_line_length = 255;
  size_t line_length = max_line_length;
  for (size_t i = 0; i < line.size(); i += line_length) {
    line_length = max_line_length;
    if (i + line_length > line.size()) {
      line_length = line.size() - i;
    } else {
      while (line_length > 0 && line[i + line_length - 1] != ' ') {
        line_length = line_length - 1;
      }
    }

    if (line_length == 0) {
      *error = "Please make sure there are no words "
               "(or character sequences not broken up by spaces or newlines) "
               "in your license file which are more than 255 characters long.";
      return false;
    }
    lines.push_back(line.substr(i, line_length));
  }
  return true;
}
