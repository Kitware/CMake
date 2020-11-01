/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestStartCommand.h"

#include <cstddef>
#include <sstream>

#include "cmCTest.h"
#include "cmCTestVC.h"
#include "cmGeneratedFileStream.h"
#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmSystemTools.h"

class cmExecutionStatus;

cmCTestStartCommand::cmCTestStartCommand()
{
  this->CreateNewTag = true;
  this->Quiet = false;
}

bool cmCTestStartCommand::InitialPass(std::vector<std::string> const& args,
                                      cmExecutionStatus& /*unused*/)
{
  if (args.empty()) {
    this->SetError("called with incorrect number of arguments");
    return false;
  }

  size_t cnt = 0;
  const char* smodel = nullptr;
  const std::string* src_dir = nullptr;
  const std::string* bld_dir = nullptr;

  while (cnt < args.size()) {
    if (args[cnt] == "GROUP" || args[cnt] == "TRACK") {
      cnt++;
      if (cnt >= args.size() || args[cnt] == "APPEND" ||
          args[cnt] == "QUIET") {
        std::ostringstream e;
        e << args[cnt - 1] << " argument missing group name";
        this->SetError(e.str());
        return false;
      }
      this->CTest->SetSpecificGroup(args[cnt].c_str());
      cnt++;
    } else if (args[cnt] == "APPEND") {
      cnt++;
      this->CreateNewTag = false;
    } else if (args[cnt] == "QUIET") {
      cnt++;
      this->Quiet = true;
    } else if (!smodel) {
      smodel = args[cnt].c_str();
      cnt++;
    } else if (!src_dir) {
      src_dir = &args[cnt];
      cnt++;
    } else if (!bld_dir) {
      bld_dir = &args[cnt];
      cnt++;
    } else {
      this->SetError("Too many arguments");
      return false;
    }
  }

  if (!src_dir) {
    src_dir = this->Makefile->GetDefinition("CTEST_SOURCE_DIRECTORY");
  }
  if (!bld_dir) {
    bld_dir = this->Makefile->GetDefinition("CTEST_BINARY_DIRECTORY");
  }
  if (!src_dir) {
    this->SetError("source directory not specified. Specify source directory "
                   "as an argument or set CTEST_SOURCE_DIRECTORY");
    return false;
  }
  if (!bld_dir) {
    this->SetError("binary directory not specified. Specify binary directory "
                   "as an argument or set CTEST_BINARY_DIRECTORY");
    return false;
  }
  if (!smodel && this->CreateNewTag) {
    this->SetError("no test model specified and APPEND not specified. Specify "
                   "either a test model or the APPEND argument");
    return false;
  }

  cmSystemTools::AddKeepPath(*src_dir);
  cmSystemTools::AddKeepPath(*bld_dir);

  this->CTest->EmptyCTestConfiguration();

  std::string sourceDir = cmSystemTools::CollapseFullPath(*src_dir);
  std::string binaryDir = cmSystemTools::CollapseFullPath(*bld_dir);
  this->CTest->SetCTestConfiguration("SourceDirectory", sourceDir,
                                     this->Quiet);
  this->CTest->SetCTestConfiguration("BuildDirectory", binaryDir, this->Quiet);

  if (smodel) {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "Run dashboard with model "
                         << smodel << std::endl
                         << "   Source directory: " << *src_dir << std::endl
                         << "   Build directory: " << *bld_dir << std::endl,
                       this->Quiet);
  } else {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "Run dashboard with "
                       "to-be-determined model"
                         << std::endl
                         << "   Source directory: " << *src_dir << std::endl
                         << "   Build directory: " << *bld_dir << std::endl,
                       this->Quiet);
  }
  const char* group = this->CTest->GetSpecificGroup();
  if (group) {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "   Group: " << group << std::endl, this->Quiet);
  }

  // Log startup actions.
  std::string startLogFile = binaryDir + "/Testing/Temporary/LastStart.log";
  cmGeneratedFileStream ofs(startLogFile);
  if (!ofs) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot create log file: LastStart.log" << std::endl);
    return false;
  }

  // Make sure the source directory exists.
  if (!this->InitialCheckout(ofs, sourceDir)) {
    return false;
  }
  if (!cmSystemTools::FileIsDirectory(sourceDir)) {
    std::ostringstream e;
    e << "given source path\n"
      << "  " << sourceDir << "\n"
      << "which is not an existing directory.  "
      << "Set CTEST_CHECKOUT_COMMAND to a command line to create it.";
    this->SetError(e.str());
    return false;
  }

  this->CTest->SetRunCurrentScript(false);
  this->CTest->SetSuppressUpdatingCTestConfiguration(true);
  int model;
  if (smodel) {
    model = cmCTest::GetTestModelFromString(smodel);
  } else {
    model = cmCTest::UNKNOWN;
  }
  this->CTest->SetTestModel(model);
  this->CTest->SetProduceXML(true);

  return this->CTest->InitializeFromCommand(this);
}

bool cmCTestStartCommand::InitialCheckout(std::ostream& ofs,
                                          std::string const& sourceDir)
{
  // Use the user-provided command to create the source tree.
  cmProp initialCheckoutCommand =
    this->Makefile->GetDefinition("CTEST_CHECKOUT_COMMAND");
  if (!initialCheckoutCommand) {
    initialCheckoutCommand =
      this->Makefile->GetDefinition("CTEST_CVS_CHECKOUT");
  }
  if (initialCheckoutCommand) {
    // Use a generic VC object to run and log the command.
    cmCTestVC vc(this->CTest, ofs);
    vc.SetSourceDirectory(sourceDir);
    if (!vc.InitialCheckout(*initialCheckoutCommand)) {
      return false;
    }
  }
  return true;
}
