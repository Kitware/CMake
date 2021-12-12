/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
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

/** \class cmCustomCommand
 * \brief A class to encapsulate a custom command
 *
 * cmCustomCommand encapsulates the properties of a custom command
 */
class cmCustomCommand
{
public:
  /** Get the output file produced by the command.  */
  const std::vector<std::string>& GetOutputs() const;
  void SetOutputs(std::vector<std::string> outputs);
  void SetOutputs(std::string output);

  /** Get the extra files produced by the command.  */
  const std::vector<std::string>& GetByproducts() const;
  void SetByproducts(std::vector<std::string> byproducts);

  /** Get the vector that holds the list of dependencies.  */
  const std::vector<std::string>& GetDepends() const;
  void SetDepends(std::vector<std::string> depends);

  bool HasMainDependency() const { return this->HasMainDependency_; }
  const std::string& GetMainDependency() const;
  void SetMainDependency(std::string main_dependency);

  /** Get the working directory.  */
  std::string const& GetWorkingDirectory() const
  {
    return this->WorkingDirectory;
  }

  void SetWorkingDirectory(const char* workingDirectory)
  {
    this->WorkingDirectory = (workingDirectory ? workingDirectory : "");
  }

  /** Get the list of command lines.  */
  const cmCustomCommandLines& GetCommandLines() const;
  void SetCommandLines(cmCustomCommandLines commandLines);

  /** Get the comment string for the command.  */
  const char* GetComment() const;
  void SetComment(const char* comment);

  /** Get a value indicating if the command uses UTF-8 output pipes. */
  bool GetStdPipesUTF8() const { return this->StdPipesUTF8; }
  void SetStdPipesUTF8(bool stdPipesUTF8)
  {
    this->StdPipesUTF8 = stdPipesUTF8;
  }

  /** Append to the list of command lines.  */
  void AppendCommands(const cmCustomCommandLines& commandLines);

  /** Append to the list of dependencies.  */
  void AppendDepends(const std::vector<std::string>& depends);

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

  /** Set/Get the depfile (used by the Ninja generator) */
  const std::string& GetDepfile() const;
  void SetDepfile(const std::string& depfile);

  /** Set/Get the job_pool (used by the Ninja generator) */
  const std::string& GetJobPool() const;
  void SetJobPool(const std::string& job_pool);

  /** Set/Get the CMP0116 status (used by the Ninja generator) */
  cmPolicies::PolicyStatus GetCMP0116Status() const;
  void SetCMP0116Status(cmPolicies::PolicyStatus cmp0116);

  /** Set/Get the associated target */
  const std::string& GetTarget() const;
  void SetTarget(const std::string& target);

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
  bool HaveComment = false;
  bool EscapeAllowMakeVars = false;
  bool EscapeOldStyle = true;
  bool UsesTerminal = false;
  bool CommandExpandLists = false;
  bool StdPipesUTF8 = false;
  bool HasMainDependency_ = false;
  cmPolicies::PolicyStatus CMP0116Status = cmPolicies::WARN;
};
