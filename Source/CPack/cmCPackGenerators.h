/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCPackGenerators_h
#define cmCPackGenerators_h

#include "cmObject.h"

class cmCPackGenericGenerator;

/** \class cmCPackGenerators
 * \brief A container for CPack generators
 *
 */
class cmCPackGenerators : public cmObject
{
public:
  cmTypeMacro(cmCPackGenerators, cmObject);

  cmCPackGenerators();
  ~cmCPackGenerators();

  //! Get the generator
  cmCPackGenericGenerator* NewGenerator(const char* name);
  void DeleteGenerator(cmCPackGenericGenerator* gen);

  typedef cmCPackGenericGenerator* CreateGeneratorCall();

  void RegisterGenerator(const char* name, CreateGeneratorCall* createGenerator);

private:
  cmCPackGenericGenerator* NewGeneratorInternal(const char* name);
  std::vector<cmCPackGenericGenerator*> m_Generators;

  typedef std::map<cmStdString, CreateGeneratorCall*> t_GeneratorCreatorsMap;
  t_GeneratorCreatorsMap m_GeneratorCreators;
};

#endif



