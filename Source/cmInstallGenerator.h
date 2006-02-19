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
#ifndef cmInstallGenerator_h
#define cmInstallGenerator_h

#include "cmStandardIncludes.h"

class cmLocalGenerator;

/** \class cmInstallGenerator
 * \brief Support class for generating install scripts.
 *
 */
class cmInstallGenerator
{
public:
  cmInstallGenerator();
  virtual ~cmInstallGenerator();

  void Generate(std::ostream& os, const char* config,
                std::vector<std::string> const& configurationTypes);

  static void AddInstallRule(std::ostream& os, const char* dest, int type,
                             const char* file, bool optional = false,
                             const char* properties = 0);

protected:
  virtual void GenerateScript(std::ostream& os)=0;

  const char* ConfigurationName;
  std::vector<std::string> const* ConfigurationTypes;
};

#endif
