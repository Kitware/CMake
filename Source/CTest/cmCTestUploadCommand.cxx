/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestUploadCommand.h"

#include <set>
#include <sstream>

#include <cm/vector>
#include <cmext/string_view>

#include "cmCTest.h"
#include "cmCTestUploadHandler.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSystemTools.h"

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

cmCTestGenericHandler* cmCTestUploadCommand::InitializeHandler()
{
  cmCTestUploadHandler* handler = this->CTest->GetUploadHandler();
  handler->Initialize();
  handler->SetFiles(
    std::set<std::string>(this->Files.begin(), this->Files.end()));
  handler->SetQuiet(this->Quiet);
  return handler;
}
