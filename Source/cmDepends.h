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
  cmDepends(const char* dir, const char* targetFile);

  /** Virtual destructor to cleanup subclasses properly.  */
  virtual ~cmDepends();

  /** Write dependencies for the target file.  */
  void Write();

  /** Check dependencies for the target file.  */
  void Check();

  /** Clear dependencies for the target file so they will be regenerated.  */
  void Clear();

  /** Get the name of the dependency make file.  */
  const char* GetMakeFileName();

  /** Get the name of the dependency mark file.  */
  const char* GetMarkFileName();

protected:

  // Write dependencies for the target file to the given stream.
  // Return true for success and false for failure.
  virtual bool WriteDependencies(std::ostream& os)=0;

  // Check dependencies for the target file in the given stream.
  // Return false if dependencies must be regenerated and true
  // otherwise.
  virtual bool CheckDependencies(std::istream& is)=0;

  // The directory in which the build rule for the target file is executed.
  std::string m_Directory;

  // The name of the target file for which dependencies are maintained.
  std::string m_TargetFile;

  // The name of the .depends.make file corresponding to the target.
  std::string m_DependsMakeFile;

  // The name of the .depends file marking when dependencies were generated.
  std::string m_DependsMarkFile;

private:
  cmDepends(cmDepends const&); // Purposely not implemented.
  void operator=(cmDepends const&); // Purposely not implemented.
};

#endif
