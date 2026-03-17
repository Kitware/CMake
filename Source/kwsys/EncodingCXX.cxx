/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
#ifdef __osf__
#  define _OSF_SOURCE
#  define _POSIX_C_SOURCE 199506L
#  define _XOPEN_SOURCE_EXTENDED
#endif

#include "kwsysPrivate.h"
#include KWSYS_HEADER(Encoding.hxx)
#include KWSYS_HEADER(Encoding.h)
#include KWSYS_HEADER(String.h)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
#  include "Encoding.h.in"
#  include "Encoding.hxx.in"
#  include "String.h"
#endif

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <vector>

#ifdef _MSC_VER
#  pragma warning(disable : 4786)
#endif

// Windows API.
#if defined(_WIN32)
#  include <windows.h>

#  include <shellapi.h>
#endif

namespace KWSYS_NAMESPACE {

Encoding::CommandLineArguments Encoding::CommandLineArguments::Main(
  int argc, char const* const* argv)
{
#ifdef _WIN32
  (void)argc;
  (void)argv;

  int ac;
  LPWSTR* w_av = CommandLineToArgvW(GetCommandLineW(), &ac);

  std::vector<std::string> av1(ac);
  std::vector<char const*> av2(ac);
  for (int i = 0; i < ac; i++) {
    av1[i] = ToNarrow(w_av[i]);
    av2[i] = av1[i].c_str();
  }
  LocalFree(w_av);
  return CommandLineArguments(ac, &av2[0]);
#else
  return CommandLineArguments(argc, argv);
#endif
}

Encoding::CommandLineArguments::CommandLineArguments(int ac,
                                                     char const* const* av)
{
  this->argv_.resize(ac + 1);
  for (int i = 0; i < ac; i++) {
    this->argv_[i] = strdup(av[i]);
  }
  this->argv_[ac] = nullptr;
}

Encoding::CommandLineArguments::CommandLineArguments(int ac,
                                                     wchar_t const* const* av)
{
  this->argv_.resize(ac + 1);
  for (int i = 0; i < ac; i++) {
    this->argv_[i] = kwsysEncoding_DupToNarrow(av[i]);
  }
  this->argv_[ac] = nullptr;
}

Encoding::CommandLineArguments::~CommandLineArguments()
{
  for (size_t i = 0; i < this->argv_.size(); i++) {
    free(argv_[i]);
  }
}

Encoding::CommandLineArguments::CommandLineArguments(
  CommandLineArguments const& other)
{
  this->argv_.resize(other.argv_.size());
  for (size_t i = 0; i < this->argv_.size(); i++) {
    this->argv_[i] = other.argv_[i] ? strdup(other.argv_[i]) : nullptr;
  }
}

Encoding::CommandLineArguments& Encoding::CommandLineArguments::operator=(
  CommandLineArguments const& other)
{
  if (this != &other) {
    size_t i;
    for (i = 0; i < this->argv_.size(); i++) {
      free(this->argv_[i]);
    }

    this->argv_.resize(other.argv_.size());
    for (i = 0; i < this->argv_.size(); i++) {
      this->argv_[i] = other.argv_[i] ? strdup(other.argv_[i]) : nullptr;
    }
  }

  return *this;
}

int Encoding::CommandLineArguments::argc() const
{
  return static_cast<int>(this->argv_.size() - 1);
}

char const* const* Encoding::CommandLineArguments::argv() const
{
  return &this->argv_[0];
}

std::wstring Encoding::ToWide(std::string const& str)
{
  std::wstring wstr;
#if defined(_WIN32)
  int const wlength =
    MultiByteToWideChar(KWSYS_ENCODING_DEFAULT_CODEPAGE, 0, str.data(),
                        int(str.size()), nullptr, 0);
  if (wlength > 0) {
    wstr.resize(wlength);
    int r = MultiByteToWideChar(KWSYS_ENCODING_DEFAULT_CODEPAGE, 0, str.data(),
                                int(str.size()), &wstr[0], wlength);
    wstr.resize(static_cast<size_t>((std::max)(0, r)));
  }
#else
  size_t pos = 0;
  size_t nullPos = 0;
  do {
    if (pos < str.size() && str.at(pos) != '\0') {
      wstr += ToWide(str.c_str() + pos);
    }
    nullPos = str.find('\0', pos);
    if (nullPos != std::string::npos) {
      pos = nullPos + 1;
      wstr += wchar_t('\0');
    }
  } while (nullPos != std::string::npos);
#endif
  return wstr;
}

std::string Encoding::ToNarrow(std::wstring const& str)
{
  std::string nstr;
#if defined(_WIN32)
  int length =
    WideCharToMultiByte(KWSYS_ENCODING_DEFAULT_CODEPAGE, 0, str.c_str(),
                        int(str.size()), nullptr, 0, nullptr, nullptr);
  if (length > 0) {
    nstr.resize(length);
    int r =
      WideCharToMultiByte(KWSYS_ENCODING_DEFAULT_CODEPAGE, 0, str.c_str(),
                          int(str.size()), &nstr[0], length, nullptr, nullptr);
    nstr.resize(static_cast<size_t>((std::max)(0, r)));
  }
#else
  size_t pos = 0;
  size_t nullPos = 0;
  do {
    if (pos < str.size() && str.at(pos) != '\0') {
      nstr += ToNarrow(str.c_str() + pos);
    }
    nullPos = str.find(wchar_t('\0'), pos);
    if (nullPos != std::string::npos) {
      pos = nullPos + 1;
      nstr += '\0';
    }
  } while (nullPos != std::string::npos);
#endif
  return nstr;
}

std::wstring Encoding::ToWide(char const* cstr)
{
  std::wstring wstr;
  size_t length = kwsysEncoding_mbstowcs(nullptr, cstr, 0);
  if (length == 0 || length == static_cast<size_t>(-1)) {
    return wstr;
  }
  ++length;
  wstr.resize(length);
  length = kwsysEncoding_mbstowcs(&wstr[0], cstr, length);
  wstr.resize(length);
  return wstr;
}

std::string Encoding::ToNarrow(wchar_t const* wcstr)
{
  std::string str;
  size_t length = kwsysEncoding_wcstombs(nullptr, wcstr, 0);
  if (length == 0 || length == static_cast<size_t>(-1)) {
    return str;
  }
  ++length;
  str.resize(length);
  length = kwsysEncoding_wcstombs(&str[0], wcstr, length);
  str.resize(length);
  return str;
}

#if defined(_WIN32)
// Convert local paths to UNC style paths:
// * https://web.archive.org/web/20250329050419/
//   https://cpp.arsenmk.com/2015/12/handling-long-paths-on-windows.html

namespace {
// UNC prefix for local paths.
std::wstring const unc_local_prefix = L"\\\\?\\";

// UNC prefix for remote paths.
std::wstring const unc_remote_prefix = L"\\\\?\\UNC\\";

// The longest UNC prefix replaces two leading backslashes.
size_t const unc_max_space = unc_remote_prefix.length() - 2;
}

std::wstring Encoding::ToWindowsExtendedPath(std::string const& source)
{
  return ToWindowsExtendedPath(ToWide(source));
}

std::wstring Encoding::ToWindowsExtendedPath(char const* source)
{
  return ToWindowsExtendedPath(ToWide(source));
}

std::wstring Encoding::ToWindowsExtendedPath(std::wstring const& wsource)
{
  // Resolve any relative paths
  std::wstring wfull;
  DWORD wfull_len;

  // Try with a stack-allocated buffer first.
  wchar_t local_buf[512];
  size_t const local_buf_len = sizeof(local_buf) / sizeof(local_buf[0]);
  wfull_len =
    GetFullPathNameW(wsource.c_str(), local_buf_len, local_buf, nullptr);
  if (wfull_len <= local_buf_len) {
    // The stack-allocated buffer was large enough.  It holds the result.

    // Reserve room for the prefix added below.
    wfull.reserve(wfull_len + unc_max_space);

    // Store the absolute path to be returned.
    wfull.assign(local_buf, wfull_len);
  } else {
    // The stack-allocated buffer was too small, but now we know how
    // much to allocate on the heap.

    // Some versions of GetFullPathNameW return a slightly too-small size.
    wfull_len += 3;

    // Reserve room for the prefix added below.
    wfull.reserve(wfull_len + unc_max_space);

    // Try again with a heap-allocated buffer.
    wfull.resize(wfull_len, L'\0');
    wfull_len =
      GetFullPathNameW(wsource.c_str(), wfull_len, &wfull[0], nullptr);
    wfull.resize(wfull_len);
  }

  if (wfull_len >= 2 && kwsysString_isalpha(wfull[0]) &&
      wfull[1] == L':') { /* C:\Foo\bar\FooBar.txt */
    wfull.insert(0, unc_local_prefix);
    return wfull;
  } else if (wfull_len >= 2 && wfull[0] == L'\\' &&
             wfull[1] == L'\\') { /* Starts with \\ */
    if (wfull_len >= 4 && wfull[2] == L'?' &&
        wfull[3] == L'\\') { /* Starts with \\?\ */
      if (wfull_len >= 8 && wfull[4] == L'U' && wfull[5] == L'N' &&
          wfull[6] == L'C' &&
          wfull[7] == L'\\') { /* \\?\UNC\Foo\bar\FooBar.txt */
        return wfull;
      } else if (wfull_len >= 6 && kwsysString_isalpha(wfull[4]) &&
                 wfull[5] == L':') { /* \\?\C:\Foo\bar\FooBar.txt */
        return wfull;
      } else if (wfull_len >= 5) { /* \\?\Foo\bar\FooBar.txt */
        wfull.replace(0, 4, unc_remote_prefix);
        return wfull;
      }
    } else if (wfull_len >= 4 && wfull[2] == L'.' &&
               wfull[3] == L'\\') { /* Starts with \\.\ a device name */
      if (wfull_len >= 6 && kwsysString_isalpha(wfull[4]) &&
          wfull[5] == L':') { /* \\.\C:\Foo\bar\FooBar.txt */
        wfull.replace(0, 4, unc_local_prefix);
        return wfull;
      } else if (wfull_len >=
                 5) { /* \\.\Foo\bar\ Device name is left unchanged */
        return wfull;
      }
    } else if (wfull_len >= 3) { /* \\Foo\bar\FooBar.txt */
      wfull.replace(0, 2, unc_remote_prefix);
      return wfull;
    }
  }

  // If this case has been reached, then the path is invalid.  Leave it
  // unchanged
  return wsource;
}
#endif

} // namespace KWSYS_NAMESPACE
