/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  Copyright (c) 2004 Alexander Neundorf, neundorf@kde.org. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmGlobalKdevelopGenerator_h
#define cmGlobalKdevelopGenerator_h

#include "cmGlobalUnixMakefileGenerator.h"

/** \class cmGlobalUnixMakefileGenerator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalUnixMakefileGenerator manages UNIX build process for a tree
 */
class cmGlobalKdevelopGenerator : public cmGlobalUnixMakefileGenerator
{
public:
  cmGlobalKdevelopGenerator();
  static cmGlobalGenerator* New() { return new cmGlobalKdevelopGenerator; }

  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalKdevelopGenerator::GetActualName();}
  static const char* GetActualName() {return "KDevelop3";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

};

#endif
