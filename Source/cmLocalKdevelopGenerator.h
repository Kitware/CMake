/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  Copyright (c) 2004 Alexander Neundorf, neundorf@kde.org. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmLocalKdevelopGenerator_h
#define cmLocalKdevelopGenerator_h

#include "cmLocalUnixMakefileGenerator.h"

class cmDependInformation;
class cmMakeDepend;
class cmTarget;
class cmSourceFile;

/** \class cmLocalKdevelopGenerator
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalKdevelopGenerator produces a LocalUnix makefile from its
 * member m_Makefile.
 */
class cmLocalKdevelopGenerator : public cmLocalUnixMakefileGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalKdevelopGenerator();

  virtual ~cmLocalKdevelopGenerator();
  
  /**
   * Generate the makefile for this directory. fromTheTop indicates if this
   * is being invoked as part of a global Generate or specific to this
   * directory. The difference is that when done from the Top we might skip
   * some steps to save time, such as dependency generation for the
   * makefiles. This is done by a direct invocation from make. 
   */
  virtual void Generate(bool fromTheTop);
protected:
  /**
    Create the foo.kdevelop file. This one calls MergeProjectFiles() 
    if it already exists, otherwise createNewProjectFile()
  */
  void CreateProjectFile(const std::string& outputDir, const std::string& projectDir,
                         const std::string& projectname, const std::string& executable, 
                         const std::string& cmakeFilePattern);
  ///! Create the foo.kdevelop.filelist file, return false if it doesn't succeed
  bool CreateFilelistFile(const std::string& outputDir, const std::string& projectDir, 
                          const std::string& projectname, std::string& cmakeFilePattern);
  ///! Reads the old foo.kdevelop line by line and only replaces the "important" lines
  void MergeProjectFiles(const std::string& outputDir, const std::string& projectDir,
                         const std::string& filename, const std::string& executable,
                         const std::string& cmakeFilePattern);
  ///! Creates a new foo.kdevelop file
  void CreateNewProjectFile(const std::string& outputDir, const std::string& projectDir, 
                            const std::string& filename, const std::string& executable, 
                            const std::string& cmakeFilePattern);

};

#endif
