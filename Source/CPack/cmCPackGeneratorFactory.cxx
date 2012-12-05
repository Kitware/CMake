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

#if !defined(_WIN32) \
 && !defined(__QNXNTO__) && !defined(__BEOS__)
#  include "cmCPackDebGenerator.h"
#  include "cmCPackRPMGenerator.h"
#endif

#ifdef _WIN32
#  include "WiX/cmCPackWIXGenerator.h"
#endif

#include "cmCPackLog.h"

#if defined(__BORLANDC__)
# pragma warn -8008 /* condition is always true */
#endif

//----------------------------------------------------------------------
cmCPackGeneratorFactory::cmCPackGeneratorFactory()
{
  if (cmCPackTGZGenerator::CanGenerate())
    {
    this->RegisterGenerator("TGZ", "Tar GZip compression",
      cmCPackTGZGenerator::CreateGenerator);
    }
  if (cmCPackSTGZGenerator::CanGenerate())
    {
    this->RegisterGenerator("STGZ", "Self extracting Tar GZip compression",
      cmCPackSTGZGenerator::CreateGenerator);
    }
  if (cmCPackNSISGenerator::CanGenerate())
    {
    this->RegisterGenerator("NSIS", "Null Soft Installer",
      cmCPackNSISGenerator::CreateGenerator);
    this->RegisterGenerator("NSIS64", "Null Soft Installer (64-bit)",
      cmCPackNSISGenerator::CreateGenerator64);
    }
#ifdef __CYGWIN__
  if (cmCPackCygwinBinaryGenerator::CanGenerate())
    {
    this->RegisterGenerator("CygwinBinary", "Cygwin Binary Installer",
                            cmCPackCygwinBinaryGenerator::CreateGenerator);
    }
  if (cmCPackCygwinSourceGenerator::CanGenerate())
    {
    this->RegisterGenerator("CygwinSource", "Cygwin Source Installer",
                            cmCPackCygwinSourceGenerator::CreateGenerator);
    }
#endif

  if (cmCPackZIPGenerator::CanGenerate())
    {
    this->RegisterGenerator("ZIP", "ZIP file format",
      cmCPackZIPGenerator::CreateGenerator);
    }
#ifdef _WIN32
  if (cmCPackWIXGenerator::CanGenerate())
    {
    this->RegisterGenerator("WIX", "MSI file format via WiX tools",
      cmCPackWIXGenerator::CreateGenerator);
    }
#endif
  if (cmCPackTarBZip2Generator::CanGenerate())
    {
    this->RegisterGenerator("TBZ2", "Tar BZip2 compression",
      cmCPackTarBZip2Generator::CreateGenerator);
    }
  if (cmCPackTarCompressGenerator::CanGenerate())
    {
    this->RegisterGenerator("TZ", "Tar Compress compression",
      cmCPackTarCompressGenerator::CreateGenerator);
    }
#ifdef __APPLE__
  if (cmCPackDragNDropGenerator::CanGenerate())
    {
    this->RegisterGenerator("DragNDrop", "Mac OSX Drag And Drop",
      cmCPackDragNDropGenerator::CreateGenerator);
    }
  if (cmCPackBundleGenerator::CanGenerate())
    {
    this->RegisterGenerator("Bundle", "Mac OSX bundle",
      cmCPackBundleGenerator::CreateGenerator);
    }
  if (cmCPackPackageMakerGenerator::CanGenerate())
    {
    this->RegisterGenerator("PackageMaker", "Mac OSX Package Maker installer",
      cmCPackPackageMakerGenerator::CreateGenerator);
    }
  if (cmCPackOSXX11Generator::CanGenerate())
    {
    this->RegisterGenerator("OSXX11", "Mac OSX X11 bundle",
      cmCPackOSXX11Generator::CreateGenerator);
    }
#endif
#if !defined(_WIN32) \
  && !defined(__QNXNTO__) && !defined(__BEOS__)
  if (cmCPackDebGenerator::CanGenerate())
    {
    this->RegisterGenerator("DEB", "Debian packages",
      cmCPackDebGenerator::CreateGenerator);
    }
  if (cmCPackRPMGenerator::CanGenerate())
    {
    this->RegisterGenerator("RPM", "RPM packages",
      cmCPackRPMGenerator::CreateGenerator);
    }
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
