/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

class cmMakefile;

class cmExperimental
{
public:
  enum class Feature
  {
    CxxModuleCMakeApi,
    WindowsKernelModeDriver,

    Sentinel,
  };

  struct FeatureData
  {
    std::string const Name;
    std::string const Uuid;
    std::string const Variable;
    std::string const Description;
    bool const ForwardThroughTryCompile;
    bool Warned;
  };

  static const FeatureData& DataForFeature(Feature f);
  static bool HasSupportEnabled(cmMakefile const& mf, Feature f);
};
