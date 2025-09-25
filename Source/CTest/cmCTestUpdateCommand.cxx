/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestUpdateCommand.h"

#include <chrono>
#include <sstream>
#include <string>
#include <vector>

#include <cm/memory>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmCLocaleEnvironmentScope.h"
#include "cmCTest.h"
#include "cmCTestBZR.h"
#include "cmCTestCVS.h"
#include "cmCTestGIT.h"
#include "cmCTestHG.h"
#include "cmCTestP4.h"
#include "cmCTestSVN.h"
#include "cmCTestVC.h"
#include "cmExecutionStatus.h"
#include "cmGeneratedFileStream.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmVersion.h"
#include "cmXMLWriter.h"

namespace {

enum
{
  e_UNKNOWN = 0,
  e_CVS,
  e_SVN,
  e_BZR,
  e_GIT,
  e_HG,
  e_P4,
  e_LAST
};

char const* TypeToString(int type)
{
  // clang-format off
  switch (type) {
    case e_CVS: return "CVS";
    case e_SVN: return "SVN";
    case e_BZR: return "BZR";
    case e_GIT: return "GIT";
    case e_HG:  return "HG";
    case e_P4:  return "P4";
    default:    return "Unknown";
  }
  // clang-format on
}

char const* TypeToCommandKey(int type)
{
  // clang-format off
  switch (type) {
    case e_CVS: return "CTEST_CVS_COMMAND";
    case e_SVN: return "CTEST_SVN_COMMAND";
    case e_BZR: return "CTEST_BZR_COMMAND";
    case e_GIT: return "CTEST_GIT_COMMAND";
    case e_HG:  return "CTEST_HG_COMMAND";
    case e_P4:  return "CTEST_P4_COMMAND";
    default:    return nullptr;
  }
  // clang-format on
}

int DetermineType(std::string const& cmd)
{
  std::string stype = cmSystemTools::LowerCase(cmd);
  if (stype.find("cvs") != std::string::npos) {
    return e_CVS;
  }
  if (stype.find("svn") != std::string::npos) {
    return e_SVN;
  }
  if (stype.find("bzr") != std::string::npos) {
    return e_BZR;
  }
  if (stype.find("git") != std::string::npos) {
    return e_GIT;
  }
  if (stype.find("hg") != std::string::npos) {
    return e_HG;
  }
  if (stype.find("p4") != std::string::npos) {
    return e_P4;
  }
  return e_UNKNOWN;
}

int DetectVCS(std::string const& dir)
{
  if (cmSystemTools::FileExists(cmStrCat(dir, "/CVS"))) {
    return e_CVS;
  }
  if (cmSystemTools::FileExists(cmStrCat(dir, "/.svn"))) {
    return e_SVN;
  }
  if (cmSystemTools::FileExists(cmStrCat(dir, "/.bzr"))) {
    return e_BZR;
  }
  if (cmSystemTools::FileExists(cmStrCat(dir, "/.git"))) {
    return e_GIT;
  }
  if (cmSystemTools::FileExists(cmStrCat(dir, "/.hg"))) {
    return e_HG;
  }
  if (cmSystemTools::FileExists(cmStrCat(dir, "/.p4"))) {
    return e_P4;
  }
  if (cmSystemTools::FileExists(cmStrCat(dir, "/.p4config"))) {
    return e_P4;
  }
  return e_UNKNOWN;
}

std::unique_ptr<cmCTestVC> MakeVC(int type, cmCTest* ctest, cmMakefile* mf,
                                  std::ostream& os)
{
  // clang-format off
  switch (type) {
    case e_CVS: return cm::make_unique<cmCTestCVS>(ctest, mf, os);
    case e_SVN: return cm::make_unique<cmCTestSVN>(ctest, mf, os);
    case e_BZR: return cm::make_unique<cmCTestBZR>(ctest, mf, os);
    case e_GIT: return cm::make_unique<cmCTestGIT>(ctest, mf, os);
    case e_HG:  return cm::make_unique<cmCTestHG> (ctest, mf, os);
    case e_P4:  return cm::make_unique<cmCTestP4> (ctest, mf, os);
    default:    return cm::make_unique<cmCTestVC> (ctest, mf, os);
  }
  // clang-format on
}

} // namespace

bool cmCTestUpdateCommand::ExecuteUpdate(UpdateArguments& args,
                                         cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();

  std::string const& source_dir = !args.Source.empty()
    ? args.Source
    : mf.GetSafeDefinition("CTEST_SOURCE_DIRECTORY");
  if (source_dir.empty()) {
    status.SetError("called with no source directory specified. "
                    "Use SOURCE argument or set CTEST_SOURCE_DIRECTORY.");
    return false;
  }

  std::string const currentTag = this->CTest->GetCurrentTag();
  if (currentTag.empty()) {
    status.SetError("called with no current tag.");
    return false;
  }

  // Detect the VCS managing the source tree.
  int updateType = DetectVCS(source_dir);

  // Get update command
  std::string updateCommand = mf.GetSafeDefinition("CTEST_UPDATE_COMMAND");

  if (updateType == e_UNKNOWN && !updateCommand.empty()) {
    updateType = DetermineType(updateCommand);
  }
  if (updateType == e_UNKNOWN) {
    updateType = DetermineType(mf.GetSafeDefinition("CTEST_UPDATE_TYPE"));
  }

  // If no update command was specified, lookup one for this VCS tool.
  if (updateCommand.empty()) {
    char const* key = TypeToCommandKey(updateType);
    if (key) {
      updateCommand = mf.GetSafeDefinition(key);
    }

    if (updateCommand.empty()) {
      std::ostringstream e;
      e << "called with no update command specified. "
           "Please set CTEST_UPDATE_COMMAND";
      if (key) {
        e << " or " << key;
      }
      e << '.';
      status.SetError(e.str());
      return false;
    }
  }

  cmGeneratedFileStream ofs;
  if (!this->CTest->GetShowOnly()) {
    std::string logFile = cmStrCat("LastUpdate_", currentTag, ".log");
    if (!this->CTest->OpenOutputFile("Temporary", logFile, ofs)) {
      status.SetError(cmStrCat("cannot create log file: ", logFile));
      return false;
    }
  }

  cmGeneratedFileStream os;
  if (!this->CTest->OpenOutputFile(currentTag, "Update.xml", os, true)) {
    status.SetError("cannot create resulting XML file: Update.xml");
    return false;
  }

  this->CTest->AddSubmitFile(cmCTest::PartUpdate, "Update.xml");

  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                     "   Updating " << TypeToString(updateType)
                                    << " repository: " << source_dir << '\n',
                     args.Quiet);

  // Make sure VCS tool messages are in English so we can parse them.
  cmCLocaleEnvironmentScope fixLocale;
  static_cast<void>(fixLocale);

  // Create an object to interact with the VCS tool.
  std::unique_ptr<cmCTestVC> vc = MakeVC(updateType, this->CTest, &mf, ofs);

  vc->SetCommandLineTool(updateCommand);
  vc->SetSourceDirectory(source_dir);

  // Cleanup the working tree.
  vc->Cleanup();

  std::string start_time = this->CTest->CurrentTime();
  auto start_time_time = std::chrono::system_clock::now();
  auto elapsed_time_start = std::chrono::steady_clock::now();

  bool updated = vc->Update();
  std::string buildname =
    cmCTest::SafeBuildIdField(mf.GetSafeDefinition("CTEST_BUILD_NAME"));

  cmXMLWriter xml(os);
  xml.StartDocument();
  xml.StartElement("Update");
  xml.Attribute("mode", "Client");
  xml.Attribute("Generator",
                std::string("ctest-") + cmVersion::GetCMakeVersion());
  xml.Element("Site", mf.GetSafeDefinition("CTEST_SITE"));
  xml.Element("BuildName", buildname);
  xml.Element("BuildStamp",
              this->CTest->GetCurrentTag() + "-" +
                this->CTest->GetTestGroupString());
  xml.Element("StartDateTime", start_time);
  xml.Element("StartTime", start_time_time);
  xml.Element("UpdateCommand", vc->GetUpdateCommandLine());
  xml.Element("UpdateType", TypeToString(updateType));

  std::string changeId = mf.GetSafeDefinition("CTEST_CHANGE_ID");
  if (!changeId.empty()) {
    xml.Element("ChangeId", changeId);
  }

  bool loadedMods = vc->WriteXML(xml);

  int localModifications = 0;
  int numUpdated = vc->GetPathCount(cmCTestVC::PathUpdated);
  if (numUpdated) {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "   Found " << numUpdated << " updated files\n",
                       args.Quiet);
  }
  if (int numModified = vc->GetPathCount(cmCTestVC::PathModified)) {
    cmCTestOptionalLog(
      this->CTest, HANDLER_OUTPUT,
      "   Found " << numModified << " locally modified files\n", args.Quiet);
    localModifications += numModified;
  }
  if (int numConflicting = vc->GetPathCount(cmCTestVC::PathConflicting)) {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "   Found " << numConflicting << " conflicting files\n",
                       args.Quiet);
    localModifications += numConflicting;
  }

  cmCTestOptionalLog(this->CTest, DEBUG, "End" << std::endl, args.Quiet);
  std::string end_time = this->CTest->CurrentTime();
  xml.Element("EndDateTime", end_time);
  xml.Element("EndTime", std::chrono::system_clock::now());
  xml.Element("ElapsedMinutes",
              std::chrono::duration_cast<std::chrono::minutes>(
                std::chrono::steady_clock::now() - elapsed_time_start)
                .count());

  xml.StartElement("UpdateReturnStatus");
  if (localModifications) {
    xml.Content("Update error: "
                "There are modified or conflicting files in the repository");
    cmCTestLog(this->CTest, WARNING,
               "   There are modified or conflicting files in the repository"
                 << std::endl);
  }
  if (!updated) {
    xml.Content("Update command failed:\n");
    xml.Content(vc->GetUpdateCommandLine());
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
               "   Update command failed: " << vc->GetUpdateCommandLine()
                                            << "\n");
  }
  xml.EndElement(); // UpdateReturnStatus
  xml.EndElement(); // Update
  xml.EndDocument();

  if (!args.ReturnValue.empty()) {
    mf.AddDefinition(args.ReturnValue,
                     std::to_string(updated && loadedMods ? numUpdated : -1));
  }

  return true;
}

bool cmCTestUpdateCommand::InitialPass(std::vector<std::string> const& args,
                                       cmExecutionStatus& status) const
{
  static auto const parser =
    cmArgumentParser<UpdateArguments>{ MakeBasicParser<UpdateArguments>() }
      .Bind("SOURCE"_s, &UpdateArguments::Source)
      .Bind("RETURN_VALUE"_s, &UpdateArguments::ReturnValue)
      .Bind("QUIET"_s, &UpdateArguments::Quiet);

  return this->Invoke(parser, args, status, [&](UpdateArguments& a) {
    return this->ExecuteUpdate(a, status);
  });
}
