/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

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

  cmCTestSubmitHandler(cmCTest* ctest);
  ~cmCTestSubmitHandler() override { this->LogFile = nullptr; }

  /*
   * The main entry point for this class
   */
  int ProcessHandler() override;

  /** Specify a set of parts (by name) to submit.  */
  void SelectParts(std::set<cmCTest::Part> const& parts);

  /** Specify a set of files to submit.  */
  void SelectFiles(std::set<std::string> const& files);

  // handle the cdash file upload protocol
  int HandleCDashUploadFile(std::string const& file, std::string const& type);

  void SetHttpHeaders(std::vector<std::string> const& v)
  {
    this->HttpHeaders.insert(this->HttpHeaders.end(), v.begin(), v.end());
  }

private:
  void SetLogFile(std::ostream* ost) { this->LogFile = ost; }

  /**
   * Submit file using various ways
   */
  bool SubmitUsingHTTP(std::string const& localprefix,
                       std::vector<std::string> const& files,
                       std::string const& remoteprefix,
                       std::string const& url);

  using cmCTestSubmitHandlerVectorOfChar = std::vector<char>;

  void ParseResponse(cmCTestSubmitHandlerVectorOfChar chunk);

  std::string GetSubmitResultsPrefix();
  int GetSubmitInactivityTimeout();

  class ResponseParser;

  std::string HTTPProxy;
  int HTTPProxyType = 0;
  std::string HTTPProxyAuth;
  std::ostream* LogFile = nullptr;
  bool SubmitPart[cmCTest::PartCount];
  bool HasWarnings = false;
  bool HasErrors = false;
  std::set<std::string> Files;
  std::vector<std::string> HttpHeaders;

  bool CDashUpload = false;
  bool InternalTest = false;

  std::string CDashUploadFile;
  std::string CDashUploadType;
  std::string RetryCount;
  std::string RetryDelay;

  friend class cmCTestSubmitCommand;
};
