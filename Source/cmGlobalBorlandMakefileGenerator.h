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
#ifndef cmGlobalBorlandMakefileGenerator_h
#define cmGlobalBorlandMakefileGenerator_h

#include "cmGlobalNMakeMakefileGenerator.h"

/** \class cmGlobalBorlandMakefileGenerator
 * \brief Write a Borland makefiles.
 *
 * cmGlobalBorlandMakefileGenerator manages nmake build process for a tree
 */
class cmGlobalBorlandMakefileGenerator : public cmGlobalNMakeMakefileGenerator
{
public:
  cmGlobalBorlandMakefileGenerator();
  ///! Get the name for the generator.
  virtual const char* GetName() {
    return cmGlobalBorlandMakefileGenerator::GetActualName();}
  static const char* GetActualName() {return "Borland Makefiles";}

  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(const char*,cmMakefile *mf);
};

#endif
