/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestStartCommand.h"

#include <cstddef>
#include <sstream>

#include "cmCTest.h"
#include "cmCTestVC.h"
#include "cmExecutionStatus.h"
#include "cmGeneratedFileStream.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

bool cmCTestStartCommand::InitialPass(std::vector<std::string> const& args,
                                      cmExecutionStatus& status) const
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  size_t cnt = 0;
  bool append = false;
  bool quiet = false;
  char const* smodel = nullptr;
  cmValue src_dir;
  cmValue bld_dir;

  while (cnt < args.size()) {
    if (args[cnt] == "GROUP" || args[cnt] == "TRACK") {
      cnt++;
      if (cnt >= args.size() || args[cnt] == "APPEND" ||
          args[cnt] == "QUIET") {
        std::ostringstream e;
        e << args[cnt - 1] << " argument missing group name";
        status.SetError(e.str());
        return false;
      }
      this->CTest->SetSpecificGroup(args[cnt].c_str());
      cnt++;
    } else if (args[cnt] == "APPEND") {
      cnt++;
      append = true;
    } else if (args[cnt] == "QUIET") {
      cnt++;
      quiet = true;
    } else if (!smodel) {
      smodel = args[cnt].c_str();
      cnt++;
    } else if (!src_dir) {
      src_dir = cmValue(args[cnt]);
      cnt++;
    } else if (!bld_dir) {
      bld_dir = cmValue(args[cnt]);
      cnt++;
    } else {
      status.SetError("Too many arguments");
      return false;
    }
  }

  cmMakefile& mf = status.GetMakefile();

  if (!src_dir) {
    src_dir = mf.GetDefinition("CTEST_SOURCE_DIRECTORY");
  }
  if (!bld_dir) {
    bld_dir = mf.GetDefinition("CTEST_BINARY_DIRECTORY");
  }
  if (!src_dir) {
    status.SetError("source directory not specified. Specify source directory "
                    "as an argument or set CTEST_SOURCE_DIRECTORY");
    return false;
  }
  if (!bld_dir) {
    status.SetError("binary directory not specified. Specify binary directory "
                    "as an argument or set CTEST_BINARY_DIRECTORY");
    return false;
  }
  if (!smodel && !append) {
    status.SetError(
      "no test model specified and APPEND not specified. Specify "
      "either a test model or the APPEND argument");
    return false;
  }

  this->CTest->EmptyCTestConfiguration();

  std::string sourceDir = cmSystemTools::CollapseFullPath(*src_dir);
  std::string binaryDir = cmSystemTools::CollapseFullPath(*bld_dir);
  this->CTest->SetCTestConfiguration("SourceDirectory", sourceDir, quiet);
  this->CTest->SetCTestConfiguration("BuildDirectory", binaryDir, quiet);

  if (smodel) {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "Run dashboard with model "
                         << smodel << std::endl
                         << "   Source directory: " << *src_dir << std::endl
                         << "   Build directory: " << *bld_dir << std::endl,
                       quiet);
  } else {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "Run dashboard with "
                       "to-be-determined model"
                         << std::endl
                         << "   Source directory: " << *src_dir << std::endl
                         << "   Build directory: " << *bld_dir << std::endl,
                       quiet);
  }
  char const* group = this->CTest->GetSpecificGroup();
  if (group) {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "   Group: " << group << std::endl, quiet);
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
  if (!this->InitialCheckout(ofs, sourceDir, status)) {
    return false;
  }
  if (!cmSystemTools::FileIsDirectory(sourceDir)) {
    std::ostringstream e;
    e << "given source path\n"
      << "  " << sourceDir << "\n"
      << "which is not an existing directory.  "
      << "Set CTEST_CHECKOUT_COMMAND to a command line to create it.";
    status.SetError(e.str());
    return false;
  }

  int model;
  if (smodel) {
    model = cmCTest::GetTestModelFromString(smodel);
  } else {
    model = cmCTest::UNKNOWN;
  }
  this->CTest->SetTestModel(model);
  this->CTest->SetProduceXML(true);

  std::string fname;

  std::string src_dir_fname = cmStrCat(sourceDir, "/CTestConfig.cmake");
  cmSystemTools::ConvertToUnixSlashes(src_dir_fname);

  std::string bld_dir_fname = cmStrCat(binaryDir, "/CTestConfig.cmake");
  cmSystemTools::ConvertToUnixSlashes(bld_dir_fname);

  if (cmSystemTools::FileExists(bld_dir_fname)) {
    fname = bld_dir_fname;
  } else if (cmSystemTools::FileExists(src_dir_fname)) {
    fname = src_dir_fname;
  }

  if (!fname.empty()) {
    cmCTestOptionalLog(
      this->CTest, OUTPUT,
      "   Reading ctest configuration file: " << fname << std::endl, quiet);
    bool readit = mf.ReadDependentFile(fname);
    if (!readit) {
      std::string m = cmStrCat("Could not find include file: ", fname);
      status.SetError(m);
      return false;
    }
  }

  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "NightlyStartTime", "CTEST_NIGHTLY_START_TIME", quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(&mf, "Site",
                                                      "CTEST_SITE", quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "BuildName", "CTEST_BUILD_NAME", quiet);

  this->CTest->Initialize(bld_dir);

  cmCTestOptionalLog(
    this->CTest, OUTPUT,
    "   Site: " << this->CTest->GetCTestConfiguration("Site") << std::endl
                << "   Build name: "
                << cmCTest::SafeBuildIdField(
                     this->CTest->GetCTestConfiguration("BuildName"))
                << std::endl,
    quiet);

  if (this->CTest->GetTestModel() == cmCTest::NIGHTLY &&
      this->CTest->GetCTestConfiguration("NightlyStartTime").empty()) {
    cmCTestOptionalLog(
      this->CTest, WARNING,
      "WARNING: No nightly start time found please set in CTestConfig.cmake"
      " or DartConfig.cmake"
        << std::endl,
      quiet);
    return false;
  }

  this->CTest->ReadCustomConfigurationFileTree(bld_dir, &mf);

  if (append) {
    if (!this->CTest->ReadExistingTag(quiet)) {
      return false;
    }
  } else {
    if (!this->CTest->CreateNewTag(quiet)) {
      return false;
    }
  }

  cmCTestOptionalLog(this->CTest, OUTPUT,
                     "   Use " << this->CTest->GetTestGroupString() << " tag: "
                               << this->CTest->GetCurrentTag() << std::endl,
                     quiet);
  return true;
}

bool cmCTestStartCommand::InitialCheckout(std::ostream& ofs,
                                          std::string const& sourceDir,
                                          cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();
  // Use the user-provided command to create the source tree.
  cmValue initialCheckoutCommand = mf.GetDefinition("CTEST_CHECKOUT_COMMAND");
  if (!initialCheckoutCommand) {
    initialCheckoutCommand = mf.GetDefinition("CTEST_CVS_CHECKOUT");
  }
  if (initialCheckoutCommand) {
    // Use a generic VC object to run and log the command.
    cmCTestVC vc(this->CTest, &mf, ofs);
    vc.SetSourceDirectory(sourceDir);
    if (!vc.InitialCheckout(*initialCheckoutCommand)) {
      return false;
    }
  }
  return true;
}
