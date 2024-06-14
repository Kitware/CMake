/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTest.h"

#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmState.h"
#include "cmValue.h"

cmTest::cmTest(cmMakefile* mf)
  : Backtrace(mf->GetBacktrace())
  , PolicyStatusCMP0158(mf->GetPolicyStatus(cmPolicies::CMP0158))
{
  this->Makefile = mf;
  this->OldStyle = true;
}

cmTest::~cmTest() = default;

cmListFileBacktrace const& cmTest::GetBacktrace() const
{
  return this->Backtrace;
}

void cmTest::SetName(const std::string& name)
{
  this->Name = name;
}

void cmTest::SetCommand(std::vector<std::string> const& command)
{
  this->Command = command;
}

cmValue cmTest::GetProperty(const std::string& prop) const
{
  cmValue retVal = this->Properties.GetPropertyValue(prop);
  if (!retVal) {
    const bool chain =
      this->Makefile->GetState()->IsPropertyChained(prop, cmProperty::TEST);
    if (chain) {
      if (cmValue p = this->Makefile->GetProperty(prop, chain)) {
        return p;
      }
    }
    return nullptr;
  }
  return retVal;
}

bool cmTest::GetPropertyAsBool(const std::string& prop) const
{
  return cmIsOn(this->GetProperty(prop));
}

void cmTest::SetProperty(const std::string& prop, cmValue value)
{
  this->Properties.SetProperty(prop, value);
}

void cmTest::AppendProperty(const std::string& prop, const std::string& value,
                            bool asString)
{
  this->Properties.AppendProperty(prop, value, asString);
}

bool cmTest::GetCommandExpandLists() const
{
  return this->CommandExpandLists;
}

void cmTest::SetCommandExpandLists(bool b)
{
  this->CommandExpandLists = b;
}
