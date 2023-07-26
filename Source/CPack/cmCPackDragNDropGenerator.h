/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

#include "cmCPackGenerator.h"

class cmGeneratedFileStream;
class cmXMLWriter;

/** \class cmCPackDragNDropGenerator
 * \brief A generator for OSX drag-n-drop installs
 */
class cmCPackDragNDropGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackDragNDropGenerator, cmCPackGenerator);

  cmCPackDragNDropGenerator();
  ~cmCPackDragNDropGenerator() override;

protected:
  int InitializeInternal() override;
  const char* GetOutputExtension() override;
  int PackageFiles() override;
  bool SupportsComponentInstallation() const override;

  bool CopyFile(std::ostringstream& source, std::ostringstream& target);
  bool CreateEmptyFile(std::ostringstream& target, size_t size);
  bool RunCommand(std::string const& command, std::string* output = nullptr);

  std::string GetComponentInstallDirNameSuffix(
    const std::string& componentName) override;

  int CreateDMG(const std::string& src_dir, const std::string& output_file);

private:
  std::string slaDirectory;
  bool singleLicense;

  struct RezDict
  {
    std::string Name;
    size_t ID;
    std::vector<unsigned char> Data;
  };

  struct RezArray
  {
    std::string Key;
    std::vector<RezDict> Entries;
  };

  struct RezDoc
  {
    RezArray LPic = { "LPic", {} };
    RezArray Menu = { "STR#", {} };
    RezArray Text = { "TEXT", {} };
    RezArray RTF = { "RTF ", {} };
  };

  void WriteRezXML(std::string const& file, RezDoc const& rez);
  void WriteRezArray(cmXMLWriter& xml, RezArray const& array);
  void WriteRezDict(cmXMLWriter& xml, RezDict const& dict);

  bool WriteLicense(RezDoc& rez, size_t licenseNumber,
                    std::string licenseLanguage,
                    const std::string& licenseFile, std::string* error);
  void EncodeLicense(RezDict& dict, std::vector<std::string> const& lines);
  void EncodeMenu(RezDict& dict, std::vector<std::string> const& lines);
  bool ReadFile(std::string const& file, std::vector<std::string>& lines,
                std::string* error);
  bool BreakLongLine(const std::string& line, std::vector<std::string>& lines,
                     std::string* error);
};
