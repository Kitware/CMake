/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestUploadCommand.h"

#include <set>
#include <sstream>
#include <utility>

#include <cm/memory>
#include <cm/vector>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmCTestGenericHandler.h"
#include "cmCTestUploadHandler.h"
#include "cmCommand.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSystemTools.h"

class cmExecutionStatus;

std::unique_ptr<cmCommand> cmCTestUploadCommand::Clone()
{
  auto ni = cm::make_unique<cmCTestUploadCommand>();
  ni->CTest = this->CTest;
  return std::unique_ptr<cmCommand>(std::move(ni));
}

void cmCTestUploadCommand::CheckArguments(HandlerArguments& arguments)
{
  auto& args = static_cast<UploadArguments&>(arguments);
  cm::erase_if(args.Files, [this](std::string const& arg) -> bool {
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

std::unique_ptr<cmCTestGenericHandler> cmCTestUploadCommand::InitializeHandler(
  HandlerArguments& arguments)
{
  auto const& args = static_cast<UploadArguments&>(arguments);
  auto handler = cm::make_unique<cmCTestUploadHandler>(this->CTest);
  handler->SetFiles(
    std::set<std::string>(args.Files.begin(), args.Files.end()));
  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}

bool cmCTestUploadCommand::InitialPass(std::vector<std::string> const& args,
                                       cmExecutionStatus& status)
{
  static auto const parser =
    cmArgumentParser<UploadArguments>{ MakeHandlerParser<UploadArguments>() }
      .Bind("FILES"_s, &UploadArguments::Files)
      .Bind("QUIET"_s, &UploadArguments::Quiet);

  return this->Invoke(parser, args, status, [&](UploadArguments& a) {
    return this->ExecuteHandlerCommand(a, status);
  });
}
