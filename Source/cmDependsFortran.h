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
#ifndef cmDependsFortran_h
#define cmDependsFortran_h

#include "cmDepends.h"

/** \class cmDependsFortran
 * \brief Dependency scanner for Fortran object files.
 */
class cmDependsFortran: public cmDepends
{
public:
  /** Checking instances need to know the build directory name and the
      relative path from the build directory to the target file.  */
  cmDependsFortran(const char* dir, const char* targetFile);

  /** Scanning need to know the build directory name, the relative
      path from the build directory to the target file, the source
      file from which to start scanning, and the include file search
      path.  */
  cmDependsFortran(const char* dir, const char* targetFile,
             const char* sourceFile, std::vector<std::string> const& includes);

  /** Virtual destructor to cleanup subclasses properly.  */
  virtual ~cmDependsFortran();

  /** Method to find an included file in the include path.  Fortran
      always searches the directory containing the including source
      first.  */
  bool FindIncludeFile(const char* dir, const char* includeName,
                       std::string& fileName);

protected:
  // Implement writing/checking methods required by superclass.
  virtual bool WriteDependencies(std::ostream& os);
  virtual bool CheckDependencies(std::istream& is);

  // The source file from which to start scanning.
  std::string m_SourceFile;

  // The include file search path.
  std::vector<std::string> const* m_IncludePath;

private:
  cmDependsFortran(cmDependsFortran const&); // Purposely not implemented.
  void operator=(cmDependsFortran const&); // Purposely not implemented.
};

#endif
