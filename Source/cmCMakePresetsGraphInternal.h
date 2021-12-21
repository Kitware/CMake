/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <memory>

#include "cmCMakePresetsGraph.h"

#define CHECK_OK(expr)                                                        \
  do {                                                                        \
    auto _result = expr;                                                      \
    if (_result != ReadFileResult::READ_OK)                                   \
      return _result;                                                         \
  } while (false)

namespace cmCMakePresetsGraphInternal {
enum class ExpandMacroResult
{
  Ok,
  Ignore,
  Error,
};

using MacroExpander = std::function<ExpandMacroResult(
  const std::string&, const std::string&, std::string&, int version)>;
}

class cmCMakePresetsGraph::Condition
{
public:
  virtual ~Condition() = default;

  virtual bool Evaluate(
    const std::vector<cmCMakePresetsGraphInternal::MacroExpander>& expanders,
    int version, cm::optional<bool>& out) const = 0;
  virtual bool IsNull() const { return false; }
};

namespace cmCMakePresetsGraphInternal {

class NullCondition : public cmCMakePresetsGraph::Condition
{
  bool Evaluate(const std::vector<MacroExpander>& /*expanders*/,
                int /*version*/, cm::optional<bool>& out) const override
  {
    out = true;
    return true;
  }

  bool IsNull() const override { return true; }
};

class ConstCondition : public cmCMakePresetsGraph::Condition
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

class EqualsCondition : public cmCMakePresetsGraph::Condition
{
public:
  bool Evaluate(const std::vector<MacroExpander>& expanders, int version,
                cm::optional<bool>& out) const override;

  std::string Lhs;
  std::string Rhs;
};

class InListCondition : public cmCMakePresetsGraph::Condition
{
public:
  bool Evaluate(const std::vector<MacroExpander>& expanders, int version,
                cm::optional<bool>& out) const override;

  std::string String;
  std::vector<std::string> List;
};

class MatchesCondition : public cmCMakePresetsGraph::Condition
{
public:
  bool Evaluate(const std::vector<MacroExpander>& expanders, int version,
                cm::optional<bool>& out) const override;

  std::string String;
  std::string Regex;
};

class AnyAllOfCondition : public cmCMakePresetsGraph::Condition
{
public:
  bool Evaluate(const std::vector<MacroExpander>& expanders, int version,
                cm::optional<bool>& out) const override;

  std::vector<std::unique_ptr<Condition>> Conditions;
  bool StopValue;
};

class NotCondition : public cmCMakePresetsGraph::Condition
{
public:
  bool Evaluate(const std::vector<MacroExpander>& expanders, int version,
                cm::optional<bool>& out) const override;

  std::unique_ptr<Condition> SubCondition;
};
}
