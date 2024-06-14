/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <iosfwd>
#include <set>
#include <string>
#include <vector>

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

/** \class cmCTestSubmitHandler
 * \brief Helper class for CTest
 *
 * Submit testing results
 *
 */
class cmCTestSubmitHandler : public cmCTestGenericHandler
{
public:
  using Superclass = cmCTestGenericHandler;

  cmCTestSubmitHandler();
  ~cmCTestSubmitHandler() override { this->LogFile = nullptr; }

  /*
   * The main entry point for this class
   */
  int ProcessHandler() override;

  void Initialize() override;

  //! Set all the submit arguments
  int ProcessCommandLineArguments(
    const std::string& currentArg, size_t& idx,
    const std::vector<std::string>& allArgs) override;

  /** Specify a set of parts (by name) to submit.  */
  void SelectParts(std::set<cmCTest::Part> const& parts);

  /** Specify a set of files to submit.  */
  void SelectFiles(std::set<std::string> const& files);

  // handle the cdash file upload protocol
  int HandleCDashUploadFile(std::string const& file, std::string const& type);

  void SetHttpHeaders(std::vector<std::string> const& v)
  {
    if (this->CommandLineHttpHeaders.empty()) {
      this->HttpHeaders = v;
    } else {
      this->HttpHeaders = this->CommandLineHttpHeaders;
      this->HttpHeaders.insert(this->HttpHeaders.end(), v.begin(), v.end());
    }
  }

private:
  void SetLogFile(std::ostream* ost) { this->LogFile = ost; }

  /**
   * Submit file using various ways
   */
  bool SubmitUsingHTTP(const std::string& localprefix,
                       const std::vector<std::string>& files,
                       const std::string& remoteprefix,
                       const std::string& url);

  using cmCTestSubmitHandlerVectorOfChar = std::vector<char>;

  void ParseResponse(cmCTestSubmitHandlerVectorOfChar chunk);

  std::string GetSubmitResultsPrefix();
  int GetSubmitInactivityTimeout();

  class ResponseParser;

  std::string HTTPProxy;
  int HTTPProxyType;
  std::string HTTPProxyAuth;
  std::ostream* LogFile;
  bool SubmitPart[cmCTest::PartCount];
  bool HasWarnings;
  bool HasErrors;
  std::set<std::string> Files;
  std::vector<std::string> CommandLineHttpHeaders;
  std::vector<std::string> HttpHeaders;
};
