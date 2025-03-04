/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmExperimental.h"

#include <cassert>
#include <cstddef>
#include <string>

#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmValue.h"

namespace {

/*
 * The `Uuid` fields of these objects should change periodically.
 * Search for other instances to keep the documentation and test suite
 * up-to-date.
 */
cmExperimental::FeatureData LookupTable[] = {
  // ExportPackageDependencies
  { "ExportPackageDependencies",
    "1942b4fa-b2c5-4546-9385-83f254070067",
    "CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_DEPENDENCIES",
    "CMake's EXPORT_PACKAGE_DEPENDENCIES support is experimental. It is meant "
    "only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Always,
    false },
  // WindowsKernelModeDriver
  { "WindowsKernelModeDriver",
    "9157bf90-2313-44d6-aefa-67cd83c8be7c",
    "CMAKE_EXPERIMENTAL_WINDOWS_KERNEL_MODE_DRIVER",
    "CMake's Windows kernel-mode driver support is experimental. It is meant "
    "only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Always,
    false },
  // CxxImportStd
  { "CxxImportStd",
    "a9e1cf81-9932-4810-974b-6eccaf14e457",
    "CMAKE_EXPERIMENTAL_CXX_IMPORT_STD",
    "CMake's support for `import std;` in C++23 and newer is experimental. It "
    "is meant only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Always,
    false },
  // ImportPackageInfo
  { "ImportPackageInfo",
    "e82e467b-f997-4464-8ace-b00808fff261",
    "CMAKE_EXPERIMENTAL_FIND_CPS_PACKAGES",
    "CMake's support for importing package information in the Common Package "
    "Specification format (via find_package) is experimental. It is meant "
    "only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Always,
    false },
  // ExportPackageInfo
  { "ExportPackageInfo",
    "b80be207-778e-46ba-8080-b23bba22639e",
    "CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_INFO",
    "CMake's support for exporting package information in the Common Package "
    "Specification format is experimental. It is meant only for "
    "experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Always,
    false },
  // ExportBuildDatabase
  { "ExportBuildDatabase",
    "4bd552e2-b7fb-429a-ab23-c83ef53f3f13",
    "CMAKE_EXPERIMENTAL_EXPORT_BUILD_DATABASE",
    "CMake's support for exporting build databases is experimental. It is "
    "meant only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Never,
    false },
  // Instrumentation
  { "Instrumentation",
    "a37d1069-1972-4901-b9c9-f194aaf2b6e0",
    "CMAKE_EXPERIMENTAL_INSTRUMENTATION",
    "CMake's support for collecting instrumentation data is experimental. It "
    "is meant only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Never,
    false },
};
static_assert(sizeof(LookupTable) / sizeof(LookupTable[0]) ==
                static_cast<size_t>(cmExperimental::Feature::Sentinel),
              "Experimental feature lookup table mismatch");

cmExperimental::FeatureData& DataForFeature(cmExperimental::Feature f)
{
  assert(f != cmExperimental::Feature::Sentinel);
  return LookupTable[static_cast<size_t>(f)];
}
}

cmExperimental::FeatureData const& cmExperimental::DataForFeature(Feature f)
{
  return ::DataForFeature(f);
}

cm::optional<cmExperimental::Feature> cmExperimental::FeatureByName(
  std::string const& name)
{
  size_t idx = 0;
  for (auto const& feature : LookupTable) {
    if (feature.Name == name) {
      return static_cast<Feature>(idx);
    }
    ++idx;
  }

  return {};
}

bool cmExperimental::HasSupportEnabled(cmMakefile const& mf, Feature f)
{
  bool enabled = false;
  auto& data = ::DataForFeature(f);

  auto value = mf.GetDefinition(data.Variable);
  if (value == data.Uuid) {
    enabled = true;
  }

  if (enabled && !data.Warned) {
    mf.IssueMessage(MessageType::AUTHOR_WARNING, data.Description);
    data.Warned = true;
  }

  return enabled;
}
