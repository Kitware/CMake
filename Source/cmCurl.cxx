/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCurl.h"

#if !defined(CMAKE_USE_SYSTEM_CURL) && !defined(_WIN32) &&                    \
  !defined(__APPLE__) && !defined(CURL_CA_BUNDLE) && !defined(CURL_CA_PATH)
#  define CMAKE_FIND_CAFILE
#endif
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#if defined(_WIN32)
#  include <vector>

#  include <windows.h>

#  include "cmsys/Encoding.hxx"
#endif

// curl versions before 7.21.5 did not provide this error code
#if defined(LIBCURL_VERSION_NUM) && LIBCURL_VERSION_NUM < 0x071505
#  define CURLE_NOT_BUILT_IN 4
#endif

#define check_curl_result(result, errstr)                                     \
  do {                                                                        \
    if ((result) != CURLE_OK && (result) != CURLE_NOT_BUILT_IN) {             \
      e += e.empty() ? "" : "\n";                                             \
      e += (errstr);                                                          \
      e += ::curl_easy_strerror(result);                                      \
    }                                                                         \
  } while (false)

std::string cmCurlSetCAInfo(::CURL* curl, const std::string& cafile)
{
  std::string e;
  if (!cafile.empty()) {
    ::CURLcode res = ::curl_easy_setopt(curl, CURLOPT_CAINFO, cafile.c_str());
    check_curl_result(res, "Unable to set TLS/SSL Verify CAINFO: ");
  }
#ifdef CMAKE_FIND_CAFILE
#  define CMAKE_CAFILE_FEDORA "/etc/pki/tls/certs/ca-bundle.crt"
  else if (cmSystemTools::FileExists(CMAKE_CAFILE_FEDORA, true)) {
    ::CURLcode res =
      ::curl_easy_setopt(curl, CURLOPT_CAINFO, CMAKE_CAFILE_FEDORA);
    check_curl_result(res, "Unable to set TLS/SSL Verify CAINFO: ");
  }
#  undef CMAKE_CAFILE_FEDORA
  else {
#  define CMAKE_CAFILE_COMMON "/etc/ssl/certs/ca-certificates.crt"
    if (cmSystemTools::FileExists(CMAKE_CAFILE_COMMON, true)) {
      ::CURLcode res =
        ::curl_easy_setopt(curl, CURLOPT_CAINFO, CMAKE_CAFILE_COMMON);
      check_curl_result(res, "Unable to set TLS/SSL Verify CAINFO: ");
    }
#  undef CMAKE_CAFILE_COMMON
#  define CMAKE_CAPATH_COMMON "/etc/ssl/certs"
    if (cmSystemTools::FileIsDirectory(CMAKE_CAPATH_COMMON)) {
      ::CURLcode res =
        ::curl_easy_setopt(curl, CURLOPT_CAPATH, CMAKE_CAPATH_COMMON);
      check_curl_result(res, "Unable to set TLS/SSL Verify CAPATH: ");
    }
#  undef CMAKE_CAPATH_COMMON
  }
#endif
  return e;
}

std::string cmCurlSetNETRCOption(::CURL* curl, const std::string& netrc_level,
                                 const std::string& netrc_file)
{
  std::string e;
  CURL_NETRC_OPTION curl_netrc_level = CURL_NETRC_LAST;
  ::CURLcode res;

  if (!netrc_level.empty()) {
    if (netrc_level == "OPTIONAL") {
      curl_netrc_level = CURL_NETRC_OPTIONAL;
    } else if (netrc_level == "REQUIRED") {
      curl_netrc_level = CURL_NETRC_REQUIRED;
    } else if (netrc_level == "IGNORED") {
      curl_netrc_level = CURL_NETRC_IGNORED;
    } else {
      e = cmStrCat("NETRC accepts OPTIONAL, IGNORED or REQUIRED but got: ",
                   netrc_level);
      return e;
    }
  }

  if (curl_netrc_level != CURL_NETRC_LAST &&
      curl_netrc_level != CURL_NETRC_IGNORED) {
    res = ::curl_easy_setopt(curl, CURLOPT_NETRC, curl_netrc_level);
    check_curl_result(res, "Unable to set netrc level: ");
    if (!e.empty()) {
      return e;
    }

    // check to see if a .netrc file has been specified
    if (!netrc_file.empty()) {
      res = ::curl_easy_setopt(curl, CURLOPT_NETRC_FILE, netrc_file.c_str());
      check_curl_result(res, "Unable to set .netrc file path : ");
    }
  }
  return e;
}

std::string cmCurlFixFileURL(std::string url)
{
  if (!cmHasLiteralPrefix(url, "file://")) {
    return url;
  }

  // libcurl 7.77 and below accidentally allowed spaces in URLs in some cases.
  // One such case was file:// URLs, which CMake has long accepted as a result.
  // Explicitly encode spaces for a URL.
  cmSystemTools::ReplaceString(url, " ", "%20");

#if defined(_WIN32)
  // libcurl doesn't support file:// urls for unicode filenames on Windows.
  // Convert string from UTF-8 to ACP if this is a file:// URL.
  std::wstring wurl = cmsys::Encoding::ToWide(url);
  if (!wurl.empty()) {
    int mblen =
      WideCharToMultiByte(CP_ACP, 0, wurl.c_str(), -1, NULL, 0, NULL, NULL);
    if (mblen > 0) {
      std::vector<char> chars(mblen);
      mblen = WideCharToMultiByte(CP_ACP, 0, wurl.c_str(), -1, &chars[0],
                                  mblen, NULL, NULL);
      if (mblen > 0) {
        url = &chars[0];
      }
    }
  }
#endif

  return url;
}
