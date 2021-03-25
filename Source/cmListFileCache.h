/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmStateSnapshot.h"
#include "cmSystemTools.h"

/** \class cmListFileCache
 * \brief A class to cache list file contents.
 *
 * cmListFileCache is a class used to cache the contents of parsed
 * cmake list files.
 */

class cmMessenger;

struct cmCommandContext
{
  struct cmCommandName
  {
    std::string Original;
    std::string Lower;
    cmCommandName() = default;
    cmCommandName(std::string name)
      : Original(std::move(name))
      , Lower(cmSystemTools::LowerCase(this->Original))
    {
    }
  } Name;
  long Line = 0;
  cmCommandContext() = default;
  cmCommandContext(std::string name, long line)
    : Name(std::move(name))
    , Line(line)
  {
  }
};

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

class cmListFileContext
{
public:
  std::string Name;
  std::string FilePath;
  long Line = 0;
  static long const DeferPlaceholderLine = -1;
  cm::optional<std::string> DeferId;

  cmListFileContext() = default;
  cmListFileContext(std::string name, std::string filePath, long line)
    : Name(std::move(name))
    , FilePath(std::move(filePath))
    , Line(line)
  {
  }

#if __cplusplus < 201703L && (!defined(_MSVC_LANG) || _MSVC_LANG < 201703L)
  cmListFileContext(const cmListFileContext& /*other*/) = default;
  cmListFileContext(cmListFileContext&& /*other*/) = default;

  cmListFileContext& operator=(const cmListFileContext& /*other*/) = default;
  cmListFileContext& operator=(cmListFileContext&& /*other*/) = delete;
#endif

  static cmListFileContext FromCommandContext(
    cmCommandContext const& lfcc, std::string const& fileName,
    cm::optional<std::string> deferId = {})
  {
    cmListFileContext lfc;
    lfc.FilePath = fileName;
    lfc.Line = lfcc.Line;
    lfc.Name = lfcc.Name.Original;
    lfc.DeferId = std::move(deferId);
    return lfc;
  }
};

std::ostream& operator<<(std::ostream&, cmListFileContext const&);
bool operator<(const cmListFileContext& lhs, const cmListFileContext& rhs);
bool operator==(cmListFileContext const& lhs, cmListFileContext const& rhs);
bool operator!=(cmListFileContext const& lhs, cmListFileContext const& rhs);

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
    return this->Impl->Name.Original;
  }

  std::string const& LowerCaseName() const noexcept
  {
    return this->Impl->Name.Lower;
  }

  long Line() const noexcept { return this->Impl->Line; }

  std::vector<cmListFileArgument> const& Arguments() const noexcept
  {
    return this->Impl->Arguments;
  }

  operator cmCommandContext const&() const noexcept { return *this->Impl; }

private:
  struct Implementation : public cmCommandContext
  {
    Implementation(std::string name, long line,
                   std::vector<cmListFileArgument> args)
      : cmCommandContext{ std::move(name), line }
      , Arguments{ std::move(args) }
    {
    }
    std::vector<cmListFileArgument> Arguments;
  };

  std::shared_ptr<Implementation const> Impl;
};

// Represent a backtrace (call stack).  Provide value semantics
// but use efficient reference-counting underneath to avoid copies.
class cmListFileBacktrace
{
public:
  // Default-constructed backtrace may not be used until after
  // set via assignment from a backtrace constructed with a
  // valid snapshot.
  cmListFileBacktrace() = default;

  // Construct an empty backtrace whose bottom sits in the directory
  // indicated by the given valid snapshot.
  cmListFileBacktrace(cmStateSnapshot const& snapshot);

  cmStateSnapshot GetBottom() const;

  // Get a backtrace with the given file scope added to the top.
  // May not be called until after construction with a valid snapshot.
  cmListFileBacktrace Push(std::string const& file) const;

  // Get a backtrace with the given call context added to the top.
  // May not be called until after construction with a valid snapshot.
  cmListFileBacktrace Push(cmListFileContext const& lfc) const;

  // Get a backtrace with the top level removed.
  // May not be called until after a matching Push.
  cmListFileBacktrace Pop() const;

  // Get the context at the top of the backtrace.
  // This may be called only if Empty() would return false.
  cmListFileContext const& Top() const;

  // Print the top of the backtrace.
  void PrintTitle(std::ostream& out) const;

  // Print the call stack below the top of the backtrace.
  void PrintCallStack(std::ostream& out) const;

  // Get the number of 'frames' in this backtrace
  size_t Depth() const;

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
