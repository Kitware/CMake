/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmSetSourceFilesPropertiesCommand_h
#define cmSetSourceFilesPropertiesCommand_h

#include "cmConfigure.h"

#include <string>
#include <vector>

#include "cmCommand.h"

class cmExecutionStatus;
class cmMakefile;

class cmSetSourceFilesPropertiesCommand : public cmCommand
{
public:
  cmCommand* Clone() CM_OVERRIDE
  {
    return new cmSetSourceFilesPropertiesCommand;
  }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE
  {
    return "set_source_files_properties";
  }

  static bool RunCommand(cmMakefile* mf,
                         std::vector<std::string>::const_iterator filebeg,
                         std::vector<std::string>::const_iterator fileend,
                         std::vector<std::string>::const_iterator propbeg,
                         std::vector<std::string>::const_iterator propend,
                         std::string& errors);
};

#endif
