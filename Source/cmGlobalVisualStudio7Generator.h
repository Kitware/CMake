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
#ifndef cmGlobalVisualStudio7Generator_h
#define cmGlobalVisualStudio7Generator_h

#include "cmGlobalGenerator.h"

class cmTarget;

/** \class cmGlobalVisualStudio7Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio7Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio7Generator : public cmGlobalGenerator
{
public:
  cmGlobalVisualStudio7Generator();
  static cmGlobalGenerator* New() { return new cmGlobalVisualStudio7Generator; }
  
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio7Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 7";}

  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
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
   * Generate the required files for building this directory. This
   * basically creates a single LocalGenerators and
   * requests that it Generate.  
   */
  virtual void LocalGenerate();

  /**
   * Generate the DSW workspace file.
   */
  virtual void OutputSLNFile();

  /**
   * Get the list of configurations
   */
  std::vector<std::string> *GetConfigurations();
      
protected:
  virtual void WriteSLNFile(std::ostream& fout);
  virtual void WriteProject(std::ostream& fout, 
                            const char* name, const char* path,
                            const cmTarget &t);
  virtual void WriteProjectDepends(std::ostream& fout, 
                           const char* name, const char* path,
                           const cmTarget &t);
  virtual void WriteProjectConfigurations(std::ostream& fout, const char* name, bool in_all);
  virtual void WriteSLNFooter(std::ostream& fout);
  virtual void WriteSLNHeader(std::ostream& fout);

  void SetupTests();
  void GenerateConfigurations(cmMakefile* mf);

  void WriteExternalProject(std::ostream& fout, 
                            const char* name, const char* path,
                            const std::vector<std::string>& dependencies);
  std::string CreateGUID(const char* name);

  std::vector<std::string> m_Configurations;
  std::map<cmStdString, cmStdString> m_GUIDMap;
};

#endif
