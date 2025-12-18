/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmExperimental.h"

#include <cassert>
#include <cstddef>
#include <string>

#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"

namespace {

/*
 * The `Uuid` fields of these objects should change periodically.
 * Search for other instances to keep the documentation and test suite
 * up-to-date.
 */
cmExperimental::FeatureData const LookupTable[] = {
  // ExportPackageDependencies
  { "ExportPackageDependencies",
    "1942b4fa-b2c5-4546-9385-83f254070067",
    "CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_DEPENDENCIES",
    "CMake's EXPORT_PACKAGE_DEPENDENCIES support is experimental. It is meant "
    "only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Always },
  // CxxImportStd
  { "CxxImportStd",
    "d0edc3af-4c50-42ea-a356-e2862fe7a444",
    "CMAKE_EXPERIMENTAL_CXX_IMPORT_STD",
    "CMake's support for `import std;` in C++23 and newer is experimental. It "
    "is meant only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Always },
  // ImportPackageInfo
  { "ImportPackageInfo",
    "e82e467b-f997-4464-8ace-b00808fff261",
    "CMAKE_EXPERIMENTAL_FIND_CPS_PACKAGES",
    "CMake's support for importing package information in the Common Package "
    "Specification format (via find_package) is experimental. It is meant "
    "only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Always },
  // ExportPackageInfo
  { "ExportPackageInfo",
    "b80be207-778e-46ba-8080-b23bba22639e",
    "CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_INFO",
    "CMake's support for exporting package information in the Common Package "
    "Specification format is experimental. It is meant only for "
    "experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Always },
  // MappedPackageInfo
  { "MappedPackageInfo",
    "ababa1b5-7099-495f-a9cd-e22d38f274f2",
    "CMAKE_EXPERIMENTAL_MAPPED_PACKAGE_INFO",
    "CMake's support for generating package information in the Common Package "
    "Specification format from CMake script exports is experimental. It is "
    "meant only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Always },
  // ExportBuildDatabase
  { "ExportBuildDatabase",
    "73194a1d-c0b5-41b9-9190-a4512925e192",
    "CMAKE_EXPERIMENTAL_EXPORT_BUILD_DATABASE",
    "CMake's support for exporting build databases is experimental. It is "
    "meant only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Never },
  // Instrumentation
  { "Instrumentation",
    "ec7aa2dc-b87f-45a3-8022-fe01c5f59984",
    "CMAKE_EXPERIMENTAL_INSTRUMENTATION",
    "CMake's support for collecting instrumentation data is experimental. It "
    "is meant only for experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Never },
  { "GenerateSbom",
    "ca494ed3-b261-4205-a01f-603c95e4cae0",
    "CMAKE_EXPERIMENTAL_GENERATE_SBOM",
    "CMake's support for generating software bill of materials (Sbom) "
    "information in SPDX format is experimental. It is meant only for "
    "experimentation and feedback to CMake developers.",
    {},
    cmExperimental::TryCompileCondition::Never },
};
static_assert(sizeof(LookupTable) / sizeof(LookupTable[0]) ==
                static_cast<size_t>(cmExperimental::Feature::Sentinel),
              "Experimental feature lookup table mismatch");

cmExperimental::FeatureData const& DataForFeature(cmExperimental::Feature f)
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
  FeatureData const& data = cmExperimental::DataForFeature(f);

  if (cmValue value = mf.GetDefinition(data.Variable)) {
    enabled = *value == data.Uuid;

    if (mf.GetGlobalGenerator()->ShouldWarnExperimental(data.Name, *value)) {
      if (enabled) {
        mf.IssueMessage(MessageType::AUTHOR_WARNING, data.Description);
      } else {
        mf.IssueMessage(
          MessageType::AUTHOR_WARNING,
          cmStrCat(
            data.Variable, " is set to incorrect value\n  ", value,
            "\n"
            "See 'Help/dev/experimental.rst' in the source tree of this "
            "version of CMake for documentation of the experimental feature "
            "and the corresponding activation value.  This project's code "
            "may require changes to work with this CMake's version of the "
            "feature."));
      }
    }
  }

  return enabled;
}
