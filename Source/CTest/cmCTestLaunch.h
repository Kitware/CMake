/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmCTestLaunchReporter.h"

namespace cmsys {
class RegularExpression;
}

/** \class cmCTestLaunch
 * \brief Launcher for make rules to report results for ctest
 *
 * This implements the 'ctest --launch' tool.
 */
class cmCTestLaunch
{
public:
  /** Entry point from ctest executable main().  */
  static int Main(int argc, const char* const argv[]);

  cmCTestLaunch(const cmCTestLaunch&) = delete;
  cmCTestLaunch& operator=(const cmCTestLaunch&) = delete;

private:
  // Initialize the launcher from its command line.
  cmCTestLaunch(int argc, const char* const* argv);
  ~cmCTestLaunch();

  // Run the real command.
  int Run();
  void RunChild();

  // Method to check the result of the real command.
  bool CheckResults();

  // Parse out launcher-specific options specified before the real command.
  bool ParseArguments(int argc, const char* const* argv);

  // The real command line appearing after launcher arguments.
  std::vector<std::string> RealArgV;

  // The real command line after response file expansion.
  std::vector<std::string> RealArgs;
  void HandleRealArg(const char* arg);

  // Whether or not any data have been written to stdout or stderr.
  bool HaveOut;
  bool HaveErr;

  // Load custom rules to match warnings and their exceptions.
  bool ScrapeRulesLoaded;
  void LoadScrapeRules();
  void LoadScrapeRules(const char* purpose,
                       std::vector<cmsys::RegularExpression>& regexps) const;
  bool ScrapeLog(std::string const& fname);

  // Helper class to generate the xml fragment.
  cmCTestLaunchReporter Reporter;

  // Configuration
  void LoadConfig();
};
