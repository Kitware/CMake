/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include <cm/optional>

#include <cm3p/curl/curl.h>

class cmCTest;

struct cmCTestCurlOpts
{
  cmCTestCurlOpts(cmCTest* ctest);
  cm::optional<int> TLSVersionOpt;
  cm::optional<bool> TLSVerifyOpt;
  bool VerifyHostOff = false;
};

class cmCTestCurl
{
public:
  cmCTestCurl(cmCTest*);
  ~cmCTestCurl();
  cmCTestCurl(const cmCTestCurl&) = delete;
  cmCTestCurl& operator=(const cmCTestCurl&) = delete;
  bool UploadFile(std::string const& local_file, std::string const& url,
                  std::string const& fields, std::string& response);
  bool HttpRequest(std::string const& url, std::string const& fields,
                   std::string& response);
  void SetHttpHeaders(std::vector<std::string> const& v)
  {
    this->HttpHeaders = v;
  }
  void SetUseHttp10On() { this->UseHttp10 = true; }
  void SetTimeOutSeconds(int s) { this->TimeOutSeconds = s; }
  void SetQuiet(bool b) { this->Quiet = b; }
  std::string Escape(std::string const& source);

protected:
  void SetProxyType();
  bool InitCurl();

private:
  cmCTest* CTest;
  cmCTestCurlOpts CurlOpts;
  CURL* Curl = nullptr;
  std::vector<std::string> HttpHeaders;
  std::string HTTPProxyAuth;
  std::string HTTPProxy;
  curl_proxytype HTTPProxyType;
  bool UseHttp10 = false;
  bool Quiet = false;
  int TimeOutSeconds = 0;
};
