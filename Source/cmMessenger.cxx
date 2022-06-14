/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMessenger.h"

#include "cmDocumentationFormatter.h"
#include "cmMessageMetadata.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include "cmsys/SystemInformation.hxx"
#endif

#include <sstream>
#include <utility>

#include "cmsys/Terminal.h"

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

static int getMessageColor(MessageType t)
{
  switch (t) {
    case MessageType::INTERNAL_ERROR:
    case MessageType::FATAL_ERROR:
    case MessageType::AUTHOR_ERROR:
      return cmsysTerminal_Color_ForegroundRed;
    case MessageType::AUTHOR_WARNING:
    case MessageType::WARNING:
      return cmsysTerminal_Color_ForegroundYellow;
    default:
      return cmsysTerminal_Color_Normal;
  }
}

static void printMessageText(std::ostream& msg, std::string const& text)
{
  msg << ":\n";
  cmDocumentationFormatter formatter;
  formatter.SetIndent("  ");
  formatter.PrintFormatted(msg, text.c_str());
}

static void displayMessage(MessageType t, std::ostringstream& msg)
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
  cmMessageMetadata md;
  md.desiredColor = getMessageColor(t);
  if (t == MessageType::FATAL_ERROR || t == MessageType::INTERNAL_ERROR ||
      t == MessageType::DEPRECATION_ERROR || t == MessageType::AUTHOR_ERROR) {
    cmSystemTools::SetErrorOccurred();
    md.title = "Error";
    cmSystemTools::Message(msg.str(), md);
  } else {
    md.title = "Warning";
    cmSystemTools::Message(msg.str(), md);
  }
}

namespace {
void PrintCallStack(std::ostream& out, cmListFileBacktrace bt,
                    cm::optional<std::string> const& topSource)
{
  // The call stack exists only if we have at least two calls on top
  // of the bottom.
  if (bt.Empty()) {
    return;
  }
  bt = bt.Pop();
  if (bt.Empty()) {
    return;
  }

  bool first = true;
  for (; !bt.Empty(); bt = bt.Pop()) {
    cmListFileContext lfc = bt.Top();
    if (lfc.Name.empty() &&
        lfc.Line != cmListFileContext::DeferPlaceholderLine) {
      // Skip this whole-file scope.  When we get here we already will
      // have printed a more-specific context within the file.
      continue;
    }
    if (first) {
      first = false;
      out << "Call Stack (most recent call first):\n";
    }
    if (topSource) {
      lfc.FilePath = cmSystemTools::RelativeIfUnder(*topSource, lfc.FilePath);
    }
    out << "  " << lfc << "\n";
  }
}
}

void cmMessenger::IssueMessage(MessageType t, const std::string& text,
                               const cmListFileBacktrace& backtrace) const
{
  bool force = false;
  // override the message type, if needed, for warnings and errors
  MessageType override = this->ConvertMessageType(t);
  if (override != t) {
    t = override;
    force = true;
  }

  if (force || this->IsMessageTypeVisible(t)) {
    this->DisplayMessage(t, text, backtrace);
  }
}

void cmMessenger::DisplayMessage(MessageType t, const std::string& text,
                                 const cmListFileBacktrace& backtrace) const
{
  std::ostringstream msg;
  if (!printMessagePreamble(t, msg)) {
    return;
  }

  // Add the immediate context.
  this->PrintBacktraceTitle(msg, backtrace);

  printMessageText(msg, text);

  // Add the rest of the context.
  PrintCallStack(msg, backtrace, this->TopSource);

  displayMessage(t, msg);
}

void cmMessenger::PrintBacktraceTitle(std::ostream& out,
                                      cmListFileBacktrace const& bt) const
{
  // The title exists only if we have a call on top of the bottom.
  if (bt.Empty()) {
    return;
  }
  cmListFileContext lfc = bt.Top();
  if (this->TopSource) {
    lfc.FilePath =
      cmSystemTools::RelativeIfUnder(*this->TopSource, lfc.FilePath);
  }
  out << (lfc.Line ? " at " : " in ") << lfc;
}

void cmMessenger::SetTopSource(cm::optional<std::string> topSource)
{
  this->TopSource = std::move(topSource);
}
