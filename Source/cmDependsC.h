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
#ifndef cmDependsC_h
#define cmDependsC_h

#include "cmDepends.h"
#include <cmsys/RegularExpression.hxx>
#include <queue>

/** \class cmDependsC
 * \brief Dependency scanner for C and C++ object files.
 */
class cmDependsC: public cmDepends
{
public:
  /** Checking instances need to know the build directory name and the
      relative path from the build directory to the target file.  */
  cmDependsC(const char* dir, const char* targetFile);

  /** Scanning need to know the build directory name, the relative
      path from the build directory to the target file, the source
      file from which to start scanning, and the include file search
      path.  */
  cmDependsC(const char* dir, const char* targetFile,
             const char* sourceFile, std::vector<std::string> const& includes);

  /** Virtual destructor to cleanup subclasses properly.  */
  virtual ~cmDependsC();

protected:
  // Implement writing/checking methods required by superclass.
  virtual bool WriteDependencies(std::ostream& os);
  virtual bool CheckDependencies(std::istream& is);

  // Method to scan a single file.
  void Scan(std::istream& is);

  // The source file from which to start scanning.
  std::string m_SourceFile;

  // The include file search path.
  std::vector<std::string> const* m_IncludePath;

  // Regular expression to identify C preprocessor include directives.
  cmsys::RegularExpression m_IncludeLineRegex;

  // Data structures for dependency graph walk.
  std::set<cmStdString> m_Encountered;
  std::queue<cmStdString> m_Unscanned;

private:
  cmDependsC(cmDependsC const&); // Purposely not implemented.
  void operator=(cmDependsC const&); // Purposely not implemented.
};

#endif
