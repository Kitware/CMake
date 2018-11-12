/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGenGlobalInitializer_h
#define cmQtAutoGenGlobalInitializer_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory> // IWYU pragma: keep
#include <string>
#include <vector>

class cmLocalGenerator;
class cmQtAutoGenInitializer;

/// @brief Initializes the QtAutoGen generators
class cmQtAutoGenGlobalInitializer
{
public:
  cmQtAutoGenGlobalInitializer(
    std::vector<cmLocalGenerator*> const& localGenerators);
  ~cmQtAutoGenGlobalInitializer();

  bool generate();

private:
  friend class cmQtAutoGenInitializer;

  bool InitializeCustomTargets();
  bool SetupCustomTargets();

  void GetOrCreateGlobalTarget(cmLocalGenerator* localGen,
                               std::string const& name,
                               std::string const& comment);

  void AddToGlobalAutoGen(cmLocalGenerator* localGen,
                          std::string const& targetName);
  void AddToGlobalAutoRcc(cmLocalGenerator* localGen,
                          std::string const& targetName);

private:
  std::vector<std::unique_ptr<cmQtAutoGenInitializer>> Initializers_;
  std::map<cmLocalGenerator*, std::string> GlobalAutoGenTargets_;
  std::map<cmLocalGenerator*, std::string> GlobalAutoRccTargets_;
};

#endif
