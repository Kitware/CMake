/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmAddCustomTargetCommand.h"

#include <utility>

#include "cmCheckCustomOutputs.h"
#include "cmCustomCommandLines.h"
#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

bool cmAddCustomTargetCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  std::string const& targetName = args[0];

  // Check the target name.
  if (targetName.find_first_of("/\\") != std::string::npos) {
    status.SetError(cmStrCat("called with invalid target name \"", targetName,
                             "\".  Target names may not contain a slash.  "
                             "Use ADD_CUSTOM_COMMAND to generate files."));
    return false;
  }

  // Accumulate one command line at a time.
  cmCustomCommandLine currentLine;

  // Save all command lines.
  cmCustomCommandLines commandLines;

  // Accumulate dependencies.
  std::vector<std::string> depends;
  std::vector<std::string> byproducts;
  std::string working_directory;
  bool verbatim = false;
  bool uses_terminal = false;
  bool command_expand_lists = false;
  std::string comment_buffer;
  const char* comment = nullptr;
  std::vector<std::string> sources;
  std::string job_pool;

  // Keep track of parser state.
  enum tdoing
  {
    doing_command,
    doing_depends,
    doing_byproducts,
    doing_working_directory,
    doing_comment,
    doing_source,
    doing_job_pool,
    doing_nothing
  };
  tdoing doing = doing_command;

  // Look for the ALL option.
  bool excludeFromAll = true;
  unsigned int start = 1;
  if (args.size() > 1) {
    if (args[1] == "ALL") {
      excludeFromAll = false;
      start = 2;
    }
  }

  // Parse the rest of the arguments.
  for (unsigned int j = start; j < args.size(); ++j) {
    std::string const& copy = args[j];

    if (copy == "DEPENDS") {
      doing = doing_depends;
    } else if (copy == "BYPRODUCTS") {
      doing = doing_byproducts;
    } else if (copy == "WORKING_DIRECTORY") {
      doing = doing_working_directory;
    } else if (copy == "VERBATIM") {
      doing = doing_nothing;
      verbatim = true;
    } else if (copy == "USES_TERMINAL") {
      doing = doing_nothing;
      uses_terminal = true;
    } else if (copy == "COMMAND_EXPAND_LISTS") {
      doing = doing_nothing;
      command_expand_lists = true;
    } else if (copy == "COMMENT") {
      doing = doing_comment;
    } else if (copy == "JOB_POOL") {
      doing = doing_job_pool;
    } else if (copy == "COMMAND") {
      doing = doing_command;

      // Save the current command before starting the next command.
      if (!currentLine.empty()) {
        commandLines.push_back(currentLine);
        currentLine.clear();
      }
    } else if (copy == "SOURCES") {
      doing = doing_source;
    } else {
      switch (doing) {
        case doing_working_directory:
          working_directory = copy;
          break;
        case doing_command:
          currentLine.push_back(copy);
          break;
        case doing_byproducts: {
          std::string filename;
          if (!cmSystemTools::FileIsFullPath(copy)) {
            filename = cmStrCat(mf.GetCurrentBinaryDirectory(), '/');
          }
          filename += copy;
          cmSystemTools::ConvertToUnixSlashes(filename);
          byproducts.push_back(cmSystemTools::CollapseFullPath(filename));
        } break;
        case doing_depends: {
          std::string dep = copy;
          cmSystemTools::ConvertToUnixSlashes(dep);
          depends.push_back(std::move(dep));
        } break;
        case doing_comment:
          comment_buffer = copy;
          comment = comment_buffer.c_str();
          break;
        case doing_source:
          sources.push_back(copy);
          break;
        case doing_job_pool:
          job_pool = copy;
          break;
        default:
          status.SetError("Wrong syntax. Unknown type of argument.");
          return false;
      }
    }
  }

  std::string::size_type pos = targetName.find_first_of("#<>");
  if (pos != std::string::npos) {
    status.SetError(cmStrCat("called with target name containing a \"",
                             targetName[pos],
                             "\".  This character is not allowed."));
    return false;
  }

  // Some requirements on custom target names already exist
  // and have been checked at this point.
  // The following restrictions overlap but depend on policy CMP0037.
  bool nameOk = cmGeneratorExpression::IsValidTargetName(targetName) &&
    !cmGlobalGenerator::IsReservedTarget(targetName);
  if (nameOk) {
    nameOk = targetName.find(':') == std::string::npos;
  }
  if (!nameOk && !mf.CheckCMP0037(targetName, cmStateEnums::UTILITY)) {
    return false;
  }

  // Store the last command line finished.
  if (!currentLine.empty()) {
    commandLines.push_back(currentLine);
    currentLine.clear();
  }

  // Enforce name uniqueness.
  {
    std::string msg;
    if (!mf.EnforceUniqueName(targetName, msg, true)) {
      status.SetError(msg);
      return false;
    }
  }

  if (commandLines.empty() && !byproducts.empty()) {
    mf.IssueMessage(MessageType::FATAL_ERROR,
                    "BYPRODUCTS may not be specified without any COMMAND");
    return true;
  }
  if (commandLines.empty() && uses_terminal) {
    mf.IssueMessage(MessageType::FATAL_ERROR,
                    "USES_TERMINAL may not be specified without any COMMAND");
    return true;
  }
  if (commandLines.empty() && command_expand_lists) {
    mf.IssueMessage(
      MessageType::FATAL_ERROR,
      "COMMAND_EXPAND_LISTS may not be specified without any COMMAND");
    return true;
  }

  if (uses_terminal && !job_pool.empty()) {
    status.SetError("JOB_POOL is shadowed by USES_TERMINAL.");
    return false;
  }

  // Make sure the byproduct names and locations are safe.
  if (!cmCheckCustomOutputs(byproducts, "BYPRODUCTS", status)) {
    return false;
  }

  // Add the utility target to the makefile.
  bool escapeOldStyle = !verbatim;
  cmTarget* target = mf.AddUtilityCommand(
    targetName, excludeFromAll, working_directory.c_str(), byproducts, depends,
    commandLines, escapeOldStyle, comment, uses_terminal, command_expand_lists,
    job_pool);

  // Add additional user-specified source files to the target.
  target->AddSources(sources);

  return true;
}
