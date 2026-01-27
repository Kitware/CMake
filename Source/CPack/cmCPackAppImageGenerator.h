/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <unordered_map>

#include <cm/optional>

#include "cmCPackGenerator.h"

/** \class cmCPackAppImageGenerator
 * \brief A generator for creating AppImages with CPack
 */
class cmCPackAppImageGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackAppImageGenerator, cmCPackGenerator);

  char const* GetOutputExtension() override { return ".AppImage"; }

  cmCPackAppImageGenerator();
  ~cmCPackAppImageGenerator() override;

protected:
  /**
   * @brief Initializes the CPack engine with our defaults
   */
  int InitializeInternal() override;

  /**
   * @brief AppImages are for single applications
   */
  bool SupportsComponentInstallation() const override { return false; }

  /**
   * Main Packaging step
   */
  int PackageFiles() override;

private:
  /**
   * @brief Finds the first installed file by it's name
   */
  cm::optional<std::string> FindFile(std::string const& filename) const;

  /**
   * @brief AppImage format requires a desktop file
   */
  cm::optional<std::string> FindDesktopFile() const;

  /**
   * @brief Parses a desktop file [Desktop Entry]
   */
  std::unordered_map<std::string, std::string> ParseDesktopFile(
    std::string const& filePath) const;

  /**
   * @brief changes the RPATH so that AppImage can find it's libraries
   */
  bool ChangeRPath();

  bool PatchElfSetRPath(std::string const& file,
                        std::string const& rpath) const;

  std::string AppimagetoolPath;
  std::string PatchElfPath;
};
