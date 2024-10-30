/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestUpdateCommand.h"

#include <utility>

#include <cm/memory>

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmCTestUpdateHandler.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"

std::unique_ptr<cmCTestGenericHandler> cmCTestUpdateCommand::InitializeHandler(
  HandlerArguments& args, cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();
  if (!args.Source.empty()) {
    this->CTest->SetCTestConfiguration(
      "SourceDirectory", cmSystemTools::CollapseFullPath(args.Source),
      args.Quiet);
  } else {
    this->CTest->SetCTestConfiguration(
      "SourceDirectory",
      cmSystemTools::CollapseFullPath(
        mf.GetSafeDefinition("CTEST_SOURCE_DIRECTORY")),
      args.Quiet);
  }
  std::string source_dir =
    this->CTest->GetCTestConfiguration("SourceDirectory");

  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "UpdateCommand", "CTEST_UPDATE_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "UpdateOptions", "CTEST_UPDATE_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "CVSCommand", "CTEST_CVS_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "CVSUpdateOptions", "CTEST_CVS_UPDATE_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "SVNCommand", "CTEST_SVN_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "SVNUpdateOptions", "CTEST_SVN_UPDATE_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "SVNOptions", "CTEST_SVN_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "BZRCommand", "CTEST_BZR_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "BZRUpdateOptions", "CTEST_BZR_UPDATE_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "GITCommand", "CTEST_GIT_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "GITUpdateOptions", "CTEST_GIT_UPDATE_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "GITInitSubmodules", "CTEST_GIT_INIT_SUBMODULES", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "GITUpdateCustom", "CTEST_GIT_UPDATE_CUSTOM", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "UpdateVersionOnly", "CTEST_UPDATE_VERSION_ONLY", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "UpdateVersionOverride", "CTEST_UPDATE_VERSION_OVERRIDE", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "HGCommand", "CTEST_HG_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "HGUpdateOptions", "CTEST_HG_UPDATE_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "P4Command", "CTEST_P4_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "P4UpdateOptions", "CTEST_P4_UPDATE_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "P4Client", "CTEST_P4_CLIENT", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "P4Options", "CTEST_P4_OPTIONS", args.Quiet);

  auto handler = cm::make_unique<cmCTestUpdateHandler>(this->CTest);
  if (source_dir.empty()) {
    status.SetError("source directory not specified. Please use SOURCE tag");
    return nullptr;
  }
  handler->SourceDirectory = source_dir;
  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}

bool cmCTestUpdateCommand::InitialPass(std::vector<std::string> const& args,
                                       cmExecutionStatus& status) const
{
  static auto const parser = MakeHandlerParser<HandlerArguments>();

  return this->Invoke(parser, args, status, [&](HandlerArguments& a) {
    return this->ExecuteHandlerCommand(a, status);
  });
}
