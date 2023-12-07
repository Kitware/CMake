/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmXcFramework.h"

#include <string>

#include <cm/string_view>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmJSONHelpers.h"
#include "cmJSONState.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPlistParser.h"
#include "cmStringAlgorithms.h"
#include "cmake.h"

namespace {
struct PlistMetadata
{
  std::string CFBundlePackageType;
  std::string XCFrameworkFormatVersion;
};

auto const PlistMetadataHelper =
  cmJSONHelperBuilder::Object<PlistMetadata>{}
    .Bind("CFBundlePackageType"_s, &PlistMetadata::CFBundlePackageType,
          cmJSONHelperBuilder::String())
    .Bind("XCFrameworkFormatVersion"_s,
          &PlistMetadata::XCFrameworkFormatVersion,
          cmJSONHelperBuilder::String());

bool PlistSupportedPlatformHelper(
  cmXcFrameworkPlistSupportedPlatform& platform, const Json::Value* value,
  cmJSONState* /*state*/)
{
  if (!value) {
    return false;
  }

  if (!value->isString()) {
    return false;
  }

  if (value->asString() == "macos"_s) {
    platform = cmXcFrameworkPlistSupportedPlatform::macOS;
    return true;
  }
  if (value->asString() == "ios"_s) {
    platform = cmXcFrameworkPlistSupportedPlatform::iOS;
    return true;
  }
  if (value->asString() == "tvos"_s) {
    platform = cmXcFrameworkPlistSupportedPlatform::tvOS;
    return true;
  }
  if (value->asString() == "watchos"_s) {
    platform = cmXcFrameworkPlistSupportedPlatform::watchOS;
    return true;
  }
  if (value->asString() == "xros"_s) {
    platform = cmXcFrameworkPlistSupportedPlatform::visionOS;
    return true;
  }

  return false;
}

bool PlistSupportedPlatformVariantHelper(
  cmXcFrameworkPlistSupportedPlatformVariant& variant,
  const Json::Value* value, cmJSONState* /*state*/)
{
  if (!value) {
    return false;
  }

  if (!value->isString()) {
    return false;
  }

  if (value->asString() == "maccatalyst"_s) {
    variant = cmXcFrameworkPlistSupportedPlatformVariant::maccatalyst;
    return true;
  }
  if (value->asString() == "simulator"_s) {
    variant = cmXcFrameworkPlistSupportedPlatformVariant::simulator;
    return true;
  }

  return false;
}

auto const PlistLibraryHelper =
  cmJSONHelperBuilder::Object<cmXcFrameworkPlistLibrary>{}
    .Bind("LibraryIdentifier"_s, &cmXcFrameworkPlistLibrary::LibraryIdentifier,
          cmJSONHelperBuilder::String())
    .Bind("LibraryPath"_s, &cmXcFrameworkPlistLibrary::LibraryPath,
          cmJSONHelperBuilder::String())
    .Bind("HeadersPath"_s, &cmXcFrameworkPlistLibrary::HeadersPath,
          cmJSONHelperBuilder::String(), false)
    .Bind("SupportedArchitectures"_s,
          &cmXcFrameworkPlistLibrary::SupportedArchitectures,
          cmJSONHelperBuilder::Vector<std::string>(
            JsonErrors::EXPECTED_TYPE("array"), cmJSONHelperBuilder::String()))
    .Bind("SupportedPlatform"_s, &cmXcFrameworkPlistLibrary::SupportedPlatform,
          PlistSupportedPlatformHelper)
    .Bind("SupportedPlatformVariant"_s,
          &cmXcFrameworkPlistLibrary::SupportedPlatformVariant,
          cmJSONHelperBuilder::Optional<
            cmXcFrameworkPlistSupportedPlatformVariant>(
            PlistSupportedPlatformVariantHelper),
          false);

auto const PlistHelper =
  cmJSONHelperBuilder::Object<cmXcFrameworkPlist>{}.Bind(
    "AvailableLibraries"_s, &cmXcFrameworkPlist::AvailableLibraries,
    cmJSONHelperBuilder::Vector<cmXcFrameworkPlistLibrary>(
      JsonErrors::EXPECTED_TYPE("array"), PlistLibraryHelper));
}

cm::optional<cmXcFrameworkPlist> cmParseXcFrameworkPlist(
  const std::string& xcframeworkPath, const cmMakefile& mf,
  const cmListFileBacktrace& bt)
{
  std::string plistPath = cmStrCat(xcframeworkPath, "/Info.plist");

  auto value = cmParsePlist(plistPath);
  if (!value) {
    mf.GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Unable to parse plist file:\n  ", plistPath), bt);
    return cm::nullopt;
  }

  cmJSONState state;

  PlistMetadata metadata;
  if (!PlistMetadataHelper(metadata, &*value, &state)) {
    mf.GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Invalid xcframework .plist file:\n  ", plistPath), bt);
    return cm::nullopt;
  }
  if (metadata.CFBundlePackageType != "XFWK"_s ||
      metadata.XCFrameworkFormatVersion != "1.0"_s) {
    mf.GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Expected:\n  ", plistPath,
               "\nto have CFBundlePackageType \"XFWK\" and "
               "XCFrameworkFormatVersion \"1.0\""),
      bt);
    return cm::nullopt;
  }

  cmXcFrameworkPlist plist;
  if (!PlistHelper(plist, &*value, &state)) {
    mf.GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Invalid xcframework .plist file:\n  ", plistPath), bt);
    return cm::nullopt;
  }
  plist.Path = plistPath;
  return cm::optional<cmXcFrameworkPlist>(plist);
}

const cmXcFrameworkPlistLibrary* cmXcFrameworkPlist::SelectSuitableLibrary(
  const cmMakefile& mf, const cmListFileBacktrace& bt) const
{
  auto systemName = mf.GetSafeDefinition("CMAKE_SYSTEM_NAME");
  cm::optional<cmXcFrameworkPlistSupportedPlatformVariant> systemVariant;
  if (mf.PlatformIsAppleSimulator()) {
    systemVariant = cmXcFrameworkPlistSupportedPlatformVariant::simulator;
  }

  for (auto const& lib : this->AvailableLibraries) {
    std::string supportedSystemName;
    switch (lib.SupportedPlatform) {
      case cmXcFrameworkPlistSupportedPlatform::macOS:
        supportedSystemName = "Darwin";
        break;
      case cmXcFrameworkPlistSupportedPlatform::iOS:
        supportedSystemName = "iOS";
        break;
      case cmXcFrameworkPlistSupportedPlatform::tvOS:
        supportedSystemName = "tvOS";
        break;
      case cmXcFrameworkPlistSupportedPlatform::watchOS:
        supportedSystemName = "watchOS";
        break;
      case cmXcFrameworkPlistSupportedPlatform::visionOS:
        supportedSystemName = "visionOS";
        break;
    }

    if (systemName == supportedSystemName &&
        systemVariant == lib.SupportedPlatformVariant) {
      return &lib;
    }
  }

  mf.GetCMakeInstance()->IssueMessage(
    MessageType::FATAL_ERROR,
    cmStrCat("Unable to find suitable library in:\n  ", this->Path,
             "\nfor system name \"", systemName, '"'),
    bt);
  return nullptr;
}
