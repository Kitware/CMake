/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMessageCommand.h"

#include <cassert>
#include <utility>

#include <cm/string_view>

#include "cm_static_string_view.hxx"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmMessenger.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

namespace {

enum class CheckingType
{
  UNDEFINED,
  CHECK_START,
  CHECK_PASS,
  CHECK_FAIL
};

std::string IndentText(std::string text, cmMakefile& mf)
{
  auto indent =
    cmJoin(cmExpandedList(mf.GetSafeDefinition("CMAKE_MESSAGE_INDENT")), "");

  const auto showContext = mf.GetCMakeInstance()->GetShowLogContext() ||
    mf.IsOn("CMAKE_MESSAGE_CONTEXT_SHOW");
  if (showContext) {
    auto context = cmJoin(
      cmExpandedList(mf.GetSafeDefinition("CMAKE_MESSAGE_CONTEXT")), ".");
    if (!context.empty()) {
      indent.insert(0u, cmStrCat("["_s, context, "] "_s));
    }
  }

  if (!indent.empty()) {
    cmSystemTools::ReplaceString(text, "\n", "\n" + indent);
    text.insert(0u, indent);
  }
  return text;
}

void ReportCheckResult(cm::string_view what, std::string result,
                       cmMakefile& mf)
{
  if (mf.GetCMakeInstance()->HasCheckInProgress()) {
    auto text = mf.GetCMakeInstance()->GetTopCheckInProgressMessage() + " - " +
      std::move(result);
    mf.DisplayStatus(IndentText(std::move(text), mf), -1);
  } else {
    mf.GetMessenger()->DisplayMessage(
      MessageType::AUTHOR_WARNING,
      cmStrCat("Ignored "_s, what, " without CHECK_START"_s),
      mf.GetBacktrace());
  }
}

} // anonymous namespace

// cmLibraryCommand
bool cmMessageCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  auto& mf = status.GetMakefile();

  auto i = args.cbegin();

  auto type = MessageType::MESSAGE;
  auto fatal = false;
  auto level = cmake::LogLevel::LOG_UNDEFINED;
  auto checkingType = CheckingType::UNDEFINED;
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
    if (mf.IsSet("CMAKE_SUPPRESS_DEVELOPER_ERRORS") &&
        !mf.IsOn("CMAKE_SUPPRESS_DEVELOPER_ERRORS")) {
      fatal = true;
      type = MessageType::AUTHOR_ERROR;
      level = cmake::LogLevel::LOG_ERROR;
    } else if (!mf.IsOn("CMAKE_SUPPRESS_DEVELOPER_WARNINGS")) {
      type = MessageType::AUTHOR_WARNING;
      level = cmake::LogLevel::LOG_WARNING;
    } else {
      return true;
    }
    ++i;
  } else if (*i == "CHECK_START") {
    level = cmake::LogLevel::LOG_STATUS;
    checkingType = CheckingType::CHECK_START;
    ++i;
  } else if (*i == "CHECK_PASS") {
    level = cmake::LogLevel::LOG_STATUS;
    checkingType = CheckingType::CHECK_PASS;
    ++i;
  } else if (*i == "CHECK_FAIL") {
    level = cmake::LogLevel::LOG_STATUS;
    checkingType = CheckingType::CHECK_FAIL;
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
    if (mf.IsOn("CMAKE_ERROR_DEPRECATED")) {
      fatal = true;
      type = MessageType::DEPRECATION_ERROR;
      level = cmake::LogLevel::LOG_ERROR;
    } else if (!mf.IsSet("CMAKE_WARN_DEPRECATED") ||
               mf.IsOn("CMAKE_WARN_DEPRECATED")) {
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

  auto desiredLevel = mf.GetCMakeInstance()->GetLogLevel();
  assert("Expected a valid log level here" &&
         desiredLevel != cmake::LogLevel::LOG_UNDEFINED);

  // Command line option takes precedence over the cache variable
  if (!mf.GetCMakeInstance()->WasLogLevelSetViaCLI()) {
    const auto desiredLevelFromCache =
      cmake::StringToLogLevel(mf.GetSafeDefinition("CMAKE_MESSAGE_LOG_LEVEL"));
    if (desiredLevelFromCache != cmake::LogLevel::LOG_UNDEFINED) {
      desiredLevel = desiredLevelFromCache;
    }
  }

  if (desiredLevel < level) {
    // Suppress the message
    return true;
  }

  auto message = cmJoin(cmMakeRange(i, args.cend()), "");

  switch (level) {
    case cmake::LogLevel::LOG_ERROR:
    case cmake::LogLevel::LOG_WARNING:
      // we've overridden the message type, above, so display it directly
      mf.GetMessenger()->DisplayMessage(type, message, mf.GetBacktrace());
      break;

    case cmake::LogLevel::LOG_NOTICE:
      cmSystemTools::Message(IndentText(message, mf));
      break;

    case cmake::LogLevel::LOG_STATUS:
      switch (checkingType) {
        case CheckingType::CHECK_START:
          mf.DisplayStatus(IndentText(message, mf), -1);
          mf.GetCMakeInstance()->PushCheckInProgressMessage(message);
          break;

        case CheckingType::CHECK_PASS:
          ReportCheckResult("CHECK_PASS"_s, message, mf);
          break;

        case CheckingType::CHECK_FAIL:
          ReportCheckResult("CHECK_FAIL"_s, message, mf);
          break;

        default:
          mf.DisplayStatus(IndentText(message, mf), -1);
          break;
      }
      break;

    case cmake::LogLevel::LOG_VERBOSE:
    case cmake::LogLevel::LOG_DEBUG:
    case cmake::LogLevel::LOG_TRACE:
      mf.DisplayStatus(IndentText(message, mf), -1);
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
