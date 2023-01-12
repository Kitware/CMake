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

namespace {
/* Modify the given `map`, setting key `op` to `value` if `value`
 * is non-null, otherwise removing key `op` (if it exists).
 */
void SetMapValue(cmCTestGenericHandler::t_StringToString& map,
                 const std::string& op, const std::string& value)
{
  map[op] = value;
}
void SetMapValue(cmCTestGenericHandler::t_StringToString& map,
                 const std::string& op, cmValue value)
{
  if (!value) {
    map.erase(op);
    return;
  }

  map[op] = *value;
}
}

void cmCTestGenericHandler::SetOption(const std::string& op,
                                      const std::string& value)
{
  SetMapValue(this->Options, op, value);
}
void cmCTestGenericHandler::SetOption(const std::string& op, cmValue value)
{
  SetMapValue(this->Options, op, value);
}

void cmCTestGenericHandler::SetPersistentOption(const std::string& op,
                                                const std::string& value)
{
  this->SetOption(op, value);
  SetMapValue(this->PersistentOptions, op, value);
}
void cmCTestGenericHandler::SetPersistentOption(const std::string& op,
                                                cmValue value)
{
  this->SetOption(op, value);
  SetMapValue(this->PersistentOptions, op, value);
}

void cmCTestGenericHandler::AddMultiOption(const std::string& op,
                                           const std::string& value)
{
  if (!value.empty()) {
    this->MultiOptions[op].emplace_back(value);
  }
}

void cmCTestGenericHandler::AddPersistentMultiOption(const std::string& op,
                                                     const std::string& value)
{
  if (!value.empty()) {
    this->MultiOptions[op].emplace_back(value);
    this->PersistentMultiOptions[op].emplace_back(value);
  }
}

void cmCTestGenericHandler::Initialize()
{
  this->AppendXML = false;
  this->TestLoad = 0;
  this->Options = this->PersistentOptions;
  this->MultiOptions = this->PersistentMultiOptions;
}

cmValue cmCTestGenericHandler::GetOption(const std::string& op)
{
  auto remit = this->Options.find(op);
  if (remit == this->Options.end()) {
    return nullptr;
  }
  return cmValue(remit->second);
}

std::vector<std::string> cmCTestGenericHandler::GetMultiOption(
  const std::string& optionName) const
{
  // Avoid inserting a key, which MultiOptions[op] would do.
  auto remit = this->MultiOptions.find(optionName);
  if (remit == this->MultiOptions.end()) {
    return {};
  }
  return remit->second;
}

bool cmCTestGenericHandler::StartResultingXML(cmCTest::Part part,
                                              const char* name,
                                              cmGeneratedFileStream& xofs)
{
  if (!name) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot create resulting XML file without providing the name"
                 << std::endl);
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
    cmSystemTools::SetFatalErrorOccurred();
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
                 << std::endl);
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
