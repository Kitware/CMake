/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestUpdateCommand.h"

#include <utility>

#include <cm/memory>

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmCTestUpdateHandler.h"
#include "cmCommand.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"

std::unique_ptr<cmCommand> cmCTestUpdateCommand::Clone()
{
  auto ni = cm::make_unique<cmCTestUpdateCommand>();
  ni->CTest = this->CTest;
  return std::unique_ptr<cmCommand>(std::move(ni));
}

std::unique_ptr<cmCTestGenericHandler> cmCTestUpdateCommand::InitializeHandler(
  HandlerArguments& args)
{
  if (!args.Source.empty()) {
    this->CTest->SetCTestConfiguration(
      "SourceDirectory", cmSystemTools::CollapseFullPath(args.Source),
      args.Quiet);
  } else {
    this->CTest->SetCTestConfiguration(
      "SourceDirectory",
      cmSystemTools::CollapseFullPath(
        this->Makefile->GetSafeDefinition("CTEST_SOURCE_DIRECTORY")),
      args.Quiet);
  }
  std::string source_dir =
    this->CTest->GetCTestConfiguration("SourceDirectory");

  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "UpdateCommand", "CTEST_UPDATE_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "UpdateOptions", "CTEST_UPDATE_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "CVSCommand", "CTEST_CVS_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "CVSUpdateOptions", "CTEST_CVS_UPDATE_OPTIONS",
    args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "SVNCommand", "CTEST_SVN_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "SVNUpdateOptions", "CTEST_SVN_UPDATE_OPTIONS",
    args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "SVNOptions", "CTEST_SVN_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "BZRCommand", "CTEST_BZR_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "BZRUpdateOptions", "CTEST_BZR_UPDATE_OPTIONS",
    args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "GITCommand", "CTEST_GIT_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "GITUpdateOptions", "CTEST_GIT_UPDATE_OPTIONS",
    args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "GITInitSubmodules", "CTEST_GIT_INIT_SUBMODULES",
    args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "GITUpdateCustom", "CTEST_GIT_UPDATE_CUSTOM", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "UpdateVersionOnly", "CTEST_UPDATE_VERSION_ONLY",
    args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "UpdateVersionOverride", "CTEST_UPDATE_VERSION_OVERRIDE",
    args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "HGCommand", "CTEST_HG_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "HGUpdateOptions", "CTEST_HG_UPDATE_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "P4Command", "CTEST_P4_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "P4UpdateOptions", "CTEST_P4_UPDATE_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "P4Client", "CTEST_P4_CLIENT", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "P4Options", "CTEST_P4_OPTIONS", args.Quiet);

  auto handler = cm::make_unique<cmCTestUpdateHandler>(this->CTest);
  if (source_dir.empty()) {
    this->SetError("source directory not specified. Please use SOURCE tag");
    return nullptr;
  }
  handler->SourceDirectory = source_dir;
  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}

bool cmCTestUpdateCommand::InitialPass(std::vector<std::string> const& args,
                                       cmExecutionStatus& status)
{
  static auto const parser = MakeHandlerParser<HandlerArguments>();

  return this->Invoke(parser, args, status, [&](HandlerArguments& a) {
    return this->ExecuteHandlerCommand(a, status);
  });
}
