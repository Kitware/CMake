/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmList.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

#include <cm/memory>
#include <cm/optional>

#include "cmsys/RegularExpression.hxx"

#include "cmAlgorithms.h"
#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmStringReplaceHelper.h"
#include "cmSystemTools.h"
#include "cmValue.h"

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

  bool operator()(std::string const& target)
  {
    return this->Regex.find(target) ^ this->IncludeMatches;
  }

private:
  cmsys::RegularExpression& Regex;
  bool const IncludeMatches;
};

// Hash of call site (FilePath:Line) for unique variable names across recursive
// calls.
std::string OutputVarFor(cm::string_view prefix, cmMakefile& makefile)
{
  cmListFileContext context = makefile.GetBacktrace().Top();
  std::size_t hash =
    std::hash<std::string>{}(cmStrCat(context.FilePath, ":", context.Line));
  return cmStrCat(prefix, hash, "_");
}

void RequireFunction(cmMakefile const& makefile,
                     std::string const& functionName,
                     std::string const& errorPrefix)
{
  cm::optional<cmStateEnums::CommandType> type =
    makefile.GetState()->GetCommandType(functionName);
  if (!type) {
    throw cmList::transform_error(
      cmStrCat(errorPrefix, ": unknown function \"", functionName, "\"."));
  }
  if (*type == cmStateEnums::CommandType::Macro) {
    throw cmList::transform_error(
      cmStrCat(errorPrefix, ": macro \"", functionName,
               "\" may not be used here;"
               " define it as a function() instead."));
  }
}

class PredicateEvaluator
{
public:
  PredicateEvaluator(std::string functionName, cmMakefile& makefile,
                     std::string errorPrefix = "sub-command TRANSFORM, "
                                               "selector PREDICATE")
    : FunctionName(std::move(functionName))
    , Makefile(&makefile)
    , ErrorPrefix(std::move(errorPrefix))
    , OutputVar(OutputVarFor("_cmake_predicate_out_", makefile))
  {
    RequireFunction(makefile, this->FunctionName, this->ErrorPrefix);
  }

  bool operator()(std::string const& value)
  {
    this->Makefile->RemoveDefinition(this->OutputVar);

    cmListFileContext context = this->Makefile->GetBacktrace().Top();
    std::vector<cmListFileArgument> funcArgs;
    funcArgs.emplace_back(value, cmListFileArgument::Quoted, context.Line);
    funcArgs.emplace_back(this->OutputVar, cmListFileArgument::Quoted,
                          context.Line);
    cmListFileFunction func{ this->FunctionName, context.Line, context.Line,
                             std::move(funcArgs) };

    cmExecutionStatus status(*this->Makefile);
    if (!this->Makefile->ExecuteCommand(func, status) ||
        status.GetNestedError()) {
      throw cmList::transform_error(
        cmStrCat(this->ErrorPrefix, ": function \"", this->FunctionName,
                 "\" failed during execution."));
    }

    cmValue result = this->Makefile->GetDefinition(this->OutputVar);
    if (!result) {
      throw cmList::transform_error(
        cmStrCat(this->ErrorPrefix, ": function \"", this->FunctionName,
                 "\" did not set the output variable."));
    }

    bool boolResult = cmIsOn(*result);
    this->Makefile->RemoveDefinition(this->OutputVar);
    return boolResult;
  }

private:
  std::string FunctionName;
  cmMakefile* Makefile = nullptr;
  std::string ErrorPrefix;
  std::string OutputVar;
};

class MatchesPredicate
{
public:
  MatchesPredicate(PredicateEvaluator& evaluator, cmList::FilterMode mode)
    : Evaluator(evaluator)
    , IncludeMatches(mode == cmList::FilterMode::INCLUDE)
  {
  }

  bool operator()(std::string const& target)
  {
    return this->Evaluator(target) ^ this->IncludeMatches;
  }

private:
  PredicateEvaluator& Evaluator;
  bool IncludeMatches;
};

class ComparatorEvaluator
{
public:
  ComparatorEvaluator(std::string functionName, cmMakefile& makefile)
    : FunctionName(std::move(functionName))
    , Makefile(&makefile)
    , OutputVar(OutputVarFor("_cmake_comparator_out_", makefile))
  {
    RequireFunction(makefile, this->FunctionName,
                    "sub-command SORT, COMPARATOR");
  }

  bool operator()(std::string const& a, std::string const& b)
  {
    this->Makefile->RemoveDefinition(this->OutputVar);

    cmListFileContext context = this->Makefile->GetBacktrace().Top();
    std::vector<cmListFileArgument> funcArgs;
    funcArgs.emplace_back(a, cmListFileArgument::Quoted, context.Line);
    funcArgs.emplace_back(b, cmListFileArgument::Quoted, context.Line);
    funcArgs.emplace_back(this->OutputVar, cmListFileArgument::Quoted,
                          context.Line);
    cmListFileFunction func{ this->FunctionName, context.Line, context.Line,
                             std::move(funcArgs) };

    cmExecutionStatus status(*this->Makefile);
    if (!this->Makefile->ExecuteCommand(func, status) ||
        status.GetNestedError()) {
      throw cmList::transform_error(
        cmStrCat("sub-command SORT, COMPARATOR: function \"",
                 this->FunctionName, "\" failed during execution."));
    }

    cmValue result = this->Makefile->GetDefinition(this->OutputVar);
    if (!result) {
      throw cmList::transform_error(
        cmStrCat("sub-command SORT, COMPARATOR: function \"",
                 this->FunctionName, "\" did not set the output variable."));
    }

    bool boolResult = cmIsOn(*result);
    this->Makefile->RemoveDefinition(this->OutputVar);
    return boolResult;
  }

private:
  std::string FunctionName;
  cmMakefile* Makefile = nullptr;
  std::string OutputVar;
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

cmList& cmList::filter(std::string const& functionName, FilterMode mode,
                       cmMakefile& makefile)
{
  try {
    PredicateEvaluator evaluator(functionName, makefile,
                                 "sub-command FILTER, mode PREDICATE");

    auto it = std::remove_if(this->Values.begin(), this->Values.end(),
                             MatchesPredicate{ evaluator, mode });
    this->Values.erase(it, this->Values.end());
  } catch (transform_error& e) {
    throw std::invalid_argument(e.what());
  }

  return *this;
}

namespace {
class StringSorter
{
protected:
  using StringFilter = std::function<std::string(std::string const&)>;

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
      ? cmsys::SystemTools::LowerCase
      : nullptr;
  }

  using ComparisonFunction =
    std::function<bool(std::string const&, std::string const&)>;
  ComparisonFunction GetComparisonFunction(CompareMethod compare)
  {
    if (compare == CompareMethod::NATURAL) {
      return std::function<bool(std::string const&, std::string const&)>(
        [](std::string const& x, std::string const& y) {
          return cmSystemTools::strverscmp(x, y) < 0;
        });
    }
    return std::function<bool(std::string const&, std::string const&)>(
      [](std::string const& x, std::string const& y) { return x < y; });
  }

public:
  StringSorter(cmList::SortConfiguration config)
    : Filters{ this->GetCompareFilter(config.Compare),
               this->GetCaseFilter(config.Case) }
    , SortMethod(this->GetComparisonFunction(config.Compare))
    , Descending(config.Order == OrderMode::DESCENDING)
  {
  }

  StringSorter(cmList::SortConfiguration config, ComparisonFunction comparator)
    : Filters{ nullptr, this->GetCaseFilter(config.Case) }
    , SortMethod(std::move(comparator))
    , Descending(config.Order == OrderMode::DESCENDING)
  {
  }

  std::string ApplyFilter(std::string const& argument)
  {
    std::string result = argument;
    for (auto const& filter : this->Filters) {
      if (filter) {
        result = filter(result);
      }
    }
    return result;
  }

  bool operator()(std::string const& a, std::string const& b)
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

cmList& cmList::sort(SortConfiguration cfg)
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

cmList& cmList::sort(SortConfiguration cfg, cmMakefile& makefile)
{
  SortConfiguration config{ cfg };

  if (config.Order == SortConfiguration::OrderMode::DEFAULT) {
    config.Order = SortConfiguration::OrderMode::ASCENDING;
  }
  if (config.Case == SortConfiguration::CaseSensitivity::DEFAULT) {
    config.Case = SortConfiguration::CaseSensitivity::SENSITIVE;
  }

  try {
    ComparatorEvaluator evaluator(config.ComparatorFunction, makefile);
    StringSorter sorter(
      config, [&evaluator](std::string const& a, std::string const& b) {
        bool result = evaluator(a, b);
        if (result && evaluator(b, a)) {
          throw cmList::transform_error(
            "sub-command SORT, COMPARATOR: function does not induce a strict "
            "weak ordering. The comparator returned TRUE for both (a, b) and "
            "(b, a).");
        }
        return result;
      });
    std::sort(this->Values.begin(), this->Values.end(), sorter);
  } catch (transform_error& e) {
    throw std::invalid_argument(e.what());
  }

  return *this;
}

namespace {
using transform_type = std::function<std::string(std::string const&)>;
using transform_error = cmList::transform_error;

class TransformSelector : public cmList::TransformSelector
{
public:
  ~TransformSelector() override = default;

  std::string Tag;

  std::string const& GetTag() override { return this->Tag; }

  virtual bool Validate(std::size_t count = 0) = 0;

  virtual bool InSelection(std::string const&) = 0;

  virtual void Transform(cmList::container_type& list,
                         transform_type const& transform)
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

  bool InSelection(std::string const&) override { return true; }
};
class TransformSelectorRegex : public TransformSelector
{
public:
  TransformSelectorRegex(std::string const& regex)
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

  bool InSelection(std::string const& value) override
  {
    return this->Regex.find(value);
  }

  cmsys::RegularExpression Regex;
};
class TransformSelectorPredicate : public TransformSelector
{
public:
  TransformSelectorPredicate(std::string const& functionName,
                             cmMakefile& makefile)
    : TransformSelector("PREDICATE")
    , Evaluator(functionName, makefile)
  {
  }

  bool Validate(std::size_t) override { return true; }

  bool InSelection(std::string const& value) override
  {
    return this->Evaluator(value);
  }

private:
  PredicateEvaluator Evaluator;
};
class TransformSelectorIndexes : public TransformSelector
{
public:
  std::vector<index_type> Indexes;

  bool InSelection(std::string const&) override { return true; }

  void Transform(std::vector<std::string>& list,
                 transform_type const& transform) override
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
                 this->Start, " > ", this->Stop, ')'));
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
  // Public because an inherited constructor keeps the base's access.
  explicit TransformAction(TransformSelector& selector)
    : Selector(selector)
  {
  }
  virtual ~TransformAction() = default;

  std::string operator()(std::string const& s)
  {
    return this->Selector.InSelection(s) ? this->ApplyTo(s) : s;
  }

protected:
  virtual std::string ApplyTo(std::string const& s) = 0;

  TransformSelector& Selector;
};
class TransformActionAppend : public TransformAction
{
public:
  TransformActionAppend(TransformSelector& selector, std::string append)
    : TransformAction(selector)
    , Append(std::move(append))
  {
  }

protected:
  std::string ApplyTo(std::string const& s) override
  {
    return cmStrCat(s, this->Append);
  }

private:
  std::string Append;
};
class TransformActionPrepend : public TransformAction
{
public:
  TransformActionPrepend(TransformSelector& selector, std::string prepend)
    : TransformAction(selector)
    , Prepend(std::move(prepend))
  {
  }

protected:
  std::string ApplyTo(std::string const& s) override
  {
    return cmStrCat(this->Prepend, s);
  }

private:
  std::string Prepend;
};
class TransformActionToUpper : public TransformAction
{
public:
  using TransformAction::TransformAction;

protected:
  std::string ApplyTo(std::string const& s) override
  {
    return cmSystemTools::UpperCase(s);
  }
};
class TransformActionToLower : public TransformAction
{
public:
  using TransformAction::TransformAction;

protected:
  std::string ApplyTo(std::string const& s) override
  {
    return cmSystemTools::LowerCase(s);
  }
};
class TransformActionStrip : public TransformAction
{
public:
  using TransformAction::TransformAction;

protected:
  std::string ApplyTo(std::string const& s) override
  {
    return cmTrimWhitespace(s);
  }
};
class TransformActionGenexStrip : public TransformAction
{
public:
  using TransformAction::TransformAction;

protected:
  std::string ApplyTo(std::string const& s) override
  {
    return cmGeneratorExpression::Preprocess(
      s, cmGeneratorExpression::StripAllGeneratorExpressions);
  }
};
class TransformActionReplace : public TransformAction
{
public:
  TransformActionReplace(TransformSelector& selector, std::string const& regex,
                         std::string const& replace)
    : TransformAction(selector)
    // Makefile is legitimately null when cmList is used directly from C++;
    // cmStringReplaceHelper handles that.
    , ReplaceHelper(cm::make_unique<cmStringReplaceHelper>(regex, replace,
                                                           selector.Makefile))
  {
    if (!this->ReplaceHelper->IsRegularExpressionValid()) {
      throw transform_error(
        cmStrCat("sub-command TRANSFORM, action REPLACE: Failed to compile "
                 "regex \"",
                 regex, "\"."));
    }
    if (!this->ReplaceHelper->IsReplaceExpressionValid()) {
      throw transform_error(cmStrCat("sub-command TRANSFORM, action REPLACE: ",
                                     this->ReplaceHelper->GetError(), '.'));
    }
  }

protected:
  std::string ApplyTo(std::string const& s) override
  {
    std::string output;

    if (!this->ReplaceHelper->Replace(s, output)) {
      throw transform_error(cmStrCat("sub-command TRANSFORM, action REPLACE: ",
                                     this->ReplaceHelper->GetError(), '.'));
    }

    return output;
  }

private:
  std::unique_ptr<cmStringReplaceHelper> ReplaceHelper;
};

class TransformActionApply : public TransformAction
{
public:
  TransformActionApply(TransformSelector& selector, std::string functionName,
                       cmMakefile& makefile)
    : TransformAction(selector)
    , FunctionName(std::move(functionName))
    , Makefile(&makefile)
    , OutputVar(OutputVarFor("_cmake_transform_apply_out_", makefile))
  {
    RequireFunction(makefile, this->FunctionName,
                    "sub-command TRANSFORM, action APPLY");
  }

protected:
  std::string ApplyTo(std::string const& s) override
  {
    // Unset the output variable before calling
    this->Makefile->RemoveDefinition(this->OutputVar);

    // Build the function call: functionName(s, outputVar)
    cmListFileContext context = this->Makefile->GetBacktrace().Top();
    std::vector<cmListFileArgument> funcArgs;
    funcArgs.emplace_back(s, cmListFileArgument::Quoted, context.Line);
    funcArgs.emplace_back(this->OutputVar, cmListFileArgument::Quoted,
                          context.Line);
    cmListFileFunction func{ this->FunctionName, context.Line, context.Line,
                             std::move(funcArgs) };

    cmExecutionStatus status(*this->Makefile);
    if (!this->Makefile->ExecuteCommand(func, status) ||
        status.GetNestedError()) {
      throw transform_error(
        cmStrCat("sub-command TRANSFORM, action APPLY: function \"",
                 this->FunctionName, "\" failed during execution."));
    }

    // Read back the output variable
    cmValue result = this->Makefile->GetDefinition(this->OutputVar);
    if (!result) {
      throw transform_error(
        cmStrCat("sub-command TRANSFORM, action APPLY: function \"",
                 this->FunctionName, "\" did not set the output variable."));
    }

    // Copy the result before cleaning up (RemoveDefinition invalidates the
    // cmValue pointer).
    std::string output = *result;

    this->Makefile->RemoveDefinition(this->OutputVar);

    return output;
  }

private:
  std::string FunctionName;
  cmMakefile* Makefile = nullptr;
  std::string OutputVar;
};

// Arity: number of arguments required for the action.
//
// Keep this a bare aggregate of literal types: CMake still builds as C++11,
// where a member initializer, a constructor, or a cm::string_view member
// would break the constexpr table below.
struct ActionDescriptor
{
  cmList::TransformAction Action;
  char const* Name;
  std::size_t Arity;
};

constexpr ActionDescriptor Descriptors[] = {
  { cmList::TransformAction::APPEND, "APPEND", 1 },
  { cmList::TransformAction::PREPEND, "PREPEND", 1 },
  { cmList::TransformAction::TOUPPER, "TOUPPER", 0 },
  { cmList::TransformAction::TOLOWER, "TOLOWER", 0 },
  { cmList::TransformAction::STRIP, "STRIP", 0 },
  { cmList::TransformAction::GENEX_STRIP, "GENEX_STRIP", 0 },
  { cmList::TransformAction::REPLACE, "REPLACE", 2 },
  { cmList::TransformAction::APPLY, "APPLY", 1 },
};

ActionDescriptor const& TransformConfigure(
  cmList::TransformAction action,
  std::unique_ptr<cmList::TransformSelector>& selector, std::size_t arity)
{
  // Not indexed by the enum value: this table is in registration order, and
  // cmList.h declares TOLOWER before TOUPPER.
  ActionDescriptor const* descriptor = nullptr;
  for (auto const& candidate : Descriptors) {
    if (candidate.Action == action) {
      descriptor = &candidate;
      break;
    }
  }

  if (!descriptor) {
    throw transform_error(cmStrCat(" sub-command TRANSFORM, ",
                                   static_cast<int>(action),
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

  return *descriptor;
}

// Precondition: TransformConfigure has validated the arity, so args is
// indexed unchecked.
std::unique_ptr<TransformAction> MakeTransformAction(
  ActionDescriptor const& descriptor, TransformSelector& selector,
  std::vector<std::string> const& args)
{
  switch (descriptor.Action) {
    case cmList::TransformAction::APPEND:
      return cm::make_unique<TransformActionAppend>(selector, args[0]);
    case cmList::TransformAction::PREPEND:
      return cm::make_unique<TransformActionPrepend>(selector, args[0]);
    case cmList::TransformAction::TOUPPER:
      return cm::make_unique<TransformActionToUpper>(selector);
    case cmList::TransformAction::TOLOWER:
      return cm::make_unique<TransformActionToLower>(selector);
    case cmList::TransformAction::STRIP:
      return cm::make_unique<TransformActionStrip>(selector);
    case cmList::TransformAction::GENEX_STRIP:
      return cm::make_unique<TransformActionGenexStrip>(selector);
    case cmList::TransformAction::REPLACE:
      return cm::make_unique<TransformActionReplace>(selector, args[0],
                                                     args[1]);
    case cmList::TransformAction::APPLY:
      // APPLY needs a cmMakefile, which this factory does not receive; only
      // the cmMakefile overload of cmList::transform can build it.
      break;
  }

  throw transform_error(
    "sub-command TRANSFORM, action APPLY requires cmMakefile context.");
}

void TransformValues(cmList::container_type& values,
                     cmList::TransformAction action,
                     std::vector<std::string> const& args,
                     std::unique_ptr<cmList::TransformSelector>& selector)
{
  ActionDescriptor const& descriptor =
    TransformConfigure(action, selector, args.size());

  auto& sel = static_cast<TransformSelector&>(*selector);
  std::unique_ptr<TransformAction> transformer =
    MakeTransformAction(descriptor, sel, args);

  sel.Transform(values, [&transformer](std::string const& s) -> std::string {
    return (*transformer)(s);
  });
}
}

std::unique_ptr<cmList::TransformSelector> cmList::TransformSelector::New()
{
  return cm::make_unique<TransformNoSelector>();
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

std::unique_ptr<cmList::TransformSelector>
cmList::TransformSelector::NewPREDICATE(std::string const& functionName,
                                        cmMakefile& makefile)
{
  return std::unique_ptr<cmList::TransformSelector>(
    new TransformSelectorPredicate(functionName, makefile));
}

cmList& cmList::transform(TransformAction action,
                          std::unique_ptr<TransformSelector> selector)
{
  TransformValues(this->Values, action, {}, selector);

  return *this;
}

cmList& cmList::transform(TransformAction action, std::string const& arg,
                          std::unique_ptr<TransformSelector> selector)
{
  TransformValues(this->Values, action, { arg }, selector);

  return *this;
}

cmList& cmList::transform(TransformAction action, std::string const& arg1,
                          std::string const& arg2,
                          std::unique_ptr<TransformSelector> selector)
{
  TransformValues(this->Values, action, { arg1, arg2 }, selector);

  return *this;
}

cmList& cmList::transform(TransformAction action,
                          std::vector<std::string> const& args,
                          std::unique_ptr<TransformSelector> selector)
{
  TransformValues(this->Values, action, args, selector);

  return *this;
}

cmList& cmList::transform(TransformAction action, std::string const& arg,
                          cmMakefile& makefile,
                          std::unique_ptr<TransformSelector> selector)
{
  // This overload performs APPLY unconditionally.  Without this check the
  // other arity-1 actions, APPEND and PREPEND, would pass the arity
  // validation below and then silently run APPLY instead.
  if (action != TransformAction::APPLY) {
    throw transform_error(
      "sub-command TRANSFORM: only action APPLY accepts a cmMakefile.");
  }

  // Validates the arity and defaults the selector.
  TransformConfigure(action, selector, 1);

  auto& sel = static_cast<::TransformSelector&>(*selector);
  TransformActionApply applyAction(sel, arg, makefile);

  sel.Transform(this->Values,
                [&applyAction](std::string const& s) -> std::string {
                  return applyAction(s);
                });

  return *this;
}

std::string& cmList::append(std::string& list, std::string&& value)
{
  if (list.empty()) {
    list = std::move(value);
  } else {
    list += cmStrCat(cmList::element_separator, value);
  }

  return list;
}
std::string& cmList::append(std::string& list, cm::string_view value)
{
  return cmList::append(list, std::string{ value });
}

std::string& cmList::prepend(std::string& list, std::string&& value)
{
  if (list.empty()) {
    list = std::move(value);
  } else {
    list.insert(0, cmStrCat(value, cmList::element_separator));
  }

  return list;
}
std::string& cmList::prepend(std::string& list, cm::string_view value)
{
  return cmList::prepend(list, std::string{ value });
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
                                         this->Values.size() - 1, ')'));
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
                                         this->Values.size(), ')'));
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
                 [this](index_type index) -> size_type {
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

std::string const& cmList::ToString(BT<std::string> const& s)
{
  return s.Value;
}
