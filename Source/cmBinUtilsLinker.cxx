/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmBinUtilsLinker.h"

#include <utility>

#include "cmCMakePath.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmRuntimeDependencyArchive.h"
#include "cmStringAlgorithms.h"

cmBinUtilsLinker::cmBinUtilsLinker(cmRuntimeDependencyArchive* archive)
  : Archive(archive)
{
}

void cmBinUtilsLinker::SetError(std::string const& e)
{
  this->Archive->SetError(e);
}

void cmBinUtilsLinker::NormalizePath(std::string& path) const
{
  std::string normalizedPath =
    cmCMakePath(path, cmCMakePath::auto_format).GenericString();

  if (path == normalizedPath) {
    return;
  }

  cmPolicies::PolicyStatus policy =
    this->Archive->GetMakefile()->GetPolicyStatus(cmPolicies::CMP0207);
  if (policy == cmPolicies::WARN) {
    this->Archive->GetMakefile()->IssueMessage(
      MessageType::AUTHOR_WARNING,
      cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0207),
               "\n"
               "Path\n  \"",
               path,
               "\"\n"
               "would be converted to\n  \"",
               normalizedPath, "\"\n"));
  } else if (policy == cmPolicies::NEW) {
    path = std::move(normalizedPath);
  }
}
