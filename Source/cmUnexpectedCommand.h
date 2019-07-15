/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmUnexpectedCommand_h
#define cmUnexpectedCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <utility>
#include <vector>

#include "cm_memory.hxx"

#include "cmCommand.h"

class cmExecutionStatus;

class cmUnexpectedCommand : public cmCommand
{
public:
  cmUnexpectedCommand(std::string name, const char* error)
    : Name(std::move(name))
    , Error(error)
  {
  }

  std::unique_ptr<cmCommand> Clone() override
  {
    return cm::make_unique<cmUnexpectedCommand>(this->Name, this->Error);
  }

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;

private:
  std::string Name;
  const char* Error;
};

#endif
