/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmsys/Status.hxx"

namespace cm {
namespace PathResolver {

class System;

/** Normalize filesystem paths according to a Policy.
 *
 * Resolved paths are always absolute, have no '..', '.', or empty
 * components, and have a trailing '/' if and only if the entire
 * path is a root component.
 *
 * The Policy determines behavior w.r.t. symbolic links, existence,
 * and matching the on-disk case (upper/lower) of existing paths.
 */
template <class Policy>
class Resolver
{
  System& OS;

public:
  /** Construct with a concrete filesystem access implementation.  */
  Resolver(System& os);

  /** Resolve the input path according to the Policy, if possible.
      On success, the resolved path is stored in 'out'.
      On failure, the non-existent path is stored in 'out'.  */
  cmsys::Status Resolve(std::string in, std::string& out) const;
};

/** Access the filesystem via runtime dispatch.
    This allows unit tests to work without accessing a real filesystem,
    which is particularly important on Windows where symbolic links
    may not be something we can create without administrator privileges.
    */
class System
{
public:
  System();
  virtual ~System() = 0;

  /** If the given path is a symbolic link, read its target.
      If the path exists but is not a symbolic link, fail
      with EINVAL or ERROR_NOT_A_REPARSE_POINT.  */
  virtual cmsys::Status ReadSymlink(std::string const& path,
                                    std::string& symlink_target) = 0;

  /** Return whether the given path exists on disk.  */
  virtual bool PathExists(std::string const& path) = 0;

  /** Get the process's working directory.  */
  virtual std::string GetWorkingDirectory() = 0;

#ifdef _WIN32
  /** Get the process's working directory on a Windows drive letter.
      This is a legacy DOS concept supported by 'cmd' shells.  */
  virtual std::string GetWorkingDirectoryOnDrive(char drive_letter) = 0;
#endif

#if defined(_WIN32) || defined(__APPLE__)
  /** Read the on-disk spelling of the last component of a file path.  */
  virtual cmsys::Status ReadName(std::string const& path,
                                 std::string& name) = 0;
#endif
};

namespace Policies {
// IWYU pragma: begin_exports

/** Normalizes paths while resolving symlinks only when followed
    by '..' components.  Does not require paths to exist, but
    reads on-disk case of paths that do exist (on Windows and macOS).  */
struct LogicalPath;

/** Normalizes paths while resolving all symlinks.  Requires paths to exist,
    and reads their on-disk case (on Windows and macOS).  */
struct RealPath;

/** Normalizes paths while assuming components followed by '..'
    components are not symlinks.  Does not require paths to exist, but
    reads on-disk case of paths that do exist (on Windows and macOS).  */
struct CasePath;

/** Normalizes paths in memory without disk access.
    Assumes components followed by '..' components are not symlinks.  */
struct NaivePath;

// IWYU pragma: end_exports
}

extern template class Resolver<Policies::LogicalPath>;
extern template class Resolver<Policies::RealPath>;
extern template class Resolver<Policies::CasePath>;
extern template class Resolver<Policies::NaivePath>;

}
}
