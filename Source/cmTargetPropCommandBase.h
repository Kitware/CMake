/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;
class cmMakefile;
class cmTarget;

class cmTargetPropCommandBase
{
public:
  cmTargetPropCommandBase(cmExecutionStatus& status);
  virtual ~cmTargetPropCommandBase() = default;

  void SetError(std::string const& e);

  enum ArgumentFlags
  {
    NO_FLAGS = 0x0,
    PROCESS_BEFORE = 0x1,
    PROCESS_AFTER = 0x2,
    PROCESS_SYSTEM = 0x4,
    PROCESS_REUSE_FROM = 0x8
  };

  bool HandleArguments(std::vector<std::string> const& args,
                       std::string const& prop, unsigned int flags = NO_FLAGS);

protected:
  std::string Property;
  cmTarget* Target = nullptr;
  cmMakefile* Makefile;

  virtual void HandleInterfaceContent(cmTarget* tgt,
                                      std::vector<std::string> const& content,
                                      bool prepend, bool system);
  virtual bool PopulateTargetProperties(
    std::string const& scope, std::vector<std::string> const& content,
    bool prepend, bool system);

private:
  virtual void HandleMissingTarget(std::string const& name) = 0;

  virtual bool HandleDirectContent(cmTarget* tgt,
                                   std::vector<std::string> const& content,
                                   bool prepend, bool system) = 0;

  virtual std::string Join(std::vector<std::string> const& content) = 0;

  bool ProcessContentArgs(std::vector<std::string> const& args,
                          unsigned int& argIndex, bool prepend, bool system);

  cmExecutionStatus& Status;
};
