/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <cstdint> // IWYU pragma: keep
#include <string>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/enum_set>
#include <cmext/string_view>

class cmMakefile;

class cmWindowsRegistry
{
public:
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

  // Registry supported types
  enum class ValueType : std::uint8_t
  {
    Reg_SZ,
    Reg_EXPAND_SZ,
    Reg_MULTI_SZ,
    Reg_DWORD,
    Reg_QWORD
  };
  using ValueTypeSet = cm::enum_set<ValueType>;

  // All types as defined by enum ValueType
  static const ValueTypeSet AllTypes;
  // same as AllTYpes but without type REG_MULTI_SZ
  static const ValueTypeSet SimpleTypes;

  cmWindowsRegistry(cmMakefile&,
                    const ValueTypeSet& supportedTypes = AllTypes);

  // Helper routine to convert string to enum value
  static cm::optional<View> ToView(cm::string_view name);
  // Helper routine to convert enum to string
  static cm::string_view FromView(View view);

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

  // Expand an expression which may contains multiple references
  // to registry keys.
  // Depending of the view specified, one or two expansions can be done.
  cm::optional<std::vector<std::string>> ExpandExpression(
    cm::string_view expression, View view, cm::string_view separator = "\0"_s);

  cm::string_view GetLastError() const;

private:
#if defined(_WIN32) && !defined(__CYGWIN__)
  std::vector<View> ComputeViews(View view);

  int TargetSize = 0;
  ValueTypeSet SupportedTypes = AllTypes;
#endif
  std::string LastError;
};
