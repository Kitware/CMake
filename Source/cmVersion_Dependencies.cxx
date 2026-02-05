/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmVersion.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include <cstddef>
#  include <string>
#  include <utility>
#  include <vector>

#  include <cm3p/archive.h>
#  include <cm3p/curl/curl.h>
#  include <cm3p/expat.h>
#  include <cm3p/json/version.h>
#  include <cm3p/kwiml/version.h>

#  include "cmThirdParty.h" // IWYU pragma: keep

#  ifdef CMAKE_USE_SYSTEM_LIBRHASH
#    include <cstdint>

#    include <cm3p/rhash.h>
#  endif
#  include <cm3p/uv.h>
#  include <cm3p/zlib.h>

#  include "cmStringAlgorithms.h"

std::vector<cmVersion::DependencyInfo> const&
cmVersion::CollectDependencyInfo()
{
  static std::vector<DependencyInfo> deps;
  if (!deps.empty()) {
    return deps;
  }

  // BZIP2 is not directly used in CMake, so it is not included here

  // BZIP2 (libarchive)
  {
    char const* bzip2Version = archive_bzlib_version();
    if (bzip2Version) {
      DependencyInfo info;
      info.name = "bzip2";
      info.version = bzip2Version;
      info.cameFrom = "libarchive";
      size_t pos = info.version.find(',');
      if (pos != std::string::npos) {
        // Convert `1.0.8, 30-Mar-2009` to `1.0.8`
        info.version.erase(pos);
      }
#  if defined(CMAKE_USE_SYSTEM_BZIP2) || defined(CMAKE_USE_SYSTEM_LIBARCHIVE)
      // System BZIP2 can be used by system or bundled libarchive
      // System libarchive always uses system BZIP2
      info.type = DependencyType::System;
#  else
      info.type = DependencyType::Bundled;
#  endif
      deps.emplace_back(std::move(info));
    }
  }

  // CPPDAP
  {
    DependencyInfo info;
    info.name = "cppdap";
#  ifdef CMAKE_USE_SYSTEM_CPPDAP
    info.type = DependencyType::System;
    // Cannot get runtime version from cppdap library
#  else
    info.type = DependencyType::Bundled;
    // Hardcoded in protocol.h header file comments
    info.version = "1.65.0";
#  endif
    deps.emplace_back(std::move(info));
  }

  // CURL
  {
    curl_version_info_data* curlVersion = curl_version_info(CURLVERSION_NOW);
    if (curlVersion) {
      // CURL itself
      {
        DependencyInfo info;
        info.name = "curl";
        if (curlVersion->version) {
          info.version = curlVersion->version;
        }
#  ifdef CMAKE_USE_SYSTEM_CURL
        info.type = DependencyType::System;
#  else
        info.type = DependencyType::Bundled;
#  endif
        deps.emplace_back(std::move(info));
      }

// Cannot use CURL_AT_LEAST_VERSION and CURL_VERSION_BITS macros,
// because they needs at least curl 7.43.0,
// but we support curl 7.29.0 from CentOS 7
#  if LIBCURL_VERSION_NUM >= 0x074200
      // NGHTTP2 (curl)
      // Added in curl 7.66.0 (0x074200), CURLVERSION_SIXTH
      if (curlVersion->age >= CURLVERSION_SIXTH &&
          curlVersion->nghttp2_version) {
        DependencyInfo info;
        info.name = "nghttp2";
        info.cameFrom = "curl";

        info.version = curlVersion->nghttp2_version;

#    if defined(CMAKE_USE_SYSTEM_NGHTTP2) || defined(CMAKE_USE_SYSTEM_CURL)
        // System CURL always uses system NGHTTP2
        // System NGHTTP2 can be used by system or bundled CURL
        info.type = DependencyType::System;
#    else
        info.type = DependencyType::Bundled;
#    endif
        deps.emplace_back(std::move(info));
      }
#  endif

      // OPENSSL (curl)
      if (curlVersion->ssl_version) {
        DependencyInfo info;
        info.name = "ssl";
        info.cameFrom = "curl";
        info.version = curlVersion->ssl_version;

        // With Multi-SSL, the version string is `OpenSSL/3.3.5,
        // BoringSSL/3.3.5`, etc.
        if (cmHasLiteralPrefix(info.version, "OpenSSL/") &&
            info.version.find('/', 8) == std::string::npos) {
          info.name = "openssl";
          info.version.erase(0, 8);
        }
        // Bundled version of OpenSSL is not presented
        // Multi-SSL can be used by system CURL only,
        // so the SSL library is always system
        info.type = DependencyType::System;
        deps.emplace_back(std::move(info));
      }

      // ZLIB (curl)
      if (curlVersion->libz_version) {
        DependencyInfo info;
        info.name = "zlib";
        info.cameFrom = "curl";
        info.version = curlVersion->libz_version;
#  if defined(CMAKE_USE_SYSTEM_ZLIB) || defined(CMAKE_USE_SYSTEM_CURL)
        // System CURL always uses system ZLIB
        // System ZLIB can be used by system or bundled CURL
        info.type = DependencyType::System;
#  else
        info.type = DependencyType::Bundled;
#  endif
        deps.emplace_back(std::move(info));
      }
    }
  }

  // EXPAT
  {
    DependencyInfo info;
    info.name = "expat";

    XML_Expat_Version version = XML_ExpatVersionInfo();
    info.version =
      cmStrCat(version.major, '.', version.minor, '.', version.micro);
#  ifdef CMAKE_USE_SYSTEM_EXPAT
    info.type = DependencyType::System;
#  else
    info.type = DependencyType::Bundled;
#  endif
    deps.emplace_back(std::move(info));
  }

  // FORM
  {
    DependencyInfo info;
    info.name = "form";
    // Cannot get any version from form library
#  ifdef CMAKE_USE_SYSTEM_FORM
    info.type = DependencyType::System;
#  else
    info.type = DependencyType::Bundled;
#  endif
    deps.emplace_back(std::move(info));
  }

  // JSONCPP
  {
    DependencyInfo info;
    info.name = "jsoncpp";
    info.version = JSONCPP_VERSION_STRING;
#  ifdef CMAKE_USE_SYSTEM_JSONCPP
    info.type = DependencyType::System;
#  else
    info.type = DependencyType::Bundled;
#  endif
    deps.emplace_back(std::move(info));
  }

  // KWIML
  {
    DependencyInfo info;
    info.name = "kwiml";

    // Library is header-only, so we can safely use the defined version
    info.version = KWIML_VERSION_STRING;

#  ifdef CMAKE_USE_SYSTEM_KWIML
    info.type = DependencyType::System;
#  else
    info.type = DependencyType::Bundled;
#  endif
    deps.emplace_back(std::move(info));
  }

  // LIBARCHIVE
  {
    DependencyInfo info;
    info.name = "libarchive";
    info.version = archive_version_string();
    if (cmHasLiteralPrefix(info.version, "libarchive ")) {
      info.version.erase(0, 11);
    }
#  ifdef CMAKE_USE_SYSTEM_LIBARCHIVE
    info.type = DependencyType::System;
#  else
    info.type = DependencyType::Bundled;
#  endif
    deps.emplace_back(std::move(info));
  }

  // LIBLZMA is not directly used in CMake, so it is not included here

  // LIBLZMA (libarchive)
  {
    char const* liblzmaVersion = archive_liblzma_version();
    if (liblzmaVersion) {
      DependencyInfo info;
      info.name = "liblzma";
      info.cameFrom = "libarchive";
      info.version = liblzmaVersion;
#  if defined(CMAKE_USE_SYSTEM_LIBLZMA) || defined(CMAKE_USE_SYSTEM_LIBARCHIVE)
      // System LIBLZMA can be used by system or bundled libarchive
      // System libarchive always uses system LIBLZMA
      info.type = DependencyType::System;
#  else
      info.type = DependencyType::Bundled;
#  endif
      deps.emplace_back(std::move(info));
    }
  }

  // LIBRHASH
  {
    DependencyInfo info;
    info.name = "librhash";
#  ifdef CMAKE_USE_SYSTEM_LIBRHASH
    info.type = DependencyType::System;
    std::uint64_t version = rhash_get_version();
    std::uint8_t major = (version >> 24) & 0xFFu;
    std::uint8_t minor = (version >> 16) & 0xFFu;
    std::uint8_t patch = (version >> 8) & 0xFFu;
    info.version = cmStrCat(major, '.', minor, '.', patch);
#  else
    info.type = DependencyType::Bundled;
    // Hardcoded in `update-librhash.bash` script
    info.version = "1.4.4";
#  endif
    deps.emplace_back(std::move(info));
  }

  // LIBUV
  {
    DependencyInfo info;
    info.name = "libuv";
    info.version = uv_version_string();
#  ifdef CMAKE_USE_SYSTEM_LIBUV
    info.type = DependencyType::System;
#  else
    info.type = DependencyType::Bundled;
#  endif
    deps.emplace_back(std::move(info));
  }

  // OPENSSL is not directly used in CMake, so it is not included here

#  if ARCHIVE_VERSION_NUMBER >= 3008000
  // OPENSSL (libarchive)
  // This function is available in libarchive 3.8.0 (3008000) and newer
  {
    char const* opensslVersion = archive_openssl_version();
    if (opensslVersion) {
      DependencyInfo info;
      info.name = "openssl";
      info.cameFrom = "libarchive";
      info.version = opensslVersion;
      // Bundled version of OpenSSL is not presented
      info.type = DependencyType::System;
      deps.emplace_back(std::move(info));
    }
  }
#  endif

  // ZLIB
  {
    DependencyInfo info;
    info.name = "zlib";
    info.version = zlibVersion();
#  ifdef CMAKE_USE_SYSTEM_ZLIB
    info.type = DependencyType::System;
#  else
    info.type = DependencyType::Bundled;
#  endif
    deps.emplace_back(std::move(info));
  }

  // ZLIB (libarchive)
  {
    char const* zlibVersion = archive_zlib_version();
    if (zlibVersion) {
      DependencyInfo info;
      info.name = "zlib";
      info.cameFrom = "libarchive";
      info.version = zlibVersion;
#  if defined(CMAKE_USE_SYSTEM_ZLIB) || defined(CMAKE_USE_SYSTEM_LIBARCHIVE)
      // System ZLIB can be used by system or bundled libarchive
      // System libarchive always uses system ZLIB
      info.type = DependencyType::System;
#  else
      info.type = DependencyType::Bundled;
#  endif
      deps.emplace_back(std::move(info));
    }
  }

  // ZSTD is not directly used in CMake, so it is not included here

  // ZSTD (libarchive)
  {
    char const* zstdVersion = archive_libzstd_version();
    if (zstdVersion) {
      DependencyInfo info;
      info.name = "zstd";
      info.cameFrom = "libarchive";
      info.version = zstdVersion;
#  if defined(CMAKE_USE_SYSTEM_ZSTD) || defined(CMAKE_USE_SYSTEM_LIBARCHIVE)
      // System ZSTD can be used by system or bundled libarchive
      // System libarchive always uses system ZSTD
      info.type = DependencyType::System;
#  else
      info.type = DependencyType::Bundled;
#  endif
      deps.emplace_back(std::move(info));
    }
  }

  return deps;
}

#endif
