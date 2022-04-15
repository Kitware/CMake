/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmWindowsRegistry.h"

#include <unordered_map>

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <algorithm>
#  include <cstdint>
#  include <exception>
#  include <iterator>
#  include <utility>
#  include <vector>

#  include <cm/memory>
#  include <cmext/string_view>

#  include <windows.h>

#  include "cmsys/Encoding.hxx"
#  include "cmsys/SystemTools.hxx"

#  include "cmMakefile.h"
#  include "cmStringAlgorithms.h"
#  include "cmValue.h"
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
namespace {
bool Is64BitWindows()
{
#  if defined(_WIN64)
  // 64-bit programs run only on Win64
  return true;
#  else
  // 32-bit programs run on both 32-bit and 64-bit Windows, so we must check.
  BOOL isWow64 = false;
  return IsWow64Process(GetCurrentProcess(), &isWow64) && isWow64;
#  endif
}

// class registry_exception
class registry_error : public std::exception
{
public:
  registry_error(std::string msg)
    : What(std::move(msg))
  {
  }
  ~registry_error() override = default;

  const char* what() const noexcept override { return What.c_str(); }

private:
  std::string What;
};

// Class KeyHandler
class KeyHandler
{
public:
  using View = cmWindowsRegistry::View;

  KeyHandler(HKEY hkey)
    : Handler(hkey)
  {
  }
  ~KeyHandler() { RegCloseKey(this->Handler); }

  static KeyHandler OpenKey(cm::string_view key, View view);

  std::string ReadValue(cm::string_view name, cm::string_view separator);

  std::vector<std::string> GetValueNames();
  std::vector<std::string> GetSubKeys();

private:
  static std::string FormatSystemError(LSTATUS status);
  static std::wstring ToWide(cm::string_view str);
  static std::string ToNarrow(const wchar_t* str, int size = -1);

  HKEY Handler;
};

KeyHandler KeyHandler::OpenKey(cm::string_view key, View view)
{
  if (view == View::Reg64 && !Is64BitWindows()) {
    throw registry_error("No 64bit registry on Windows32.");
  }

  auto start = key.find_first_of("\\/"_s);
  auto rootKey = key.substr(0, start);
  HKEY hRootKey;

  if (rootKey == "HKCU"_s || rootKey == "HKEY_CURRENT_USER"_s) {
    hRootKey = HKEY_CURRENT_USER;
  } else if (rootKey == "HKLM"_s || rootKey == "HKEY_LOCAL_MACHINE"_s) {
    hRootKey = HKEY_LOCAL_MACHINE;
  } else if (rootKey == "HKCR"_s || rootKey == "HKEY_CLASSES_ROOT"_s) {
    hRootKey = HKEY_CLASSES_ROOT;
  } else if (rootKey == "HKCC"_s || rootKey == "HKEY_CURRENT_CONFIG"_s) {
    hRootKey = HKEY_CURRENT_CONFIG;
  } else if (rootKey == "HKU"_s || rootKey == "HKEY_USERS"_s) {
    hRootKey = HKEY_USERS;
  } else {
    throw registry_error(cmStrCat(rootKey, ": invalid root key."));
  }
  std::wstring subKey;
  if (start != cm::string_view::npos) {
    subKey = ToWide(key.substr(start + 1));
  }
  // Update path format
  std::replace(subKey.begin(), subKey.end(), L'/', L'\\');

  REGSAM options = KEY_READ;
  if (Is64BitWindows()) {
    options |= view == View::Reg64 ? KEY_WOW64_64KEY : KEY_WOW64_32KEY;
  }

  HKEY hKey;
  if (LSTATUS status = RegOpenKeyExW(hRootKey, subKey.c_str(), 0, options,
                                     &hKey) != ERROR_SUCCESS) {
    throw registry_error(FormatSystemError(status));
  }

  return KeyHandler(hKey);
}

std::string KeyHandler::FormatSystemError(LSTATUS status)
{
  std::string formattedMessage{ "Windows Registry: unexpected error." };
  LPWSTR message = nullptr;
  DWORD size = 1024;
  if (FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr,
        status, 0, reinterpret_cast<LPWSTR>(&message), size, nullptr) != 0) {
    try {
      formattedMessage = cmTrimWhitespace(ToNarrow(message));
    } catch (...) {
      // ignore any exception because this method can be called
      // as part of the raise of an exception
    }
  }
  LocalFree(message);

  return formattedMessage;
}

std::wstring KeyHandler::ToWide(cm::string_view str)
{
  std::wstring wstr;

  if (str.empty()) {
    return wstr;
  }

  const auto wlength =
    MultiByteToWideChar(KWSYS_ENCODING_DEFAULT_CODEPAGE, 0, str.data(),
                        int(str.size()), nullptr, 0);
  if (wlength > 0) {
    auto wdata = cm::make_unique<wchar_t[]>(wlength);
    const auto r =
      MultiByteToWideChar(KWSYS_ENCODING_DEFAULT_CODEPAGE, 0, str.data(),
                          int(str.size()), wdata.get(), wlength);
    if (r > 0) {
      wstr = std::wstring(wdata.get(), wlength);
    } else {
      throw registry_error(FormatSystemError(GetLastError()));
    }
  } else {
    throw registry_error(FormatSystemError(GetLastError()));
  }

  return wstr;
}

std::string KeyHandler::ToNarrow(const wchar_t* wstr, int size)
{
  std::string str;

  if (size == 0 || (size == -1 && wstr[0] == L'\0')) {
    return str;
  }

  const auto length =
    WideCharToMultiByte(KWSYS_ENCODING_DEFAULT_CODEPAGE, 0, wstr, size,
                        nullptr, 0, nullptr, nullptr);
  if (length > 0) {
    auto data = cm::make_unique<char[]>(length);
    const auto r =
      WideCharToMultiByte(KWSYS_ENCODING_DEFAULT_CODEPAGE, 0, wstr, size,
                          data.get(), length, nullptr, nullptr);
    if (r > 0) {
      if (size == -1) {
        str = std::string(data.get());
      } else {
        str = std::string(data.get(), length);
      }
    } else {
      throw registry_error(FormatSystemError(GetLastError()));
    }
  } else {
    throw registry_error(FormatSystemError(GetLastError()));
  }

  return str;
}

std::string KeyHandler::ReadValue(cm::string_view name,
                                  cm::string_view separator)
{
  LSTATUS status;
  DWORD size;
  // pick-up maximum size for value
  if ((status = RegQueryInfoKeyW(this->Handler, nullptr, nullptr, nullptr,
                                 nullptr, nullptr, nullptr, nullptr, nullptr,
                                 &size, nullptr, nullptr)) != ERROR_SUCCESS) {
    throw registry_error(this->FormatSystemError(status));
  }
  auto data = cm::make_unique<BYTE[]>(size);
  DWORD type;
  auto valueName = this->ToWide(name);
  if ((status = RegQueryValueExW(this->Handler, valueName.c_str(), nullptr,
                                 &type, data.get(), &size)) != ERROR_SUCCESS) {
    throw registry_error(this->FormatSystemError(status));
  }
  switch (type) {
    case REG_SZ:
      return this->ToNarrow(reinterpret_cast<wchar_t*>(data.get()));
      break;
    case REG_EXPAND_SZ: {
      auto expandSize = ExpandEnvironmentStringsW(
        reinterpret_cast<wchar_t*>(data.get()), nullptr, 0);
      auto expandData = cm::make_unique<wchar_t[]>(expandSize + 1);
      if (ExpandEnvironmentStringsW(reinterpret_cast<wchar_t*>(data.get()),
                                    expandData.get(), expandSize + 1) == 0) {
        throw registry_error(this->FormatSystemError(GetLastError()));
      } else {
        return this->ToNarrow(expandData.get());
      }
    } break;
    case REG_DWORD:
      return std::to_string(*reinterpret_cast<std::uint32_t*>(data.get()));
      break;
    case REG_QWORD:
      return std::to_string(*reinterpret_cast<std::uint64_t*>(data.get()));
      break;
    case REG_MULTI_SZ: {
      // replace separator with semicolon
      auto sep = this->ToWide(separator)[0];
      std::replace(reinterpret_cast<wchar_t*>(data.get()),
                   reinterpret_cast<wchar_t*>(data.get()) +
                     (size / sizeof(wchar_t)) - 1,
                   sep, L';');
      return this->ToNarrow(reinterpret_cast<wchar_t*>(data.get()));
    } break;
    default:
      throw registry_error(cmStrCat(type, ": unsupported type."));
  }
}

std::vector<std::string> KeyHandler::GetValueNames()
{
  LSTATUS status;
  DWORD maxSize;
  // pick-up maximum size for value names
  if ((status = RegQueryInfoKeyW(
         this->Handler, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
         nullptr, &maxSize, nullptr, nullptr, nullptr)) != ERROR_SUCCESS) {
    throw registry_error(this->FormatSystemError(status));
  }
  // increment size for final null
  auto data = cm::make_unique<wchar_t[]>(++maxSize);
  DWORD index = 0;
  DWORD size = maxSize;

  std::vector<std::string> valueNames;

  while ((status = RegEnumValueW(this->Handler, index, data.get(), &size,
                                 nullptr, nullptr, nullptr, nullptr)) ==
         ERROR_SUCCESS) {
    auto name = this->ToNarrow(data.get());
    valueNames.push_back(name.empty() ? "(default)" : name);
    size = maxSize;
    ++index;
  }

  if (status != ERROR_NO_MORE_ITEMS) {
    throw registry_error(this->FormatSystemError(status));
  }

  return valueNames;
}

std::vector<std::string> KeyHandler::GetSubKeys()
{
  LSTATUS status;
  DWORD size;
  // pick-up maximum size for subkeys
  if ((status = RegQueryInfoKeyW(
         this->Handler, nullptr, nullptr, nullptr, nullptr, &size, nullptr,
         nullptr, nullptr, nullptr, nullptr, nullptr)) != ERROR_SUCCESS) {
    throw registry_error(this->FormatSystemError(status));
  }
  // increment size for final null
  auto data = cm::make_unique<wchar_t[]>(++size);
  DWORD index = 0;
  std::vector<std::string> subKeys;

  while ((status = RegEnumKeyW(this->Handler, index, data.get(), size)) ==
         ERROR_SUCCESS) {
    subKeys.push_back(this->ToNarrow(data.get()));
    ++index;
  }
  if (status != ERROR_NO_MORE_ITEMS) {
    throw registry_error(this->FormatSystemError(status));
  }

  return subKeys;
}
}
#endif

// class cmWindowsRegistry
cmWindowsRegistry::cmWindowsRegistry(cmMakefile& makefile)
#if !defined(_WIN32) || defined(__CYGWIN__)
  : LastError("No Registry on this platform.")
#endif
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  if (cmValue targetSize = makefile.GetDefinition("CMAKE_SIZEOF_VOID_P")) {
    this->TargetSize = targetSize == "8" ? 64 : 32;
  }
#else
  (void)makefile;
#endif
}

cm::optional<cmWindowsRegistry::View> cmWindowsRegistry::ToView(
  cm::string_view name)
{
  static std::unordered_map<cm::string_view, cmWindowsRegistry::View>
    ViewDefinitions{
      { "BOTH"_s, View::Both },     { "HOST"_s, View::Host },
      { "TARGET"_s, View::Target }, { "32"_s, View::Reg32 },
      { "64"_s, View::Reg64 },      { "32_64"_s, View::Reg32_64 },
      { "64_32"_s, View::Reg64_32 }
    };

  auto it = ViewDefinitions.find(name);

  return it == ViewDefinitions.end() ? cm::nullopt
                                     : cm::optional{ it->second };
}

cm::string_view cmWindowsRegistry::FromView(View view)
{
  static std::unordered_map<cmWindowsRegistry::View, cm::string_view>
    ViewDefinitions{
      { View::Both, "BOTH"_s },     { View::Host, "HOST"_s },
      { View::Target, "TARGET"_s }, { View::Reg32, "32"_s },
      { View::Reg64, "64"_s },      { View::Reg32_64, "32_64"_s },
      { View::Reg64_32, "64_32"_s }
    };

  auto it = ViewDefinitions.find(view);

  return it == ViewDefinitions.end() ? ""_s : it->second;
}

cm::string_view cmWindowsRegistry::GetLastError() const
{
  return this->LastError;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
std::vector<cmWindowsRegistry::View> cmWindowsRegistry::ComputeViews(View view)
{
  switch (view) {
    case View::Both:
      switch (this->TargetSize) {
        case 64:
          return std::vector<View>{ View::Reg64, View::Reg32 };
          break;
        case 32:
          return Is64BitWindows()
            ? std::vector<View>{ View::Reg32, View::Reg64 }
            : std::vector<View>{ View::Reg32 };
          break;
        default:
          // No language specified, fallback to host architecture
          return Is64BitWindows()
            ? std::vector<View>{ View::Reg64, View::Reg32 }
            : std::vector<View>{ View::Reg32 };
          break;
      }
      break;
    case View::Target:
      switch (this->TargetSize) {
        case 64:
          return std::vector<View>{ View::Reg64 };
          break;
        case 32:
          return std::vector<View>{ View::Reg32 };
          break;
        default:
          break;
      }
      CM_FALLTHROUGH;
    case View::Host:
      return std::vector<View>{ Is64BitWindows() ? View::Reg64 : View::Reg32 };
      break;
    case View::Reg64_32:
      return Is64BitWindows() ? std::vector<View>{ View::Reg64, View::Reg32 }
                              : std::vector<View>{ View::Reg32 };
      break;
    case View::Reg32_64:
      return Is64BitWindows() ? std::vector<View>{ View::Reg32, View::Reg64 }
                              : std::vector<View>{ View::Reg32 };
      break;
    default:
      return std::vector<View>{ view };
      break;
  }
}
#endif

cm::optional<std::string> cmWindowsRegistry::ReadValue(
  cm::string_view key, cm::string_view name, View view,
  cm::string_view separator)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  // compute list of registry views
  auto views = this->ComputeViews(view);

  if (cmsys::SystemTools::Strucmp(name.data(), "(default)") == 0) {
    // handle magic name for default value
    name = ""_s;
  }
  if (separator.empty()) {
    separator = "\0"_s;
  }

  for (auto v : views) {
    try {
      this->LastError.clear();
      auto handler = KeyHandler::OpenKey(key, v);
      return handler.ReadValue(name, separator);
    } catch (const registry_error& e) {
      this->LastError = e.what();
      continue;
    }
  }
#else
  (void)key;
  (void)name;
  (void)view;
  (void)separator;
#endif
  return cm::nullopt;
}

cm::optional<std::vector<std::string>> cmWindowsRegistry::GetValueNames(
  cm::string_view key, View view)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  this->LastError.clear();
  // compute list of registry views
  auto views = this->ComputeViews(view);
  std::vector<std::string> valueNames;
  bool querySuccessful = false;

  for (auto v : views) {
    try {
      auto handler = KeyHandler::OpenKey(key, v);
      auto list = handler.GetValueNames();
      std::move(list.begin(), list.end(), std::back_inserter(valueNames));
      querySuccessful = true;
    } catch (const registry_error& e) {
      this->LastError = e.what();
      continue;
    }
  }
  if (!valueNames.empty()) {
    // value names must be unique and sorted
    std::sort(valueNames.begin(), valueNames.end());
    valueNames.erase(std::unique(valueNames.begin(), valueNames.end()),
                     valueNames.end());
  }

  if (querySuccessful) {
    // At least one query was successful, so clean-up any error message
    this->LastError.clear();
    return valueNames;
  }
#else
  (void)key;
  (void)view;
#endif
  return cm::nullopt;
}

cm::optional<std::vector<std::string>> cmWindowsRegistry::GetSubKeys(
  cm::string_view key, View view)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  this->LastError.clear();
  // compute list of registry views
  auto views = this->ComputeViews(view);
  std::vector<std::string> subKeys;
  bool querySuccessful = false;

  for (auto v : views) {
    try {
      auto handler = KeyHandler::OpenKey(key, v);
      auto list = handler.GetSubKeys();
      std::move(list.begin(), list.end(), std::back_inserter(subKeys));
      querySuccessful = true;
    } catch (const registry_error& e) {
      this->LastError = e.what();
      continue;
    }
  }
  if (!subKeys.empty()) {
    // keys must be unique and sorted
    std::sort(subKeys.begin(), subKeys.end());
    subKeys.erase(std::unique(subKeys.begin(), subKeys.end()), subKeys.end());
  }

  if (querySuccessful) {
    // At least one query was successful, so clean-up any error message
    this->LastError.clear();
    return subKeys;
  }
#else
  (void)key;
  (void)view;
#endif
  return cm::nullopt;
}
