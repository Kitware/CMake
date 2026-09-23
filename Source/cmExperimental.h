/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include <cm/optional>

class cmMakefile;

class cmExperimental
{
public:
  enum class Feature
  {
    ExportPackageDependencies,
    MappedPackageInfo,
    ExportBuildDatabase,
    GenerateSbom,
    Rust,

    Sentinel,
  };

  enum class TryCompileCondition
  {
    Always,
    SkipCompilerChecks,
    Never,
  };

  struct FeatureData
  {
    std::string Name;
    std::string Uuid;
    std::string Variable;
    std::string Description;
    std::vector<std::string> TryCompileVariables;
    TryCompileCondition ForwardThroughTryCompile;
  };

  static FeatureData const& DataForFeature(Feature f);
  static cm::optional<Feature> FeatureByName(std::string const& name);
  static bool HasSupportEnabled(cmMakefile const& mf, Feature f);
};
