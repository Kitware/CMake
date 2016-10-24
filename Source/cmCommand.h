/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCommand_h
#define cmCommand_h

#include "cmObject.h"

#include "cmCommandArgumentsHelper.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"

/** \class cmCommand
 * \brief Superclass for all commands in CMake.
 *
 * cmCommand is the base class for all commands in CMake. A command
 * manifests as an entry in CMakeLists.txt and produces one or
 * more makefile rules. Commands are associated with a particular
 * makefile. This base class cmCommand defines the API for commands
 * to support such features as enable/disable, inheritance,
 * documentation, and construction.
 */
class cmCommand
{
public:
  /**
   * Construct the command. By default it is enabled with no makefile.
   */
  cmCommand()
    : Makefile(CM_NULLPTR)
    , Enabled(true)
  {
  }

  /**
   * Need virtual destructor to destroy real command type.
   */
  virtual ~cmCommand() {}

  /**
   * Specify the makefile.
   */
  void SetMakefile(cmMakefile* m) { this->Makefile = m; }
  cmMakefile* GetMakefile() { return this->Makefile; }

  /**
   * This is called by the cmMakefile when the command is first
   * encountered in the CMakeLists.txt file.  It expands the command's
   * arguments and then invokes the InitialPass.
   */
  virtual bool InvokeInitialPass(const std::vector<cmListFileArgument>& args,
                                 cmExecutionStatus& status);

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus&) = 0;

  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass() {}

  /**
   * Does this command have a final pass?  Query after InitialPass.
   */
  virtual bool HasFinalPass() const { return false; }

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() = 0;

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return false; }

  /**
   * This determines if the command is defined in a cmake script.
   * It is the case for cmMacroHelperCommand and cmFunctionHelperCommand.
   */
  virtual bool IsUserDefined() const { return false; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const = 0;

  /**
   * Enable the command.
   */
  void EnabledOn() { this->Enabled = true; }

  /**
   * Disable the command.
   */
  void EnabledOff() { this->Enabled = false; }

  /**
   * Query whether the command is enabled.
   */
  bool GetEnabled() const { return this->Enabled; }

  /**
   * Disable or enable the command.
   */
  void SetEnabled(bool enabled) { this->Enabled = enabled; }

  /**
   * Return the last error string.
   */
  const char* GetError();

  /**
   * Set the error message
   */
  void SetError(const std::string& e);

  /** Check if the command is disallowed by a policy.  */
  bool Disallowed(cmPolicies::PolicyID pol, const char* e);

protected:
  cmMakefile* Makefile;
  cmCommandArgumentsHelper Helper;

private:
  bool Enabled;
  std::string Error;
};

#endif
