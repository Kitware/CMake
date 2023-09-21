/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmMakefile;

class cmExperimental
{
public:
  enum class Feature
  {
    WindowsKernelModeDriver,

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
    std::string const Name;
    std::string const Uuid;
    std::string const Variable;
    std::string const Description;
    std::vector<std::string> const TryCompileVariables;
    TryCompileCondition const ForwardThroughTryCompile;
    bool Warned;
  };

  static const FeatureData& DataForFeature(Feature f);
  static bool HasSupportEnabled(cmMakefile const& mf, Feature f);
};
