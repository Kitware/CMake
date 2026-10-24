/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmPkgConfigParser.h"

#include <algorithm>
#include <cstddef>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cmllpkgc/llpkgc.h>

cmPkgConfigValueItem::cmPkgConfigValueItem(std::string text,
                                           bool isSubstitution)
  : Text{ std::move(text) }
  , IsSubstitution{ isSubstitution }
{
}

cmPkgConfigEntry::cmPkgConfigEntry(std::vector<cmPkgConfigValueItem> value,
                                   std::string ident,
                                   cmPkgConfigEntryKind kind)
  : Value{ std::move(value) }
  , Ident{ std::move(ident) }
  , Kind{ kind }
{
}

cmPkgConfigFileParser::cmPkgConfigFileParser()
{
  llpkgc_file_init(static_cast<llpkgc_file_t*>(this), &Settings_);
}

cmPkgConfigFileParser::cmPkgConfigFileParser(
  cmPkgConfigFileParser const& other)
  : cmPkgConfigFileParser()
{
  *this = other;
}

cmPkgConfigFileParser::cmPkgConfigFileParser(
  cmPkgConfigFileParser&& other) noexcept
  : cmPkgConfigFileParser()
{
  *this = std::move(other);
}

cmPkgConfigFileParser& cmPkgConfigFileParser::operator=(
  cmPkgConfigFileParser const& other)
{
  if (this != &other) {
    this->ResetCallbacks();
    llpkgc_file_init(static_cast<llpkgc_file_t*>(this), &Settings_);
    CurrentValue_.clear();
    Ident_ = other.Ident_;
    Data_ = other.Data_;
  }
  return *this;
}

cmPkgConfigFileParser& cmPkgConfigFileParser::operator=(
  cmPkgConfigFileParser&& other) noexcept
{
  if (this != &other) {
    this->ResetCallbacks();
    llpkgc_file_init(static_cast<llpkgc_file_t*>(this), &Settings_);
    CurrentValue_.clear();
    Ident_ = std::move(other.Ident_);
    Data_ = std::move(other.Data_);
  }
  return *this;
}

void cmPkgConfigFileParser::ResetCallbacks()
{
  Settings_.on_ident = OnIdentTr;
  Settings_.on_value_literal = OnValueLiteralTr;
  Settings_.on_value_substitution = OnValueSubstitutionTr;
  Settings_.on_property_complete = OnPropertyCompleteTr;
  Settings_.on_variable_complete = OnVariableCompleteTr;
  Settings_.on_barewords_complete = OnBarewordsCompleteTr;
  Settings_.on_value_literal_complete = OnValueLiteralCompleteTr;
  Settings_.on_value_substitution_complete = OnValueSubstitutionCompleteTr;
  Settings_.on_value_complete = OnValueCompleteTr;
  Settings_.on_pkgc_complete = nullptr;
}

llpkgc_file_errno_t cmPkgConfigFileParser::Parse(char const* input,
                                                 std::size_t len)
{
  return llpkgc_file_execute(static_cast<llpkgc_file_t*>(this), input, len);
}

llpkgc_file_errno_t cmPkgConfigFileParser::Finish()
{
  return llpkgc_file_finish(static_cast<llpkgc_file_t*>(this));
}

llpkgc_file_errno_t cmPkgConfigFileParser::GetErrno() const
{
  return llpkgc_file_get_errno(static_cast<llpkgc_file_t const*>(this));
}

char const* cmPkgConfigFileParser::ErrorReason() const
{
  return llpkgc_file_get_error_reason(static_cast<llpkgc_file_t const*>(this));
}

std::ptrdiff_t cmPkgConfigFileParser::ErrorOffset(char const* base) const
{
  char const* pos =
    llpkgc_file_get_error_pos(static_cast<llpkgc_file_t const*>(this));
  if (pos == nullptr) {
    return -1;
  }
  return pos - base;
}

int cmPkgConfigFileParser::OnIdentNext(char const* at, std::size_t len)
{
  Ident_.append(at, len);
  return 0;
}

int cmPkgConfigFileParser::OnIdentNextTr(llpkgc_file_t* parser, char const* at,
                                         std::size_t len)
{
  return static_cast<cmPkgConfigFileParser*>(parser)->OnIdentNext(at, len);
}

void cmPkgConfigFileParser::AppendValue(std::string text, bool isSubstitution)
{
  if (!isSubstitution && !Data_.empty() && !Data_.back().Value.empty() &&
      !Data_.back().Value.back().IsSubstitution) {
    Data_.back().Value.back().Text += text;
    return;
  }

  Data_.back().Value.emplace_back(std::move(text), isSubstitution);
}

int cmPkgConfigFileParser::OnSpanNext(char const* at, std::size_t len)
{
  CurrentValue_.append(at, len);
  return 0;
}

int cmPkgConfigFileParser::OnSpanNextTr(llpkgc_file_t* parser, char const* at,
                                        std::size_t len)
{
  return static_cast<cmPkgConfigFileParser*>(parser)->OnSpanNext(at, len);
}

int cmPkgConfigFileParser::OnIdent(char const* at, std::size_t len)
{
  Ident_.assign(at, len);
  Settings_.on_ident = OnIdentNextTr;
  return 0;
}

int cmPkgConfigFileParser::OnIdentTr(llpkgc_file_t* parser, char const* at,
                                     std::size_t len)
{
  return static_cast<cmPkgConfigFileParser*>(parser)->OnIdent(at, len);
}

int cmPkgConfigFileParser::OnPropertyComplete()
{
  Data_.emplace_back(std::vector<cmPkgConfigValueItem>{}, Ident_,
                     cmPkgConfigEntryKind::Property);
  Settings_.on_ident = OnIdentTr;
  return 0;
}

int cmPkgConfigFileParser::OnPropertyCompleteTr(llpkgc_file_t* parser)
{
  return static_cast<cmPkgConfigFileParser*>(parser)->OnPropertyComplete();
}

int cmPkgConfigFileParser::OnVariableComplete()
{
  Data_.emplace_back(std::vector<cmPkgConfigValueItem>{}, Ident_,
                     cmPkgConfigEntryKind::Variable);
  Settings_.on_ident = OnIdentTr;
  return 0;
}

int cmPkgConfigFileParser::OnVariableCompleteTr(llpkgc_file_t* parser)
{
  return static_cast<cmPkgConfigFileParser*>(parser)->OnVariableComplete();
}

int cmPkgConfigFileParser::OnBarewordsComplete()
{
  Data_.emplace_back(std::vector<cmPkgConfigValueItem>{}, Ident_,
                     cmPkgConfigEntryKind::Barewords);
  Settings_.on_ident = OnIdentTr;
  return 0;
}

int cmPkgConfigFileParser::OnBarewordsCompleteTr(llpkgc_file_t* parser)
{
  return static_cast<cmPkgConfigFileParser*>(parser)->OnBarewordsComplete();
}

int cmPkgConfigFileParser::OnValueLiteral(char const* at, std::size_t len)
{
  CurrentValue_.assign(at, len);
  Settings_.on_value_literal = OnSpanNextTr;
  return 0;
}

int cmPkgConfigFileParser::OnValueLiteralTr(llpkgc_file_t* parser,
                                            char const* at, std::size_t len)
{
  return static_cast<cmPkgConfigFileParser*>(parser)->OnValueLiteral(at, len);
}

int cmPkgConfigFileParser::OnValueLiteralComplete()
{
  Settings_.on_value_literal = OnValueLiteralTr;
  AppendValue(std::move(CurrentValue_), false);
  return 0;
}

int cmPkgConfigFileParser::OnValueLiteralCompleteTr(llpkgc_file_t* parser)
{
  return static_cast<cmPkgConfigFileParser*>(parser)->OnValueLiteralComplete();
}

int cmPkgConfigFileParser::OnValueSubstitution(char const* at, std::size_t len)
{
  CurrentValue_.assign(at, len);
  Settings_.on_value_substitution = OnSpanNextTr;
  return 0;
}

int cmPkgConfigFileParser::OnValueSubstitutionTr(llpkgc_file_t* parser,
                                                 char const* at,
                                                 std::size_t len)
{
  return static_cast<cmPkgConfigFileParser*>(parser)->OnValueSubstitution(at,
                                                                          len);
}

int cmPkgConfigFileParser::OnValueSubstitutionComplete()
{
  Settings_.on_value_substitution = OnValueSubstitutionTr;
  AppendValue(std::move(CurrentValue_), true);
  return 0;
}

int cmPkgConfigFileParser::OnValueSubstitutionCompleteTr(llpkgc_file_t* parser)
{
  return static_cast<cmPkgConfigFileParser*>(parser)
    ->OnValueSubstitutionComplete();
}

int cmPkgConfigFileParser::OnValueComplete()
{
  return 0;
}

int cmPkgConfigFileParser::OnValueCompleteTr(llpkgc_file_t* parser)
{
  return static_cast<cmPkgConfigFileParser*>(parser)->OnValueComplete();
}

cmPkgConfigFragmentParser::cmPkgConfigFragmentParser()
{
  llpkgc_frag_init(static_cast<llpkgc_frag_t*>(this), &Settings_);
}

llpkgc_frag_errno_t cmPkgConfigFragmentParser::Parse(char const* input,
                                                     std::size_t len)
{
  return llpkgc_frag_execute(static_cast<llpkgc_frag_t*>(this), input, len);
}

llpkgc_frag_errno_t cmPkgConfigFragmentParser::Finish()
{
  return llpkgc_frag_finish(static_cast<llpkgc_frag_t*>(this));
}

llpkgc_frag_errno_t cmPkgConfigFragmentParser::GetErrno() const
{
  return llpkgc_frag_get_errno(static_cast<llpkgc_frag_t const*>(this));
}

char const* cmPkgConfigFragmentParser::ErrorReason() const
{
  return llpkgc_frag_get_error_reason(static_cast<llpkgc_frag_t const*>(this));
}

int cmPkgConfigFragmentParser::OnFragmentText(char const* at, std::size_t len)
{
  CurrentFragment_.append(at, len);
  return 0;
}

int cmPkgConfigFragmentParser::OnFragmentTextTr(llpkgc_frag_t* parser,
                                                char const* at,
                                                std::size_t len)
{
  return static_cast<cmPkgConfigFragmentParser*>(parser)->OnFragmentText(at,
                                                                         len);
}

int cmPkgConfigFragmentParser::OnFragmentComplete()
{
  Fragments_.push_back(std::move(CurrentFragment_));
  CurrentFragment_.clear();
  return 0;
}

int cmPkgConfigFragmentParser::OnFragmentCompleteTr(llpkgc_frag_t* parser)
{
  return static_cast<cmPkgConfigFragmentParser*>(parser)->OnFragmentComplete();
}

cmPkgConfigDependencyParser::cmPkgConfigDependencyParser()
{
  llpkgc_dep_init(static_cast<llpkgc_dep_t*>(this), &Settings_);
}

llpkgc_dep_errno_t cmPkgConfigDependencyParser::Parse(char const* input,
                                                      std::size_t len)
{
  return llpkgc_dep_execute(static_cast<llpkgc_dep_t*>(this), input, len);
}

llpkgc_dep_errno_t cmPkgConfigDependencyParser::Finish()
{
  return llpkgc_dep_finish(static_cast<llpkgc_dep_t*>(this));
}

llpkgc_dep_errno_t cmPkgConfigDependencyParser::GetErrno() const
{
  return llpkgc_dep_get_errno(static_cast<llpkgc_dep_t const*>(this));
}

char const* cmPkgConfigDependencyParser::ErrorReason() const
{
  return llpkgc_dep_get_error_reason(static_cast<llpkgc_dep_t const*>(this));
}

int cmPkgConfigDependencyParser::OnDependencyName(char const* at,
                                                  std::size_t len)
{
  Current_.Name.append(at, len);
  return 0;
}

int cmPkgConfigDependencyParser::OnDependencyNameTr(llpkgc_dep_t* parser,
                                                    char const* at,
                                                    std::size_t len)
{
  return static_cast<cmPkgConfigDependencyParser*>(parser)->OnDependencyName(
    at, len);
}

int cmPkgConfigDependencyParser::OnDependencyVersion(char const* at,
                                                     std::size_t len)
{
  Current_.Version.append(at, len);
  return 0;
}

int cmPkgConfigDependencyParser::OnDependencyVersionTr(llpkgc_dep_t* parser,
                                                       char const* at,
                                                       std::size_t len)
{
  return static_cast<cmPkgConfigDependencyParser*>(parser)
    ->OnDependencyVersion(at, len);
}

int cmPkgConfigDependencyParser::OnDependencyComplete()
{
  Current_.Operation = static_cast<cmPkgConfigDependencyOperation>(
    llpkgc_dep_get_comparator(static_cast<llpkgc_dep_t*>(this)));
  Dependencies_.push_back(std::move(Current_));
  Current_ = cmPkgConfigDependencySpec{};
  return 0;
}

int cmPkgConfigDependencyParser::OnDependencyCompleteTr(llpkgc_dep_t* parser)
{
  return static_cast<cmPkgConfigDependencyParser*>(parser)
    ->OnDependencyComplete();
}

llpkgc_file_errno_t cmPkgConfigParser::Parse(char const* input,
                                             std::size_t len)
{
  return File_.Parse(input, len);
}

llpkgc_file_errno_t cmPkgConfigParser::Finish()
{
  return File_.Finish();
}

llpkgc_file_errno_t cmPkgConfigParser::Finish(char const* input,
                                              std::size_t len)
{
  auto err = Parse(input, len);
  if (err != PFCE_OK && err != PFCE_NEED_MORE) {
    return err;
  }
  return Finish();
}

llpkgc_file_errno_t cmPkgConfigParser::GetErrno() const
{
  return File_.GetErrno();
}

char const* cmPkgConfigParser::ErrorReason() const
{
  return File_.ErrorReason();
}

std::ptrdiff_t cmPkgConfigParser::ErrorOffset(char const* base) const
{
  return File_.ErrorOffset(base);
}

void cmPkgConfigParser::SetVariable(std::string name, std::string value)
{
  ValueItems variable;
  variable.emplace_back(std::move(value), false);
  StoredVariables_[std::move(name)] = std::move(variable);
  Cache_.clear();
}

std::string cmPkgConfigParser::GetVariable(std::string const& name)
{
  return ResolveValue(StoredVariables_[name]);
}

bool cmPkgConfigParser::HasProperty(std::string const& name) const
{
  return std::any_of(StoredProperties_.begin(), StoredProperties_.end(),
                     [&name](cmPkgConfigEntry const& entry) -> bool {
                       return entry.Ident == name;
                     });
}

std::vector<std::string> cmPkgConfigParser::GetFragments(
  std::string const& name)
{
  auto it = Cache_.find(name);
  if (it != Cache_.end()) {
    return it->second.Fragments;
  }

  cmPkgConfigResolvedValue cache;
  if (!ResolveAndParse(name, cache)) {
    return {};
  }
  return cache.Fragments;
}

std::vector<cmPkgConfigDependencySpec> cmPkgConfigParser::GetDependencies(
  std::string const& name)
{
  auto it = Cache_.find(name);
  if (it != Cache_.end()) {
    return it->second.Dependencies;
  }

  cmPkgConfigResolvedValue cache;
  if (!ResolveAndParse(name, cache)) {
    return {};
  }
  return cache.Dependencies;
}

std::string cmPkgConfigParser::GetLiteral(std::string const& name)
{
  auto it = Cache_.find(name);
  if (it != Cache_.end()) {
    return it->second.Literal;
  }

  cmPkgConfigResolvedValue cache;
  if (!ResolveAndParse(name, cache)) {
    return {};
  }
  return cache.Literal;
}

void cmPkgConfigParser::ParseComplete()
{
  StoredVariables_.clear();
  StoredProperties_.clear();
  Cache_.clear();

  for (auto const& entry : File_.Data_) {
    switch (entry.Kind) {
      case cmPkgConfigEntryKind::Variable:
        StoredVariables_[entry.Ident] = entry.Value;
        break;
      case cmPkgConfigEntryKind::Property:
        StoredProperties_.push_back(entry);
        break;
      default:
        break;
    }
  }
}

std::string cmPkgConfigParser::ResolveValue(ValueItems const& items)
{
  return ResolveValueImpl(items, {});
}

std::string cmPkgConfigParser::ResolveValueImpl(
  ValueItems const& items, std::set<std::string> resolving)
{
  std::string result;
  if (items.empty()) {
    return result;
  }

  for (auto const& item : items) {
    if (item.IsSubstitution) {
      if (resolving.count(item.Text)) {
        result += "${";
        result += item.Text;
        result += "}";
        continue;
      }
      auto it = StoredVariables_.find(item.Text);
      if (it != StoredVariables_.end()) {
        resolving.insert(item.Text);
        result += ResolveValueImpl(it->second, resolving);
        resolving.erase(item.Text);
      }
    } else {
      result += item.Text;
    }
  }

  return result;
}

bool cmPkgConfigParser::ResolveAndParse(std::string const& name,
                                        cmPkgConfigResolvedValue& cache)
{
  cmPkgConfigEntry const* entry = nullptr;
  for (auto const& candidate : StoredProperties_) {
    if (candidate.Ident == name) {
      entry = &candidate;
      break;
    }
  }
  if (entry == nullptr) {
    return false;
  }

  std::string resolved = ResolveValue(entry->Value);
  cache.Literal = resolved;

  auto kind = llpkgc_classify_field(name.data(), name.size());
  if (kind == LLPKGC_VALUE_FRAGMENTS) {
    cmPkgConfigFragmentParser fragmentParser;
    auto err = fragmentParser.Parse(resolved.data(), resolved.size());
    if (err == PFFE_OK && fragmentParser.Finish() == PFFE_OK) {
      cache.Fragments = fragmentParser.Fragments();
    }
  } else if (kind == LLPKGC_VALUE_DEPENDENCIES) {
    cmPkgConfigDependencyParser dependencyParser;
    auto err = dependencyParser.Parse(resolved.data(), resolved.size());
    if (err == PDEP_OK && dependencyParser.Finish() == PDEP_OK) {
      cache.Dependencies = dependencyParser.Dependencies();
    }
  }

  Cache_[name] = cache;
  return true;
}
