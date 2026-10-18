/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <string>

/** \brief Hit count of one function in an lcov tracefile.  */
struct cmLcovFunction
{
  // A constructor, so that brace init such as { "f1", 2 } works in C++11.
  cmLcovFunction(std::string name = std::string(),
                 unsigned long long hits = 0);

  // The name as written, without any "@<line>" suffix.
  std::string Name;
  unsigned long long Hits;
};

/** \brief Line and function hit counts of one source file.  */
struct cmLcovSourceFile
{
  /** True if the file has a line or a function record.  */
  bool HasData() const;

  // DA records: line -> hits.
  std::map<long, unsigned long long> Lines;
  // FN records: definition line -> data.
  std::map<long, cmLcovFunction> Functions;
};

/** \class cmLcovTracefile
 * \brief Read, merge and write lcov tracefiles.
 *
 * The model holds the line and function records of the lcov tracefile
 * format.  Branch records are not kept.
 */
class cmLcovTracefile
{
public:
  // TN record, empty means none.
  std::string TestName;
  // SF path -> data.
  std::map<std::string, cmLcovSourceFile> Files;

  /** Add the counts of another tracefile.  Lines and functions are united
      and their hits added.  A function keeps its existing name.  The test
      name does not change.  */
  void Merge(cmLcovTracefile const& other);

  /** True if some file has a line or a function record.  */
  bool HasData() const;

  /** Write the tracefile in lcov format.  Leave out the files that have
      no line and no function record.  */
  void Write(std::ostream& os) const;

  /** Read a tracefile in lcov format and add its records to this object.
      Callers read into a fresh object.  On failure the object holds
      partial data and the caller must discard it.  */
  bool Read(std::istream& is, std::string& error);
};
