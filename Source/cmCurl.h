/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include <cm/optional>
#include <cm/string_view>

#include <cm3p/curl/curl.h>

void cmCurlInitOnce();
cm::optional<int> cmCurlParseTLSVersion(cm::string_view tls_version);
cm::optional<std::string> cmCurlPrintTLSVersion(int curl_tls_version);
std::string cmCurlSetCAInfo(::CURL* curl, std::string const& cafile = {});
std::string cmCurlSetNETRCOption(::CURL* curl, std::string const& netrc_level,
                                 std::string const& netrc_file);
std::string cmCurlFixFileURL(std::string url);

::CURL* cm_curl_easy_init();
