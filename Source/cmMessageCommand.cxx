/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMessageCommand.h"

#include "cmAlgorithms.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmMessenger.h"
#include "cmRange.h"
#include "cmSystemTools.h"

class cmExecutionStatus;

// cmLibraryCommand
bool cmMessageCommand::InitialPass(std::vector<std::string> const& args,
                                   cmExecutionStatus&)
{
  if (args.empty()) {
    this->SetError("called with incorrect number of arguments");
    return false;
  }
  std::vector<std::string>::const_iterator i = args.begin();

  MessageType type = MessageType::MESSAGE;
  bool status = false;
  bool fatal = false;
  if (*i == "SEND_ERROR") {
    type = MessageType::FATAL_ERROR;
    ++i;
  } else if (*i == "FATAL_ERROR") {
    fatal = true;
    type = MessageType::FATAL_ERROR;
    ++i;
  } else if (*i == "WARNING") {
    type = MessageType::WARNING;
    ++i;
  } else if (*i == "AUTHOR_WARNING") {
    if (this->Makefile->IsSet("CMAKE_SUPPRESS_DEVELOPER_ERRORS") &&
        !this->Makefile->IsOn("CMAKE_SUPPRESS_DEVELOPER_ERRORS")) {
      fatal = true;
      type = MessageType::AUTHOR_ERROR;
    } else if (!this->Makefile->IsOn("CMAKE_SUPPRESS_DEVELOPER_WARNINGS")) {
      type = MessageType::AUTHOR_WARNING;
    } else {
      return true;
    }
    ++i;
  } else if (*i == "STATUS") {
    status = true;
    ++i;
  } else if (*i == "DEPRECATION") {
    if (this->Makefile->IsOn("CMAKE_ERROR_DEPRECATED")) {
      fatal = true;
      type = MessageType::DEPRECATION_ERROR;
    } else if ((!this->Makefile->IsSet("CMAKE_WARN_DEPRECATED") ||
                this->Makefile->IsOn("CMAKE_WARN_DEPRECATED"))) {
      type = MessageType::DEPRECATION_WARNING;
    } else {
      return true;
    }
    ++i;
  }

  std::string message = cmJoin(cmMakeRange(i, args.end()), std::string());

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
