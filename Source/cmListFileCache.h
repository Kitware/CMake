/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmSystemTools.h"

/** \class cmListFileCache
 * \brief A class to cache list file contents.
 *
 * cmListFileCache is a class used to cache the contents of parsed
 * cmake list files.
 */

class cmMessenger;

struct cmListFileArgument
{
  enum Delimiter
  {
    Unquoted,
    Quoted,
    Bracket
  };
  cmListFileArgument() = default;
  cmListFileArgument(std::string v, Delimiter d, long line)
    : Value(std::move(v))
    , Delim(d)
    , Line(line)
  {
  }
  bool operator==(const cmListFileArgument& r) const
  {
    return (this->Value == r.Value) && (this->Delim == r.Delim);
  }
  bool operator!=(const cmListFileArgument& r) const { return !(*this == r); }
  std::string Value;
  Delimiter Delim = Unquoted;
  long Line = 0;
};

class cmListFileFunction
{
public:
  cmListFileFunction(std::string name, long line,
                     std::vector<cmListFileArgument> args)
    : Impl{ std::make_shared<Implementation>(std::move(name), line,
                                             std::move(args)) }
  {
  }

  std::string const& OriginalName() const noexcept
  {
    return this->Impl->OriginalName;
  }

  std::string const& LowerCaseName() const noexcept
  {
    return this->Impl->LowerCaseName;
  }

  long Line() const noexcept { return this->Impl->Line; }

  std::vector<cmListFileArgument> const& Arguments() const noexcept
  {
    return this->Impl->Arguments;
  }

private:
  struct Implementation
  {
    Implementation(std::string name, long line,
                   std::vector<cmListFileArgument> args)
      : OriginalName{ std::move(name) }
      , LowerCaseName{ cmSystemTools::LowerCase(this->OriginalName) }
      , Line{ line }
      , Arguments{ std::move(args) }
    {
    }

    std::string OriginalName;
    std::string LowerCaseName;
    long Line = 0;
    std::vector<cmListFileArgument> Arguments;
  };

  std::shared_ptr<Implementation const> Impl;
};

class cmListFileContext
{
public:
  std::string Name;
  std::string FilePath;
  long Line = 0;
  static long const DeferPlaceholderLine = -1;
  cm::optional<std::string> DeferId;

  cmListFileContext() = default;
  cmListFileContext(cmListFileContext&& /*other*/) = default;
  cmListFileContext(const cmListFileContext& /*other*/) = default;
  cmListFileContext& operator=(const cmListFileContext& /*other*/) = default;
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
  cmListFileContext& operator=(cmListFileContext&& /*other*/) = default;
#else
  // The move assignment operators for several STL classes did not become
  // noexcept until C++17, which causes some tools to warn about this move
  // assignment operator throwing an exception when it shouldn't.
  cmListFileContext& operator=(cmListFileContext&& /*other*/) = delete;
#endif

  cmListFileContext(std::string name, std::string filePath, long line)
    : Name(std::move(name))
    , FilePath(std::move(filePath))
    , Line(line)
  {
  }

  static cmListFileContext FromListFileFunction(
    cmListFileFunction const& lff, std::string const& fileName,
    cm::optional<std::string> deferId = {})
  {
    cmListFileContext lfc;
    lfc.FilePath = fileName;
    lfc.Line = lff.Line();
    lfc.Name = lff.OriginalName();
    lfc.DeferId = std::move(deferId);
    return lfc;
  }
};

std::ostream& operator<<(std::ostream&, cmListFileContext const&);
bool operator<(const cmListFileContext& lhs, const cmListFileContext& rhs);
bool operator==(cmListFileContext const& lhs, cmListFileContext const& rhs);
bool operator!=(cmListFileContext const& lhs, cmListFileContext const& rhs);

// Represent a backtrace (call stack).  Provide value semantics
// but use efficient reference-counting underneath to avoid copies.
class cmListFileBacktrace
{
public:
  // Default-constructed backtrace is empty.
  cmListFileBacktrace() = default;

  // Get a backtrace with the given file scope added to the top.
  cmListFileBacktrace Push(std::string const& file) const;

  // Get a backtrace with the given call context added to the top.
  cmListFileBacktrace Push(cmListFileContext const& lfc) const;

  // Get a backtrace with the top level removed.
  // May not be called until after a matching Push.
  cmListFileBacktrace Pop() const;

  // Get the context at the top of the backtrace.
  // This may be called only if Empty() would return false.
  cmListFileContext const& Top() const;

  // Return true if this backtrace is empty.
  bool Empty() const;

private:
  struct Entry;
  std::shared_ptr<Entry const> TopEntry;
  cmListFileBacktrace(std::shared_ptr<Entry const> parent,
                      cmListFileContext const& lfc);
  cmListFileBacktrace(std::shared_ptr<Entry const> top);
};

// Wrap type T as a value with a backtrace.  For purposes of
// ordering and equality comparison, only the original value is
// used.  The backtrace is considered incidental.
template <typename T>
class BT
{
public:
  BT(T v = T(), cmListFileBacktrace bt = cmListFileBacktrace())
    : Value(std::move(v))
    , Backtrace(std::move(bt))
  {
  }
  T Value;
  cmListFileBacktrace Backtrace;
  friend bool operator==(BT<T> const& l, BT<T> const& r)
  {
    return l.Value == r.Value;
  }
  friend bool operator<(BT<T> const& l, BT<T> const& r)
  {
    return l.Value < r.Value;
  }
  friend bool operator==(BT<T> const& l, T const& r) { return l.Value == r; }
  friend bool operator==(T const& l, BT<T> const& r) { return l == r.Value; }
};

std::ostream& operator<<(std::ostream& os, BT<std::string> const& s);

// Wrap type T as a value with potentially multiple backtraces.  For purposes
// of ordering and equality comparison, only the original value is used.  The
// backtrace is considered incidental.
template <typename T>
class BTs
{
public:
  BTs(T v = T(), cmListFileBacktrace bt = cmListFileBacktrace())
    : Value(std::move(v))
  {
    this->Backtraces.emplace_back(std::move(bt));
  }
  T Value;
  std::vector<cmListFileBacktrace> Backtraces;
  friend bool operator==(BTs<T> const& l, BTs<T> const& r)
  {
    return l.Value == r.Value;
  }
  friend bool operator<(BTs<T> const& l, BTs<T> const& r)
  {
    return l.Value < r.Value;
  }
  friend bool operator==(BTs<T> const& l, T const& r) { return l.Value == r; }
  friend bool operator==(T const& l, BTs<T> const& r) { return l == r.Value; }
};

std::vector<BT<std::string>> ExpandListWithBacktrace(
  std::string const& list,
  cmListFileBacktrace const& bt = cmListFileBacktrace());

struct cmListFile
{
  bool ParseFile(const char* path, cmMessenger* messenger,
                 cmListFileBacktrace const& lfbt);

  bool ParseString(const char* str, const char* virtual_filename,
                   cmMessenger* messenger, cmListFileBacktrace const& lfbt);

  std::vector<cmListFileFunction> Functions;
};
