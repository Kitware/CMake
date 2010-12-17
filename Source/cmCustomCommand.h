/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCustomCommand_h
#define cmCustomCommand_h

#include "cmStandardIncludes.h"
class cmMakefile;
class cmListFileBacktrace;

/** \class cmCustomCommand
 * \brief A class to encapsulate a custom command
 *
 * cmCustomCommand encapsulates the properties of a custom command
 */
class cmCustomCommand
{
public:
  /** Default and copy constructors for STL containers.  */
  cmCustomCommand();
  cmCustomCommand(const cmCustomCommand& r);

  /** Main constructor specifies all information for the command.  */
  cmCustomCommand(cmMakefile* mf,
                  const std::vector<std::string>& outputs,
                  const std::vector<std::string>& depends,
                  const cmCustomCommandLines& commandLines,
                  const char* comment,
                  const char* workingDirectory);

  ~cmCustomCommand();

  /** Get the output file produced by the command.  */
  const std::vector<std::string>& GetOutputs() const;

  /** Get the working directory.  */
  const char* GetWorkingDirectory() const;

  /** Get the vector that holds the list of dependencies.  */
  const std::vector<std::string>& GetDepends() const;

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

  typedef std::pair<cmStdString, cmStdString> ImplicitDependsPair;
  class ImplicitDependsList: public std::vector<ImplicitDependsPair> {};
  void SetImplicitDepends(ImplicitDependsList const&);
  void AppendImplicitDepends(ImplicitDependsList const&);
  ImplicitDependsList const& GetImplicitDepends() const;

private:
  std::vector<std::string> Outputs;
  std::vector<std::string> Depends;
  cmCustomCommandLines CommandLines;
  bool HaveComment;
  std::string Comment;
  std::string WorkingDirectory;
  bool EscapeAllowMakeVars;
  bool EscapeOldStyle;
  cmListFileBacktrace* Backtrace;
  ImplicitDependsList ImplicitDepends;
};

#endif
