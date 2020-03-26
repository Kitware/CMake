/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestResourceSpec.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cm_jsoncpp_reader.h"
#include "cm_jsoncpp_value.h"

static const cmsys::RegularExpression IdentifierRegex{ "^[a-z_][a-z0-9_]*$" };
static const cmsys::RegularExpression IdRegex{ "^[a-z0-9_]+$" };

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

  if (!root.isObject()) {
    return ReadFileResult::INVALID_ROOT;
  }

  int majorVersion = 1;
  int minorVersion = 0;
  if (root.isMember("version")) {
    auto const& version = root["version"];
    if (version.isObject()) {
      if (!version.isMember("major") || !version.isMember("minor")) {
        return ReadFileResult::INVALID_VERSION;
      }
      auto const& major = version["major"];
      auto const& minor = version["minor"];
      if (!major.isInt() || !minor.isInt()) {
        return ReadFileResult::INVALID_VERSION;
      }
      majorVersion = major.asInt();
      minorVersion = minor.asInt();
    } else {
      return ReadFileResult::INVALID_VERSION;
    }
  } else {
    return ReadFileResult::NO_VERSION;
  }

  if (majorVersion != 1 || minorVersion != 0) {
    return ReadFileResult::UNSUPPORTED_VERSION;
  }

  auto const& local = root["local"];
  if (!local.isArray()) {
    return ReadFileResult::INVALID_SOCKET_SPEC;
  }
  if (local.size() > 1) {
    return ReadFileResult::INVALID_SOCKET_SPEC;
  }

  if (local.empty()) {
    this->LocalSocket.Resources.clear();
    return ReadFileResult::READ_OK;
  }

  auto const& localSocket = local[0];
  if (!localSocket.isObject()) {
    return ReadFileResult::INVALID_SOCKET_SPEC;
  }
  std::map<std::string, std::vector<cmCTestResourceSpec::Resource>> resources;
  cmsys::RegularExpressionMatch match;
  for (auto const& key : localSocket.getMemberNames()) {
    if (IdentifierRegex.find(key.c_str(), match)) {
      auto const& value = localSocket[key];
      auto& r = resources[key];
      if (value.isArray()) {
        for (auto const& item : value) {
          if (item.isObject()) {
            cmCTestResourceSpec::Resource resource;

            if (!item.isMember("id")) {
              return ReadFileResult::INVALID_RESOURCE;
            }
            auto const& id = item["id"];
            if (!id.isString()) {
              return ReadFileResult::INVALID_RESOURCE;
            }
            resource.Id = id.asString();
            if (!IdRegex.find(resource.Id.c_str(), match)) {
              return ReadFileResult::INVALID_RESOURCE;
            }

            if (item.isMember("slots")) {
              auto const& capacity = item["slots"];
              if (!capacity.isConvertibleTo(Json::uintValue)) {
                return ReadFileResult::INVALID_RESOURCE;
              }
              resource.Capacity = capacity.asUInt();
            } else {
              resource.Capacity = 1;
            }

            r.push_back(resource);
          } else {
            return ReadFileResult::INVALID_RESOURCE;
          }
        }
      } else {
        return ReadFileResult::INVALID_RESOURCE_TYPE;
      }
    }
  }

  this->LocalSocket.Resources = std::move(resources);
  return ReadFileResult::READ_OK;
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
