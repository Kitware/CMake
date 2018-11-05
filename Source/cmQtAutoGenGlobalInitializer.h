/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGenGlobalInitializer_h
#define cmQtAutoGenGlobalInitializer_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
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
  bool InitializeCustomTargets();
  bool SetupCustomTargets();

private:
  std::vector<std::unique_ptr<cmQtAutoGenInitializer>> Initializers_;
};

#endif
