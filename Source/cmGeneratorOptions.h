/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

/** Flag if byproducts shall also be considered.  */
enum class cmSourceOutputKind
{
  OutputOnly,
  OutputOrByproduct
};

/** What scanner to use for dependencies lookup.  */
enum class cmDependencyScannerKind
{
  CMake,
  Compiler
};

/** What to compute language flags for */
enum class cmBuildStep
{
  Compile,
  Link
};

/** What compilation mode the swift files are in */
enum class cmSwiftCompileMode
{
  Wholemodule,
  Incremental,
  Singlefile,
  Unknown,
};
