/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

/** \class cmCallVisualStudioMacro
 * \brief Control class for communicating with CMake's Visual Studio macros
 *
 * Find running instances of Visual Studio by full path solution name.
 * Call a Visual Studio IDE macro in any of those instances.
 */
class cmCallVisualStudioMacro
{
public:
  //! Call the named macro in instances of Visual Studio with the
  //! given solution file open. Pass "ALL" for slnFile to call the
  //! macro in each Visual Studio instance.
  static int CallMacro(std::string const& slnFile, std::string const& macro,
                       std::string const& args, bool logErrorsAsMessages);

  //! Count the number of running instances of Visual Studio with the
  //! given solution file open. Pass "ALL" for slnFile to count all
  //! running Visual Studio instances.
  static int GetNumberOfRunningVisualStudioInstances(
    std::string const& slnFile);

protected:
private:
};
