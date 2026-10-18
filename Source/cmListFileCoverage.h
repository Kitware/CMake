/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmLcovTracefile.h"

class cmListFileFunction;

/** \class cmListFileCoverage
 * \brief Record line hits and function calls of CMake listfiles.
 *
 * The environment variable CMAKE_LISTFILE_COVERAGE_DIR turns the collector
 * on.  One collector exists per process.  At exit it writes one lcov
 * tracefile into that directory.  CMake runs listfile code on one thread,
 * so the collector does no locking.
 */
class cmListFileCoverage
{
public:
  /** Read CMAKE_LISTFILE_COVERAGE_DIR.  Unset or empty: coverage stays off,
      returns true.  Otherwise check the path, create the directory, and
      test that a file can be written there.  On failure return false and
      set error to the message text, without the "CMake Error: " prefix.
      A second call while coverage is on does nothing and returns true.  */
  static bool Start(std::string& error);

  /** The collector, or nullptr while coverage is off.  */
  static cmListFileCoverage* Get();

  /** Write the tracefile if any registered file has a line or a
      function.  Ignore write failures.  Turn coverage off.  */
  static void Finish();

  /** While alive, CommandRan(lff, ...) ignores this exact object.  Used
      where one command builds another on its own line.  */
  class SyntheticCommandScope
  {
  public:
    explicit SyntheticCommandScope(cmListFileFunction const& lff);
    ~SyntheticCommandScope();

    SyntheticCommandScope(SyntheticCommandScope const&) = delete;
    SyntheticCommandScope& operator=(SyntheticCommandScope const&) = delete;

  private:
    cmListFileFunction const* Command = nullptr;
  };

  /** Register the executable lines and the function and macro definitions
      of a parsed file.  A second call for the same path adds new lines and
      functions and keeps existing counts.  */
  void FileParsed(std::string const& path,
                  std::vector<cmListFileFunction> const& functions);

  /** Count a hit on a line of a registered file.  */
  void CommandRan(cmListFileFunction const& lff, std::string const& path,
                  long line);

  /** Count a call of the function or macro defined on a line of a
      registered file.  */
  void FunctionCalled(std::string const& path, long definitionLine);

  /** The recorded data, with no test name.  */
  cmLcovTracefile ToTracefile() const;

  /** The sorted, unique start lines of the commands that count.  */
  static std::vector<long> ExecutableLines(
    std::vector<cmListFileFunction> const& functions);

private:
  explicit cmListFileCoverage(std::string outputPath);

  std::string OutputPath;
  std::map<std::string, cmLcovSourceFile> Files;
  std::set<cmListFileFunction const*> SyntheticCommands;
};
