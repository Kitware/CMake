/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmELF_h
#define cmELF_h

#if !defined(CMAKE_USE_ELF_PARSER)
# error "This file may be included only if CMAKE_USE_ELF_PARSER is enabled."
#endif

class cmELFInternal;

/** \class cmELF
 * \brief Executable and Link Format (ELF) parser.
 */
class cmELF
{
public:
  /** Construct with the name of the ELF input file to parse.  */
  cmELF(const char* fname);

  /** Destruct.   */
  ~cmELF();

  /** Get the error message if any.  */
  std::string const& GetErrorMessage() const
    {
    return this->ErrorMessage;
    }

  /** Boolean conversion.  True if the ELF file is valid.  */
  operator bool() const { return this->Valid(); }

  /** Enumeration of ELF file types.  */
  enum FileType
  {
    FileTypeInvalid,
    FileTypeRelocatableObject,
    FileTypeExecutable,
    FileTypeSharedLibrary,
    FileTypeCore
  };

  /** Get the type of the file opened.  */
  FileType GetFileType() const;

  /** Get the number of ELF sections present.  */
  unsigned int GetNumberOfSections() const;

  /** Get the SONAME field if any.  */
  bool GetSOName(std::string& soname);

  /** Print human-readable information about the ELF file.  */
  void PrintInfo(std::ostream& os) const;

private:
  friend class cmELFInternal;
  bool Valid() const;
  cmELFInternal* Internal;
  std::string ErrorMessage;
};

#endif
