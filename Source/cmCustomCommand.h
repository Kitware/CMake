/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <utility>
#include <vector>

#include "cmCustomCommandLines.h"
#include "cmListFileCache.h"
#include "cmPolicies.h"

class cmImplicitDependsList
  : public std::vector<std::pair<std::string, std::string>>
{
};

class cmStateSnapshot;

/** \class cmCustomCommand
 * \brief A class to encapsulate a custom command
 *
 * cmCustomCommand encapsulates the properties of a custom command
 */
class cmCustomCommand
{
public:
  /** Get the output file produced by the command.  */
  std::vector<std::string> const& GetOutputs() const;
  void SetOutputs(std::vector<std::string> outputs);
  void SetOutputs(std::string output);

  /** Get the extra files produced by the command.  */
  std::vector<std::string> const& GetByproducts() const;
  void SetByproducts(std::vector<std::string> byproducts);

  /** Get the vector that holds the list of dependencies.  */
  std::vector<std::string> const& GetDepends() const;
  void SetDepends(std::vector<std::string> depends);

  bool HasMainDependency() const { return this->HasMainDependency_; }
  std::string const& GetMainDependency() const;
  void SetMainDependency(std::string main_dependency);

  /** Get the working directory.  */
  std::string const& GetWorkingDirectory() const
  {
    return this->WorkingDirectory;
  }

  void SetWorkingDirectory(char const* workingDirectory)
  {
    this->WorkingDirectory = (workingDirectory ? workingDirectory : "");
  }

  /** Get the list of command lines.  */
  cmCustomCommandLines const& GetCommandLines() const;
  void SetCommandLines(cmCustomCommandLines commandLines);

  /** Get the comment string for the command.  */
  char const* GetComment() const;
  void SetComment(char const* comment);

  /** Get a value indicating if the command uses UTF-8 output pipes. */
  bool GetStdPipesUTF8() const { return this->StdPipesUTF8; }
  void SetStdPipesUTF8(bool stdPipesUTF8)
  {
    this->StdPipesUTF8 = stdPipesUTF8;
  }

  /** Append to the list of command lines.  */
  void AppendCommands(cmCustomCommandLines const& commandLines);

  /** Append to the list of dependencies.  */
  void AppendDepends(std::vector<std::string> const& depends);

  /** Set/Get whether old-style escaping should be used.  */
  bool GetEscapeOldStyle() const;
  void SetEscapeOldStyle(bool b);

  /** Set/Get whether the build tool can replace variables in
      arguments to the command.  */
  bool GetEscapeAllowMakeVars() const;
  void SetEscapeAllowMakeVars(bool b);

  /** Backtrace of the command that created this custom command.  */
  cmListFileBacktrace const& GetBacktrace() const;
  void SetBacktrace(cmListFileBacktrace lfbt);

  void SetImplicitDepends(cmImplicitDependsList const&);
  void AppendImplicitDepends(cmImplicitDependsList const&);
  cmImplicitDependsList const& GetImplicitDepends() const;

  /** Set/Get whether this custom command should be given access to the
      real console (if possible).  */
  bool GetUsesTerminal() const;
  void SetUsesTerminal(bool b);

  /** Set/Get whether lists in command lines should be expanded. */
  bool GetCommandExpandLists() const;
  void SetCommandExpandLists(bool b);

  /** Set/Get whether to use additional dependencies coming from
      users of OUTPUT of the custom command. */
  bool GetDependsExplicitOnly() const;
  void SetDependsExplicitOnly(bool b);

  /** Set/Get the depfile (used by the Ninja generator) */
  std::string const& GetDepfile() const;
  void SetDepfile(std::string const& depfile);

  /** Set/Get the job_pool (used by the Ninja generator) */
  std::string const& GetJobPool() const;
  void SetJobPool(std::string const& job_pool);

  /** Set/Get whether this custom command should be given access to the
      jobserver (if possible).  */
  bool GetJobserverAware() const;
  void SetJobserverAware(bool b);

#define DECLARE_CC_POLICY_ACCESSOR(P)                                         \
  cmPolicies::PolicyStatus Get##P##Status() const;
  CM_FOR_EACH_CUSTOM_COMMAND_POLICY(DECLARE_CC_POLICY_ACCESSOR)
#undef DECLARE_CC_POLICY_ACCESSOR

  /** Record policy values from state snapshot */
  void RecordPolicyValues(cmStateSnapshot const& snapshot);

  /** Set/Get the associated target */
  std::string const& GetTarget() const;
  void SetTarget(std::string const& target);

  /** Set/Get the custom command rolee */
  std::string const& GetRole() const;
  void SetRole(std::string const& role);

  /** Record if the custom command can be used for code generation. */
  bool GetCodegen() const { return Codegen; }
  void SetCodegen(bool b) { Codegen = b; }

private:
  std::vector<std::string> Outputs;
  std::vector<std::string> Byproducts;
  std::vector<std::string> Depends;
  cmCustomCommandLines CommandLines;
  cmListFileBacktrace Backtrace;
  cmImplicitDependsList ImplicitDepends;
  std::string Target;
  std::string Comment;
  std::string WorkingDirectory;
  std::string Depfile;
  std::string JobPool;
  std::string Role;
  bool JobserverAware = false;
  bool HaveComment = false;
  bool EscapeAllowMakeVars = false;
  bool EscapeOldStyle = true;
  bool UsesTerminal = false;
  bool CommandExpandLists = false;
  bool StdPipesUTF8 = false;
  bool HasMainDependency_ = false;
  bool DependsExplicitOnly = false;
  bool Codegen = false;

// Policies are NEW for synthesized custom commands, and set by cmMakefile for
// user-created custom commands.
#define DECLARE_CC_POLICY_FIELD(P)                                            \
  cmPolicies::PolicyStatus P##Status = cmPolicies::NEW;
  CM_FOR_EACH_CUSTOM_COMMAND_POLICY(DECLARE_CC_POLICY_FIELD)
#undef DECLARE_CC_POLICY_FIELD
};
