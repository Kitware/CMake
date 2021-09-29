/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

class cmExecutionStatus;
class cmMakefile;
struct cmListFileArgument;

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
   * Construct the command. By default it has no makefile.
   */
  cmCommand() = default;

  /**
   * Need virtual destructor to destroy real command type.
   */
  virtual ~cmCommand() = default;

  cmCommand(cmCommand const&) = delete;
  cmCommand& operator=(cmCommand const&) = delete;

  /**
   * Specify the makefile.
   */
  cmMakefile* GetMakefile() { return this->Makefile; }

  void SetExecutionStatus(cmExecutionStatus* s);
  cmExecutionStatus* GetExecutionStatus() { return this->Status; }

  /**
   * This is called by the cmMakefile when the command is first
   * encountered in the CMakeLists.txt file.  It expands the command's
   * arguments and then invokes the InitialPass.
   */
  bool InvokeInitialPass(const std::vector<cmListFileArgument>& args,
                         cmExecutionStatus& status);

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus&) = 0;

  /**
   * This is a virtual constructor for the command.
   */
  virtual std::unique_ptr<cmCommand> Clone() = 0;

  /**
   * Set the error message
   */
  void SetError(const std::string& e);

protected:
  cmMakefile* Makefile = nullptr;

private:
  cmExecutionStatus* Status = nullptr;
};

class cmLegacyCommandWrapper
{
public:
  explicit cmLegacyCommandWrapper(std::unique_ptr<cmCommand> cmd);

  cmLegacyCommandWrapper(cmLegacyCommandWrapper const& other);
  cmLegacyCommandWrapper& operator=(cmLegacyCommandWrapper const& other);

  cmLegacyCommandWrapper(cmLegacyCommandWrapper&&) = default;
  cmLegacyCommandWrapper& operator=(cmLegacyCommandWrapper&&) = default;

  bool operator()(std::vector<cmListFileArgument> const& args,
                  cmExecutionStatus& status) const;

private:
  std::unique_ptr<cmCommand> Command;
};
