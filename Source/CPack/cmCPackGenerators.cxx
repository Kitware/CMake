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

#include "cmCPackGenerators.h"

#include "cmCPackGenericGenerator.h"
#include "cmCPackTGZGenerator.h"
#include "cmCPackTarCompressGenerator.h"
#include "cmCPackZIPGenerator.h"
#include "cmCPackSTGZGenerator.h"
#include "cmCPackNSISGenerator.h"
#include "cmCPackPackageMakerGenerator.h"

#include "cmCPackLog.h"

//----------------------------------------------------------------------
cmCPackGenerators::cmCPackGenerators()
{
  this->RegisterGenerator("TGZ", cmCPackTGZGenerator::CreateGenerator);
  this->RegisterGenerator("STGZ", cmCPackSTGZGenerator::CreateGenerator);
  this->RegisterGenerator("NSIS", cmCPackNSISGenerator::CreateGenerator);
  this->RegisterGenerator("ZIP", cmCPackZIPGenerator::CreateGenerator);
  this->RegisterGenerator("TZ", cmCPackTarCompressGenerator::CreateGenerator);
  this->RegisterGenerator("PackageMaker",
    cmCPackPackageMakerGenerator::CreateGenerator);
}

//----------------------------------------------------------------------
cmCPackGenerators::~cmCPackGenerators()
{
  std::vector<cmCPackGenericGenerator*>::iterator it;
  for ( it = this->Generators.begin(); it != this->Generators.end(); ++ it )
    {
    delete *it;
    }
}

//----------------------------------------------------------------------
cmCPackGenericGenerator* cmCPackGenerators::NewGenerator(const char* name)
{
  cmCPackGenericGenerator* gen = this->NewGeneratorInternal(name);
  if ( !gen )
    {
    return 0;
    }
  this->Generators.push_back(gen);
  gen->SetLogger(this->Logger);
  return gen;
}

//----------------------------------------------------------------------
cmCPackGenericGenerator* cmCPackGenerators::NewGeneratorInternal(
  const char* name)
{
  if ( !name )
    {
    return 0;
    }
  cmCPackGenerators::t_GeneratorCreatorsMap::iterator it
    = this->GeneratorCreators.find(name);
  if ( it == this->GeneratorCreators.end() )
    {
    return 0;
    }
  return (it->second)();
}

//----------------------------------------------------------------------
void cmCPackGenerators::RegisterGenerator(const char* name,
  CreateGeneratorCall* createGenerator)
{
  if ( !name || !createGenerator )
    {
    cmCPack_Log(this->Logger, cmCPackLog::LOG_ERROR,
      "Cannot register generator" << std::endl);
    return;
    }
  this->GeneratorCreators[name] = createGenerator;
}
