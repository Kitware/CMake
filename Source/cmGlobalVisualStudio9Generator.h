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
#ifndef cmGlobalVisualStudio9Generator_h
#define cmGlobalVisualStudio9Generator_h

#include "cmGlobalVisualStudio8Generator.h"


/** \class cmGlobalVisualStudio9Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio9Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio9Generator : 
  public cmGlobalVisualStudio8Generator
{
public:
  cmGlobalVisualStudio9Generator();
  static cmGlobalGenerator* New() { 
    return new cmGlobalVisualStudio9Generator; }
  
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio9Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 9 2008";}
  virtual void AddPlatformDefinitions(cmMakefile* mf);
  
  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;

  ///! create the correct local generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages, 
                              cmMakefile *, bool optional);
  virtual void WriteSLNHeader(std::ostream& fout);

  /**
   * Where does this version of Visual Studio look for macros for the
   * current user? Returns the empty string if this version of Visual
   * Studio does not implement support for VB macros.
   */
  virtual std::string GetUserMacrosDirectory();
};
#endif
