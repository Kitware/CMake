/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestUploadCommand.h"

#include <set>
#include <sstream>
#include <utility>

#include <cm/memory>
#include <cm/vector>
#include <cmext/string_view>

#include "cmCTestGenericHandler.h"
#include "cmCTestUploadHandler.h"
#include "cmCommand.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSystemTools.h"

std::unique_ptr<cmCommand> cmCTestUploadCommand::Clone()
{
  auto ni = cm::make_unique<cmCTestUploadCommand>();
  ni->CTest = this->CTest;
  return std::unique_ptr<cmCommand>(std::move(ni));
}

void cmCTestUploadCommand::BindArguments()
{
  this->Bind("FILES"_s, this->Files);
  this->Bind("QUIET"_s, this->Quiet);
  this->Bind("CAPTURE_CMAKE_ERROR"_s, this->CaptureCMakeError);
}

void cmCTestUploadCommand::CheckArguments()
{
  cm::erase_if(this->Files, [this](std::string const& arg) -> bool {
    if (!cmSystemTools::FileExists(arg)) {
      std::ostringstream e;
      e << "File \"" << arg << "\" does not exist. Cannot submit "
        << "a non-existent file.";
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return true;
    }
    return false;
  });
}

std::unique_ptr<cmCTestGenericHandler>
cmCTestUploadCommand::InitializeHandler()
{
  auto handler = cm::make_unique<cmCTestUploadHandler>(this->CTest);
  handler->SetFiles(
    std::set<std::string>(this->Files.begin(), this->Files.end()));
  handler->SetQuiet(this->Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}
