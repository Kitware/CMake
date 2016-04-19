/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmListFileCache_h
#define cmListFileCache_h

#include "cmStandardIncludes.h"

#include "cmState.h"

/** \class cmListFileCache
 * \brief A class to cache list file contents.
 *
 * cmListFileCache is a class used to cache the contents of parsed
 * cmake list files.
 */

class cmMakefile;

struct cmCommandContext
{
  std::string Name;
  long Line;
  cmCommandContext(): Name(), Line(0) {}
};

struct cmListFileArgument
{
  enum Delimiter
    {
    Unquoted,
    Quoted,
    Bracket
    };
  cmListFileArgument(): Value(), Delim(Unquoted), Line(0) {}
  cmListFileArgument(const cmListFileArgument& r)
    : Value(r.Value), Delim(r.Delim), Line(r.Line) {}
  cmListFileArgument(const std::string& v, Delimiter d, long line)
    : Value(v), Delim(d), Line(line) {}
  bool operator == (const cmListFileArgument& r) const
    {
    return (this->Value == r.Value) && (this->Delim == r.Delim);
    }
  bool operator != (const cmListFileArgument& r) const
    {
    return !(*this == r);
    }
  std::string Value;
  Delimiter Delim;
  long Line;
};

class cmListFileContext
{
public:
  std::string Name;
  std::string FilePath;
  long Line;
  cmListFileContext(): Name(), FilePath(), Line(0) {}

  static cmListFileContext FromCommandContext(cmCommandContext const& lfcc,
                                              std::string const& fileName)
  {
    cmListFileContext lfc;
    lfc.FilePath = fileName;
    lfc.Line = lfcc.Line;
    lfc.Name = lfcc.Name;
    return lfc;
  }
};

std::ostream& operator<<(std::ostream&, cmListFileContext const&);
bool operator<(const cmListFileContext& lhs, const cmListFileContext& rhs);
bool operator==(cmListFileContext const& lhs, cmListFileContext const& rhs);
bool operator!=(cmListFileContext const& lhs, cmListFileContext const& rhs);

struct cmListFileFunction: public cmCommandContext
{
  std::vector<cmListFileArgument> Arguments;
};

// Represent a backtrace (call stack).  Provide value semantics
// but use efficient reference-counting underneath to avoid copies.
class cmListFileBacktrace
{
public:
  // Default-constructed backtrace may not be used until after
  // set via assignment from a backtrace constructed with a
  // valid snapshot.
  cmListFileBacktrace();

  // Construct an empty backtrace whose bottom sits in the directory
  // indicated by the given valid snapshot.
  cmListFileBacktrace(cmState::Snapshot snapshot);

  // Backtraces may be copied and assigned as values.
  cmListFileBacktrace(cmListFileBacktrace const& r);
  cmListFileBacktrace& operator=(cmListFileBacktrace const& r);
  ~cmListFileBacktrace();

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
  // Returns an empty context if the backtrace is empty.
  cmListFileContext const& Top() const;

  // Print the top of the backtrace.
  void PrintTitle(std::ostream& out) const;

  // Print the call stack below the top of the backtrace.
  void PrintCallStack(std::ostream& out) const;
private:
  struct Entry;
  cmState::Snapshot Bottom;
  Entry* Cur;
  cmListFileBacktrace(cmState::Snapshot bottom, Entry* up,
                      cmListFileContext const& lfc);
  cmListFileBacktrace(cmState::Snapshot bottom, Entry* cur);
};

struct cmListFile
{
  bool ParseFile(const char* path,
                 bool topLevel,
                 cmMakefile *mf);

  std::vector<cmListFileFunction> Functions;
};

#endif
