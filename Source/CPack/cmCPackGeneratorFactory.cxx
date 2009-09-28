/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackGeneratorFactory.h"

#include "cmCPackGenerator.h"
#include "cmCPackTGZGenerator.h"
#include "cmCPackTarBZip2Generator.h"
#include "cmCPackTarCompressGenerator.h"
#include "cmCPackZIPGenerator.h"
#include "cmCPackSTGZGenerator.h"
#include "cmCPackNSISGenerator.h"
#ifdef __APPLE__
#  include "cmCPackDragNDropGenerator.h"
#  include "cmCPackBundleGenerator.h"
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
#  include "cmCPackRPMGenerator.h"
#endif


#include "cmCPackLog.h"

//----------------------------------------------------------------------
cmCPackGeneratorFactory::cmCPackGeneratorFactory()
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
  this->RegisterGenerator("DragNDrop", "Mac OSX Drag And Drop",
    cmCPackDragNDropGenerator::CreateGenerator);
  this->RegisterGenerator("Bundle", "Mac OSX bundle",
    cmCPackBundleGenerator::CreateGenerator);
  this->RegisterGenerator("PackageMaker", "Mac OSX Package Maker installer",
    cmCPackPackageMakerGenerator::CreateGenerator);
  this->RegisterGenerator("OSXX11", "Mac OSX X11 bundle",
    cmCPackOSXX11Generator::CreateGenerator);
#endif
#if !defined(_WIN32) && !defined(__APPLE__) \
  && !defined(__QNXNTO__) && !defined(__BEOS__)
  this->RegisterGenerator("DEB", "Debian packages",
    cmCPackDebGenerator::CreateGenerator);
  this->RegisterGenerator("RPM", "RPM packages",
    cmCPackRPMGenerator::CreateGenerator);
#endif
}

//----------------------------------------------------------------------
cmCPackGeneratorFactory::~cmCPackGeneratorFactory()
{
  std::vector<cmCPackGenerator*>::iterator it;
  for ( it = this->Generators.begin(); it != this->Generators.end(); ++ it )
    {
    delete *it;
    }
}

//----------------------------------------------------------------------
cmCPackGenerator* cmCPackGeneratorFactory::NewGenerator(const char* name)
{
  cmCPackGenerator* gen = this->NewGeneratorInternal(name);
  if ( !gen )
    {
    return 0;
    }
  this->Generators.push_back(gen);
  gen->SetLogger(this->Logger);
  return gen;
}

//----------------------------------------------------------------------
cmCPackGenerator* cmCPackGeneratorFactory::NewGeneratorInternal(
  const char* name)
{
  if ( !name )
    {
    return 0;
    }
  cmCPackGeneratorFactory::t_GeneratorCreatorsMap::iterator it
    = this->GeneratorCreators.find(name);
  if ( it == this->GeneratorCreators.end() )
    {
    return 0;
    }
  return (it->second)();
}

//----------------------------------------------------------------------
void cmCPackGeneratorFactory::RegisterGenerator(const char* name,
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
