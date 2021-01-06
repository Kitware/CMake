/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>

/** \class cmFileTimes
 * \brief Loads and stores file times.
 */
class cmFileTimes
{
public:
  cmFileTimes();
  //! Calls Load()
  cmFileTimes(std::string const& fileName);
  ~cmFileTimes();

  //! @return true, if file times were loaded successfully
  bool IsValid() const { return (this->times != nullptr); }
  //! Try to load the file times from @a fileName and @return IsValid()
  bool Load(std::string const& fileName);
  //! Stores the file times at @a fileName (if IsValid())
  bool Store(std::string const& fileName) const;

  //! Copies the file times of @a fromFile to @a toFile
  static bool Copy(std::string const& fromFile, std::string const& toFile);

private:
#ifdef _WIN32
  class WindowsHandle;
#endif
  class Times;
  std::unique_ptr<Times> times;
};
