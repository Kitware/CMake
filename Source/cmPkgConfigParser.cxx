/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmPkgConfigParser.h"

#include <cstddef>
#include <vector>

#include <cm/string_view>

#include <cmllpkgc/llpkgc.h>

cmPkgConfigValueElement::cmPkgConfigValueElement(bool isVariable,
                                                 cm::string_view data)
  : IsVariable{ isVariable }
  , Data{ data }
{
}

cmPkgConfigEntry::cmPkgConfigEntry(bool isVariable, cm::string_view key)
  : IsVariable{ isVariable }
  , Key{ key }
{
}

cmPkgConfigParser::cmPkgConfigParser()
{
  llpkgc_init(static_cast<llpkgc_t*>(this), &Settings_);
}

llpkgc_errno_t cmPkgConfigParser::Parse(char* buf, std::size_t len)
{
  return llpkgc_execute(static_cast<llpkgc_t*>(this), buf, len);
}

llpkgc_errno_t cmPkgConfigParser::Finish()
{
  return llpkgc_finish(static_cast<llpkgc_t*>(this));
}

llpkgc_errno_t cmPkgConfigParser::Finish(char* buf, std::size_t len)
{
  Parse(buf, len);
  return llpkgc_finish(static_cast<llpkgc_t*>(this));
}

int cmPkgConfigParser::OnSpanNext(const char*, std::size_t len)
{
  Len_ += len;
  return 0;
}

int cmPkgConfigParser::OnSpanNextTr(llpkgc_t* parser, const char* at,
                                    std::size_t len)
{
  return static_cast<cmPkgConfigParser*>(parser)->OnSpanNext(at, len);
}

int cmPkgConfigParser::OnKey(const char* at, std::size_t len)
{
  Ptr_ = at;
  Len_ = len;
  Settings_.on_key = OnSpanNextTr;
  return 0;
}

int cmPkgConfigParser::OnKeyTr(llpkgc_t* parser, const char* at,
                               std::size_t len)
{
  return static_cast<cmPkgConfigParser*>(parser)->OnKey(at, len);
}

int cmPkgConfigParser::OnKeywordComplete()
{
  Data_.emplace_back(false, cm::string_view{ Ptr_, Len_ });
  Settings_.on_key = OnKeyTr;
  return 0;
}

int cmPkgConfigParser::OnKeywordCompleteTr(llpkgc_t* parser)
{
  return static_cast<cmPkgConfigParser*>(parser)->OnKeywordComplete();
}

int cmPkgConfigParser::OnVariableComplete()
{
  Data_.emplace_back(true, cm::string_view{ Ptr_, Len_ });
  Settings_.on_key = OnKeyTr;
  return 0;
}

int cmPkgConfigParser::OnVariableCompleteTr(llpkgc_t* parser)
{
  return static_cast<cmPkgConfigParser*>(parser)->OnVariableComplete();
}

int cmPkgConfigParser::OnValueLiteral(const char* at, std::size_t len)
{
  Ptr_ = at;
  Len_ = len;
  Settings_.on_value_literal = OnSpanNextTr;
  return 0;
}

int cmPkgConfigParser::OnValueLiteralTr(llpkgc_t* parser, const char* at,
                                        std::size_t len)
{
  return static_cast<cmPkgConfigParser*>(parser)->OnValueLiteral(at, len);
}

int cmPkgConfigParser::OnValueLiteralComplete()
{
  Settings_.on_value_literal = OnValueLiteralTr;

  if (Len_) {
    Data_.back().Val.emplace_back(false, cm::string_view{ Ptr_, Len_ });
  }

  return 0;
}

int cmPkgConfigParser::OnValueLiteralCompleteTr(llpkgc_t* parser)
{
  return static_cast<cmPkgConfigParser*>(parser)->OnValueLiteralComplete();
}

int cmPkgConfigParser::OnValueVariable(const char* at, std::size_t len)
{
  Ptr_ = at;
  Len_ = len;
  Settings_.on_value_variable = OnSpanNextTr;
  return 0;
}

int cmPkgConfigParser::OnValueVariableTr(llpkgc_t* parser, const char* at,
                                         std::size_t len)
{
  return static_cast<cmPkgConfigParser*>(parser)->OnValueVariable(at, len);
}

int cmPkgConfigParser::OnValueVariableComplete()
{
  Settings_.on_value_variable = OnValueVariableTr;
  Data_.back().Val.emplace_back(true, cm::string_view{ Ptr_, Len_ });
  return 0;
}

int cmPkgConfigParser::OnValueVariableCompleteTr(llpkgc_t* parser)
{
  return static_cast<cmPkgConfigParser*>(parser)->OnValueVariableComplete();
}
