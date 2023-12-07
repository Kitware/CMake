/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

/* This code was originally taken from part of the Clang-Tidy LLVM project and
 * modified for use with CMake under the following original license: */

//===--- HeaderGuard.h - clang-tidy -----------------------------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions. See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <clang-tidy/ClangTidyCheck.h>
#include <clang-tidy/utils/FileExtensionsUtils.h>

#if LLVM_VERSION_MAJOR >= 17
#  include <clang-tidy/FileExtensionsSet.h>
#else
namespace clang {
namespace tidy {
using utils::FileExtensionsSet;
} // namespace tidy
} // namespace clang
#endif

namespace clang {
namespace tidy {
namespace cmake {

/// Finds and replaces header guards with pragma once.
/// The check supports these options:
///   - `HeaderFileExtensions`: a semicolon-separated list of filename
///     extensions of header files (The filename extension should not contain
///     "." prefix). ";h;hh;hpp;hxx" by default.
///
///     For extension-less header files, using an empty string or leaving an
///     empty string between ";" if there are other filename extensions.
class UsePragmaOnceCheck : public ClangTidyCheck
{
public:
  UsePragmaOnceCheck(StringRef Name, ClangTidyContext* Context)
    : ClangTidyCheck(Name, Context)
    , RawStringHeaderFileExtensions(Options.getLocalOrGlobal(
        "HeaderFileExtensions", utils::defaultHeaderFileExtensions()))
  {
    utils::parseFileExtensions(RawStringHeaderFileExtensions,
                               HeaderFileExtensions,
                               utils::defaultFileExtensionDelimiters());
  }
  void storeOptions(ClangTidyOptions::OptionMap& Opts) override;
  void registerPPCallbacks(const SourceManager& SM, Preprocessor* PP,
                           Preprocessor* ModuleExpanderPP) override;

  /// Returns ``true`` if the check should add pragma once to the file
  /// if it has none.
  virtual bool shouldSuggestToAddPragmaOnce(StringRef Filename);

private:
  std::string RawStringHeaderFileExtensions;
  FileExtensionsSet HeaderFileExtensions;
};

} // namespace cmake
} // namespace tidy
} // namespace clang
