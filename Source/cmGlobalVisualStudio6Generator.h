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
#ifndef cmGlobalVisualStudio6Generator_h
#define cmGlobalVisualStudio6Generator_h

#include "cmGlobalGenerator.h"

class cmTarget;

/** \class cmGlobalVisualStudio6Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio6Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio6Generator : public cmGlobalGenerator
{
public:
  cmGlobalVisualStudio6Generator();
  static cmGlobalGenerator* New() { return new cmGlobalVisualStudio6Generator; }
  
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio6Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 6";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(const char*, cmMakefile *mf);

  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  virtual int TryCompile(const char *srcdir, const char *bindir,
                         const char *projectName, const char *targetName,
                         std::string *output);

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.  
   */
  virtual void Generate();

  /**
   * Generate the DSW workspace file.
   */
  virtual void OutputDSWFile();

private:
  void GenerateConfigurations(cmMakefile* mf);
  void SetupTests();
  void WriteDSWFile(std::ostream& fout);
  void WriteDSWHeader(std::ostream& fout);
  void WriteProject(std::ostream& fout, 
                    const char* name, const char* path,
                    const cmTarget &t);
  void WriteExternalProject(std::ostream& fout, 
                            const char* name, const char* path,
                            const std::vector<std::string>& dependencies);
  void WriteDSWFooter(std::ostream& fout);
};

#endif
