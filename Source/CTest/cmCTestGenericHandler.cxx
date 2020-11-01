/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestGenericHandler.h"

#include <sstream>
#include <utility>

#include "cmCTest.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmCTestGenericHandler::cmCTestGenericHandler()
{
  this->HandlerVerbose = cmSystemTools::OUTPUT_NONE;
  this->CTest = nullptr;
  this->SubmitIndex = 0;
  this->AppendXML = false;
  this->Quiet = false;
  this->TestLoad = 0;
}

cmCTestGenericHandler::~cmCTestGenericHandler() = default;

void cmCTestGenericHandler::SetOption(const std::string& op, const char* value)
{
  if (!value) {
    auto remit = this->Options.find(op);
    if (remit != this->Options.end()) {
      this->Options.erase(remit);
    }
    return;
  }

  this->Options[op] = value;
}

void cmCTestGenericHandler::SetPersistentOption(const std::string& op,
                                                const char* value)
{
  this->SetOption(op, value);
  if (!value) {
    auto remit = this->PersistentOptions.find(op);
    if (remit != this->PersistentOptions.end()) {
      this->PersistentOptions.erase(remit);
    }
    return;
  }

  this->PersistentOptions[op] = value;
}

void cmCTestGenericHandler::Initialize()
{
  this->AppendXML = false;
  this->TestLoad = 0;
  this->Options.clear();
  for (auto const& po : this->PersistentOptions) {
    this->Options[po.first] = po.second;
  }
}

const char* cmCTestGenericHandler::GetOption(const std::string& op)
{
  auto remit = this->Options.find(op);
  if (remit == this->Options.end()) {
    return nullptr;
  }
  return remit->second.c_str();
}

bool cmCTestGenericHandler::StartResultingXML(cmCTest::Part part,
                                              const char* name,
                                              cmGeneratedFileStream& xofs)
{
  if (!name) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot create resulting XML file without providing the name"
                 << std::endl;);
    return false;
  }
  std::ostringstream ostr;
  ostr << name;
  if (this->SubmitIndex > 0) {
    ostr << "_" << this->SubmitIndex;
  }
  ostr << ".xml";
  if (this->CTest->GetCurrentTag().empty()) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Current Tag empty, this may mean NightlyStartTime / "
               "CTEST_NIGHTLY_START_TIME was not set correctly. Or "
               "maybe you forgot to call ctest_start() before calling "
               "ctest_configure()."
                 << std::endl);
    cmSystemTools::SetFatalErrorOccured();
    return false;
  }
  if (!this->CTest->OpenOutputFile(this->CTest->GetCurrentTag(), ostr.str(),
                                   xofs, true)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot create resulting XML file: " << ostr.str()
                                                    << std::endl);
    return false;
  }
  this->CTest->AddSubmitFile(part, ostr.str());
  return true;
}

bool cmCTestGenericHandler::StartLogFile(const char* name,
                                         cmGeneratedFileStream& xofs)
{
  if (!name) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot create log file without providing the name"
                 << std::endl;);
    return false;
  }
  std::ostringstream ostr;
  ostr << "Last" << name;
  if (this->SubmitIndex > 0) {
    ostr << "_" << this->SubmitIndex;
  }
  if (!this->CTest->GetCurrentTag().empty()) {
    ostr << "_" << this->CTest->GetCurrentTag();
  }
  ostr << ".log";
  this->LogFileNames[name] =
    cmStrCat(this->CTest->GetBinaryDir(), "/Testing/Temporary/", ostr.str());
  if (!this->CTest->OpenOutputFile("Temporary", ostr.str(), xofs)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot create log file: " << ostr.str() << std::endl);
    return false;
  }
  return true;
}
