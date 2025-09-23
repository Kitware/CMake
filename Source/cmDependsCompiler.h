/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

#include "cmDepends.h"

class cmLocalUnixMakefileGenerator3;

/** \class cmDepends
 * \brief Dependencies files manager.
 *
 * This class is responsible for maintaining a compiler_depends.make file in
 * the build tree corresponding to an object file.
 */
class cmDependsCompiler
{
public:
  cmDependsCompiler() = default;
  ~cmDependsCompiler() = default;

  /** should this be verbose in its output */
  void SetVerbose(bool verb) { this->Verbose = verb; }

  /** Set the local generator for the directory in which we are
      scanning dependencies.  This is not a full local generator; it
      has been setup to do relative path conversions for the current
      directory.  */
  void SetLocalGenerator(cmLocalUnixMakefileGenerator3* lg)
  {
    this->LocalGenerator = lg;
  }

  /** Read dependencies for the target file. Return true if
      dependencies didn't changed and false if not.
      Up-to-date Dependencies will be stored in deps. */
  bool CheckDependencies(
    std::string const& internalDepFile,
    std::vector<std::string> const& depFiles,
    cmDepends::DependencyMap& dependencies,
    std::function<bool(std::string const&)> const& isValidPath);

  /** Write dependencies for the target file.  */
  void WriteDependencies(cmDepends::DependencyMap const& dependencies,
                         std::ostream& makeDepends,
                         std::ostream& internalDepends);

  /** Clear dependencies for the target so they will be regenerated.  */
  void ClearDependencies(std::vector<std::string> const& depFiles);

private:
  bool Verbose = false;
  cmLocalUnixMakefileGenerator3* LocalGenerator = nullptr;
};
