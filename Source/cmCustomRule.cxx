/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include <string>
#include <utility>
#include <vector>

#include "cmRule.h"

class cmMakefile;

cmCustomRule::cmCustomRule(cmMakefile& makefile, std::string name,
                           CustomCommands commands,
                           std::vector<std::string> outputs,
                           cm::RuleScope scope)
  : cmRule(makefile, name, scope)
  , Commands(std::move(commands))
  , Outputs(std::move(outputs))
{
}

cmCustomRule::CustomCommands const& cmCustomRule::GetCommands() const
{
  return this->Commands;
}
std::vector<std::string> const& cmCustomRule::GetOutputs() const
{
  return this->Outputs;
}

void cmCustomRule::SetByproducts(std::vector<std::string> byproducts)
{
  this->Byproducts = std::move(byproducts);
}
std::vector<std::string> const& cmCustomRule::GetByproducts() const
{
  return this->Byproducts;
}

void cmCustomRule::SetDepends(std::vector<std::string> depends)
{
  this->Depends = std::move(depends);
}
std::vector<std::string> const& cmCustomRule::GetDepends() const
{
  return this->Depends;
}

void cmCustomRule::SetDepfile(std::string depfile)
{
  this->Depfile = std::move(depfile);
}
std::string const& cmCustomRule::GetDepfile() const
{
  return this->Depfile;
}
