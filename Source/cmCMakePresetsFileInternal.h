/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <memory>

#include "cmCMakePresetsFile.h"

#define CHECK_OK(expr)                                                        \
  do {                                                                        \
    auto _result = expr;                                                      \
    if (_result != ReadFileResult::READ_OK)                                   \
      return _result;                                                         \
  } while (false)

namespace cmCMakePresetsFileInternal {
enum class ExpandMacroResult
{
  Ok,
  Ignore,
  Error,
};

using MacroExpander = std::function<ExpandMacroResult(
  const std::string&, const std::string&, std::string&, int version)>;
}

class cmCMakePresetsFile::Condition
{
public:
  virtual ~Condition() = default;

  virtual bool Evaluate(
    const std::vector<cmCMakePresetsFileInternal::MacroExpander>& expanders,
    int version, cm::optional<bool>& out) const = 0;
  virtual bool IsNull() const { return false; }
};

namespace cmCMakePresetsFileInternal {

class NullCondition : public cmCMakePresetsFile::Condition
{
  bool Evaluate(const std::vector<MacroExpander>& /*expanders*/,
                int /*version*/, cm::optional<bool>& out) const override
  {
    out = true;
    return true;
  }

  bool IsNull() const override { return true; }
};

class ConstCondition : public cmCMakePresetsFile::Condition
{
public:
  bool Evaluate(const std::vector<MacroExpander>& /*expanders*/,
                int /*version*/, cm::optional<bool>& out) const override
  {
    out = this->Value;
    return true;
  }

  bool Value;
};

class EqualsCondition : public cmCMakePresetsFile::Condition
{
public:
  bool Evaluate(const std::vector<MacroExpander>& expanders, int version,
                cm::optional<bool>& out) const override;

  std::string Lhs;
  std::string Rhs;
};

class InListCondition : public cmCMakePresetsFile::Condition
{
public:
  bool Evaluate(const std::vector<MacroExpander>& expanders, int version,
                cm::optional<bool>& out) const override;

  std::string String;
  std::vector<std::string> List;
};

class MatchesCondition : public cmCMakePresetsFile::Condition
{
public:
  bool Evaluate(const std::vector<MacroExpander>& expanders, int version,
                cm::optional<bool>& out) const override;

  std::string String;
  std::string Regex;
};

class AnyAllOfCondition : public cmCMakePresetsFile::Condition
{
public:
  bool Evaluate(const std::vector<MacroExpander>& expanders, int version,
                cm::optional<bool>& out) const override;

  std::vector<std::unique_ptr<Condition>> Conditions;
  bool StopValue;
};

class NotCondition : public cmCMakePresetsFile::Condition
{
public:
  bool Evaluate(const std::vector<MacroExpander>& expanders, int version,
                cm::optional<bool>& out) const override;

  std::unique_ptr<Condition> SubCondition;
};
}
