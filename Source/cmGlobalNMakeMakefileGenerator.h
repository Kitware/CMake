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
#ifndef cmGlobalNMakeMakefileGenerator_h
#define cmGlobalNMakeMakefileGenerator_h

#include "cmGlobalUNIXMakefileGenerator.h"

/** \class cmGlobalNMakeMakefileGenerator
 * \brief Write a NMake makefiles.
 *
 * cmGlobalNMakeMakefileGenerator manages nmake build process for a tree
 */
class cmGlobalNMakeMakefileGenerator : public cmGlobalUnixMakefileGenerator
{
public:
  ///! Get the name for the generator.
  virtual const char* GetName() {
    return cmGlobalNMakeMakefileGenerator::GetActualName();}
  static const char* GetActualName() {return "NMake Makefiles";}

  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(const char*,cmMakefile *mf);

  /**
   * Try to determine system infomation from another generator
   */
  virtual void EnableLanguagesFromGenerator(cmGlobalGenerator *gen, 
                                            cmMakefile *mf) 
    {
      this->cmGlobalGenerator::EnableLanguagesFromGenerator(gen,mf);
    }
      
};

#endif
