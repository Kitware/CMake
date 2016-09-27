/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmOutputRequiredFilesCommand_h
#define cmOutputRequiredFilesCommand_h

#include "cmCommand.h"

class cmDependInformation;

class cmOutputRequiredFilesCommand : public cmCommand
{
public:
  cmTypeMacro(cmOutputRequiredFilesCommand, cmCommand);
  cmCommand* Clone() CM_OVERRIDE { return new cmOutputRequiredFilesCommand; }
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;
  std::string GetName() const CM_OVERRIDE { return "output_required_files"; }

  void ListDependencies(cmDependInformation const* info, FILE* fout,
                        std::set<cmDependInformation const*>* visited);

private:
  std::string File;
  std::string OutputFile;
};

#endif
