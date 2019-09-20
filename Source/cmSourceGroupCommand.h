/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmSourceGroupCommand_h
#define cmSourceGroupCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <vector>

#include <cm/memory>

#include "cmCommand.h"

class cmExecutionStatus;

/** \class cmSourceGroupCommand
 * \brief Adds a cmSourceGroup to the cmMakefile.
 *
 * cmSourceGroupCommand is used to define cmSourceGroups which split up
 * source files in to named, organized groups in the generated makefiles.
 */
class cmSourceGroupCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  std::unique_ptr<cmCommand> Clone() override
  {
    return cm::make_unique<cmSourceGroupCommand>();
  }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;

private:
  using ParsedArguments = std::map<std::string, std::vector<std::string>>;
  using ExpectedOptions = std::vector<std::string>;

  ExpectedOptions getExpectedOptions() const;

  bool isExpectedOption(const std::string& argument,
                        const ExpectedOptions& expectedOptions);

  void parseArguments(const std::vector<std::string>& args,
                      cmSourceGroupCommand::ParsedArguments& parsedArguments);

  bool processTree(ParsedArguments& parsedArguments, std::string& errorMsg);

  bool checkArgumentsPreconditions(const ParsedArguments& parsedArguments,
                                   std::string& errorMsg) const;
  bool checkSingleParameterArgumentPreconditions(
    const std::string& argument, const ParsedArguments& parsedArguments,
    std::string& errorMsg) const;
};

#endif
