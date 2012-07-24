/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Nicolas Despres <nicolas.despres@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmOSXBundleGenerator_h
#define cmOSXBundleGenerator_h

#include "cmStandardIncludes.h"
#include "cmSourceFile.h"

#include <string>
#include <set>

class cmTarget;
class cmMakefile;
class cmLocalGenerator;

class cmOSXBundleGenerator
{
public:
  static void PrepareTargetProperties(cmTarget* target);

  cmOSXBundleGenerator(cmTarget* target,
                       std::string targetNameOut,
                       const char* configName);

  void CreateAppBundle(std::string& targetName, std::string& outpath);
  void CreateFramework(std::string const& targetName);
  void CreateCFBundle(std::string& targetName, std::string& outpath);

  struct MacOSXContentGeneratorType
  {
    virtual ~MacOSXContentGeneratorType() {}
    virtual void operator()(cmSourceFile& source, const char* pkgloc) = 0;
  };

  void GenerateMacOSXContentStatements(
    std::vector<cmSourceFile*> const& sources,
    MacOSXContentGeneratorType* generator);
  std::string InitMacOSXContentDirectory(const char* pkgloc);

  std::string GetMacContentDirectory() const
  { return this->MacContentDirectory; }
  std::string GetFrameworkVersion() const
  { return this->FrameworkVersion; }
  void SetMacContentFolders(std::set<cmStdString>* macContentFolders)
  { this->MacContentFolders = macContentFolders; }

private:
  bool MustSkip();

private:
  cmTarget* Target;
  cmMakefile* Makefile;
  cmLocalGenerator* LocalGenerator;
  std::string TargetNameOut;
  const char* ConfigName;
  std::string MacContentDirectory;
  std::string FrameworkVersion;
  std::set<cmStdString>* MacContentFolders;
};


#endif
