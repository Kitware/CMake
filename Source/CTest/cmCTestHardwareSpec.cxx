/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestHardwareSpec.h"

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

bool cmCTestHardwareSpec::ReadFromJSONFile(const std::string& filename)
{
  cmsys::ifstream fin(filename.c_str());
  if (!fin) {
    return false;
  }

  Json::Value root;
  Json::CharReaderBuilder builder;
  if (!Json::parseFromStream(builder, fin, &root, nullptr)) {
    return false;
  }

  if (!root.isObject()) {
    return false;
  }

  auto const& local = root["local"];
  if (!local.isArray()) {
    return false;
  }
  if (local.size() > 1) {
    return false;
  }

  if (local.empty()) {
    this->LocalSocket.Resources.clear();
    return true;
  }

  auto const& localSocket = local[0];
  if (!localSocket.isObject()) {
    return false;
  }
  std::map<std::string, std::vector<cmCTestHardwareSpec::Resource>> resources;
  cmsys::RegularExpressionMatch match;
  for (auto const& key : localSocket.getMemberNames()) {
    if (IdentifierRegex.find(key.c_str(), match)) {
      auto const& value = localSocket[key];
      auto& r = resources[key];
      if (value.isArray()) {
        for (auto const& item : value) {
          if (item.isObject()) {
            cmCTestHardwareSpec::Resource resource;

            if (!item.isMember("id")) {
              return false;
            }
            auto const& id = item["id"];
            if (!id.isString()) {
              return false;
            }
            resource.Id = id.asString();
            if (!IdRegex.find(resource.Id.c_str(), match)) {
              return false;
            }

            if (item.isMember("slots")) {
              auto const& capacity = item["slots"];
              if (!capacity.isConvertibleTo(Json::uintValue)) {
                return false;
              }
              resource.Capacity = capacity.asUInt();
            } else {
              resource.Capacity = 1;
            }

            r.push_back(resource);
          } else {
            return false;
          }
        }
      } else {
        return false;
      }
    }
  }

  this->LocalSocket.Resources = std::move(resources);
  return true;
}

bool cmCTestHardwareSpec::operator==(const cmCTestHardwareSpec& other) const
{
  return this->LocalSocket == other.LocalSocket;
}

bool cmCTestHardwareSpec::operator!=(const cmCTestHardwareSpec& other) const
{
  return !(*this == other);
}

bool cmCTestHardwareSpec::Socket::operator==(
  const cmCTestHardwareSpec::Socket& other) const
{
  return this->Resources == other.Resources;
}

bool cmCTestHardwareSpec::Socket::operator!=(
  const cmCTestHardwareSpec::Socket& other) const
{
  return !(*this == other);
}

bool cmCTestHardwareSpec::Resource::operator==(
  const cmCTestHardwareSpec::Resource& other) const
{
  return this->Id == other.Id && this->Capacity == other.Capacity;
}

bool cmCTestHardwareSpec::Resource::operator!=(
  const cmCTestHardwareSpec::Resource& other) const
{
  return !(*this == other);
}
