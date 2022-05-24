/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <set>
#include <string>
#include <vector>

class cmGeneratorTarget;
class cmLocalGenerator;
class cmMakefile;
class cmSourceFile;

class cmOSXBundleGenerator
{
public:
  cmOSXBundleGenerator(cmGeneratorTarget* target);

  struct SkipParts
  {
    SkipParts()
      : infoPlist(false)
    {
    }
    bool infoPlist; // NOLINT(modernize-use-default-member-init)
  };

  // create an app bundle at a given root, and return
  // the directory within the bundle that contains the executable
  void CreateAppBundle(const std::string& targetName, std::string& root,
                       const std::string& config);

  // create a framework at a given root
  void CreateFramework(const std::string& targetName, const std::string& root,
                       const std::string& config,
                       const SkipParts& skipParts = SkipParts());

  // create a cf bundle at a given root
  void CreateCFBundle(const std::string& targetName, const std::string& root,
                      const std::string& config);

  struct MacOSXContentGeneratorType
  {
    virtual ~MacOSXContentGeneratorType() = default;
    virtual void operator()(cmSourceFile const& source, const char* pkgloc,
                            const std::string& config) = 0;
  };

  void GenerateMacOSXContentStatements(
    std::vector<cmSourceFile const*> const& sources,
    MacOSXContentGeneratorType* generator, const std::string& config);
  std::string InitMacOSXContentDirectory(const char* pkgloc,
                                         const std::string& config);

  void SetMacContentFolders(std::set<std::string>* macContentFolders)
  {
    this->MacContentFolders = macContentFolders;
  }

private:
  bool MustSkip();

  cmGeneratorTarget* GT;
  cmMakefile* Makefile;
  cmLocalGenerator* LocalGenerator;
  std::set<std::string>* MacContentFolders = nullptr;
};
