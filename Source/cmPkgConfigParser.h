/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <cmllpkgc/llpkgc.h>

struct cmPkgConfigValueItem
{
  cmPkgConfigValueItem() = default;
  cmPkgConfigValueItem(std::string text, bool isSubstitution);

  std::string Text;
  bool IsSubstitution = false;
};

enum class cmPkgConfigEntryKind
{
  Property,
  Variable,
  Barewords,
};

struct cmPkgConfigEntry
{
  cmPkgConfigEntry() = default;
  cmPkgConfigEntry(std::vector<cmPkgConfigValueItem> value, std::string ident,
                   cmPkgConfigEntryKind kind);

  std::vector<cmPkgConfigValueItem> Value;
  std::string Ident;
  cmPkgConfigEntryKind Kind = cmPkgConfigEntryKind::Barewords;
};

class cmPkgConfigFileParser : private llpkgc_file_t
{
  friend class cmPkgConfigParser;

public:
  cmPkgConfigFileParser();
  cmPkgConfigFileParser(cmPkgConfigFileParser const& other);
  cmPkgConfigFileParser(cmPkgConfigFileParser&& other) noexcept;
  cmPkgConfigFileParser& operator=(cmPkgConfigFileParser const& other);
  cmPkgConfigFileParser& operator=(cmPkgConfigFileParser&& other) noexcept;

  llpkgc_file_errno_t Parse(char const* input, std::size_t len);
  llpkgc_file_errno_t Finish();

  llpkgc_file_errno_t GetErrno() const;
  char const* ErrorReason() const;
  std::ptrdiff_t ErrorOffset(char const* base) const;

private:
  void ResetCallbacks();

  int OnIdentNext(char const* at, std::size_t len);
  static int OnIdentNextTr(llpkgc_file_t* parser, char const* at,
                           std::size_t len);

  void AppendValue(std::string text, bool isSubstitution);

  int OnSpanNext(char const* at, std::size_t len);
  static int OnSpanNextTr(llpkgc_file_t* parser, char const* at,
                          std::size_t len);

  int OnIdent(char const* at, std::size_t len);
  static int OnIdentTr(llpkgc_file_t* parser, char const* at, std::size_t len);

  int OnPropertyComplete();
  static int OnPropertyCompleteTr(llpkgc_file_t* parser);

  int OnVariableComplete();
  static int OnVariableCompleteTr(llpkgc_file_t* parser);

  int OnBarewordsComplete();
  static int OnBarewordsCompleteTr(llpkgc_file_t* parser);

  int OnValueLiteral(char const* at, std::size_t len);
  static int OnValueLiteralTr(llpkgc_file_t* parser, char const* at,
                              std::size_t len);

  int OnValueLiteralComplete();
  static int OnValueLiteralCompleteTr(llpkgc_file_t* parser);

  int OnValueSubstitution(char const* at, std::size_t len);
  static int OnValueSubstitutionTr(llpkgc_file_t* parser, char const* at,
                                   std::size_t len);

  int OnValueSubstitutionComplete();
  static int OnValueSubstitutionCompleteTr(llpkgc_file_t* parser);

  int OnValueComplete();
  static int OnValueCompleteTr(llpkgc_file_t* parser);

  llpkgc_file_settings_t Settings_{
    OnIdentTr,
    OnValueLiteralTr,
    OnValueSubstitutionTr,
    OnPropertyCompleteTr,
    OnVariableCompleteTr,
    OnBarewordsCompleteTr,
    OnValueLiteralCompleteTr,
    OnValueSubstitutionCompleteTr,
    OnValueCompleteTr,
    nullptr, // on_pkgc_complete
  };

  std::string CurrentValue_;
  std::string Ident_;
  std::vector<cmPkgConfigEntry> Data_;
};

class cmPkgConfigFragmentParser : private llpkgc_frag_t
{
public:
  cmPkgConfigFragmentParser();

  llpkgc_frag_errno_t Parse(char const* input, std::size_t len);
  llpkgc_frag_errno_t Finish();

  llpkgc_frag_errno_t GetErrno() const;
  char const* ErrorReason() const;

  std::vector<std::string> const& Fragments() const { return Fragments_; }

private:
  int OnFragmentText(char const* at, std::size_t len);
  static int OnFragmentTextTr(llpkgc_frag_t* parser, char const* at,
                              std::size_t len);

  int OnFragmentComplete();
  static int OnFragmentCompleteTr(llpkgc_frag_t* parser);

  llpkgc_frag_settings_t Settings_{
    OnFragmentTextTr, OnFragmentCompleteTr,
    nullptr, // on_fraglist_complete
  };

  std::string CurrentFragment_;
  std::vector<std::string> Fragments_;
};

enum class cmPkgConfigDependencyOperation
{
  None = LLPKGC_DEP_ANY,
  LessThan = LLPKGC_DEP_LESS_THAN,
  LessThanEqual = LLPKGC_DEP_LESS_THAN_EQUAL,
  Equal = LLPKGC_DEP_EQUAL,
  NotEqual = LLPKGC_DEP_NOT_EQUAL,
  GreaterThan = LLPKGC_DEP_GREATER_THAN,
  GreaterThanEqual = LLPKGC_DEP_GREATER_THAN_EQUAL,
};

struct cmPkgConfigDependencySpec
{
  std::string Name;
  cmPkgConfigDependencyOperation Operation =
    cmPkgConfigDependencyOperation::None;
  std::string Version;
};

class cmPkgConfigDependencyParser : private llpkgc_dep_t
{
public:
  cmPkgConfigDependencyParser();

  llpkgc_dep_errno_t Parse(char const* input, std::size_t len);
  llpkgc_dep_errno_t Finish();

  llpkgc_dep_errno_t GetErrno() const;
  char const* ErrorReason() const;

  std::vector<cmPkgConfigDependencySpec> const& Dependencies() const
  {
    return Dependencies_;
  }

private:
  int OnDependencyName(char const* at, std::size_t len);
  static int OnDependencyNameTr(llpkgc_dep_t* parser, char const* at,
                                std::size_t len);

  int OnDependencyVersion(char const* at, std::size_t len);
  static int OnDependencyVersionTr(llpkgc_dep_t* parser, char const* at,
                                   std::size_t len);

  int OnDependencyComplete();
  static int OnDependencyCompleteTr(llpkgc_dep_t* parser);

  llpkgc_dep_settings_t Settings_{
    OnDependencyNameTr, OnDependencyVersionTr, OnDependencyCompleteTr,
    nullptr, // on_dep_list_complete
  };

  cmPkgConfigDependencySpec Current_;
  std::vector<cmPkgConfigDependencySpec> Dependencies_;
};

struct cmPkgConfigResolvedValue
{
  std::vector<std::string> Fragments;
  std::vector<cmPkgConfigDependencySpec> Dependencies;
  std::string Literal;
};

class cmPkgConfigParser
{
public:
  cmPkgConfigParser() = default;

  llpkgc_file_errno_t Parse(char const* input, std::size_t len);
  llpkgc_file_errno_t Finish();
  llpkgc_file_errno_t Finish(char const* data, std::size_t len);

  llpkgc_file_errno_t GetErrno() const;
  char const* ErrorReason() const;
  std::ptrdiff_t ErrorOffset(char const* base) const;

  void SetVariable(std::string name, std::string value);
  std::string GetVariable(std::string const& name);
  bool HasProperty(std::string const& name) const;
  std::vector<std::string> GetFragments(std::string const& name);
  std::vector<cmPkgConfigDependencySpec> GetDependencies(
    std::string const& name);
  std::string GetLiteral(std::string const& name);

  void ParseComplete();

private:
  using ValueItems = std::vector<cmPkgConfigValueItem>;

  std::string ResolveValue(ValueItems const& items);
  std::string ResolveValueImpl(ValueItems const& items,
                               std::set<std::string> resolving);

  bool ResolveAndParse(std::string const& name,
                       cmPkgConfigResolvedValue& cache);

  cmPkgConfigFileParser File_;
  std::unordered_map<std::string, ValueItems> StoredVariables_;
  std::vector<cmPkgConfigEntry> StoredProperties_;
  std::unordered_map<std::string, cmPkgConfigResolvedValue> Cache_;
};
