/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <string>

#include <cm/optional>
#include <cm/string_view>

#if !defined(CMake_USE_XCOFF_PARSER)
#  error "This file may be included only if CMake_USE_XCOFF_PARSER is enabled."
#endif

class cmXCOFFInternal;

/** \class cmXCOFF
 * \brief XCOFF parser.
 */
class cmXCOFF
{
public:
  enum class Mode
  {
    ReadOnly,
    ReadWrite
  };

  /** Construct with the name of the XCOFF input file to parse.  */
  cmXCOFF(const char* fname, Mode = Mode::ReadOnly);

  /** Destruct.   */
  ~cmXCOFF();

  cmXCOFF(cmXCOFF&&) noexcept;
  cmXCOFF(cmXCOFF const&) = delete;
  cmXCOFF& operator=(cmXCOFF&&) noexcept;
  cmXCOFF& operator=(cmXCOFF const&) = delete;

  /** Get the error message if any.  */
  std::string const& GetErrorMessage() const { return this->ErrorMessage; }

  /** Boolean conversion.  True if the XCOFF file is valid.  */
  explicit operator bool() const { return this->Valid(); }

  /** Get the LIBPATH (RPATH) parsed from the file, if any.  */
  cm::optional<cm::string_view> GetLibPath() const;

  /** Set the LIBPATH (RPATH).
      Works only if cmXCOFF was constructed with Mode::ReadWrite.  */
  bool SetLibPath(cm::string_view libPath);

  /** Remove the LIBPATH (RPATH).
      Works only if cmXCOFF was constructed with Mode::ReadWrite.  */
  bool RemoveLibPath();

private:
  friend class cmXCOFFInternal;
  bool Valid() const;
  std::unique_ptr<cmXCOFFInternal> Internal;
  std::string ErrorMessage;
};
