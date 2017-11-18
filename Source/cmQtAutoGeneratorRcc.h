/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGeneratorRcc_h
#define cmQtAutoGeneratorRcc_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmFilePathChecksum.h"
#include "cmQtAutoGen.h"
#include "cmQtAutoGenerator.h"

#include <string>
#include <vector>

class cmMakefile;

class cmQtAutoGeneratorRcc : public cmQtAutoGenerator
{
  CM_DISABLE_COPY(cmQtAutoGeneratorRcc)
public:
  cmQtAutoGeneratorRcc();

private:
  // -- Initialization & settings
  bool InfoFileRead(cmMakefile* makefile);
  void SettingsFileRead(cmMakefile* makefile);
  bool SettingsFileWrite();
  // -- Central processing
  bool Process(cmMakefile* makefile) override;
  bool RccGenerate();

  // -- Config settings
  std::string ConfigSuffix;
  cmQtAutoGen::MultiConfig MultiConfig;
  // -- Settings
  bool SettingsChanged;
  std::string SettingsFile;
  std::string SettingsString;
  // -- Directories
  std::string ProjectSourceDir;
  std::string ProjectBinaryDir;
  std::string CurrentSourceDir;
  std::string CurrentBinaryDir;
  std::string AutogenBuildDir;
  cmFilePathChecksum FilePathChecksum;
  // -- Qt environment
  std::string QtMajorVersion;
  std::string RccExecutable;
  // -- Job
  std::string QrcFile;
  std::string RccFile;
  std::vector<std::string> Options;
  std::vector<std::string> Inputs;
};

#endif
