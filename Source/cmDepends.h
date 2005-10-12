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
#ifndef cmDepends_h
#define cmDepends_h

#include "cmStandardIncludes.h"

class cmFileTimeComparison;

/** \class cmDepends
 * \brief Dependency scanner superclass.
 *
 * This class is responsible for maintaining a .depends.make file in
 * the build tree corresponding to an object file.  Subclasses help it
 * maintain dependencies for particular languages.
 */
class cmDepends
{
public:
  /** Instances need to know the build directory name and the relative
      path from the build directory to the target file.  */
  cmDepends();
  
  /** at what level will the compile be done from */
  void SetCompileDirectory(const char *dir) {m_CompileDirectory = dir;};
    
  /** should this be verbose in its output */
  void SetVerbose(bool verb) { m_Verbose = verb; }
    
  /** Virtual destructor to cleanup subclasses properly.  */
  virtual ~cmDepends();

  /** Write dependencies for the target file.  */
  bool Write(const char *src, const char *obj,
    std::ostream &makeDepends, std::ostream &internalDepends);
  
  /** Check dependencies for the target file.  */
  void Check(const char *makeFile, const char* internalFile);

  /** Clear dependencies for the target file so they will be regenerated.  */
  void Clear(const char *file);

  /** Set the file comparison object */
  void SetFileComparison(cmFileTimeComparison* fc) { m_FileComparison = fc; }

protected:

  // Write dependencies for the target file to the given stream.
  // Return true for success and false for failure.
  virtual bool WriteDependencies(const char *src, const char* obj,
    std::ostream& makeDepends, std::ostream& internalDepends)=0;

  // Check dependencies for the target file in the given stream.
  // Return false if dependencies must be regenerated and true
  // otherwise.
  virtual bool CheckDependencies(std::istream& internalDepends);

  // The directory in which the build rule for the target file is executed.
  std::string m_Directory;
  std::string m_CompileDirectory;

  // Flag for verbose output.
  bool m_Verbose;
  cmFileTimeComparison* m_FileComparison;

  size_t m_MaxPath;
  char* m_Dependee;
  char* m_Depender;

private:
  cmDepends(cmDepends const&); // Purposely not implemented.
  void operator=(cmDepends const&); // Purposely not implemented.
};

#endif
