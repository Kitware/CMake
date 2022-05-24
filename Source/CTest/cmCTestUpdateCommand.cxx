/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestUpdateCommand.h"

#include "cmCTest.h"
#include "cmCTestUpdateHandler.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"

cmCTestGenericHandler* cmCTestUpdateCommand::InitializeHandler()
{
  if (!this->Source.empty()) {
    this->CTest->SetCTestConfiguration(
      "SourceDirectory", cmSystemTools::CollapseFullPath(this->Source),
      this->Quiet);
  } else {
    this->CTest->SetCTestConfiguration(
      "SourceDirectory",
      cmSystemTools::CollapseFullPath(
        this->Makefile->GetSafeDefinition("CTEST_SOURCE_DIRECTORY")),
      this->Quiet);
  }
  std::string source_dir =
    this->CTest->GetCTestConfiguration("SourceDirectory");

  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "UpdateCommand", "CTEST_UPDATE_COMMAND", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "UpdateOptions", "CTEST_UPDATE_OPTIONS", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "CVSCommand", "CTEST_CVS_COMMAND", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "CVSUpdateOptions", "CTEST_CVS_UPDATE_OPTIONS",
    this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "SVNCommand", "CTEST_SVN_COMMAND", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "SVNUpdateOptions", "CTEST_SVN_UPDATE_OPTIONS",
    this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "SVNOptions", "CTEST_SVN_OPTIONS", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "BZRCommand", "CTEST_BZR_COMMAND", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "BZRUpdateOptions", "CTEST_BZR_UPDATE_OPTIONS",
    this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "GITCommand", "CTEST_GIT_COMMAND", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "GITUpdateOptions", "CTEST_GIT_UPDATE_OPTIONS",
    this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "GITInitSubmodules", "CTEST_GIT_INIT_SUBMODULES",
    this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "GITUpdateCustom", "CTEST_GIT_UPDATE_CUSTOM", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "UpdateVersionOnly", "CTEST_UPDATE_VERSION_ONLY",
    this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "UpdateVersionOverride", "CTEST_UPDATE_VERSION_OVERRIDE",
    this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "HGCommand", "CTEST_HG_COMMAND", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "HGUpdateOptions", "CTEST_HG_UPDATE_OPTIONS", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "P4Command", "CTEST_P4_COMMAND", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "P4UpdateOptions", "CTEST_P4_UPDATE_OPTIONS", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "P4Client", "CTEST_P4_CLIENT", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "P4Options", "CTEST_P4_OPTIONS", this->Quiet);

  cmCTestUpdateHandler* handler = this->CTest->GetUpdateHandler();
  handler->Initialize();
  if (source_dir.empty()) {
    this->SetError("source directory not specified. Please use SOURCE tag");
    return nullptr;
  }
  handler->SetOption("SourceDirectory", source_dir);
  handler->SetQuiet(this->Quiet);
  return handler;
}
