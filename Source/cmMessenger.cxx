/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMessenger.h"

#include "cmDocumentationFormatter.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include "cmsys/SystemInformation.hxx"
#endif

#include <sstream>

MessageType cmMessenger::ConvertMessageType(MessageType t) const
{
  bool warningsAsErrors;

  if (t == MessageType::AUTHOR_WARNING || t == MessageType::AUTHOR_ERROR) {
    warningsAsErrors = this->GetDevWarningsAsErrors();
    if (warningsAsErrors && t == MessageType::AUTHOR_WARNING) {
      t = MessageType::AUTHOR_ERROR;
    } else if (!warningsAsErrors && t == MessageType::AUTHOR_ERROR) {
      t = MessageType::AUTHOR_WARNING;
    }
  } else if (t == MessageType::DEPRECATION_WARNING ||
             t == MessageType::DEPRECATION_ERROR) {
    warningsAsErrors = this->GetDeprecatedWarningsAsErrors();
    if (warningsAsErrors && t == MessageType::DEPRECATION_WARNING) {
      t = MessageType::DEPRECATION_ERROR;
    } else if (!warningsAsErrors && t == MessageType::DEPRECATION_ERROR) {
      t = MessageType::DEPRECATION_WARNING;
    }
  }

  return t;
}

bool cmMessenger::IsMessageTypeVisible(MessageType t) const
{
  bool isVisible = true;

  if (t == MessageType::DEPRECATION_ERROR) {
    if (!this->GetDeprecatedWarningsAsErrors()) {
      isVisible = false;
    }
  } else if (t == MessageType::DEPRECATION_WARNING) {
    if (this->GetSuppressDeprecatedWarnings()) {
      isVisible = false;
    }
  } else if (t == MessageType::AUTHOR_ERROR) {
    if (!this->GetDevWarningsAsErrors()) {
      isVisible = false;
    }
  } else if (t == MessageType::AUTHOR_WARNING) {
    if (this->GetSuppressDevWarnings()) {
      isVisible = false;
    }
  }

  return isVisible;
}

static bool printMessagePreamble(MessageType t, std::ostream& msg)
{
  // Construct the message header.
  if (t == MessageType::FATAL_ERROR) {
    msg << "CMake Error";
  } else if (t == MessageType::INTERNAL_ERROR) {
    msg << "CMake Internal Error (please report a bug)";
  } else if (t == MessageType::LOG) {
    msg << "CMake Debug Log";
  } else if (t == MessageType::DEPRECATION_ERROR) {
    msg << "CMake Deprecation Error";
  } else if (t == MessageType::DEPRECATION_WARNING) {
    msg << "CMake Deprecation Warning";
  } else if (t == MessageType::AUTHOR_WARNING) {
    msg << "CMake Warning (dev)";
  } else if (t == MessageType::AUTHOR_ERROR) {
    msg << "CMake Error (dev)";
  } else {
    msg << "CMake Warning";
  }
  return true;
}

void printMessageText(std::ostream& msg, std::string const& text)
{
  msg << ":\n";
  cmDocumentationFormatter formatter;
  formatter.SetIndent("  ");
  formatter.PrintFormatted(msg, text.c_str());
}

void displayMessage(MessageType t, std::ostringstream& msg)
{
  // Add a note about warning suppression.
  if (t == MessageType::AUTHOR_WARNING) {
    msg << "This warning is for project developers.  Use -Wno-dev to suppress "
           "it.";
  } else if (t == MessageType::AUTHOR_ERROR) {
    msg << "This error is for project developers. Use -Wno-error=dev to "
           "suppress it.";
  }

  // Add a terminating blank line.
  msg << "\n";

#if !defined(CMAKE_BOOTSTRAP)
  // Add a C++ stack trace to internal errors.
  if (t == MessageType::INTERNAL_ERROR) {
    std::string stack = cmsys::SystemInformation::GetProgramStack(0, 0);
    if (!stack.empty()) {
      if (cmHasLiteralPrefix(stack, "WARNING:")) {
        stack = "Note:" + stack.substr(8);
      }
      msg << stack << "\n";
    }
  }
#endif

  // Output the message.
  if (t == MessageType::FATAL_ERROR || t == MessageType::INTERNAL_ERROR ||
      t == MessageType::DEPRECATION_ERROR || t == MessageType::AUTHOR_ERROR) {
    cmSystemTools::SetErrorOccured();
    cmSystemTools::Message(msg.str(), "Error");
  } else {
    cmSystemTools::Message(msg.str(), "Warning");
  }
}

void cmMessenger::IssueMessage(MessageType t, const std::string& text,
                               const cmListFileBacktrace& backtrace) const
{
  bool force = false;
  if (!force) {
    // override the message type, if needed, for warnings and errors
    MessageType override = this->ConvertMessageType(t);
    if (override != t) {
      t = override;
      force = true;
    }
  }

  if (!force && !this->IsMessageTypeVisible(t)) {
    return;
  }
  this->DisplayMessage(t, text, backtrace);
}

void cmMessenger::DisplayMessage(MessageType t, const std::string& text,
                                 const cmListFileBacktrace& backtrace) const
{
  std::ostringstream msg;
  if (!printMessagePreamble(t, msg)) {
    return;
  }

  // Add the immediate context.
  backtrace.PrintTitle(msg);

  printMessageText(msg, text);

  // Add the rest of the context.
  backtrace.PrintCallStack(msg);

  displayMessage(t, msg);
}
