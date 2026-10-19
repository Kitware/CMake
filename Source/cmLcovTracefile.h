/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

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

  /** Read the tracefiles at PATHS and merge them into this object.  Skip a
      file that can not be read, and call WARN with a message for it.
      Return false if a file was skipped.  */
  bool MergeFiles(std::vector<std::string> const& paths,
                  std::function<void(std::string const&)> const& warn);

  /** Remove the files under BUILDDIR, such as generated files and
      try_compile projects, and call REMOVED with the path of each one.
      Remove nothing if BUILDDIR is empty or is SOURCEDIR, as in an
      in-source build.  */
  void RemoveBuildTreeFiles(
    std::string const& sourceDir, std::string const& buildDir,
    std::function<void(std::string const&)> const& removed);

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
