/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmList.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <set>
#include <stdexcept>
#include <utility>

#include <cm/memory>

#include "cmsys/RegularExpression.hxx"

#include "cmAlgorithms.h"
#include "cmGeneratorExpression.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmStringReplaceHelper.h"
#include "cmSystemTools.h"

cm::string_view cmList::element_separator{ ";" };

cmList cmList::sublist(size_type pos, size_type length) const
{
  if (pos >= this->Values.size()) {
    throw std::out_of_range(cmStrCat(
      "begin index: ", pos, " is out of range 0 - ", this->Values.size() - 1));
  }

  size_type count = (length == npos || pos + length > this->size())
    ? this->size()
    : pos + length;
  return this->sublist(this->begin() + pos, this->begin() + count);
}

cmList::size_type cmList::find(cm::string_view value) const
{
  auto res = std::find(this->Values.begin(), this->Values.end(), value);
  if (res == this->Values.end()) {
    return npos;
  }

  return std::distance(this->Values.begin(), res);
}

cmList& cmList::remove_duplicates()
{
  auto newEnd = cmRemoveDuplicates(this->Values);
  this->Values.erase(newEnd, this->Values.end());

  return *this;
}

namespace {
class MatchesRegex
{
public:
  MatchesRegex(cmsys::RegularExpression& regex, cmList::FilterMode mode)
    : Regex(regex)
    , IncludeMatches(mode == cmList::FilterMode::INCLUDE)
  {
  }

  bool operator()(const std::string& target)
  {
    return this->Regex.find(target) ^ this->IncludeMatches;
  }

private:
  cmsys::RegularExpression& Regex;
  const bool IncludeMatches;
};
}

cmList& cmList::filter(cm::string_view pattern, FilterMode mode)
{
  cmsys::RegularExpression regex(std::string{ pattern });
  if (!regex.is_valid()) {
    throw std::invalid_argument(
      cmStrCat("sub-command FILTER, mode REGEX failed to compile regex \"",
               pattern, "\"."));
  }

  auto it = std::remove_if(this->Values.begin(), this->Values.end(),
                           MatchesRegex{ regex, mode });
  this->Values.erase(it, this->Values.end());

  return *this;
}

namespace {
class StringSorter
{
protected:
  using StringFilter = std::function<std::string(const std::string&)>;

  using OrderMode = cmList::SortConfiguration::OrderMode;
  using CompareMethod = cmList::SortConfiguration::CompareMethod;
  using CaseSensitivity = cmList::SortConfiguration::CaseSensitivity;

  StringFilter GetCompareFilter(CompareMethod compare)
  {
    return (compare == CompareMethod::FILE_BASENAME)
      ? cmSystemTools::GetFilenameName
      : nullptr;
  }

  StringFilter GetCaseFilter(CaseSensitivity sensitivity)
  {
    return (sensitivity == CaseSensitivity::INSENSITIVE)
      ? cmSystemTools::LowerCase
      : nullptr;
  }

  using ComparisonFunction =
    std::function<bool(const std::string&, const std::string&)>;
  ComparisonFunction GetComparisonFunction(CompareMethod compare)
  {
    if (compare == CompareMethod::NATURAL) {
      return std::function<bool(const std::string&, const std::string&)>(
        [](const std::string& x, const std::string& y) {
          return cmSystemTools::strverscmp(x, y) < 0;
        });
    }
    return std::function<bool(const std::string&, const std::string&)>(
      [](const std::string& x, const std::string& y) { return x < y; });
  }

public:
  StringSorter(cmList::SortConfiguration const& config)
    : Filters{ this->GetCompareFilter(config.Compare),
               this->GetCaseFilter(config.Case) }
    , SortMethod(this->GetComparisonFunction(config.Compare))
    , Descending(config.Order == OrderMode::DESCENDING)
  {
  }

  std::string ApplyFilter(const std::string& argument)
  {
    std::string result = argument;
    for (auto const& filter : this->Filters) {
      if (filter != nullptr) {
        result = filter(result);
      }
    }
    return result;
  }

  bool operator()(const std::string& a, const std::string& b)
  {
    std::string af = this->ApplyFilter(a);
    std::string bf = this->ApplyFilter(b);
    bool result;
    if (this->Descending) {
      result = this->SortMethod(bf, af);
    } else {
      result = this->SortMethod(af, bf);
    }
    return result;
  }

private:
  StringFilter Filters[2] = { nullptr, nullptr };
  ComparisonFunction SortMethod;
  bool Descending;
};
}

cmList::SortConfiguration::SortConfiguration() = default;

cmList& cmList::sort(const SortConfiguration& cfg)
{
  SortConfiguration config{ cfg };

  if (config.Order == SortConfiguration::OrderMode::DEFAULT) {
    config.Order = SortConfiguration::OrderMode::ASCENDING;
  }
  if (config.Compare == SortConfiguration::CompareMethod::DEFAULT) {
    config.Compare = SortConfiguration::CompareMethod::STRING;
  }
  if (config.Case == SortConfiguration::CaseSensitivity::DEFAULT) {
    config.Case = SortConfiguration::CaseSensitivity::SENSITIVE;
  }

  if ((config.Compare == SortConfiguration::CompareMethod::STRING) &&
      (config.Case == SortConfiguration::CaseSensitivity::SENSITIVE) &&
      (config.Order == SortConfiguration::OrderMode::ASCENDING)) {
    std::sort(this->Values.begin(), this->Values.end());
  } else {
    StringSorter sorter(config);
    std::sort(this->Values.begin(), this->Values.end(), sorter);
  }

  return *this;
}

namespace {
using transform_type = std::function<std::string(const std::string&)>;
using transform_error = cmList::transform_error;

class TransformSelector : public cmList::TransformSelector
{
public:
  ~TransformSelector() override = default;

  std::string Tag;

  const std::string& GetTag() override { return this->Tag; }

  virtual bool Validate(std::size_t count = 0) = 0;

  virtual bool InSelection(const std::string&) = 0;

  virtual void Transform(cmList::container_type& list,
                         const transform_type& transform)
  {
    std::transform(list.begin(), list.end(), list.begin(), transform);
  }

protected:
  TransformSelector(std::string&& tag)
    : Tag(std::move(tag))
  {
  }
};

class TransformNoSelector : public TransformSelector
{
public:
  TransformNoSelector()
    : TransformSelector("NO SELECTOR")
  {
  }

  bool Validate(std::size_t) override { return true; }

  bool InSelection(const std::string&) override { return true; }
};
class TransformSelectorRegex : public TransformSelector
{
public:
  TransformSelectorRegex(const std::string& regex)
    : TransformSelector("REGEX")
    , Regex(regex)
  {
  }
  TransformSelectorRegex(std::string&& regex)
    : TransformSelector("REGEX")
    , Regex(regex)
  {
  }

  bool Validate(std::size_t) override { return this->Regex.is_valid(); }

  bool InSelection(const std::string& value) override
  {
    return this->Regex.find(value);
  }

  cmsys::RegularExpression Regex;
};
class TransformSelectorIndexes : public TransformSelector
{
public:
  std::vector<index_type> Indexes;

  bool InSelection(const std::string&) override { return true; }

  void Transform(std::vector<std::string>& list,
                 const transform_type& transform) override
  {
    this->Validate(list.size());

    for (auto index : this->Indexes) {
      list[index] = transform(list[index]);
    }
  }

protected:
  TransformSelectorIndexes(std::string&& tag)
    : TransformSelector(std::move(tag))
  {
  }
  TransformSelectorIndexes(std::string&& tag,
                           std::vector<index_type> const& indexes)
    : TransformSelector(std::move(tag))
    , Indexes(indexes)
  {
  }
  TransformSelectorIndexes(std::string&& tag,
                           std::vector<index_type>&& indexes)
    : TransformSelector(std::move(tag))
    , Indexes(indexes)
  {
  }

  index_type NormalizeIndex(index_type index, std::size_t count)
  {
    if (index < 0) {
      index = static_cast<index_type>(count) + index;
    }
    if (index < 0 || count <= static_cast<std::size_t>(index)) {
      throw transform_error(cmStrCat(
        "sub-command TRANSFORM, selector ", this->Tag, ", index: ", index,
        " out of range (-", count, ", ", count - 1, ")."));
    }
    return index;
  }
};
class TransformSelectorAt : public TransformSelectorIndexes
{
public:
  TransformSelectorAt(std::vector<index_type> const& indexes)
    : TransformSelectorIndexes("AT", indexes)
  {
  }
  TransformSelectorAt(std::vector<index_type>&& indexes)
    : TransformSelectorIndexes("AT", std::move(indexes))
  {
  }

  bool Validate(std::size_t count) override
  {
    decltype(this->Indexes) indexes;

    for (auto index : this->Indexes) {
      indexes.push_back(this->NormalizeIndex(index, count));
    }
    this->Indexes = std::move(indexes);

    return true;
  }
};
class TransformSelectorFor : public TransformSelectorIndexes
{
public:
  TransformSelectorFor(index_type start, index_type stop, index_type step)
    : TransformSelectorIndexes("FOR")
    , Start(start)
    , Stop(stop)
    , Step(step)
  {
  }

  bool Validate(std::size_t count) override
  {
    this->Start = this->NormalizeIndex(this->Start, count);
    this->Stop = this->NormalizeIndex(this->Stop, count);

    // Does stepping move us further from the end?
    if (this->Start > this->Stop) {
      throw transform_error(
        cmStrCat("sub-command TRANSFORM, selector FOR "
                 "expects <start> to be no greater than <stop> (",
                 this->Start, " > ", this->Stop, ")"));
    }

    // compute indexes
    auto size = (this->Stop - this->Start + 1) / this->Step;
    if ((this->Stop - this->Start + 1) % this->Step != 0) {
      size += 1;
    }

    this->Indexes.resize(size);
    auto start = this->Start;
    auto step = this->Step;
    std::generate(this->Indexes.begin(), this->Indexes.end(),
                  [&start, step]() -> index_type {
                    auto r = start;
                    start += step;
                    return r;
                  });

    return true;
  }

private:
  index_type Start, Stop, Step;
};

class TransformAction
{
public:
  virtual ~TransformAction() = default;

  void Initialize(TransformSelector* selector) { this->Selector = selector; }
  virtual void Initialize(TransformSelector*, const std::string&) {}
  virtual void Initialize(TransformSelector*, const std::string&,
                          const std::string&)
  {
  }
  virtual void Initialize(TransformSelector* selector,
                          const std::vector<std::string>&)
  {
    this->Initialize(selector);
  }

  virtual std::string operator()(const std::string& s) = 0;

protected:
  TransformSelector* Selector;
};
class TransformActionAppend : public TransformAction
{
public:
  using TransformAction::Initialize;

  void Initialize(TransformSelector* selector,
                  const std::string& append) override
  {
    TransformAction::Initialize(selector);
    this->Append = append;
  }
  void Initialize(TransformSelector* selector,
                  const std::vector<std::string>& append) override
  {
    this->Initialize(selector, append.front());
  }

  std::string operator()(const std::string& s) override
  {
    if (this->Selector->InSelection(s)) {
      return cmStrCat(s, this->Append);
    }

    return s;
  }

private:
  std::string Append;
};
class TransformActionPrepend : public TransformAction
{
public:
  using TransformAction::Initialize;

  void Initialize(TransformSelector* selector,
                  const std::string& prepend) override
  {
    TransformAction::Initialize(selector);
    this->Prepend = prepend;
  }
  void Initialize(TransformSelector* selector,
                  const std::vector<std::string>& prepend) override
  {
    this->Initialize(selector, prepend.front());
  }

  std::string operator()(const std::string& s) override
  {
    if (this->Selector->InSelection(s)) {
      return cmStrCat(this->Prepend, s);
    }

    return s;
  }

private:
  std::string Prepend;
};
class TransformActionToUpper : public TransformAction
{
public:
  std::string operator()(const std::string& s) override
  {
    if (this->Selector->InSelection(s)) {
      return cmSystemTools::UpperCase(s);
    }

    return s;
  }
};
class TransformActionToLower : public TransformAction
{
public:
  std::string operator()(const std::string& s) override
  {
    if (this->Selector->InSelection(s)) {
      return cmSystemTools::LowerCase(s);
    }

    return s;
  }
};
class TransformActionStrip : public TransformAction
{
public:
  std::string operator()(const std::string& s) override
  {
    if (this->Selector->InSelection(s)) {
      return cmTrimWhitespace(s);
    }

    return s;
  }
};
class TransformActionGenexStrip : public TransformAction
{
public:
  std::string operator()(const std::string& s) override
  {
    if (this->Selector->InSelection(s)) {
      return cmGeneratorExpression::Preprocess(
        s, cmGeneratorExpression::StripAllGeneratorExpressions);
    }

    return s;
  }
};
class TransformActionReplace : public TransformAction
{
public:
  using TransformAction::Initialize;

  void Initialize(TransformSelector* selector, const std::string& regex,
                  const std::string& replace) override
  {
    TransformAction::Initialize(selector);
    this->ReplaceHelper =
      cm::make_unique<cmStringReplaceHelper>(regex, replace);

    if (!this->ReplaceHelper->IsRegularExpressionValid()) {
      throw transform_error(
        cmStrCat("sub-command TRANSFORM, action REPLACE: Failed to compile "
                 "regex \"",
                 regex, "\"."));
    }
    if (!this->ReplaceHelper->IsReplaceExpressionValid()) {
      throw transform_error(cmStrCat("sub-command TRANSFORM, action REPLACE: ",
                                     this->ReplaceHelper->GetError(), "."));
    }
  }
  void Initialize(TransformSelector* selector,
                  const std::vector<std::string>& args) override
  {
    this->Initialize(selector, args[0], args[1]);
  }

  std::string operator()(const std::string& s) override
  {
    if (this->Selector->InSelection(s)) {
      // Scan through the input for all matches.
      std::string output;

      if (!this->ReplaceHelper->Replace(s, output)) {
        throw transform_error(
          cmStrCat("sub-command TRANSFORM, action REPLACE: ",
                   this->ReplaceHelper->GetError(), "."));
      }

      return output;
    }

    return s;
  }

private:
  std::unique_ptr<cmStringReplaceHelper> ReplaceHelper;
};

// Descriptor of action
// Arity: number of arguments required for the action
// Transform: Object implementing the action
struct ActionDescriptor
{
  ActionDescriptor(cmList::TransformAction action)
    : Action(action)
  {
  }
  ActionDescriptor(cmList::TransformAction action, std::string name,
                   std::size_t arity,
                   std::unique_ptr<TransformAction> transform)
    : Action(action)
    , Name(std::move(name))
    , Arity(arity)
    , Transform(std::move(transform))
  {
  }

  operator cmList::TransformAction() const { return this->Action; }

  cmList::TransformAction Action;
  std::string Name;
  std::size_t Arity = 0;
  std::unique_ptr<TransformAction> Transform;
};

// Build a set of supported actions.
using ActionDescriptorSet = std::set<
  ActionDescriptor,
  std::function<bool(cmList::TransformAction, cmList::TransformAction)>>;

ActionDescriptorSet Descriptors([](cmList::TransformAction x,
                                   cmList::TransformAction y) {
  return x < y;
});

ActionDescriptorSet::iterator TransformConfigure(
  cmList::TransformAction action,
  std::unique_ptr<cmList::TransformSelector>& selector, std::size_t arity)
{
  if (Descriptors.empty()) {
    Descriptors.emplace(cmList::TransformAction::APPEND, "APPEND", 1,
                        cm::make_unique<TransformActionAppend>());
    Descriptors.emplace(cmList::TransformAction::PREPEND, "PREPEND", 1,
                        cm::make_unique<TransformActionPrepend>());
    Descriptors.emplace(cmList::TransformAction::TOUPPER, "TOUPPER", 0,
                        cm::make_unique<TransformActionToUpper>());
    Descriptors.emplace(cmList::TransformAction::TOLOWER, "TOLOWER", 0,
                        cm::make_unique<TransformActionToLower>());
    Descriptors.emplace(cmList::TransformAction::STRIP, "STRIP", 0,
                        cm::make_unique<TransformActionStrip>());
    Descriptors.emplace(cmList::TransformAction::GENEX_STRIP, "GENEX_STRIP", 0,
                        cm::make_unique<TransformActionGenexStrip>());
    Descriptors.emplace(cmList::TransformAction::REPLACE, "REPLACE", 2,
                        cm::make_unique<TransformActionReplace>());
  }

  auto descriptor = Descriptors.find(action);
  if (descriptor == Descriptors.end()) {
    throw transform_error(cmStrCat(" sub-command TRANSFORM, ",
                                   std::to_string(static_cast<int>(action)),
                                   " invalid action."));
  }

  if (descriptor->Arity != arity) {
    throw transform_error(cmStrCat("sub-command TRANSFORM, action ",
                                   descriptor->Name, " expects ",
                                   descriptor->Arity, " argument(s)."));
  }
  if (!selector) {
    selector = cm::make_unique<TransformNoSelector>();
  }

  return descriptor;
}
}

std::unique_ptr<cmList::TransformSelector> cmList::TransformSelector::NewAT(
  std::initializer_list<index_type> indexes)
{
  return cm::make_unique<TransformSelectorAt>(
    std::vector<index_type>{ indexes.begin(), indexes.end() });
  ;
}
std::unique_ptr<cmList::TransformSelector> cmList::TransformSelector::NewAT(
  std::vector<index_type> const& indexes)
{
  return cm::make_unique<TransformSelectorAt>(indexes);
}
std::unique_ptr<cmList::TransformSelector> cmList::TransformSelector::NewAT(
  std::vector<index_type>&& indexes)
{
  return cm::make_unique<TransformSelectorAt>(std::move(indexes));
}

std::unique_ptr<cmList::TransformSelector> cmList::TransformSelector::NewFOR(
  std::initializer_list<index_type> indexes)
{
  if (indexes.size() < 2 || indexes.size() > 3) {
    throw transform_error("sub-command TRANSFORM, selector FOR "
                          "expects 2 or 3 arguments");
  }
  if (indexes.size() == 3 && *(indexes.begin() + 2) < 0) {
    throw transform_error("sub-command TRANSFORM, selector FOR expects "
                          "positive numeric value for <step>.");
  }

  return cm::make_unique<TransformSelectorFor>(
    *indexes.begin(), *(indexes.begin() + 1),
    indexes.size() == 3 ? *(indexes.begin() + 2) : 1);
}
std::unique_ptr<cmList::TransformSelector> cmList::TransformSelector::NewFOR(
  std::vector<index_type> const& indexes)
{
  if (indexes.size() < 2 || indexes.size() > 3) {
    throw transform_error("sub-command TRANSFORM, selector FOR "
                          "expects 2 or 3 arguments");
  }
  if (indexes.size() == 3 && indexes[2] < 0) {
    throw transform_error("sub-command TRANSFORM, selector FOR expects "
                          "positive numeric value for <step>.");
  }

  return cm::make_unique<TransformSelectorFor>(
    indexes[0], indexes[1], indexes.size() == 3 ? indexes[2] : 1);
}
std::unique_ptr<cmList::TransformSelector> cmList::TransformSelector::NewFOR(
  std::vector<index_type>&& indexes)
{
  if (indexes.size() < 2 || indexes.size() > 3) {
    throw transform_error("sub-command TRANSFORM, selector FOR "
                          "expects 2 or 3 arguments");
  }
  if (indexes.size() == 3 && indexes[2] < 0) {
    throw transform_error("sub-command TRANSFORM, selector FOR expects "
                          "positive numeric value for <step>.");
  }

  return cm::make_unique<TransformSelectorFor>(
    indexes[0], indexes[1], indexes.size() == 3 ? indexes[2] : 1);
}

std::unique_ptr<cmList::TransformSelector> cmList::TransformSelector::NewREGEX(
  std::string const& regex)
{
  std::unique_ptr<::TransformSelector> selector =
    cm::make_unique<TransformSelectorRegex>(regex);
  if (!selector->Validate()) {
    throw transform_error(
      cmStrCat("sub-command TRANSFORM, selector REGEX failed to compile "
               "regex \"",
               regex, "\"."));
  }
  // weird construct to please all compilers
  return std::unique_ptr<cmList::TransformSelector>(selector.release());
}
std::unique_ptr<cmList::TransformSelector> cmList::TransformSelector::NewREGEX(
  std::string&& regex)
{
  std::unique_ptr<::TransformSelector> selector =
    cm::make_unique<TransformSelectorRegex>(std::move(regex));
  if (!selector->Validate()) {
    throw transform_error(
      cmStrCat("sub-command TRANSFORM, selector REGEX failed to compile "
               "regex \"",
               regex, "\"."));
  }
  // weird construct to please all compilers
  return std::unique_ptr<cmList::TransformSelector>(selector.release());
}

cmList& cmList::transform(TransformAction action,
                          std::unique_ptr<TransformSelector> selector)
{
  auto descriptor = TransformConfigure(action, selector, 0);

  descriptor->Transform->Initialize(
    static_cast<::TransformSelector*>(selector.get()));

  static_cast<::TransformSelector&>(*selector).Transform(
    this->Values, [&descriptor](const std::string& s) -> std::string {
      return (*descriptor->Transform)(s);
    });

  return *this;
}

cmList& cmList::transform(TransformAction action, std::string const& arg,
                          std::unique_ptr<TransformSelector> selector)
{
  auto descriptor = TransformConfigure(action, selector, 1);

  descriptor->Transform->Initialize(
    static_cast<::TransformSelector*>(selector.get()), arg);

  static_cast<::TransformSelector&>(*selector).Transform(
    this->Values, [&descriptor](const std::string& s) -> std::string {
      return (*descriptor->Transform)(s);
    });

  return *this;
}

cmList& cmList::transform(TransformAction action, std::string const& arg1,
                          std::string const& arg2,
                          std::unique_ptr<TransformSelector> selector)
{
  auto descriptor = TransformConfigure(action, selector, 2);

  descriptor->Transform->Initialize(
    static_cast<::TransformSelector*>(selector.get()), arg1, arg2);

  static_cast<::TransformSelector&>(*selector).Transform(
    this->Values, [&descriptor](const std::string& s) -> std::string {
      return (*descriptor->Transform)(s);
    });

  return *this;
}

cmList& cmList::transform(TransformAction action,
                          std::vector<std::string> const& args,
                          std::unique_ptr<TransformSelector> selector)
{
  auto descriptor = TransformConfigure(action, selector, args.size());

  descriptor->Transform->Initialize(
    static_cast<::TransformSelector*>(selector.get()), args);

  static_cast<::TransformSelector&>(*selector).Transform(
    this->Values, [&descriptor](const std::string& s) -> std::string {
      return (*descriptor->Transform)(s);
    });

  return *this;
}

std::string cmList::join(cm::string_view glue) const
{
  return cmJoin(this->Values, glue);
}

std::string& cmList::append(std::string& list, cm::string_view value)
{
  if (list.empty()) {
    list = std::string(value);
  } else {
    list += cmStrCat(cmList::element_separator, value);
  }

  return list;
}

std::string& cmList::prepend(std::string& list, cm::string_view value)
{
  if (list.empty()) {
    list = std::string(value);
  } else {
    list.insert(0, cmStrCat(value, cmList::element_separator));
  }

  return list;
}

cmList::size_type cmList::ComputeIndex(index_type pos, bool boundCheck) const
{
  if (boundCheck) {
    if (this->Values.empty()) {
      throw std::out_of_range(
        cmStrCat("index: ", pos, " out of range (0, 0)"));
    }

    auto index = pos;
    if (!this->Values.empty()) {
      auto length = this->Values.size();
      if (index < 0) {
        index = static_cast<index_type>(length) + index;
      }
      if (index < 0 || length <= static_cast<size_type>(index)) {
        throw std::out_of_range(cmStrCat("index: ", pos, " out of range (-",
                                         this->Values.size(), ", ",
                                         this->Values.size() - 1, ")"));
      }
    }
    return index;
  }

  return pos < 0 ? this->Values.size() + pos : pos;
}
cmList::size_type cmList::ComputeInsertIndex(index_type pos,
                                             bool boundCheck) const
{
  if (boundCheck) {
    if (this->Values.empty() && pos != 0) {
      throw std::out_of_range(
        cmStrCat("index: ", pos, " out of range (0, 0)"));
    }

    auto index = pos;
    if (!this->Values.empty()) {
      auto length = this->Values.size();
      if (index < 0) {
        index = static_cast<index_type>(length) + index;
      }
      if (index < 0 || length < static_cast<size_type>(index)) {
        throw std::out_of_range(cmStrCat("index: ", pos, " out of range (-",
                                         this->Values.size(), ", ",
                                         this->Values.size(), ")"));
      }
    }
    return index;
  }

  return pos < 0 ? this->Values.size() + pos : pos;
}

cmList cmList::GetItems(std::vector<index_type>&& indexes) const
{
  cmList listItems;

  for (auto index : indexes) {
    listItems.emplace_back(this->get_item(index));
  }

  return listItems;
}

cmList& cmList::RemoveItems(std::vector<index_type>&& indexes)
{
  if (indexes.empty()) {
    return *this;
  }

  // compute all indexes
  std::vector<size_type> idx(indexes.size());
  std::transform(indexes.cbegin(), indexes.cend(), idx.begin(),
                 [this](const index_type& index) -> size_type {
                   return this->ComputeIndex(index);
                 });

  std::sort(idx.begin(), idx.end(),
            [](size_type l, size_type r) { return l > r; });
  auto newEnd = std::unique(idx.begin(), idx.end());
  idx.erase(newEnd, idx.end());

  for (auto index : idx) {
    this->erase(this->begin() + index);
  }

  return *this;
}

cmList& cmList::RemoveItems(std::vector<std::string>&& items)
{
  std::sort(items.begin(), items.end());
  auto last = std::unique(items.begin(), items.end());
  auto first = items.begin();

  auto newEnd = cmRemoveMatching(this->Values, cmMakeRange(first, last));
  this->Values.erase(newEnd, this->Values.end());

  return *this;
}

cmList::container_type::iterator cmList::Insert(
  container_type& container, container_type::const_iterator pos,
  std::string&& value, ExpandElements expandElements,
  EmptyElements emptyElements)
{
  auto delta = std::distance(container.cbegin(), pos);
  auto insertPos = container.begin() + delta;

  if (expandElements == ExpandElements::Yes) {
    // If argument is empty, it is an empty list.
    if (emptyElements == EmptyElements::No && value.empty()) {
      return insertPos;
    }

    // if there are no ; in the name then just copy the current string
    if (value.find(';') == std::string::npos) {
      return container.insert(insertPos, std::move(value));
    }

    std::string newValue;
    // Break the string at non-escaped semicolons not nested in [].
    int squareNesting = 0;
    auto last = value.begin();
    auto const cend = value.end();
    for (auto c = last; c != cend; ++c) {
      switch (*c) {
        case '\\': {
          // We only want to allow escaping of semicolons.  Other
          // escapes should not be processed here.
          auto cnext = c + 1;
          if ((cnext != cend) && *cnext == ';') {
            newValue.append(last, c);
            // Skip over the escape character
            last = cnext;
            c = cnext;
          }
        } break;
        case '[': {
          ++squareNesting;
        } break;
        case ']': {
          --squareNesting;
        } break;
        case ';': {
          // brackets.
          if (squareNesting == 0) {
            newValue.append(last, c);
            // Skip over the semicolon
            last = c + 1;
            if (!newValue.empty() || emptyElements == EmptyElements::Yes) {
              // Add the last argument.
              insertPos = container.insert(insertPos, newValue);
              insertPos++;
              newValue.clear();
            }
          }
        } break;
        default: {
          // Just append this character.
        } break;
      }
    }
    newValue.append(last, cend);
    if (!newValue.empty() || emptyElements == EmptyElements::Yes) {
      // Add the last argument.
      container.insert(insertPos, std::move(newValue));
    }
  } else if (!value.empty() || emptyElements == EmptyElements::Yes) {
    return container.insert(insertPos, std::move(value));
  }
  return container.begin() + delta;
}
