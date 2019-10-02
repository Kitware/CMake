/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmStringReplaceHelper_h
#define cmStringReplaceHelper_h

#include <string>
#include <utility>
#include <vector>

#include "cmsys/RegularExpression.hxx"

class cmMakefile;

class cmStringReplaceHelper
{
public:
  cmStringReplaceHelper(const std::string& regex, std::string replace_expr,
                        cmMakefile* makefile = nullptr);

  bool IsRegularExpressionValid() const
  {
    return this->RegularExpression.is_valid();
  }
  bool IsReplaceExpressionValid() const
  {
    return this->ValidReplaceExpression;
  }

  bool Replace(const std::string& input, std::string& output);

  const std::string& GetError() { return this->ErrorString; }

private:
  class RegexReplacement
  {
  public:
    RegexReplacement(const char* s)
      : Number(-1)
      , Value(s)
    {
    }
    RegexReplacement(std::string s)
      : Number(-1)
      , Value(std::move(s))
    {
    }
    RegexReplacement(int n)
      : Number(n)
    {
    }
    RegexReplacement() = default;

    int Number;
    std::string Value;
  };

  void ParseReplaceExpression();

  std::string ErrorString;
  std::string RegExString;
  cmsys::RegularExpression RegularExpression;
  bool ValidReplaceExpression = true;
  std::string ReplaceExpression;
  std::vector<RegexReplacement> Replacements;
  cmMakefile* Makefile = nullptr;
};

#endif
