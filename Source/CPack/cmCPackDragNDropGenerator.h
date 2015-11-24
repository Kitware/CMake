/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackDragNDropGenerator_h
#define cmCPackDragNDropGenerator_h

#include "cmCPackGenerator.h"

class cmGeneratedFileStream;

/** \class cmCPackDragNDropGenerator
 * \brief A generator for OSX drag-n-drop installs
 */
class cmCPackDragNDropGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackDragNDropGenerator, cmCPackGenerator);

  cmCPackDragNDropGenerator();
  virtual ~cmCPackDragNDropGenerator();

protected:
  virtual int InitializeInternal();
  virtual const char* GetOutputExtension();
  int PackageFiles();
  bool SupportsComponentInstallation() const;


  bool CopyFile(std::ostringstream& source, std::ostringstream& target);
  bool CreateEmptyFile(std::ostringstream& target, size_t size);
  bool RunCommand(std::ostringstream& command, std::string* output = 0);

  std::string
  GetComponentInstallDirNameSuffix(const std::string& componentName);

  int CreateDMG(const std::string& src_dir, const std::string& output_file);

  std::string InstallPrefix;

private:
  std::string slaDirectory;
  bool singleLicense;

  void WriteLicense(cmGeneratedFileStream& outputStream, int licenseNumber,
    std::string licenseLanguage, std::string licenseFile = "");
  void BreakLongLine(const std::string& line,
    std::vector<std::string>& lines);
  void EscapeQuotes(std::string& line);
};

#endif
