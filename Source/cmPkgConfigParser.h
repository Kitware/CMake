/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <vector>

#include <cm/string_view>

#include <cmllpkgc/llpkgc.h>

struct cmPkgConfigValueElement
{

  cmPkgConfigValueElement() = default;

  cmPkgConfigValueElement(bool isVariable, cm::string_view data);

  bool IsVariable;
  cm::string_view Data;
};

struct cmPkgConfigEntry
{

  cmPkgConfigEntry() = default;

  cmPkgConfigEntry(bool isVariable, cm::string_view key);

  bool IsVariable;
  cm::string_view Key;
  std::vector<cmPkgConfigValueElement> Val;
};

class cmPkgConfigParser : llpkgc_t
{
public:
  cmPkgConfigParser();

  llpkgc_errno_t Parse(char* buf, std::size_t len);

  llpkgc_errno_t Finish();
  llpkgc_errno_t Finish(char* buf, std::size_t len);

  std::vector<cmPkgConfigEntry>& Data() { return Data_; }

private:
  int OnSpanNext(const char*, std::size_t len);
  static int OnSpanNextTr(llpkgc_t* parser, const char* at, std::size_t len);

  int OnKey(const char* at, std::size_t len);
  static int OnKeyTr(llpkgc_t* parser, const char* at, std::size_t len);

  int OnKeywordComplete();
  static int OnKeywordCompleteTr(llpkgc_t* parser);

  int OnVariableComplete();
  static int OnVariableCompleteTr(llpkgc_t* parser);

  int OnValueLiteral(const char* at, std::size_t len);
  static int OnValueLiteralTr(llpkgc_t* parser, const char* at,
                              std::size_t len);

  int OnValueLiteralComplete();
  static int OnValueLiteralCompleteTr(llpkgc_t* parser);

  int OnValueVariable(const char* at, std::size_t len);
  static int OnValueVariableTr(llpkgc_t* parser, const char* at,
                               std::size_t len);

  int OnValueVariableComplete();
  static int OnValueVariableCompleteTr(llpkgc_t* parser);

  llpkgc_settings_t Settings_{
    OnKeyTr,
    OnValueLiteralTr,
    OnValueVariableTr,
    nullptr, // on_line_begin
    OnKeywordCompleteTr,
    OnVariableCompleteTr,
    OnValueLiteralCompleteTr,
    OnValueVariableCompleteTr,
    nullptr, // on_value_complete
    nullptr, // on_pkgc_complete
  };

  const char* Ptr_;
  std::size_t Len_;
  std::vector<cmPkgConfigEntry> Data_;
};
