/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmMessenger_h
#define cmMessenger_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmListFileCache.h"
#include "cmMessageType.h"

class cmMessenger
{
public:
  void IssueMessage(
    MessageType t, std::string const& text,
    cmListFileBacktrace const& backtrace = cmListFileBacktrace()) const;

  void DisplayMessage(MessageType t, std::string const& text,
                      cmListFileBacktrace const& backtrace) const;

  void SetSuppressDevWarnings(bool suppress)
  {
    this->SuppressDevWarnings = suppress;
  }
  void SetSuppressDeprecatedWarnings(bool suppress)
  {
    this->SuppressDeprecatedWarnings = suppress;
  }
  void SetDevWarningsAsErrors(bool error)
  {
    this->DevWarningsAsErrors = error;
  }
  void SetDeprecatedWarningsAsErrors(bool error)
  {
    this->DeprecatedWarningsAsErrors = error;
  }

  bool GetSuppressDevWarnings() const { return this->SuppressDevWarnings; }
  bool GetSuppressDeprecatedWarnings() const
  {
    return this->SuppressDeprecatedWarnings;
  }
  bool GetDevWarningsAsErrors() const { return this->DevWarningsAsErrors; }
  bool GetDeprecatedWarningsAsErrors() const
  {
    return this->DeprecatedWarningsAsErrors;
  }

private:
  bool IsMessageTypeVisible(MessageType t) const;
  MessageType ConvertMessageType(MessageType t) const;

  bool SuppressDevWarnings = false;
  bool SuppressDeprecatedWarnings = false;
  bool DevWarningsAsErrors = false;
  bool DeprecatedWarningsAsErrors = false;
};

#endif
