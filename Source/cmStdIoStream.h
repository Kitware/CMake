/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstdio>
#include <iosfwd>

namespace cm {
namespace StdIo {

/**
 * Identify the kind of terminal to which a stream is attached, if any.
 */
enum class TermKind
{
  /** Not an interactive terminal.  */
  None,
  /** A VT100 terminal.  */
  VT100,
#ifdef _WIN32
  /** A Windows Console that does not support VT100 sequences.  */
  Console,
#endif
};

/**
 * Represent stdin, stdout, or stderr stream metadata.
 */
class Stream
{
public:
  /** The kind of terminal to which the stream is attached, if any.  */
  TermKind Kind() const { return this->Kind_; }

  /** The underlying C++ stream.  */
  std::ios& IOS() const { return this->IOS_; }

  /** The underlying file descriptor.  */
  int FD() const { return this->FD_; }

#ifdef _WIN32
  /** The underlying HANDLE of an attached Windows Console, if any.  */
  void* Console() const { return this->Console_; }
#endif

protected:
  enum class Direction
  {
    In,
    Out,
  };

  Stream(std::ios& s, FILE* file, Direction direction);
  ~Stream(); // NOLINT(performance-trivially-destructible)

private:
  std::ios& IOS_;
  int FD_ = -1;
  TermKind Kind_ = TermKind::None;

#ifdef _WIN32
  void* Console_ = nullptr;
  unsigned long ConsoleOrigMode_ = 0;
#endif
};

/**
 * Represent stdin metadata.
 */
class IStream : public Stream
{
  friend class Globals;
  IStream(std::istream& is, FILE* file);

public:
  /** The underlying C++ stream.  */
  std::istream& IOS() const;
};

/**
 * Represent stdout or stderr metadata.
 */
class OStream : public Stream
{
  friend class Globals;
  OStream(std::ostream& os, FILE* file);

public:
  /** The underlying C++ stream.  */
  std::ostream& IOS() const;
};

/** Metadata for stdin.  */
IStream& In();

/** Metadata for stdout.  */
OStream& Out();

/** Metadata for stderr.  */
OStream& Err();

}
}
