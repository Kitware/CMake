/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmLocalUnixMakefileGenerator_h
#define cmLocalUnixMakefileGenerator_h

#include "cmLocalGenerator.h"

class cmMakeDepend;
class cmTarget;
class cmSourceFile;

/** \class cmLocalUnixMakefileGenerator
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalUnixMakefileGenerator produces a LocalUnix makefile from its
 * member m_Makefile.
 */
class cmLocalUnixMakefileGenerator : public cmLocalGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalUnixMakefileGenerator();

  virtual ~cmLocalUnixMakefileGenerator();
  
  /**
   * Generate the makefile for this directory. fromTheTop indicates if this
   * is being invoked as part of a global Generate or specific to this
   * directory. The difference is that when done from the Top we might skip
   * some steps to save time, such as dependency generation for the
   * makefiles. This is done by a direct invocation from make. 
   */
  virtual void Generate(bool fromTheTop);

  /**
   * Output the depend information for all the classes 
   * in the makefile.  These would have been generated
   * by the class cmMakeDepend.
   */
  virtual bool OutputObjectDepends(std::ostream&);

  /**
   * Output the check depend information for all the classes 
   * in the makefile.  These would have been generated
   * by the class cmMakeDepend.
   */
  virtual void OutputCheckDepends(std::ostream&);

protected:
  virtual void ProcessDepends(const cmMakeDepend &md);
  virtual void OutputMakefile(const char* file, bool withDepends);
  virtual void OutputTargetRules(std::ostream& fout);
  virtual void OutputLinkLibraries(std::ostream&, const char* name, const cmTarget &);

  virtual void OutputSharedLibraryRule(std::ostream&, const char* name,
                                       const cmTarget &);
  virtual void OutputModuleLibraryRule(std::ostream&, const char* name, 
                                       const cmTarget &);
  virtual void OutputStaticLibraryRule(std::ostream&, const char* name,
                                       const cmTarget &);
  virtual void OutputExecutableRule(std::ostream&, const char* name,
                                    const cmTarget &);
  virtual void OutputUtilityRule(std::ostream&, const char* name,
                                 const cmTarget &);
  
  virtual void OutputTargets(std::ostream&);
  virtual void OutputSubDirectoryRules(std::ostream&);
  virtual void OutputDependLibs(std::ostream&);
  virtual void OutputLibDepend(std::ostream&, const char*);
  virtual void OutputExeDepend(std::ostream&, const char*);
  virtual void OutputCustomRules(std::ostream&);
  virtual void OutputMakeVariables(std::ostream&);
  virtual void OutputMakeRules(std::ostream&);
  virtual void OutputInstallRules(std::ostream&);
  virtual void OutputSourceObjectBuildRules(std::ostream& fout);
  virtual void OutputBuildObjectFromSource(std::ostream& fout,
                                           const char* shortName,
                                           const cmSourceFile& source,
                                           const char* extraCompileFlags,
                                           bool sharedTarget);
  
  virtual void BuildInSubDirectory(std::ostream& fout,
                                   const char* directory,
                                   const char* target1,
                                   const char* target2,
                                   bool silent = false);

  virtual void OutputSubDirectoryVars(std::ostream& fout,
                                      const char* var,
                                      const char* target,
                                      const char* target1,
                                      const char* target2,
                                      const char* depend,
                                      const std::vector<std::string>&
                                      SubDirectories,
                                      bool silent = false);

  virtual void OutputMakeRule(std::ostream&, 
                              const char* comment,
                              const char* target,
                              const char* depends, 
                              const char* command,
                              const char* command2 = 0,
                              const char* command3 = 0,
                              const char* command4 = 0);
  virtual void OutputBuildTargetInDir(std::ostream& fout,
                                      const char* path,
                                      const char* library,
                                      const char* fullpath,
                                      const char* outputPath);
  ///! return true if the two paths are the same
  virtual bool SamePath(const char* path1, const char* path2);
  virtual std::string GetOutputExtension(const char* sourceExtension);
  virtual void OutputIncludeMakefile(std::ostream&, const char* file);
  void SetObjectFileExtension(const char* e) { m_ObjectFileExtension = e;}
  void SetExecutableExtension(const char* e) { m_ExecutableExtension = e;}
  void SetStaticLibraryExtension(const char* e) {m_StaticLibraryExtension = e;}
  void SetSharedLibraryExtension(const char* e) {m_SharedLibraryExtension = e;}
  void SetLibraryPrefix(const char* e) { m_LibraryPrefix = e;}
  std::string CreateTargetRules(const cmTarget &target,
                                const char* targetName);
  virtual std::string CreateMakeVariable(const char* s, const char* s2)
    {
      return std::string(s) + std::string(s2);
    }
  
  ///! if the OS is case insensitive then return a lower case of the path.
  virtual std::string LowerCasePath(const char* path)
    {
      return std::string(path);
    }
  
protected:
  std::string m_ExecutableOutputPath;
  std::string m_LibraryOutputPath;
  std::string m_SharedLibraryExtension;
  std::string m_ObjectFileExtension;
  std::string m_ExecutableExtension;
  std::string m_StaticLibraryExtension;
  std::string m_LibraryPrefix;
private:
};

#endif
