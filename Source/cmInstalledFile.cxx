/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstalledFile.h"

#include <utility>

#include "cmGeneratorExpression.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmValue.h"

cmInstalledFile::cmInstalledFile() = default;

cmInstalledFile::~cmInstalledFile() = default;

cmInstalledFile::Property::Property() = default;

cmInstalledFile::Property::~Property() = default;

void cmInstalledFile::SetName(cmMakefile* mf, std::string const& name)
{
  cmListFileBacktrace backtrace = mf->GetBacktrace();
  cmGeneratorExpression ge(*mf->GetCMakeInstance(), backtrace);

  this->Name = name;
  this->NameExpression = ge.Parse(name);
}

std::string const& cmInstalledFile::GetName() const
{
  return this->Name;
}

cmCompiledGeneratorExpression const& cmInstalledFile::GetNameExpression() const
{
  return *(this->NameExpression);
}

void cmInstalledFile::RemoveProperty(std::string const& prop)
{
  this->Properties.erase(prop);
}

void cmInstalledFile::SetProperty(cmMakefile const* mf,
                                  std::string const& prop,
                                  std::string const& value)
{
  this->RemoveProperty(prop);
  this->AppendProperty(mf, prop, value);
}

void cmInstalledFile::AppendProperty(cmMakefile const* mf,
                                     std::string const& prop,
                                     std::string const& value,
                                     bool /*asString*/)
{
  cmListFileBacktrace backtrace = mf->GetBacktrace();
  cmGeneratorExpression ge(*mf->GetCMakeInstance(), backtrace);

  Property& property = this->Properties[prop];
  property.ValueExpressions.push_back(ge.Parse(value));
}

bool cmInstalledFile::HasProperty(std::string const& prop) const
{
  return this->Properties.find(prop) != this->Properties.end();
}

bool cmInstalledFile::GetProperty(std::string const& prop,
                                  std::string& value) const
{
  auto i = this->Properties.find(prop);
  if (i == this->Properties.end()) {
    return false;
  }

  Property const& property = i->second;

  std::string output;
  std::string separator;

  for (auto const& ve : property.ValueExpressions) {
    output += separator;
    output += ve->GetInput();
    separator = ";";
  }

  value = output;
  return true;
}

bool cmInstalledFile::GetPropertyAsBool(std::string const& prop) const
{
  std::string value;
  bool isSet = this->GetProperty(prop, value);
  return isSet && cmIsOn(value);
}

std::vector<std::string> cmInstalledFile::GetPropertyAsList(
  std::string const& prop) const
{
  std::string value;
  this->GetProperty(prop, value);

  return std::move(cmList(value).data());
}
