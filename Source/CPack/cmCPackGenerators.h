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

class cmCPackLog;
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

  void RegisterGenerator(const char* name,
    const char* generatorDescription,
    CreateGeneratorCall* createGenerator);

  void SetLogger(cmCPackLog* logger) { this->Logger = logger; }

  typedef std::map<cmStdString, cmStdString> DescriptionsMap;
  const DescriptionsMap& GetGeneratorsList() const
    { return this->GeneratorDescriptions; }

private:
  cmCPackGenericGenerator* NewGeneratorInternal(const char* name);
  std::vector<cmCPackGenericGenerator*> Generators;

  typedef std::map<cmStdString, CreateGeneratorCall*> t_GeneratorCreatorsMap;
  t_GeneratorCreatorsMap GeneratorCreators;
  DescriptionsMap GeneratorDescriptions;
  cmCPackLog* Logger;
};

#endif
