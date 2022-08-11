/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestResourceSpec.h"

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cmext/string_view>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmJSONHelpers.h"

namespace {
using JSONHelperBuilder =
  cmJSONHelperBuilder<cmCTestResourceSpec::ReadFileResult>;
const cmsys::RegularExpression IdentifierRegex{ "^[a-z_][a-z0-9_]*$" };
const cmsys::RegularExpression IdRegex{ "^[a-z0-9_]+$" };

struct Version
{
  int Major = 1;
  int Minor = 0;
};

struct TopVersion
{
  struct Version Version;
};

auto const VersionFieldHelper =
  JSONHelperBuilder::Int(cmCTestResourceSpec::ReadFileResult::READ_OK,
                         cmCTestResourceSpec::ReadFileResult::INVALID_VERSION);

auto const VersionHelper = JSONHelperBuilder::Required<Version>(
  cmCTestResourceSpec::ReadFileResult::NO_VERSION,
  JSONHelperBuilder::Object<Version>(
    cmCTestResourceSpec::ReadFileResult::READ_OK,
    cmCTestResourceSpec::ReadFileResult::INVALID_VERSION)
    .Bind("major"_s, &Version::Major, VersionFieldHelper)
    .Bind("minor"_s, &Version::Minor, VersionFieldHelper));

auto const RootVersionHelper =
  JSONHelperBuilder::Object<TopVersion>(
    cmCTestResourceSpec::ReadFileResult::READ_OK,
    cmCTestResourceSpec::ReadFileResult::INVALID_ROOT)
    .Bind("version"_s, &TopVersion::Version, VersionHelper, false);

cmCTestResourceSpec::ReadFileResult ResourceIdHelper(std::string& out,
                                                     const Json::Value* value)
{
  auto result = JSONHelperBuilder::String(
    cmCTestResourceSpec::ReadFileResult::READ_OK,
    cmCTestResourceSpec::ReadFileResult::INVALID_RESOURCE)(out, value);
  if (result != cmCTestResourceSpec::ReadFileResult::READ_OK) {
    return result;
  }
  cmsys::RegularExpressionMatch match;
  if (!IdRegex.find(out.c_str(), match)) {
    return cmCTestResourceSpec::ReadFileResult::INVALID_RESOURCE;
  }
  return cmCTestResourceSpec::ReadFileResult::READ_OK;
}

auto const ResourceHelper =
  JSONHelperBuilder::Object<cmCTestResourceSpec::Resource>(
    cmCTestResourceSpec::ReadFileResult::READ_OK,
    cmCTestResourceSpec::ReadFileResult::INVALID_RESOURCE)
    .Bind("id"_s, &cmCTestResourceSpec::Resource::Id, ResourceIdHelper)
    .Bind("slots"_s, &cmCTestResourceSpec::Resource::Capacity,
          JSONHelperBuilder::UInt(
            cmCTestResourceSpec::ReadFileResult::READ_OK,
            cmCTestResourceSpec::ReadFileResult::INVALID_RESOURCE, 1),
          false);

auto const ResourceListHelper =
  JSONHelperBuilder::Vector<cmCTestResourceSpec::Resource>(
    cmCTestResourceSpec::ReadFileResult::READ_OK,
    cmCTestResourceSpec::ReadFileResult::INVALID_RESOURCE_TYPE,
    ResourceHelper);

auto const ResourceMapHelper =
  JSONHelperBuilder::MapFilter<std::vector<cmCTestResourceSpec::Resource>>(
    cmCTestResourceSpec::ReadFileResult::READ_OK,
    cmCTestResourceSpec::ReadFileResult::INVALID_SOCKET_SPEC,
    ResourceListHelper, [](const std::string& key) -> bool {
      cmsys::RegularExpressionMatch match;
      return IdentifierRegex.find(key.c_str(), match);
    });

auto const SocketSetHelper = JSONHelperBuilder::Vector<
  std::map<std::string, std::vector<cmCTestResourceSpec::Resource>>>(
  cmCTestResourceSpec::ReadFileResult::READ_OK,
  cmCTestResourceSpec::ReadFileResult::INVALID_SOCKET_SPEC, ResourceMapHelper);

cmCTestResourceSpec::ReadFileResult SocketHelper(
  cmCTestResourceSpec::Socket& out, const Json::Value* value)
{
  std::vector<
    std::map<std::string, std::vector<cmCTestResourceSpec::Resource>>>
    sockets;
  cmCTestResourceSpec::ReadFileResult result = SocketSetHelper(sockets, value);
  if (result != cmCTestResourceSpec::ReadFileResult::READ_OK) {
    return result;
  }
  if (sockets.size() > 1) {
    return cmCTestResourceSpec::ReadFileResult::INVALID_SOCKET_SPEC;
  }
  if (sockets.empty()) {
    out.Resources.clear();
  } else {
    out.Resources = std::move(sockets[0]);
  }
  return cmCTestResourceSpec::ReadFileResult::READ_OK;
}

auto const LocalRequiredHelper =
  JSONHelperBuilder::Required<cmCTestResourceSpec::Socket>(
    cmCTestResourceSpec::ReadFileResult::INVALID_SOCKET_SPEC, SocketHelper);

auto const RootHelper = JSONHelperBuilder::Object<cmCTestResourceSpec>(
                          cmCTestResourceSpec::ReadFileResult::READ_OK,
                          cmCTestResourceSpec::ReadFileResult::INVALID_ROOT)
                          .Bind("local", &cmCTestResourceSpec::LocalSocket,
                                LocalRequiredHelper, false);
}

cmCTestResourceSpec::ReadFileResult cmCTestResourceSpec::ReadFromJSONFile(
  const std::string& filename)
{
  cmsys::ifstream fin(filename.c_str());
  if (!fin) {
    return ReadFileResult::FILE_NOT_FOUND;
  }

  Json::Value root;
  Json::CharReaderBuilder builder;
  if (!Json::parseFromStream(builder, fin, &root, nullptr)) {
    return ReadFileResult::JSON_PARSE_ERROR;
  }

  TopVersion version;
  ReadFileResult result;
  if ((result = RootVersionHelper(version, &root)) !=
      ReadFileResult::READ_OK) {
    return result;
  }
  if (version.Version.Major != 1 || version.Version.Minor != 0) {
    return ReadFileResult::UNSUPPORTED_VERSION;
  }

  return RootHelper(*this, &root);
}

const char* cmCTestResourceSpec::ResultToString(ReadFileResult result)
{
  switch (result) {
    case ReadFileResult::READ_OK:
      return "OK";

    case ReadFileResult::FILE_NOT_FOUND:
      return "File not found";

    case ReadFileResult::JSON_PARSE_ERROR:
      return "JSON parse error";

    case ReadFileResult::INVALID_ROOT:
      return "Invalid root object";

    case ReadFileResult::NO_VERSION:
      return "No version specified";

    case ReadFileResult::INVALID_VERSION:
      return "Invalid version object";

    case ReadFileResult::UNSUPPORTED_VERSION:
      return "Unsupported version";

    case ReadFileResult::INVALID_SOCKET_SPEC:
      return "Invalid socket object";

    case ReadFileResult::INVALID_RESOURCE_TYPE:
      return "Invalid resource type object";

    case ReadFileResult::INVALID_RESOURCE:
      return "Invalid resource object";

    default:
      return "Unknown";
  }
}

bool cmCTestResourceSpec::operator==(const cmCTestResourceSpec& other) const
{
  return this->LocalSocket == other.LocalSocket;
}

bool cmCTestResourceSpec::operator!=(const cmCTestResourceSpec& other) const
{
  return !(*this == other);
}

bool cmCTestResourceSpec::Socket::operator==(
  const cmCTestResourceSpec::Socket& other) const
{
  return this->Resources == other.Resources;
}

bool cmCTestResourceSpec::Socket::operator!=(
  const cmCTestResourceSpec::Socket& other) const
{
  return !(*this == other);
}

bool cmCTestResourceSpec::Resource::operator==(
  const cmCTestResourceSpec::Resource& other) const
{
  return this->Id == other.Id && this->Capacity == other.Capacity;
}

bool cmCTestResourceSpec::Resource::operator!=(
  const cmCTestResourceSpec::Resource& other) const
{
  return !(*this == other);
}
