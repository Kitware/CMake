/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <string>

#if !defined(CMake_USE_MACH_PARSER)
#  error "This file may be included only if CMake_USE_MACH_PARSER is enabled."
#endif

class cmMachOInternal;

/** \class cmMachO
 * \brief Executable and Link Format (Mach-O) parser.
 */
class cmMachO
{
public:
  /** Construct with the name of the Mach-O input file to parse.  */
  cmMachO(const char* fname);

  /** Destruct.   */
  ~cmMachO();

  /** Get the error message if any.  */
  std::string const& GetErrorMessage() const;

  /** Boolean conversion.  True if the Mach-O file is valid.  */
  explicit operator bool() const { return this->Valid(); }

  /** Get Install name from binary **/
  bool GetInstallName(std::string& install_name);

  /** Print human-readable information about the Mach-O file.  */
  void PrintInfo(std::ostream& os) const;

private:
  friend class cmMachOInternal;
  bool Valid() const;
  std::unique_ptr<cmMachOInternal> Internal;
};
