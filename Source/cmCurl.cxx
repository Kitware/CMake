/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCurl.h"

#include <cm/string_view>
#include <cmext/string_view>

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

// curl versions before 7.52.0 did not provide TLS 1.3 support
#if defined(LIBCURL_VERSION_NUM) && LIBCURL_VERSION_NUM < 0x073400
#  define CURL_SSLVERSION_TLSv1_3 CURL_SSLVERSION_LAST
#endif

// curl versions before 7.64.1 referred to Secure Transport as DarwinSSL
#if defined(LIBCURL_VERSION_NUM) && LIBCURL_VERSION_NUM < 0x074001
#  define CURLSSLBACKEND_SECURETRANSPORT CURLSSLBACKEND_DARWINSSL
#endif

// Make sure we keep up with new TLS versions supported by curl.
// Do this only for our vendored curl to avoid breaking builds
// against external future versions of curl.
#if !defined(CMAKE_USE_SYSTEM_CURL)
static_assert(CURL_SSLVERSION_LAST == 8,
              "A new CURL_SSLVERSION_ may be available!");
#endif

void cmCurlInitOnce()
{
  // curl 7.56.0 introduced curl_global_sslset.
#if defined(__APPLE__) && defined(CMAKE_USE_SYSTEM_CURL) &&                   \
  defined(LIBCURL_VERSION_NUM) && LIBCURL_VERSION_NUM >= 0x073800
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  cm::optional<std::string> curl_ssl_backend =
    cmSystemTools::GetEnvVar("CURL_SSL_BACKEND");
  if (!curl_ssl_backend || curl_ssl_backend->empty()) {
    curl_version_info_data* cv = curl_version_info(CURLVERSION_FIRST);
    // curl 8.3.0 through 8.5.x did not re-initialize LibreSSL correctly,
    // so prefer the Secure Transport backend by default in those versions.
    if (cv->version_num >= 0x080300 && cv->version_num < 0x080600) {
      curl_global_sslset(CURLSSLBACKEND_SECURETRANSPORT, NULL, NULL);
    }
  }
#endif
}

cm::optional<int> cmCurlParseTLSVersion(cm::string_view tls_version)
{
  cm::optional<int> v;
  if (tls_version == "1.0"_s) {
    v = CURL_SSLVERSION_TLSv1_0;
  } else if (tls_version == "1.1"_s) {
    v = CURL_SSLVERSION_TLSv1_1;
  } else if (tls_version == "1.2"_s) {
    v = CURL_SSLVERSION_TLSv1_2;
  } else if (tls_version == "1.3"_s) {
    v = CURL_SSLVERSION_TLSv1_3;
  }
  return v;
}

cm::optional<std::string> cmCurlPrintTLSVersion(int curl_tls_version)
{
  cm::optional<std::string> s;
  switch (curl_tls_version) {
    case CURL_SSLVERSION_TLSv1_0:
      s = "CURL_SSLVERSION_TLSv1_0"_s;
      break;
    case CURL_SSLVERSION_TLSv1_1:
      s = "CURL_SSLVERSION_TLSv1_1"_s;
      break;
    case CURL_SSLVERSION_TLSv1_2:
      s = "CURL_SSLVERSION_TLSv1_2"_s;
      break;
    case CURL_SSLVERSION_TLSv1_3:
      s = "CURL_SSLVERSION_TLSv1_3"_s;
      break;
  }
  return s;
}

std::string cmCurlSetCAInfo(::CURL* curl, const std::string& cafile)
{
  std::string e;
  std::string env_ca;
  if (!cafile.empty()) {
    ::CURLcode res = ::curl_easy_setopt(curl, CURLOPT_CAINFO, cafile.c_str());
    check_curl_result(res, "Unable to set TLS/SSL Verify CAINFO: ");
  }
  /* Honor the user-configurable OpenSSL environment variables. */
  else if (cmSystemTools::GetEnv("SSL_CERT_FILE", env_ca) &&
           cmSystemTools::FileExists(env_ca, true)) {
    ::CURLcode res = ::curl_easy_setopt(curl, CURLOPT_CAINFO, env_ca.c_str());
    check_curl_result(res, "Unable to set TLS/SSL Verify CAINFO: ");
  } else if (cmSystemTools::GetEnv("SSL_CERT_DIR", env_ca) &&
             cmSystemTools::FileIsDirectory(env_ca)) {
    ::CURLcode res = ::curl_easy_setopt(curl, CURLOPT_CAPATH, env_ca.c_str());
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
    int mblen = WideCharToMultiByte(CP_ACP, 0, wurl.c_str(), -1, nullptr, 0,
                                    nullptr, nullptr);
    if (mblen > 0) {
      std::vector<char> chars(mblen);
      mblen = WideCharToMultiByte(CP_ACP, 0, wurl.c_str(), -1, &chars[0],
                                  mblen, nullptr, nullptr);
      if (mblen > 0) {
        url = &chars[0];
      }
    }
  }
#endif

  return url;
}
