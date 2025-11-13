/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSpdxSerializer.h"

#include <utility>
#include <vector>

#include <cm3p/json/writer.h>

#include "cmSbomObject.h"

cmSpdxSerializer::cmSpdxSerializer()
{
  Json::StreamWriterBuilder builder = Json::StreamWriterBuilder();
  builder["indentation"] = "  ";
  Writer.reset(builder.newStreamWriter());
}

void cmSpdxSerializer::BeginObject()
{
  CurrentValue = Json::objectValue;
}

void cmSpdxSerializer::BeginArray()
{
  CurrentValue = Json::arrayValue;
}

void cmSpdxSerializer::EndObject()
{
}

void cmSpdxSerializer::EndArray()
{
}

void cmSpdxSerializer::AddReference(std::string const& id)
{
  CurrentValue = id;
}

void cmSpdxSerializer::AddString(std::string const& key,
                                 std::string const& value)
{
  if (!value.empty()) {
    CurrentValue[key] = value;
  }
}

void cmSpdxSerializer::AddVisitable(std::string const& key,
                                    cmSbomObject const& visitable)
{
  if (visitable.IsNull()) {
    return;
  }

  Json::Value parentValue = std::move(CurrentValue);
  visitable.Serialize(*this);
  Json::Value childValue = std::move(CurrentValue);

  CurrentValue = std::move(parentValue);
  CurrentValue[key] = std::move(childValue);
}

void cmSpdxSerializer::AddVectorIfPresent(std::string const& key,
                                          std::vector<cmSbomObject> const& vec)
{
  if (vec.empty()) {
    return;
  }
  Json::Value parentValue = std::move(CurrentValue);
  Json::Value childValue(Json::arrayValue);

  for (auto const& item : vec) {
    if (item.IsNull()) {
      continue;
    }

    item.Serialize(*this);

    if (!CurrentValue.isNull()) {
      childValue.append(std::move(CurrentValue));
    }
  }

  CurrentValue = std::move(parentValue);
  CurrentValue[key] = std::move(childValue);
}

void cmSpdxSerializer::AddVectorIfPresent(std::string const& key,
                                          std::vector<std::string> const& vec)
{
  if (vec.empty()) {
    return;
  }
  Json::Value parentValue = std::move(CurrentValue);
  Json::Value childValue(Json::arrayValue);

  for (auto const& item : vec) {
    if (item.empty()) {
      continue;
    }
    childValue.append(item);
  }

  CurrentValue = std::move(parentValue);
  CurrentValue[key] = std::move(childValue);
}

bool cmSpdxSerializer::WriteSbom(std::ostream& os,
                                 cmSbomObject const& document)
{
  if (document.IsNull()) {
    return false;
  }

  document.Serialize(*this);
  Writer->write(CurrentValue, &os);
  return os.good();
}
