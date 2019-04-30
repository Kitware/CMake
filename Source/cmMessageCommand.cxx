/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMessageCommand.h"

#include "cmAlgorithms.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmMessenger.h"
#include "cmRange.h"
#include "cmSystemTools.h"
#include "cmake.h"

#include <cassert>

class cmExecutionStatus;

// cmLibraryCommand
bool cmMessageCommand::InitialPass(std::vector<std::string> const& args,
                                   cmExecutionStatus&)
{
  if (args.empty()) {
    this->SetError("called with incorrect number of arguments");
    return false;
  }
  auto i = args.cbegin();

  auto type = MessageType::MESSAGE;
  auto status = false;
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
    if (this->Makefile->IsSet("CMAKE_SUPPRESS_DEVELOPER_ERRORS") &&
        !this->Makefile->IsOn("CMAKE_SUPPRESS_DEVELOPER_ERRORS")) {
      fatal = true;
      type = MessageType::AUTHOR_ERROR;
      level = cmake::LogLevel::LOG_ERROR;
    } else if (!this->Makefile->IsOn("CMAKE_SUPPRESS_DEVELOPER_WARNINGS")) {
      type = MessageType::AUTHOR_WARNING;
      level = cmake::LogLevel::LOG_WARNING;
    } else {
      return true;
    }
    ++i;
  } else if (*i == "STATUS") {
    status = true;
    level = cmake::LogLevel::LOG_STATUS;
    ++i;
  } else if (*i == "VERBOSE") {
    status = true;
    level = cmake::LogLevel::LOG_VERBOSE;
    ++i;
  } else if (*i == "DEBUG") {
    status = true;
    level = cmake::LogLevel::LOG_DEBUG;
    ++i;
  } else if (*i == "TRACE") {
    status = true;
    level = cmake::LogLevel::LOG_TRACE;
    ++i;
  } else if (*i == "DEPRECATION") {
    if (this->Makefile->IsOn("CMAKE_ERROR_DEPRECATED")) {
      fatal = true;
      type = MessageType::DEPRECATION_ERROR;
      level = cmake::LogLevel::LOG_ERROR;
    } else if ((!this->Makefile->IsSet("CMAKE_WARN_DEPRECATED") ||
                this->Makefile->IsOn("CMAKE_WARN_DEPRECATED"))) {
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

  auto desiredLevel = this->Makefile->GetCMakeInstance()->GetLogLevel();
  assert("Expected a valid log level here" &&
         desiredLevel != cmake::LogLevel::LOG_UNDEFINED);

  if (desiredLevel < level) {
    // Suppress the message
    return true;
  }

  auto message = cmJoin(cmMakeRange(i, args.cend()), "");

  if (type != MessageType::MESSAGE) {
    // we've overridden the message type, above, so display it directly
    cmMessenger* m = this->Makefile->GetMessenger();
    m->DisplayMessage(type, message, this->Makefile->GetBacktrace());
  } else {
    if (status) {
      this->Makefile->DisplayStatus(message, -1);
    } else {
      cmSystemTools::Message(message);
    }
  }
  if (fatal) {
    cmSystemTools::SetFatalErrorOccured();
  }
  return true;
}
