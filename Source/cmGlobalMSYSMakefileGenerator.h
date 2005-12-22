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
#ifndef cmGlobalMSYSMakefileGenerator_h
#define cmGlobalMSYSMakefileGenerator_h

#include "cmGlobalUNIXMakefileGenerator3.h"

/** \class cmGlobalMSYSMakefileGenerator
 * \brief Write a NMake makefiles.
 *
 * cmGlobalMSYSMakefileGenerator manages nmake build process for a tree
 */
class cmGlobalMSYSMakefileGenerator : public cmGlobalUnixMakefileGenerator3
{
public:
  cmGlobalMSYSMakefileGenerator();
  static cmGlobalGenerator* New() { return new cmGlobalMSYSMakefileGenerator; }
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalMSYSMakefileGenerator::GetActualName();}
  static const char* GetActualName() {return "MSYS Makefiles";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages, cmMakefile *);
};

#endif
