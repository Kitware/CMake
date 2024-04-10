/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

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
    "5c2d848d-4efa-4529-a768-efd57171bf68",
    "CMAKE_EXPERIMENTAL_WINDOWS_KERNEL_MODE_DRIVER",
    "CMake's Windows kernel-mode driver support is experimental. It is meant "
    "only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Always,
    false },
  // CxxImportStd
  { "CxxImportStd",
    "0e5b6991-d74f-4b3d-a41c-cf096e0b2508",
    "CMAKE_EXPERIMENTAL_CXX_IMPORT_STD",
    "CMake's support for `import std;` in C++23 and newer is experimental. It "
    "is meant only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Always,
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

const cmExperimental::FeatureData& cmExperimental::DataForFeature(Feature f)
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
