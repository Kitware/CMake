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
#ifndef cmExtraCodeBlocksGenerator_h
#define cmExtraCodeBlocksGenerator_h

#include "cmExternalMakefileProjectGenerator.h"

class cmLocalGenerator;
class cmMakefile;

/** \class cmExtraCodeBlocksGenerator
 * \brief Write CodeBlocks project files for Makefile based projects
 *
 * This generator is in early alpha stage.
 */
class cmExtraCodeBlocksGenerator : public cmExternalMakefileProjectGenerator
{
public:
  cmExtraCodeBlocksGenerator();
  virtual void SetGlobalGenerator(cmGlobalGenerator* generator);

  virtual const char* GetName() const
                         { return cmExtraCodeBlocksGenerator::GetActualName();}
  static const char* GetActualName()                    { return "CodeBlocks";}
  static cmExternalMakefileProjectGenerator* New() 
                                     { return new cmExtraCodeBlocksGenerator; }
  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry, 
                                const char* fullName) const;

  virtual void Generate();
private:

  /** Create the foo.kdevelop file. This one calls MergeProjectFiles()
    if it already exists, otherwise createNewProjectFile() The project
    files will be created in \a outputDir (in the build tree), the
    kdevelop project dir will be set to \a projectDir (in the source
    tree). \a cmakeFilePattern consists of a lists of all cmake
    listfiles used by this CMakeLists.txt */
  void CreateProjectFile(const std::vector<cmLocalGenerator*>& lgs);

  ///! Creates a new foo.kdevelop and a new foo.kdevses file
  void CreateNewProjectFile(const std::vector<cmLocalGenerator*>& lgs,
                                const std::string& filename);

};

#endif
