/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <string>
#include <utility>

#include "cmRule.h"

class cmMakefile;

cmSpecializedRule::cmSpecializedRule(cmMakefile& makefile, std::string name,
                                     cmRule const& parent, cm::RuleScope scope)
  : cmRule(parent, makefile, std::move(name), scope)
  , ParentRule(parent)
{
}

std::string const& cmSpecializedRule::GetParentName() const
{
  return this->ParentRule.GetName();
}

cmSpecializedRule::CustomCommands const& cmSpecializedRule::GetCommands() const
{
  return this->ParentRule.GetCommands();
}
std::vector<std::string> const& cmSpecializedRule::GetOutputs() const
{
  return this->ParentRule.GetOutputs();
}

std::vector<std::string> const& cmSpecializedRule::GetByproducts() const
{
  return this->ParentRule.GetByproducts();
}

std::vector<std::string> const& cmSpecializedRule::GetDepends() const
{
  return this->ParentRule.GetDepends();
}

std::string const& cmSpecializedRule::GetDepfile() const
{
  return this->ParentRule.GetDepfile();
}
