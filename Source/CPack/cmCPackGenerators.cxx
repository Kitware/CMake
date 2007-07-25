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
#include "cmCPackTarBZip2Generator.h"
#include "cmCPackTarCompressGenerator.h"
#include "cmCPackZIPGenerator.h"
#include "cmCPackSTGZGenerator.h"
#include "cmCPackNSISGenerator.h"
#ifdef __APPLE__
#  include "cmCPackPackageMakerGenerator.h"
#  include "cmCPackOSXX11Generator.h"
#endif

#ifdef __CYGWIN__
#  include "cmCPackCygwinBinaryGenerator.h"
#  include "cmCPackCygwinSourceGenerator.h"
#endif

#if !defined(_WIN32) && !defined(__APPLE__) \
 && !defined(__QNXNTO__) && !defined(__BEOS__)
#  include "cmCPackDebGenerator.h"
#endif


#include "cmCPackLog.h"

//----------------------------------------------------------------------
cmCPackGenerators::cmCPackGenerators()
{
  this->RegisterGenerator("TGZ", "Tar GZip compression",
    cmCPackTGZGenerator::CreateGenerator);
  this->RegisterGenerator("STGZ", "Self extracting Tar GZip compression",
    cmCPackSTGZGenerator::CreateGenerator);
  this->RegisterGenerator("NSIS", "Null Soft Installer",
    cmCPackNSISGenerator::CreateGenerator);
#ifdef __CYGWIN__
  this->RegisterGenerator("CygwinBinary", "Cygwin Binary Installer",
                          cmCPackCygwinBinaryGenerator::CreateGenerator);
  this->RegisterGenerator("CygwinSource", "Cygwin Source Installer",
                          cmCPackCygwinSourceGenerator::CreateGenerator);
#endif

  this->RegisterGenerator("ZIP", "ZIP file format",
    cmCPackZIPGenerator::CreateGenerator);
  this->RegisterGenerator("TBZ2", "Tar BZip2 compression",
    cmCPackTarBZip2Generator::CreateGenerator);
  this->RegisterGenerator("TZ", "Tar Compress compression",
    cmCPackTarCompressGenerator::CreateGenerator);
#ifdef __APPLE__
  this->RegisterGenerator("PackageMaker", "Mac OSX Package Maker installer",
    cmCPackPackageMakerGenerator::CreateGenerator);
  this->RegisterGenerator("OSXX11", "Mac OSX X11 bundle",
    cmCPackOSXX11Generator::CreateGenerator);
#endif
#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__QNXNTO__) && !defined(__BEOS__)
  this->RegisterGenerator("DEB", "Debian packages",
    cmCPackDebGenerator::CreateGenerator);
#endif
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
  const char* generatorDescription,
  CreateGeneratorCall* createGenerator)
{
  if ( !name || !createGenerator )
    {
    cmCPack_Log(this->Logger, cmCPackLog::LOG_ERROR,
      "Cannot register generator" << std::endl);
    return;
    }
  this->GeneratorCreators[name] = createGenerator;
  this->GeneratorDescriptions[name] = generatorDescription;
}
