/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmExprParserHelper.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "cmExprLexer.h"
#include "cmStringAlgorithms.h"

int cmExpr_yyparse(yyscan_t yyscanner);
//
cmExprParserHelper::cmExprParserHelper()
{
  this->FileLine = -1;
  this->FileName = nullptr;
  this->Result = 0;
}

cmExprParserHelper::~cmExprParserHelper() = default;

int cmExprParserHelper::ParseString(char const* str, int verb)
{
  if (!str) {
    return 0;
  }
  // printf("Do some parsing: %s\n", str);

  this->Verbose = verb;
  this->InputBuffer = str;
  this->InputBufferPos = 0;
  this->CurrentLine = 0;

  this->Result = 0;

  yyscan_t yyscanner;
  cmExpr_yylex_init(&yyscanner);
  cmExpr_yyset_extra(this, yyscanner);

  try {
    int res = cmExpr_yyparse(yyscanner);
    if (res != 0) {
      std::string e =
        cmStrCat("cannot parse the expression: \"", this->InputBuffer,
                 "\": ", this->ErrorString, '.');
      this->SetError(std::move(e));
    }
  } catch (std::runtime_error const& fail) {
    std::string e = cmStrCat("cannot evaluate the expression: \"",
                             this->InputBuffer, "\": ", fail.what(), '.');
    this->SetError(std::move(e));
  } catch (std::out_of_range const&) {
    std::string e =
      cmStrCat("cannot evaluate the expression: \"", this->InputBuffer,
               "\": a numeric value is out of range.");
    this->SetError(std::move(e));
  } catch (...) {
    std::string e =
      cmStrCat("cannot parse the expression: \"", this->InputBuffer, "\".");
    this->SetError(std::move(e));
  }
  cmExpr_yylex_destroy(yyscanner);
  if (!this->ErrorString.empty()) {
    return 0;
  }

  if (this->Verbose) {
    std::cerr << "Expanding [" << str << "] produced: [" << this->Result << "]"
              << std::endl;
  }
  return 1;
}

int cmExprParserHelper::LexInput(char* buf, int maxlen)
{
  // std::cout << "JPLexInput ";
  // std::cout.write(buf, maxlen);
  // std::cout << std::endl;
  if (maxlen < 1) {
    return 0;
  }
  if (this->InputBufferPos < this->InputBuffer.size()) {
    buf[0] = this->InputBuffer[this->InputBufferPos++];
    if (buf[0] == '\n') {
      this->CurrentLine++;
    }
    return (1);
  }
  buf[0] = '\n';
  return (0);
}

void cmExprParserHelper::Error(char const* str)
{
  unsigned long pos = static_cast<unsigned long>(this->InputBufferPos);
  std::ostringstream ostr;
  ostr << str << " (" << pos << ")";
  this->ErrorString = ostr.str();
}

void cmExprParserHelper::Warning(std::string str)
{
  this->WarningString = cmStrCat(this->WarningString, std::move(str), '\n');
}

void cmExprParserHelper::UnexpectedChar(char c)
{
  unsigned long pos = static_cast<unsigned long>(this->InputBufferPos);
  std::ostringstream ostr;
  ostr << "Unexpected character in expression at position " << pos << ": " << c
       << "\n";
  this->WarningString += ostr.str();
}

void cmExprParserHelper::SetResult(std::int64_t value)
{
  this->Result = value;
}

void cmExprParserHelper::SetError(std::string errorString)
{
  this->ErrorString = std::move(errorString);
}

std::int64_t cmExprParserHelper::ShL(std::int64_t l, std::int64_t r)
{
  if (l < 0) {
    this->Warning(
      cmStrCat("left shift of negative value in:\n  ", l, " << ", r));
  }
  if (r < 0) {
    this->Warning(
      cmStrCat("shift exponent is negative in:\n  ", l, " << ", r));
    r &= 0x3F;
  }
  if (r >= 64) {
    this->Warning(
      cmStrCat("shift exponent is too large in:\n  ", l, " << ", r));
    r &= 0x3F;
  }
  return static_cast<std::int64_t>(static_cast<std::uint64_t>(l) << r);
}

std::int64_t cmExprParserHelper::ShR(std::int64_t l, std::int64_t r)
{
  if (r < 0) {
    this->Warning(
      cmStrCat("shift exponent is negative in:\n  ", l, " >> ", r));
    r &= 0x3F;
  }
  if (r >= 64) {
    this->Warning(
      cmStrCat("shift exponent is too large in:\n  ", l, " >> ", r));
    r &= 0x3F;
  }
  return l >> r;
}

std::int64_t cmExprParserHelper::Add(std::int64_t l, std::int64_t r)
{
  std::int64_t sum;
  if (this->AddOverflow(l, r, &sum)) {
    this->Warning(cmStrCat("signed integer overflow in:\n  ", l, " + ", r));
  }
  return sum;
}

std::int64_t cmExprParserHelper::Sub(std::int64_t l, std::int64_t r)
{
  std::int64_t diff;
  if (this->SubOverflow(l, r, &diff)) {
    this->Warning(cmStrCat("signed integer overflow in:\n  ", l, " - ", r));
  }
  return diff;
}

std::int64_t cmExprParserHelper::Mul(std::int64_t l, std::int64_t r)
{
  std::int64_t prod;
  if (this->MulOverflow(l, r, &prod)) {
    this->Warning(cmStrCat("signed integer overflow in:\n  ", l, " * ", r));
  }
  return prod;
}

std::int64_t cmExprParserHelper::Div(std::int64_t l, std::int64_t r)
{
  if (r == 0) {
    throw std::overflow_error("divide by zero");
  }
  return l / r;
}

std::int64_t cmExprParserHelper::Mod(std::int64_t l, std::int64_t r)
{
  if (r == 0) {
    throw std::overflow_error("modulo by zero");
  }
  return l % r;
}

// The __has_builtin preprocessor check was added in Clang 2.6 and GCC 10.
// The __builtin_X_overflow intrinsics were added in Clang 3.4 and GCC 5.
#ifndef __has_builtin
#  if defined(__GNUC__) && __GNUC__ >= 5
#    define __has_builtin(x) 1
#  else
#    define __has_builtin(x) 0
#  endif
#endif

bool cmExprParserHelper::AddOverflow(long l, long r, long* p)
{
#if __has_builtin(__builtin_saddl_overflow)
  return __builtin_saddl_overflow(l, r, p);
#else
  *p = l + r;
  return false;
#endif
}

bool cmExprParserHelper::AddOverflow(long long l, long long r, long long* p)
{
#if __has_builtin(__builtin_saddll_overflow)
  return __builtin_saddll_overflow(l, r, p);
#else
  *p = l + r;
  return false;
#endif
}

bool cmExprParserHelper::SubOverflow(long l, long r, long* p)
{
#if __has_builtin(__builtin_ssubl_overflow)
  return __builtin_ssubl_overflow(l, r, p);
#else
  *p = l - r;
  return false;
#endif
}

bool cmExprParserHelper::SubOverflow(long long l, long long r, long long* p)
{
#if __has_builtin(__builtin_ssubll_overflow)
  return __builtin_ssubll_overflow(l, r, p);
#else
  *p = l - r;
  return false;
#endif
}

bool cmExprParserHelper::MulOverflow(long l, long r, long* p)
{
#if __has_builtin(__builtin_smull_overflow)
  return __builtin_smull_overflow(l, r, p);
#else
  *p = l * r;
  return false;
#endif
}

bool cmExprParserHelper::MulOverflow(long long l, long long r, long long* p)
{
#if __has_builtin(__builtin_smulll_overflow)
  return __builtin_smulll_overflow(l, r, p);
#else
  *p = l * r;
  return false;
#endif
}
