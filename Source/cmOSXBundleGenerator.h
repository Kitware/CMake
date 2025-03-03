/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
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
    SkipParts() {} // NOLINT(modernize-use-equals-default)

    bool InfoPlist = false;
    bool TextStubs = true;
  };

  // create an app bundle at a given root, and return
  // the directory within the bundle that contains the executable
  void CreateAppBundle(std::string const& targetName, std::string& root,
                       std::string const& config);

  // create a framework at a given root
  void CreateFramework(std::string const& targetName, std::string const& root,
                       std::string const& config,
                       SkipParts const& skipParts = SkipParts{});

  // create a cf bundle at a given root
  void CreateCFBundle(std::string const& targetName, std::string const& root,
                      std::string const& config);

  struct MacOSXContentGeneratorType
  {
    virtual ~MacOSXContentGeneratorType() = default;
    virtual void operator()(cmSourceFile const& source, char const* pkgloc,
                            std::string const& config) = 0;
  };

  void GenerateMacOSXContentStatements(
    std::vector<cmSourceFile const*> const& sources,
    MacOSXContentGeneratorType* generator, std::string const& config);
  std::string InitMacOSXContentDirectory(char const* pkgloc,
                                         std::string const& config);

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
