/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallSubdirectoryGenerator.h"

#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmPolicies.h"
#include "cmScriptGenerator.h"
#include "cmSystemTools.h"

#include <sstream>
#include <vector>

cmInstallSubdirectoryGenerator::cmInstallSubdirectoryGenerator(
  cmMakefile* makefile, const char* binaryDirectory, bool excludeFromAll)
  : cmInstallGenerator(nullptr, std::vector<std::string>(), nullptr,
                       MessageDefault, excludeFromAll)
  , Makefile(makefile)
  , BinaryDirectory(binaryDirectory)
{
}

cmInstallSubdirectoryGenerator::~cmInstallSubdirectoryGenerator() = default;

bool cmInstallSubdirectoryGenerator::HaveInstall()
{
  for (auto generator : this->Makefile->GetInstallGenerators()) {
    if (generator->HaveInstall()) {
      return true;
    }
  }

  return false;
}

void cmInstallSubdirectoryGenerator::CheckCMP0082(
  bool& haveSubdirectoryInstall, bool& /*unused*/)
{
  if (this->HaveInstall()) {
    haveSubdirectoryInstall = true;
  }
}

bool cmInstallSubdirectoryGenerator::Compute(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;
  return true;
}

void cmInstallSubdirectoryGenerator::GenerateScript(std::ostream& os)
{
  if (!this->ExcludeFromAll) {
    cmPolicies::PolicyStatus status =
      this->LocalGenerator->GetPolicyStatus(cmPolicies::CMP0082);
    switch (status) {
      case cmPolicies::WARN:
      case cmPolicies::OLD:
        // OLD behavior is handled in cmLocalGenerator::GenerateInstallRules()
        break;

      case cmPolicies::NEW:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS: {
        Indent indent;
        std::string odir = this->BinaryDirectory;
        cmSystemTools::ConvertToUnixSlashes(odir);
        os << indent << "if(NOT CMAKE_INSTALL_LOCAL_ONLY)\n"
           << indent.Next()
           << "# Include the install script for the subdirectory.\n"
           << indent.Next() << "include(\"" << odir
           << "/cmake_install.cmake\")\n"
           << indent << "endif()\n\n";
      } break;
    }
  }
}
