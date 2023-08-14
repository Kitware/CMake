/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "cmQtAutoGen.h"

class cmLocalGenerator;
class cmQtAutoGenInitializer;

/// @brief Initializes the QtAutoGen generators
class cmQtAutoGenGlobalInitializer
{
public:
  /// @brief Collection of QtAutogen related keywords
  class Keywords
  {
  public:
    Keywords();

    std::string AUTOMOC;
    std::string AUTOUIC;
    std::string AUTORCC;

    std::string AUTOMOC_EXECUTABLE;
    std::string AUTOUIC_EXECUTABLE;
    std::string AUTORCC_EXECUTABLE;

    std::string SKIP_AUTOGEN;
    std::string SKIP_AUTOMOC;
    std::string SKIP_AUTOUIC;
    std::string SKIP_AUTORCC;

    std::string AUTOUIC_OPTIONS;
    std::string AUTORCC_OPTIONS;

    std::string qrc;
    std::string ui;
  };

  cmQtAutoGenGlobalInitializer(
    std::vector<std::unique_ptr<cmLocalGenerator>> const& localGenerators);
  ~cmQtAutoGenGlobalInitializer();

  Keywords const& kw() const { return this->Keywords_; }

  bool InitializeCustomTargets();
  bool SetupCustomTargets();

private:
  friend class cmQtAutoGenInitializer;

  void GetOrCreateGlobalTarget(cmLocalGenerator* localGen,
                               std::string const& name,
                               std::string const& comment);

  void AddToGlobalAutoGen(cmLocalGenerator* localGen,
                          std::string const& targetName);
  void AddToGlobalAutoRcc(cmLocalGenerator* localGen,
                          std::string const& targetName);

  cmQtAutoGen::CompilerFeaturesHandle GetCompilerFeatures(
    std::string const& generator, std::string const& executable,
    std::string& error);

  std::vector<std::unique_ptr<cmQtAutoGenInitializer>> Initializers_;
  std::map<cmLocalGenerator*, std::string> GlobalAutoGenTargets_;
  std::map<cmLocalGenerator*, std::string> GlobalAutoRccTargets_;
  std::unordered_map<std::string, cmQtAutoGen::CompilerFeaturesHandle>
    CompilerFeatures_;
  Keywords const Keywords_;
};
