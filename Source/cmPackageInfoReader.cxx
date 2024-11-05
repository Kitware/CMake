/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmPackageInfoReader.h"

#include <limits>

#include <cm/string_view>
#include <cmext/string_view>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>
#include <cm3p/json/version.h>

#include "cmsys/FStream.hxx"

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {

Json::Value ReadJson(std::string const& fileName)
{
  // Open the specified file.
  cmsys::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary);
  if (!file) {
#if JSONCPP_VERSION_HEXA < 0x01070300
    return Json::Value::null;
#else
    return Json::Value::nullSingleton();
#endif
  }

  // Read file content and translate JSON.
  Json::Value data;
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  if (!Json::parseFromStream(builder, file, &data, nullptr)) {
#if JSONCPP_VERSION_HEXA < 0x01070300
    return Json::Value::null;
#else
    return Json::Value::nullSingleton();
#endif
  }

  return data;
}

bool CheckSchemaVersion(Json::Value const& data)
{
  std::string const& version = data["cps_version"].asString();

  // Check that a valid version is specified.
  if (version.empty()) {
    return false;
  }

  // Check that we understand this version.
  return cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER_EQUAL,
                                       version, "0.12") &&
    cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, version, "1");

  // TODO Eventually this probably needs to return the version tuple, and
  // should share code with cmPackageInfoReader::ParseVersion.
}

} // namespace

std::unique_ptr<cmPackageInfoReader> cmPackageInfoReader::Read(
  std::string const& path)
{
  // Read file and perform some basic validation:
  //   - the input is valid JSON
  //   - the input is a JSON object
  //   - the input has a "cps_version" that we (in theory) know how to parse
  Json::Value data = ReadJson(path);
  if (!data.isObject() || !CheckSchemaVersion(data)) {
    return nullptr;
  }

  //   - the input has a "name" attribute that is a non-empty string
  Json::Value const& name = data["name"];
  if (!name.isString() || name.empty()) {
    return nullptr;
  }

  //   - the input has a "components" attribute that is a JSON object
  if (!data["components"].isObject()) {
    return nullptr;
  }

  // Seems sane enough to hand back to the caller.
  std::unique_ptr<cmPackageInfoReader> reader{ new cmPackageInfoReader };
  reader->Data = data;

  return reader;
}

std::string cmPackageInfoReader::GetName() const
{
  return this->Data["name"].asString();
}

cm::optional<std::string> cmPackageInfoReader::GetVersion() const
{
  Json::Value const& version = this->Data["version"];
  if (version.isString()) {
    return version.asString();
  }
  return cm::nullopt;
}

std::vector<unsigned> cmPackageInfoReader::ParseVersion() const
{
  // Check that we have a version.
  cm::optional<std::string> const& version = this->GetVersion();
  if (!version) {
    return {};
  }

  std::vector<unsigned> result;
  cm::string_view remnant{ *version };

  // Check if we know how to parse the version.
  Json::Value const& schema = this->Data["version_schema"];
  if (schema.isNull() || cmStrCaseEq(schema.asString(), "simple"_s)) {
    // Keep going until we run out of parts.
    while (!remnant.empty()) {
      std::string::size_type n = remnant.find('.');
      cm::string_view part = remnant.substr(0, n);
      if (n == std::string::npos) {
        remnant = {};
      } else {
        remnant = remnant.substr(n + 1);
      }

      unsigned long const value = std::stoul(std::string{ part }, &n);
      if (n == 0 || value > std::numeric_limits<unsigned>::max()) {
        // The part was not a valid number or is too big.
        return {};
      }
      result.push_back(static_cast<unsigned>(value));
    }
  }

  return result;
}
