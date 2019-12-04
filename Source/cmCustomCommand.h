/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCustomCommand_h
#define cmCustomCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <utility>
#include <vector>

#include "cmCustomCommandLines.h"
#include "cmListFileCache.h"

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
  /** Main constructor specifies all information for the command.  */
  cmCustomCommand(std::vector<std::string> outputs,
                  std::vector<std::string> byproducts,
                  std::vector<std::string> depends,
                  cmCustomCommandLines commandLines, cmListFileBacktrace lfbt,
                  const char* comment, const char* workingDirectory);

  /** Get the output file produced by the command.  */
  const std::vector<std::string>& GetOutputs() const;

  /** Get the extra files produced by the command.  */
  const std::vector<std::string>& GetByproducts() const;

  /** Get the vector that holds the list of dependencies.  */
  const std::vector<std::string>& GetDepends() const;

  /** Get the working directory.  */
  std::string const& GetWorkingDirectory() const
  {
    return this->WorkingDirectory;
  }

  /** Get the list of command lines.  */
  const cmCustomCommandLines& GetCommandLines() const;

  /** Get the comment string for the command.  */
  const char* GetComment() const;

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

private:
  std::vector<std::string> Outputs;
  std::vector<std::string> Byproducts;
  std::vector<std::string> Depends;
  cmCustomCommandLines CommandLines;
  cmListFileBacktrace Backtrace;
  cmImplicitDependsList ImplicitDepends;
  std::string Comment;
  std::string WorkingDirectory;
  std::string Depfile;
  std::string JobPool;
  bool HaveComment = false;
  bool EscapeAllowMakeVars = false;
  bool EscapeOldStyle = true;
  bool UsesTerminal = false;
  bool CommandExpandLists = false;
};

#endif
