/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmWorkingDirectory.h"

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmWorkingDirectory::cmWorkingDirectory(std::string const& newdir)
{
  this->OldDir = cmSystemTools::GetLogicalWorkingDirectory();
  this->SetDirectory(newdir);
}

cmWorkingDirectory::~cmWorkingDirectory()
{
  this->Pop();
}

bool cmWorkingDirectory::SetDirectory(std::string const& newdir)
{
  cmsys::Status status = cmSystemTools::SetLogicalWorkingDirectory(newdir);
  if (status) {
    this->Error.clear();
    return true;
  }
  this->Error = cmStrCat("Failed to change working directory to \"", newdir,
                         "\": ", status.GetString());
  return false;
}

void cmWorkingDirectory::Pop()
{
  if (!this->OldDir.empty()) {
    this->SetDirectory(this->OldDir);
    this->OldDir.clear();
  }
}
