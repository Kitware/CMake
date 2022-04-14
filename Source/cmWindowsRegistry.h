/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

class cmMakefile;

class cmWindowsRegistry
{
public:
  cmWindowsRegistry(cmMakefile&);

  enum class View
  {
    Both,
    Target,
    Host,
    Reg64_32,
    Reg32_64,
    Reg32,
    Reg64
  };

  cm::optional<std::string> ReadValue(cm::string_view key,
                                      View view = View::Both,
                                      cm::string_view separator = "\0"_s)
  {
    return this->ReadValue(key, ""_s, view, separator);
  }
  cm::optional<std::string> ReadValue(cm::string_view key,
                                      cm::string_view name,
                                      View view = View::Both,
                                      cm::string_view separator = "\0"_s);

  cm::optional<std::vector<std::string>> GetValueNames(cm::string_view key,
                                                       View view = View::Both);
  cm::optional<std::vector<std::string>> GetSubKeys(cm::string_view key,
                                                    View view = View::Both);

  cm::string_view GetLastError() const;

private:
#if defined(_WIN32) && !defined(__CYGWIN__)
  std::vector<View> ComputeViews(View view);

  int TargetSize = 0;
#endif
  std::string LastError;
};
