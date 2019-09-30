/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGlobalCommonGenerator_h
#define cmGlobalCommonGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <vector>

#include "cmGlobalGenerator.h"

class cmake;
class cmGeneratorTarget;
class cmLocalGenerator;

/** \class cmGlobalCommonGenerator
 * \brief Common infrastructure for Makefile and Ninja global generators.
 */
class cmGlobalCommonGenerator : public cmGlobalGenerator
{
public:
  cmGlobalCommonGenerator(cmake* cm);
  ~cmGlobalCommonGenerator() override;

  struct DirectoryTarget
  {
    cmLocalGenerator* LG = nullptr;
    struct Target
    {
      cmGeneratorTarget const* GT = nullptr;
      bool ExcludeFromAll = false;
    };
    std::vector<Target> Targets;
    struct Dir
    {
      std::string Path;
      bool ExcludeFromAll = false;
    };
    std::vector<Dir> Children;
  };
  std::map<std::string, DirectoryTarget> ComputeDirectoryTargets() const;
};

#endif
