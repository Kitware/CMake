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
#ifndef cmGlobalUnixMakefileGenerator_h
#define cmGlobalUnixMakefileGenerator_h

#include "cmGlobalGenerator.h"

/** \class cmGlobalUnixMakefileGenerator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalUnixMakefileGenerator manages UNIX build process for a tree
 */
class cmGlobalUnixMakefileGenerator : public cmGlobalGenerator
{
public:
  cmGlobalUnixMakefileGenerator();
  static cmGlobalGenerator* New() { return new cmGlobalUnixMakefileGenerator; }

  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalUnixMakefileGenerator::GetActualName();}
  static const char* GetActualName() {return "Unix Makefiles";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(const char*, cmMakefile *mf);

};

#endif
