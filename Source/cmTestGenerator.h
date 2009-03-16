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
#ifndef cmTestGenerator_h
#define cmTestGenerator_h

#include "cmScriptGenerator.h"

class cmTest;

/** \class cmTestGenerator
 * \brief Support class for generating install scripts.
 *
 */
class cmTestGenerator: public cmScriptGenerator
{
public:
  cmTestGenerator(cmTest* test,
                  std::vector<std::string> const&
                  configurations = std::vector<std::string>());
  virtual ~cmTestGenerator();

protected:
  virtual void GenerateScriptConfigs(std::ostream& os, Indent const& indent);
  virtual void GenerateScriptActions(std::ostream& os, Indent const& indent);

  cmTest* Test;
  bool TestGenerated;
};

#endif
