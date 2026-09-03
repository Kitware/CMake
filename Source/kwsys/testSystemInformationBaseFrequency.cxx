/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
#include "kwsysPrivate.h"
#include KWSYS_HEADER(SystemTools.hxx)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
#  include "SystemTools.hxx.in"
#endif

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

// Internal resolvers under test, driven with an injected sysfs tree.
namespace kwsys {
namespace SystemInformationDetail {
float LinuxBaseFrequencyMHz(std::string const& cpuRoot);
float LinuxMaxFrequencyMHz(std::string const& cpuRoot);
}
}

namespace {
int failures = 0;

void writeFile(std::string const& path, std::string const& content)
{
  std::ofstream f(path.c_str());
  f << content;
}

void expect(char const* name, float actual, float expected)
{
  if (std::fabs(actual - expected) > 0.01f) {
    std::cerr << "FAIL: " << name << ": expected " << expected << " got "
              << actual << "\n";
    ++failures;
  }
}
}

int testSystemInformationBaseFrequency(int, char*[])
{
  using kwsys::SystemInformationDetail::LinuxBaseFrequencyMHz;
  using kwsys::SystemInformationDetail::LinuxMaxFrequencyMHz;
  using kwsys::SystemTools;

  std::string const root = "testSystemInformationBaseFrequency.dir";

  // Intel P-state base_frequency (kHz) -> MHz.
  SystemTools::RemoveADirectory(root);
  SystemTools::MakeDirectory(root + "/cpu0/cpufreq");
  writeFile(root + "/online", "0-15\n");
  writeFile(root + "/cpu0/cpufreq/base_frequency", "2200000\n");
  expect("base_frequency", LinuxBaseFrequencyMHz(root), 2200.0f);

  // cpuinfo_max_freq (kHz) -> MHz.
  writeFile(root + "/cpu0/cpufreq/cpuinfo_max_freq", "5000000\n");
  expect("cpuinfo_max_freq", LinuxMaxFrequencyMHz(root), 5000.0f);

  // Representative CPU follows the lowest online id.
  SystemTools::RemoveADirectory(root);
  SystemTools::MakeDirectory(root + "/cpu2/cpufreq");
  writeFile(root + "/online", "2-3,8\n");
  writeFile(root + "/cpu2/cpufreq/base_frequency", "2400000\n");
  expect("representative-cpu", LinuxBaseFrequencyMHz(root), 2400.0f);

  // AMD CPPC nominal_freq (already MHz) when base_frequency is absent.
  SystemTools::RemoveADirectory(root);
  SystemTools::MakeDirectory(root + "/cpu0/acpi_cppc");
  writeFile(root + "/cpu0/acpi_cppc/nominal_freq", "3000\n");
  expect("nominal_freq", LinuxBaseFrequencyMHz(root), 3000.0f);

  // Malformed and zero values are rejected (treated as unavailable).
  SystemTools::RemoveADirectory(root);
  SystemTools::MakeDirectory(root + "/cpu0/cpufreq");
  writeFile(root + "/cpu0/cpufreq/base_frequency", "not-a-number\n");
  expect("malformed", LinuxBaseFrequencyMHz(root), 0.0f);
  writeFile(root + "/cpu0/cpufreq/base_frequency", "0\n");
  expect("zero", LinuxBaseFrequencyMHz(root), 0.0f);

  // An absent tree yields no value.
  SystemTools::RemoveADirectory(root);
  expect("absent-base", LinuxBaseFrequencyMHz(root), 0.0f);
  expect("absent-max", LinuxMaxFrequencyMHz(root), 0.0f);

  SystemTools::RemoveADirectory(root);
  return failures == 0 ? 0 : 1;
}
