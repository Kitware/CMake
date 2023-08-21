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
  // CxxModuleCMakeApi
  { "CxxModuleCMakeApi",
    "ac01f462-0f5f-432a-86aa-acef252918a6",
    "CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API",
    "CMake's C++ module support is experimental. It is meant only for "
    "experimentation and feedback to CMake developers.",
    { "CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE",
      "CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FORMAT",
      "CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FLAG" },
    cmExperimental::TryCompileCondition::SkipCompilerChecks,
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
