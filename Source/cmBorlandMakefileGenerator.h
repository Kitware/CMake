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
#ifndef cmBorlandMakefileGenerator_h
#define cmBorlandMakefileGenerator_h

#include "cmNMakeMakefileGenerator.h"

/** \class cmBorlandMakefileGenerator
 * \brief Write an NMake makefile.
 *
 * cmBorlandMakefileGenerator produces a Unix makefile from its
 * member m_Makefile.
 */
class cmBorlandMakefileGenerator : public cmNMakeMakefileGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmBorlandMakefileGenerator();

  virtual ~cmBorlandMakefileGenerator();
  
  ///! Get the name for the generator.
  virtual const char* GetName() {return "Borland Makefiles";}

  ///! virtual copy constructor
  virtual cmMakefileGenerator* CreateObject() 
    { return new cmBorlandMakefileGenerator;}

  ///! figure out about the current system information
  virtual void EnableLanguage(const char*); 
protected:
  virtual void OutputMakeVariables(std::ostream&);
  
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
  virtual std::string GetOutputExtension(const char* sourceExtension); 
  ///! return true if the two paths are the same (checks short paths)
  virtual bool SamePath(const char* path1, const char* path2);
  virtual std::string CreateMakeVariable(const char* s, const char* s2);
  std::map<cmStdString, cmStdString> m_MakeVariableMap;
  std::map<cmStdString, cmStdString> m_ShortMakeVariableMap;
};

#endif
