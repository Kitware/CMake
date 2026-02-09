/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include <cm/optional>
#include <cm/string_view>

#include <cm3p/curl/curl.h>

// curl versions before 7.87.0 did not provide CURL_WRITEFUNC_ERROR
#if defined(LIBCURL_VERSION_NUM) && LIBCURL_VERSION_NUM < 0x075700
#  define CURL_WRITEFUNC_ERROR 0xFFFFFFFF
#endif

cm::optional<int> cmCurlParseTLSVersion(cm::string_view tls_version);
cm::optional<std::string> cmCurlPrintTLSVersion(int curl_tls_version);
std::string cmCurlSetCAInfo(::CURL* curl, std::string const& cafile = {});
std::string cmCurlSetNETRCOption(::CURL* curl, std::string const& netrc_level,
                                 std::string const& netrc_file);

::CURLcode cm_curl_global_init(long flags);
::CURL* cm_curl_easy_init();
