/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmCPackGenerator.h"

/** \class cmCPackAPKGenerator
 * \brief A generator for Android APK packages
 *
 * The APK is assembled by driving the Android SDK build tools
 * (``aapt2``, ``d8``, ``zipalign`` and ``apksigner``) directly,
 * i.e. without Gradle.  The actual work is implemented by the
 * ``Internal/CPack/CPackAPK.cmake`` script.
 */
class cmCPackAPKGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackAPKGenerator, cmCPackGenerator);

  cmCPackAPKGenerator();
  ~cmCPackAPKGenerator() override;

  char const* GetOutputExtension() override { return ".apk"; }

protected:
  int InitializeInternal() override;

  /**
   * An APK holds a single application, components are not supported.
   */
  bool SupportsComponentInstallation() const override { return false; }

  /**
   * Everything is packaged relative to the root of the APK.
   */
  bool SupportsAbsoluteDestination() const override { return false; }

  int PackageFiles() override;
};
