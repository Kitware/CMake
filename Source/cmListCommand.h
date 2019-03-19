/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmListCommand_h
#define cmListCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmCommand.h"

class cmExecutionStatus;

/** \class cmListCommand
 * \brief Common list operations
 *
 */
class cmListCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() override { return new cmListCommand; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;

protected:
  bool HandleLengthCommand(std::vector<std::string> const& args);
  bool HandleGetCommand(std::vector<std::string> const& args);
  bool HandleAppendCommand(std::vector<std::string> const& args);
  bool HandlePrependCommand(std::vector<std::string> const& args);
  bool HandlePopBackCommand(std::vector<std::string> const& args);
  bool HandlePopFrontCommand(std::vector<std::string> const& args);
  bool HandleFindCommand(std::vector<std::string> const& args);
  bool HandleInsertCommand(std::vector<std::string> const& args);
  bool HandleJoinCommand(std::vector<std::string> const& args);
  bool HandleRemoveAtCommand(std::vector<std::string> const& args);
  bool HandleRemoveItemCommand(std::vector<std::string> const& args);
  bool HandleRemoveDuplicatesCommand(std::vector<std::string> const& args);
  bool HandleTransformCommand(std::vector<std::string> const& args);
  bool HandleSortCommand(std::vector<std::string> const& args);
  bool HandleSublistCommand(std::vector<std::string> const& args);
  bool HandleReverseCommand(std::vector<std::string> const& args);
  bool HandleFilterCommand(std::vector<std::string> const& args);
  bool FilterRegex(std::vector<std::string> const& args, bool includeMatches,
                   std::string const& listName,
                   std::vector<std::string>& varArgsExpanded);

  bool GetList(std::vector<std::string>& list, const std::string& var);
  bool GetListString(std::string& listString, const std::string& var);
};

#endif
