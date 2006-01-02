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
  this->RegisterGenerator("PackageMaker", cmCPackPackageMakerGenerator::CreateGenerator);
}

//----------------------------------------------------------------------
cmCPackGenerators::~cmCPackGenerators()
{
  std::vector<cmCPackGenericGenerator*>::iterator it;
  for ( it = m_Generators.begin(); it != m_Generators.end(); ++ it )
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
  if ( !gen->Initialize(name) )
    {
    delete gen;
    return 0;
    }
  m_Generators.push_back(gen);
  gen->SetLogger(m_Logger);
  return gen;
}

//----------------------------------------------------------------------
cmCPackGenericGenerator* cmCPackGenerators::NewGeneratorInternal(const char* name)
{
  if ( !name )
    {
    return 0;
    }
  cmCPackGenerators::t_GeneratorCreatorsMap::iterator it = m_GeneratorCreators.find(name);
  if ( it == m_GeneratorCreators.end() )
    {
    return 0;
    }
  return (it->second)();
}

//----------------------------------------------------------------------
void cmCPackGenerators::RegisterGenerator(const char* name, CreateGeneratorCall* createGenerator)
{
  if ( !name || !createGenerator )
    {
    cmCPack_Log(m_Logger, cmCPackLog::LOG_ERROR, "Cannot register generator" << std::endl);
    return;
    }
  m_GeneratorCreators[name] = createGenerator;
}
