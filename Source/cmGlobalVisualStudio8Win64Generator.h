/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio8Win64Generator_h
#define cmGlobalVisualStudio8Win64Generator_h

#include "cmGlobalVisualStudio8Generator.h"


/** \class cmGlobalVisualStudio8Win64Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio8Win64Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio8Win64Generator : 
  public cmGlobalVisualStudio8Generator
{
public:
  cmGlobalVisualStudio8Win64Generator();
  static cmGlobalGenerator* New() { 
    return new cmGlobalVisualStudio8Win64Generator; }
  
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio8Win64Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 8 2005 Win64";}

  virtual const char* GetPlatformName() const {return "x64";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;

  ///! create the correct local generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void AddPlatformDefinitions(cmMakefile *);
};
#endif
