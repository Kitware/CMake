/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmMessenger.h"

#include <algorithm>
#include <array>
#include <sstream>
#include <utility>

#include <cmext/string_view>

#include "cmDocumentationFormatter.h"
#include "cmMessageMetadata.h"
#include "cmMessageType.h"
#include "cmStateSnapshot.h"
#include "cmStdIoTerminal.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include "cmsys/SystemInformation.hxx"

#  include "cmSarifLog.h"
#endif

#ifdef CMake_ENABLE_DEBUGGER
#  include "cmDebuggerAdapter.h"
#endif

namespace {
char const* getMessageTypeStr(MessageType t)
{
  switch (t) {
    case MessageType::FATAL_ERROR:
      return "Error";
    case MessageType::INTERNAL_ERROR:
      return "Internal Error (please report a bug)";
    case MessageType::LOG:
      return "Debug Log";
    default:
      break;
  }
  return "Warning";
}

std::string getDiagnosticCategoryStr(cmDiagnosticCategory category)
{
  std::string out = cmSystemTools::LowerCase(
    cmDiagnostics::GetCategoryString(category).substr(4));
  std::replace(out.begin(), out.end(), '_', '-');
  return out;
}

cm::StdIo::TermAttr getMessageColor(MessageType t)
{
  switch (t) {
    case MessageType::INTERNAL_ERROR:
    case MessageType::FATAL_ERROR:
      return cm::StdIo::TermAttr::ForegroundRed;
    case MessageType::WARNING:
      return cm::StdIo::TermAttr::ForegroundYellow;
    default:
      return cm::StdIo::TermAttr::Normal;
  }
}

bool isAuthorDiagnostic(cmDiagnosticCategory category)
{
  while (category != cmDiagnostics::CMD_NONE) {
    if (category == cmDiagnostics::CMD_AUTHOR) {
      return true;
    }
    category = cmDiagnostics::CategoryInfo[category].Parent;
  }
  return false;
}

void printMessageText(std::ostream& msg, std::string const& text)
{
  msg << ":\n";
  cmDocumentationFormatter formatter;
  formatter.SetIndent(2u);
  formatter.PrintFormatted(msg, text);
}

void displayMessage(MessageType type, cmDiagnosticCategory category,
                    std::ostringstream& msg)
{
  if (isAuthorDiagnostic(category)) {
    // Add a note about warning suppression.
    std::ostringstream text;
    if (type == MessageType::WARNING) {
      text << "This warning is for project developers.  "
              "Use -Wno-author";
      if (category != cmDiagnostics::CMD_AUTHOR) {
        text << " or -Wno-" << getDiagnosticCategoryStr(category);
      }
    } else if (type == MessageType::FATAL_ERROR) {
      text << "This error is for project developers.  "
              "Use -Wno-error=author";
      if (category != cmDiagnostics::CMD_AUTHOR) {
        text << " or -Wno-error=" << getDiagnosticCategoryStr(category);
      }
    }
    text << " to suppress it.";

    cmDocumentationFormatter formatter;
    formatter.PrintFormatted(msg, text.str());
  } else {
    // Add a terminating blank line.
    msg << '\n';
  }

#if !defined(CMAKE_BOOTSTRAP)
  // Add a C++ stack trace to internal errors.
  if (type == MessageType::INTERNAL_ERROR) {
    std::string stack = cmsys::SystemInformation::GetProgramStack(0, 0);
    if (!stack.empty()) {
      if (cmHasLiteralPrefix(stack, "WARNING:")) {
        stack = "Note:" + stack.substr(8);
      }
      msg << stack << '\n';
    }
  }
#endif

  // Output the message.
  cmMessageMetadata md;
  md.attrs = getMessageColor(type);
  if (type == MessageType::FATAL_ERROR ||
      type == MessageType::INTERNAL_ERROR) {
    cmSystemTools::SetErrorOccurred();
    md.title = "Error";
  } else {
    md.title = "Warning";
  }
  cmSystemTools::Message(msg.str(), md);
}

void PrintCallStack(std::ostream& out, cmListFileBacktrace bt,
                    cm::optional<std::string> const& topSource)
{
  // The call stack exists only if we have at least two calls on top
  // of the bottom.
  if (bt.Empty()) {
    return;
  }
  std::string lastFilePath = bt.Top().FilePath;
  bt = bt.Pop();
  if (bt.Empty()) {
    return;
  }

  bool first = true;
  for (; !bt.Empty(); bt = bt.Pop()) {
    cmListFileContext lfc = bt.Top();
    if (lfc.Name.empty() &&
        lfc.Line != cmListFileContext::DeferPlaceholderLine &&
        lfc.FilePath == lastFilePath) {
      // An entry with no function name is frequently preceded (in the stack)
      // by a more specific entry.  When this happens (as verified by the
      // preceding entry referencing the same file path), skip the less
      // specific entry, as we have already printed the more specific one.
      continue;
    }
    if (first) {
      first = false;
      out << "Call Stack (most recent call first):\n";
    }
    lastFilePath = lfc.FilePath;
    if (topSource) {
      lfc.FilePath = cmSystemTools::RelativeIfUnder(*topSource, lfc.FilePath);
    }
    out << "  " << lfc << '\n';
  }
}

} // anonymous namespace

void cmMessenger::IssueMessage(MessageType t, std::string const& text,
                               cmListFileBacktrace const& backtrace) const
{
  this->DisplayMessage(t, cmDiagnostics::CMD_NONE, text, backtrace);
}

void cmMessenger::IssueDiagnostic(cmDiagnosticCategory category,
                                  std::string const& text,
                                  cmStateSnapshot const& fallbackContext,
                                  cmDiagnosticContext const& context) const
{
  cmDiagnosticAction const action = [&] {
    if (context.HasState) {
      cmDiagnosticAction const ca = context.DiagnosticState[category];
      if (ca != cmDiagnostics::Undefined) {
        return ca;
      }

      // If the context has recorded states, but not the state we want, this
      // implies that we had the opportunity to record the state and failed to
      // do so. Ask users to report this.
      std::string msg =
        cmStrCat("Stored diagnostic context did not record state for "_s,
                 cmDiagnostics::GetCategoryString(category),
                 ".  Please report this as a bug.\n"_s);
      this->IssueMessage(MessageType::LOG, msg, context.GetBacktrace());
    }

    return fallbackContext.GetDiagnostic(category);
  }();

  switch (action) {
    case cmDiagnostics::FatalError:
      cmSystemTools::SetFatalErrorOccurred();
      CM_FALLTHROUGH;
    case cmDiagnostics::SendError:
      cmSystemTools::SetErrorOccurred();
      this->DisplayMessage(MessageType::FATAL_ERROR, category, text,
                           context.GetBacktrace());
      break;
    case cmDiagnostics::Warn:
      this->DisplayMessage(MessageType::WARNING, category, text,
                           context.GetBacktrace());
      break;
    default:
      return;
  }
}

void cmMessenger::DisplayMessage(MessageType type,
                                 cmDiagnosticCategory category,
                                 std::string const& text,
                                 cmListFileBacktrace const& backtrace) const
{
  std::ostringstream msg;

  // Print the message preamble.
  msg << "CMake " << getMessageTypeStr(type);
  if (category != cmDiagnostics::CMD_NONE) {
    msg << " (" << getDiagnosticCategoryStr(category) << ')';
  }

  // Add the immediate context.
  this->PrintBacktraceTitle(msg, backtrace);

  printMessageText(msg, text);

  // Add the rest of the context.
  PrintCallStack(msg, backtrace, this->TopSource);

  displayMessage(type, category, msg);

#ifndef CMAKE_BOOTSTRAP
  // Add message to SARIF logs
  this->SarifLog.LogMessage(type, text, backtrace);
#endif

#ifdef CMake_ENABLE_DEBUGGER
  if (DebuggerAdapter) {
    DebuggerAdapter->OnMessageOutput(type, msg.str());
  }
#endif
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
