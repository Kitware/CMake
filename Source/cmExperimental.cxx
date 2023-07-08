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

struct FeatureData
{
  std::string const Uuid;
  std::string const Variable;
  std::string const Description;
  bool Warned;
} LookupTable[] = {
  // CxxModuleCMakeApi
  { "bf70d4b0-9fb7-465c-9803-34014e70d112",
    "CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API",
    "CMake's C++ module support is experimental. It is meant only for "
    "experimentation and feedback to CMake developers.",
    false },
};
static_assert(sizeof(LookupTable) / sizeof(LookupTable[0]) ==
                static_cast<size_t>(cmExperimental::Feature::Sentinel),
              "Experimental feature lookup table mismatch");

FeatureData& DataForFeature(cmExperimental::Feature f)
{
  assert(f != cmExperimental::Feature::Sentinel);
  return LookupTable[static_cast<size_t>(f)];
}
}

bool cmExperimental::HasSupportEnabled(cmMakefile const& mf, Feature f)
{
  bool enabled = false;
  auto& data = DataForFeature(f);

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
