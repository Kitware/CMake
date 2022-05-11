/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

/** \brief Defines how to resolve packages **/
enum class PackageResolveMode
{
  /** \brief Behavior is defined by preset or cache variable (e.g.
     CMAKE_VS_NUGET_PACKAGE_RESTORE). This is the default. **/
  Default,

  /** \brief Ignore behavior defined by preset or cache variable and forces
     packages to be resolved prior to build. **/
  Force,

  /** \brief Ignore behavior defined by preset or cache variable and forces
     packages to be resolved, but skip the actual build. **/
  OnlyResolve,

  /** \brief Ignore behavior defined by preset or cache variable and don't
     resolve any packages **/
  Disable
};

struct cmBuildOptions
{
public:
  cmBuildOptions() noexcept = default;
  explicit cmBuildOptions(bool clean, bool fast,
                          PackageResolveMode resolveMode) noexcept
    : Clean(clean)
    , Fast(fast)
    , ResolveMode(resolveMode)
  {
  }
  explicit cmBuildOptions(const cmBuildOptions&) noexcept = default;
  cmBuildOptions& operator=(const cmBuildOptions&) noexcept = default;

  bool Clean = false;
  bool Fast = false;
  PackageResolveMode ResolveMode = PackageResolveMode::Default;
};
