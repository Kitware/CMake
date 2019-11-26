/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMessageCommand.h"

#include <cassert>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmMessenger.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

// cmLibraryCommand
bool cmMessageCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  auto i = args.cbegin();

  auto type = MessageType::MESSAGE;
  auto fatal = false;
  auto level = cmake::LogLevel::LOG_UNDEFINED;
  if (*i == "SEND_ERROR") {
    type = MessageType::FATAL_ERROR;
    level = cmake::LogLevel::LOG_ERROR;
    ++i;
  } else if (*i == "FATAL_ERROR") {
    fatal = true;
    type = MessageType::FATAL_ERROR;
    level = cmake::LogLevel::LOG_ERROR;
    ++i;
  } else if (*i == "WARNING") {
    type = MessageType::WARNING;
    level = cmake::LogLevel::LOG_WARNING;
    ++i;
  } else if (*i == "AUTHOR_WARNING") {
    if (status.GetMakefile().IsSet("CMAKE_SUPPRESS_DEVELOPER_ERRORS") &&
        !status.GetMakefile().IsOn("CMAKE_SUPPRESS_DEVELOPER_ERRORS")) {
      fatal = true;
      type = MessageType::AUTHOR_ERROR;
      level = cmake::LogLevel::LOG_ERROR;
    } else if (!status.GetMakefile().IsOn(
                 "CMAKE_SUPPRESS_DEVELOPER_WARNINGS")) {
      type = MessageType::AUTHOR_WARNING;
      level = cmake::LogLevel::LOG_WARNING;
    } else {
      return true;
    }
    ++i;
  } else if (*i == "STATUS") {
    level = cmake::LogLevel::LOG_STATUS;
    ++i;
  } else if (*i == "VERBOSE") {
    level = cmake::LogLevel::LOG_VERBOSE;
    ++i;
  } else if (*i == "DEBUG") {
    level = cmake::LogLevel::LOG_DEBUG;
    ++i;
  } else if (*i == "TRACE") {
    level = cmake::LogLevel::LOG_TRACE;
    ++i;
  } else if (*i == "DEPRECATION") {
    if (status.GetMakefile().IsOn("CMAKE_ERROR_DEPRECATED")) {
      fatal = true;
      type = MessageType::DEPRECATION_ERROR;
      level = cmake::LogLevel::LOG_ERROR;
    } else if (!status.GetMakefile().IsSet("CMAKE_WARN_DEPRECATED") ||
               status.GetMakefile().IsOn("CMAKE_WARN_DEPRECATED")) {
      type = MessageType::DEPRECATION_WARNING;
      level = cmake::LogLevel::LOG_WARNING;
    } else {
      return true;
    }
    ++i;
  } else if (*i == "NOTICE") {
    // `NOTICE` message type is going to be output to stderr
    level = cmake::LogLevel::LOG_NOTICE;
    ++i;
  } else {
    // Messages w/o any type are `NOTICE`s
    level = cmake::LogLevel::LOG_NOTICE;
  }
  assert("Message log level expected to be set" &&
         level != cmake::LogLevel::LOG_UNDEFINED);

  auto desiredLevel = status.GetMakefile().GetCMakeInstance()->GetLogLevel();
  assert("Expected a valid log level here" &&
         desiredLevel != cmake::LogLevel::LOG_UNDEFINED);

  if (desiredLevel < level) {
    // Suppress the message
    return true;
  }

  auto message = cmJoin(cmMakeRange(i, args.cend()), "");

  if (cmake::LogLevel::LOG_NOTICE <= level) {
    // Check if any indentation has requested:
    // `CMAKE_MESSAGE_INDENT` is a list of "padding" pieces
    // to be joined and prepended to the message lines.
    auto indent = cmJoin(cmExpandedList(status.GetMakefile().GetSafeDefinition(
                           "CMAKE_MESSAGE_INDENT")),
                         "");
    // Make every line of the `message` indented
    // NOTE Can't reuse `cmDocumentationFormatter::PrintPreformatted`
    // here cuz it appends `\n` to the EOM ;-(
    cmSystemTools::ReplaceString(message, "\n", "\n" + indent);
    message = indent + message;
  }

  switch (level) {
    case cmake::LogLevel::LOG_ERROR:
    case cmake::LogLevel::LOG_WARNING:
      // we've overridden the message type, above, so display it directly
      status.GetMakefile().GetMessenger()->DisplayMessage(
        type, message, status.GetMakefile().GetBacktrace());
      break;

    case cmake::LogLevel::LOG_NOTICE:
      cmSystemTools::Message(message);
      break;

    case cmake::LogLevel::LOG_STATUS:
    case cmake::LogLevel::LOG_VERBOSE:
    case cmake::LogLevel::LOG_DEBUG:
    case cmake::LogLevel::LOG_TRACE:
      status.GetMakefile().DisplayStatus(message, -1);
      break;

    default:
      assert("Unexpected log level! Review the `cmMessageCommand.cxx`." &&
             false);
      break;
  }

  if (fatal) {
    cmSystemTools::SetFatalErrorOccured();
  }
  return true;
}
