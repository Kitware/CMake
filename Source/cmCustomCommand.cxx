/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCustomCommand.h"

#include <cassert>
#include <utility>

#include <cmext/algorithm>

#include "cmStateSnapshot.h"

std::vector<std::string> const& cmCustomCommand::GetOutputs() const
{
  return this->Outputs;
}

void cmCustomCommand::SetOutputs(std::vector<std::string> outputs)
{
  this->Outputs = std::move(outputs);
}

void cmCustomCommand::SetOutputs(std::string output)
{
  this->Outputs = { std::move(output) };
}

std::vector<std::string> const& cmCustomCommand::GetByproducts() const
{
  return this->Byproducts;
}

void cmCustomCommand::SetByproducts(std::vector<std::string> byproducts)
{
  this->Byproducts = std::move(byproducts);
}

std::vector<std::string> const& cmCustomCommand::GetDepends() const
{
  return this->Depends;
}

void cmCustomCommand::SetDepends(std::vector<std::string> depends)
{
  if (this->HasMainDependency_) {
    depends.insert(depends.begin(), std::move(this->Depends[0]));
  }

  Depends = std::move(depends);
}

std::string const& cmCustomCommand::GetMainDependency() const
{
  assert(this->HasMainDependency_);
  return this->Depends[0];
}

void cmCustomCommand::SetMainDependency(std::string main_dependency)
{
  if (this->HasMainDependency_) {
    assert(!main_dependency.empty());
    this->Depends[0] = std::move(main_dependency);
  } else if (main_dependency.empty()) {
    // Do nothing.
  } else {
    this->Depends.insert(this->Depends.begin(), std::move(main_dependency));
    this->HasMainDependency_ = true;
  }
}

cmCustomCommandLines const& cmCustomCommand::GetCommandLines() const
{
  return this->CommandLines;
}

void cmCustomCommand::SetCommandLines(cmCustomCommandLines commandLines)
{
  this->CommandLines = std::move(commandLines);
}

char const* cmCustomCommand::GetComment() const
{
  char const* no_comment = nullptr;
  return this->HaveComment ? this->Comment.c_str() : no_comment;
}

void cmCustomCommand::SetComment(char const* comment)
{
  this->Comment = comment ? comment : "";
  this->HaveComment = (comment != nullptr);
}

void cmCustomCommand::AppendCommands(cmCustomCommandLines const& commandLines)
{
  cm::append(this->CommandLines, commandLines);
}

void cmCustomCommand::AppendDepends(std::vector<std::string> const& depends)
{
  cm::append(this->Depends, depends);
}

bool cmCustomCommand::GetEscapeOldStyle() const
{
  return this->EscapeOldStyle;
}

void cmCustomCommand::SetEscapeOldStyle(bool b)
{
  this->EscapeOldStyle = b;
}

bool cmCustomCommand::GetEscapeAllowMakeVars() const
{
  return this->EscapeAllowMakeVars;
}

void cmCustomCommand::SetEscapeAllowMakeVars(bool b)
{
  this->EscapeAllowMakeVars = b;
}

cmListFileBacktrace const& cmCustomCommand::GetBacktrace() const
{
  return this->Backtrace;
}

void cmCustomCommand::SetBacktrace(cmListFileBacktrace lfbt)
{
  this->Backtrace = std::move(lfbt);
}

cmImplicitDependsList const& cmCustomCommand::GetImplicitDepends() const
{
  return this->ImplicitDepends;
}

void cmCustomCommand::SetImplicitDepends(cmImplicitDependsList const& l)
{
  this->ImplicitDepends = l;
}

void cmCustomCommand::AppendImplicitDepends(cmImplicitDependsList const& l)
{
  cm::append(this->ImplicitDepends, l);
}

bool cmCustomCommand::GetUsesTerminal() const
{
  return this->UsesTerminal;
}

void cmCustomCommand::SetUsesTerminal(bool b)
{
  this->UsesTerminal = b;
}

void cmCustomCommand::SetRole(std::string const& role)
{
  this->Role = role;
}

std::string const& cmCustomCommand::GetRole() const
{
  return this->Role;
}

bool cmCustomCommand::GetCommandExpandLists() const
{
  return this->CommandExpandLists;
}

void cmCustomCommand::SetCommandExpandLists(bool b)
{
  this->CommandExpandLists = b;
}

bool cmCustomCommand::GetDependsExplicitOnly() const
{
  return this->DependsExplicitOnly;
}

void cmCustomCommand::SetDependsExplicitOnly(bool b)
{
  this->DependsExplicitOnly = b;
}

std::string const& cmCustomCommand::GetDepfile() const
{
  return this->Depfile;
}

void cmCustomCommand::SetDepfile(std::string const& depfile)
{
  this->Depfile = depfile;
}

std::string const& cmCustomCommand::GetJobPool() const
{
  return this->JobPool;
}

void cmCustomCommand::SetJobPool(std::string const& job_pool)
{
  this->JobPool = job_pool;
}

bool cmCustomCommand::GetJobserverAware() const
{
  return this->JobserverAware;
}

void cmCustomCommand::SetJobserverAware(bool b)
{
  this->JobserverAware = b;
}

#define DEFINE_CC_POLICY_ACCESSOR(P)                                          \
  cmPolicies::PolicyStatus cmCustomCommand::Get##P##Status() const            \
  {                                                                           \
    return this->P##Status;                                                   \
  }
CM_FOR_EACH_CUSTOM_COMMAND_POLICY(DEFINE_CC_POLICY_ACCESSOR)
#undef DEFINE_CC_POLICY_ACCESSOR

void cmCustomCommand::RecordPolicyValues(cmStateSnapshot const& snapshot){
#define SET_CC_POLICY(P) this->P##Status = snapshot.GetPolicy(cmPolicies::P);
  CM_FOR_EACH_CUSTOM_COMMAND_POLICY(SET_CC_POLICY)
#undef SET_CC_POLICY
}

std::string const& cmCustomCommand::GetTarget() const
{
  return this->Target;
}

void cmCustomCommand::SetTarget(std::string const& target)
{
  this->Target = target;
}
