/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmExecutionStatus_h
#define cmExecutionStatus_h

#include <cmConfigure.h> // IWYU pragma: keep

#include <string>

class cmMakefile;

/** \class cmExecutionStatus
 * \brief Superclass for all command status classes
 *
 * when a command is involked it may set values on a command status instance
 */
class cmExecutionStatus
{
public:
  cmExecutionStatus(cmMakefile& makefile)
    : Makefile(makefile)
    , Error("unknown error.")
  {
  }

  cmMakefile& GetMakefile() { return this->Makefile; }

  void SetError(std::string const& e) { this->Error = e; }
  std::string const& GetError() const { return this->Error; }

  void SetReturnInvoked() { this->ReturnInvoked = true; }
  bool GetReturnInvoked() const { return this->ReturnInvoked; }

  void SetBreakInvoked() { this->BreakInvoked = true; }
  bool GetBreakInvoked() const { return this->BreakInvoked; }

  void SetContinueInvoked() { this->ContinueInvoked = true; }
  bool GetContinueInvoked() const { return this->ContinueInvoked; }

  void SetNestedError() { this->NestedError = true; }
  bool GetNestedError() const { return this->NestedError; }

private:
  cmMakefile& Makefile;
  std::string Error;
  bool ReturnInvoked = false;
  bool BreakInvoked = false;
  bool ContinueInvoked = false;
  bool NestedError = false;
};

#endif
