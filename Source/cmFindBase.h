/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <utility>
#include <vector>

#include "cmFindCommon.h"
#include "cmStateTypes.h"

class cmConfigureLog;
class cmExecutionStatus;
class cmMakefile;

/** \class cmFindBase
 * \brief Base class for most FIND_XXX commands.
 *
 * cmFindBase is a parent class for cmFindProgramCommand, cmFindPathCommand,
 * and cmFindLibraryCommand, cmFindFileCommand
 */
class cmFindBase : public cmFindCommon
{
public:
  cmFindBase(std::string findCommandName, cmExecutionStatus& status);
  virtual ~cmFindBase() = default;

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool ParseArguments(std::vector<std::string> const& args);

  /**
   * To check validity of a found path using user's validator, if any
   */
  bool Validate(std::string const& path) const;

protected:
  friend class cmFindBaseDebugState;
  void ExpandPaths();

  bool IsFound() const;
  bool IsDefined() const;

  void NormalizeFindResult();
  void StoreFindResult(std::string const& value);

  // actual find command name
  std::string FindCommandName;

  // use by command during find
  std::string VariableDocumentation;
  cmStateEnums::CacheEntryType VariableType = cmStateEnums::UNINITIALIZED;
  std::string VariableName;
  std::vector<std::string> Names;
  bool NamesPerDir = false;
  bool NamesPerDirAllowed = false;

  // CMAKE_*_PATH CMAKE_SYSTEM_*_PATH FRAMEWORK|LIBRARY|INCLUDE|PROGRAM
  std::string EnvironmentPath; // LIB,INCLUDE

  bool AlreadyInCacheWithoutMetaInfo = false;
  bool StoreResultInCache = true;

  bool Required = false;

  std::string ValidatorName;

private:
  enum class FindState
  {
    Undefined,
    Found,
    NotFound,
  };
  // see if the VariableName is already set,
  // also copy the documentation from the cache to VariableDocumentation
  // if it has documentation in the cache
  FindState GetInitialState();
  FindState InitialState = FindState::Undefined;

  // Add pieces of the search.
  void FillPackageRootPath();
  void FillCMakeVariablePath();
  void FillCMakeEnvironmentPath();
  void FillUserHintsPath();
  void FillSystemEnvironmentPath();
  void FillCMakeSystemVariablePath();
  void FillUserGuessPath();
};

class cmFindBaseDebugState
{
public:
  explicit cmFindBaseDebugState(std::string name, cmFindBase const* findBase);
  ~cmFindBaseDebugState();

  void FoundAt(std::string const& path, std::string regexName = std::string());
  void FailedAt(std::string const& path,
                std::string regexName = std::string());

private:
  struct DebugLibState
  {
    DebugLibState() = default;
    DebugLibState(std::string&& n, std::string p)
      : regexName(n)
      , path(std::move(p))
    {
    }
    std::string regexName;
    std::string path;
  };

#ifndef CMAKE_BOOTSTRAP
  void WriteFindEvent(cmConfigureLog& log, cmMakefile const& mf) const;
#endif

  cmFindBase const* FindCommand;
  std::string CommandName;
  bool TrackSearchProgress() const;
  std::vector<DebugLibState> FailedSearchLocations;
  DebugLibState FoundSearchLocation;
};
