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

#include "cmUnixMakefileGenerator.h"

/** \class cmNMakeMakefileGenerator
 * \brief Write an NMake makefile.
 *
 * cmNMakeMakefileGenerator produces a Unix makefile from its
 * member m_Makefile.
 */
class cmNMakeMakefileGenerator : public cmUnixMakefileGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmNMakeMakefileGenerator();

  virtual ~cmNMakeMakefileGenerator();
  
  ///! Get the name for the generator.
  virtual const char* GetName() {return "NMake Makefiles";}

  ///! virtual copy constructor
  virtual cmMakefileGenerator* CreateObject() 
    { return new cmNMakeMakefileGenerator;}

  ///! figure out about the current system information
  virtual void ComputeSystemInfo(); 
protected:
  std::string ShortPath(const char* path);
  std::string ShortPathCommand(const char* command);
  virtual void OutputMakeVariables(std::ostream&);
  virtual void BuildInSubDirectory(std::ostream& fout,
                                   const char* directory,
                                   const char* target1,
                                   const char* target2);
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
  virtual void OutputBuildLibraryInDir(std::ostream& fout,
				       const char* path,
				       const char* library,
				       const char* fullpath); 
  ///! return true if the two paths are the same (checks short paths)
  virtual bool SamePath(const char* path1, const char* path2);
  void SetLibraryPathOption(const char* lib){ m_LibraryPathOption = lib;}
  void SetLibraryLinkOption(const char* lib){ m_LibraryLinkOption = lib;}
  virtual std::string ConvertToOutputPath(const char* s);
private:
  std::string m_LibraryPathOption;// option to specifiy a link path -LIBPATH 
  std::string m_LibraryLinkOption; // option to specify a library (like -l, empty for nmake)
};

#endif
