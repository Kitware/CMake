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
#ifndef cmInstallScriptGenerator_h
#define cmInstallScriptGenerator_h

#include "cmInstallGenerator.h"

/** \class cmInstallScriptGenerator
 * \brief Generate target installation rules.
 */
class cmInstallScriptGenerator: public cmInstallGenerator
{
public:
  cmInstallScriptGenerator(const char* script);
  virtual ~cmInstallScriptGenerator();

protected:
  virtual void GenerateScript(std::ostream& os);
  std::string Script;
};

#endif
