/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <clang-tidy/ClangTidyModule.h>
#include <clang-tidy/ClangTidyModuleRegistry.h>

#include "OstringstreamUseCmstrcatCheck.h"
#include "StringConcatenationUseCmstrcatCheck.h"
#include "UseBespokeEnumClassCheck.h"
#include "UseCmstrlenCheck.h"
#include "UseCmsysFstreamCheck.h"
#include "UsePragmaOnceCheck.h"

namespace clang {
namespace tidy {
namespace cmake {
class CMakeClangTidyModule : public ClangTidyModule
{
public:
  void addCheckFactories(ClangTidyCheckFactories& CheckFactories) override
  {
    CheckFactories.registerCheck<UseCmstrlenCheck>("cmake-use-cmstrlen");
    CheckFactories.registerCheck<UseCmsysFstreamCheck>(
      "cmake-use-cmsys-fstream");
    CheckFactories.registerCheck<UseBespokeEnumClassCheck>(
      "cmake-use-bespoke-enum-class");
    CheckFactories.registerCheck<OstringstreamUseCmstrcatCheck>(
      "cmake-ostringstream-use-cmstrcat");
    CheckFactories.registerCheck<UsePragmaOnceCheck>("cmake-use-pragma-once");
    CheckFactories.registerCheck<StringConcatenationUseCmstrcatCheck>(
      "cmake-string-concatenation-use-cmstrcat");
  }
};

static ClangTidyModuleRegistry::Add<CMakeClangTidyModule> X(
  "cmake-clang-tidy", "Adds lint checks for the CMake code base.");
}
}
}
