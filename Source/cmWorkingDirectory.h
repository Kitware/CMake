/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

/** \class cmWorkingDirectory
 * \brief An RAII class to manipulate the working directory.
 *
 * The current working directory is set to the location given to the
 * constructor. The working directory can be changed again as needed
 * by calling SetDirectory(). When the object is destroyed, the destructor
 * will restore the working directory to what it was when the object was
 * created, regardless of any calls to SetDirectory() in the meantime.
 */
class cmWorkingDirectory
{
public:
  cmWorkingDirectory(std::string const& newdir);
  ~cmWorkingDirectory();

  cmWorkingDirectory(cmWorkingDirectory const&) = delete;
  cmWorkingDirectory& operator=(cmWorkingDirectory const&) = delete;

  bool SetDirectory(std::string const& newdir);
  void Pop();
  bool Failed() const { return !this->Error.empty(); }
  std::string const& GetError() const { return this->Error; }
  std::string const& GetOldDirectory() const { return this->OldDir; }

private:
  std::string OldDir;
  std::string Error;
};
