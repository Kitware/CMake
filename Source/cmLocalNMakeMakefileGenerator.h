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
#ifndef cmNMakeMakefileGenerator_h
#define cmNMakeMakefileGenerator_h

#include "cmLocalUnixMakefileGenerator.h"

/** \class cmLocalNMakeMakefileGenerator
 * \brief Write an NMake makefile.
 *
 * cmLocalNMakeMakefileGenerator produces a Unix makefile from its
 * member m_Makefile.
 */
class cmLocalNMakeMakefileGenerator : public cmLocalUnixMakefileGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalNMakeMakefileGenerator();

  virtual ~cmLocalNMakeMakefileGenerator();
  
protected:
  std::string ShortPath(const char* path);
  std::string ShortPathCommand(const char* command);
  virtual void OutputMakeVariables(std::ostream&);
  virtual void BuildInSubDirectory(std::ostream& fout,
                                   const char* directory,
                                   const char* target1,
                                   const char* target2,
                                   bool silent = false);
  void OutputMakeRule(std::ostream& fout, 
                      const char* comment,
                      const char* target,
                      const char* depends, 
                      const char* command,
                      const char* command2=0,
                      const char* command3=0,
                      const char* command4=0); 
  
  
  virtual void OutputBuildObjectFromSource(std::ostream& fout,
                                           const char* shortName,
                                           const cmSourceFile& source,
                                           const char* extraCompileFlags,
                                           bool sharedTarget); 
  virtual void OutputSharedLibraryRule(std::ostream&, const char* name,
                                       const cmTarget &);
  virtual void OutputModuleLibraryRule(std::ostream&, const char* name, 
                                       const cmTarget &);
  virtual void OutputStaticLibraryRule(std::ostream&, const char* name,
                                       const cmTarget &);
  virtual void OutputExecutableRule(std::ostream&, const char* name,
                                    const cmTarget &);
  virtual void OutputLinkLibraries(std::ostream& fout,
                                   const char* targetLibrary,
                                   const cmTarget &tgt);
  virtual std::string GetOutputExtension(const char* sourceExtension); 
  virtual void OutputIncludeMakefile(std::ostream&, const char* file);
  virtual void OutputBuildTargetInDir(std::ostream& fout,
                                      const char* path,
                                      const char* library,
                                      const char* fullpath,
                                      const char* outputPath); 
  ///! return true if the two paths are the same (checks short paths)
  virtual bool SamePath(const char* path1, const char* path2);
  void SetLibraryPathOption(const char* lib){ m_LibraryPathOption = lib;}
  void SetLibraryLinkOption(const char* lib){ m_LibraryLinkOption = lib;}

  virtual std::string CreateMakeVariable(const char* s, const char* s2);
  virtual std::string LowerCasePath(const char* path);
private:
  std::string m_LibraryPathOption;// option to specifiy a link path -LIBPATH 
  std::string m_LibraryLinkOption; // option to specify a library (like -l, empty for nmake)
};

#endif
