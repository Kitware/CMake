/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmExecutionStatus_h
#define cmExecutionStatus_h

/** \class cmExecutionStatus
 * \brief Superclass for all command status classes
 *
 * when a command is involked it may set values on a command status instance
 */
class cmExecutionStatus
{
public:
  cmExecutionStatus() { this->Clear(); }

  void SetReturnInvoked(bool val) { this->ReturnInvoked = val; }
  bool GetReturnInvoked() { return this->ReturnInvoked; }

  void SetBreakInvoked(bool val) { this->BreakInvoked = val; }
  bool GetBreakInvoked() { return this->BreakInvoked; }

  void SetContinueInvoked(bool val) { this->ContinueInvoked = val; }
  bool GetContinueInvoked() { return this->ContinueInvoked; }

  void Clear()
  {
    this->ReturnInvoked = false;
    this->BreakInvoked = false;
    this->ContinueInvoked = false;
    this->NestedError = false;
  }
  void SetNestedError(bool val) { this->NestedError = val; }
  bool GetNestedError() { return this->NestedError; }

private:
  bool ReturnInvoked;
  bool BreakInvoked;
  bool ContinueInvoked;
  bool NestedError;
};

#endif
