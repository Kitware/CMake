/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmCMakePath.h"

#include <string>

#if defined(_WIN32)
#  include <cstdlib>
#endif

#include <cm/filesystem>
#include <cm/string_view>

#if defined(_WIN32)
#  include "cmStringAlgorithms.h"
#endif

cmCMakePath& cmCMakePath::ReplaceWideExtension(cm::string_view extension)
{
  auto file = this->Path.filename().string();
  if (!file.empty() && file != "." && file != "..") {
    auto pos = file.find('.', file[0] == '.' ? 1 : 0);
    if (pos != std::string::npos) {
      file.erase(pos);
    }
  }
  if (!extension.empty()) {
    if (extension[0] != '.') {
      file += '.';
    }
    file.append(std::string(extension));
  }
  this->Path.replace_filename(file);
  return *this;
}

cmCMakePath cmCMakePath::GetWideExtension() const
{
  auto file = this->Path.filename().string();
  if (file.empty() || file == "." || file == "..") {
    return cmCMakePath{};
  }

  auto pos = file.find('.', file[0] == '.' ? 1 : 0);
  if (pos != std::string::npos) {
    return cm::string_view(file.data() + pos, file.length() - pos);
  }

  return cmCMakePath{};
}

cmCMakePath cmCMakePath::GetNarrowStem() const
{
  auto stem = this->Path.stem().string();
  if (!stem.empty()) {
    auto pos = stem.find('.', stem[0] == '.' ? 1 : 0);
    if (pos != std::string::npos) {
      return stem.substr(0, pos);
    }
  }
  return stem;
}

cmCMakePath cmCMakePath::Absolute(const cm::filesystem::path& base) const
{
  if (this->Path.is_relative()) {
    auto path = base;
    path /= this->Path;
    // filesystem::path::operator/= use preferred_separator ('\' on Windows)
    // so converts back to '/'
    return path.generic_string();
  }
  return *this;
}

bool cmCMakePath::IsPrefix(const cmCMakePath& path) const
{
  auto prefix_it = this->Path.begin();
  auto prefix_end = this->Path.end();
  auto path_it = path.Path.begin();
  auto path_end = path.Path.end();

  while (prefix_it != prefix_end && path_it != path_end &&
         *prefix_it == *path_it) {
    ++prefix_it;
    ++path_it;
  }
  return (prefix_it == prefix_end) ||
    (prefix_it->empty() && path_it != path_end);
}

std::string cmCMakePath::FormatPath(std::string path, format fmt)
{
#if defined(_WIN32)
  if (fmt == auto_format || fmt == native_format) {
    auto prefix = path.substr(0, 4);
    for (auto& c : prefix) {
      if (c == '\\') {
        c = '/';
      }
    }
    // remove Windows long filename marker
    if (prefix == "//?/"_s) {
      path.erase(0, 4);
    }
    if (cmHasPrefix(path, "UNC/"_s) || cmHasPrefix(path, "UNC\\"_s)) {
      path.erase(0, 2);
      path[0] = '/';
    }
  }
#else
  static_cast<void>(fmt);
#endif
  return path;
}

void cmCMakePath::GetNativePath(std::string& path) const
{
  cm::filesystem::path tmp(this->Path);
  tmp.make_preferred();

  path = tmp.string();
}
void cmCMakePath::GetNativePath(std::wstring& path) const
{
  cm::filesystem::path tmp(this->Path);
  tmp.make_preferred();

  path = tmp.wstring();

#if defined(_WIN32)
  // Windows long filename
  static std::wstring UNC(L"\\\\?\\UNC");
  static std::wstring PREFIX(L"\\\\?\\");

  if (this->IsAbsolute() && path.length() > _MAX_PATH - 12) {
    if (this->HasRootName() && path[0] == L'\\') {
      path = UNC + path.substr(1);
    } else {
      path = PREFIX + path;
    }
  }
#endif
}
