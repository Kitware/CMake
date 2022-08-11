/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackGeneratorFactory.h"

#include <ostream>
#include <utility>

#include "IFW/cmCPackIFWGenerator.h"
#ifdef HAVE_FREEBSD_PKG
#  include "cmCPackFreeBSDGenerator.h"
#endif
#include "cmCPackArchiveGenerator.h"
#include "cmCPackDebGenerator.h"
#include "cmCPackExternalGenerator.h"
#include "cmCPackGenerator.h"
#include "cmCPackLog.h"
#include "cmCPackNSISGenerator.h"
#include "cmCPackNuGetGenerator.h"
#include "cmCPackSTGZGenerator.h"

#ifdef __APPLE__
#  include "cmCPackBundleGenerator.h"
#  include "cmCPackDragNDropGenerator.h"
#  include "cmCPackProductBuildGenerator.h"
#endif

#ifdef __CYGWIN__
#  include "cmCPackCygwinBinaryGenerator.h"
#  include "cmCPackCygwinSourceGenerator.h"
#endif

#if !defined(_WIN32) && !defined(__QNXNTO__) && !defined(__BEOS__) &&         \
  !defined(__HAIKU__)
#  include "cmCPackRPMGenerator.h"
#endif

#if defined(_WIN32) || (defined(__CYGWIN__) && defined(HAVE_LIBUUID))
#  include "WiX/cmCPackWIXGenerator.h"
#endif

cmCPackGeneratorFactory::cmCPackGeneratorFactory()
{
  if (cmCPackArchiveGenerator::CanGenerate()) {
    this->RegisterGenerator("7Z", "7-Zip file format",
                            cmCPackArchiveGenerator::Create7ZGenerator);
    this->RegisterGenerator("TBZ2", "Tar BZip2 compression",
                            cmCPackArchiveGenerator::CreateTBZ2Generator);
    this->RegisterGenerator("TGZ", "Tar GZip compression",
                            cmCPackArchiveGenerator::CreateTGZGenerator);
    this->RegisterGenerator("TXZ", "Tar XZ compression",
                            cmCPackArchiveGenerator::CreateTXZGenerator);
    this->RegisterGenerator("TZ", "Tar Compress compression",
                            cmCPackArchiveGenerator::CreateTZGenerator);
    this->RegisterGenerator("TZST", "Tar Zstandard compression",
                            cmCPackArchiveGenerator::CreateTZSTGenerator);
    this->RegisterGenerator("ZIP", "ZIP file format",
                            cmCPackArchiveGenerator::CreateZIPGenerator);
  }
  if (cmCPackSTGZGenerator::CanGenerate()) {
    this->RegisterGenerator("STGZ", "Self extracting Tar GZip compression",
                            cmCPackSTGZGenerator::CreateGenerator);
  }
  if (cmCPackNSISGenerator::CanGenerate()) {
    this->RegisterGenerator("NSIS", "Null Soft Installer",
                            cmCPackNSISGenerator::CreateGenerator);
    this->RegisterGenerator("NSIS64", "Null Soft Installer (64-bit)",
                            cmCPackNSISGenerator::CreateGenerator64);
  }
  if (cmCPackIFWGenerator::CanGenerate()) {
    this->RegisterGenerator("IFW", "Qt Installer Framework",
                            cmCPackIFWGenerator::CreateGenerator);
  }
#ifdef __CYGWIN__
  if (cmCPackCygwinBinaryGenerator::CanGenerate()) {
    this->RegisterGenerator("CygwinBinary", "Cygwin Binary Installer",
                            cmCPackCygwinBinaryGenerator::CreateGenerator);
  }
  if (cmCPackCygwinSourceGenerator::CanGenerate()) {
    this->RegisterGenerator("CygwinSource", "Cygwin Source Installer",
                            cmCPackCygwinSourceGenerator::CreateGenerator);
  }
#endif
#if defined(_WIN32) || (defined(__CYGWIN__) && defined(HAVE_LIBUUID))
  if (cmCPackWIXGenerator::CanGenerate()) {
    this->RegisterGenerator("WIX", "MSI file format via WiX tools",
                            cmCPackWIXGenerator::CreateGenerator);
  }
#endif
  if (cmCPackDebGenerator::CanGenerate()) {
    this->RegisterGenerator("DEB", "Debian packages",
                            cmCPackDebGenerator::CreateGenerator);
  }
  if (cmCPackNuGetGenerator::CanGenerate()) {
    this->RegisterGenerator("NuGet", "NuGet packages",
                            cmCPackNuGetGenerator::CreateGenerator);
  }
  if (cmCPackExternalGenerator::CanGenerate()) {
    this->RegisterGenerator("External", "CPack External packages",
                            cmCPackExternalGenerator::CreateGenerator);
  }
#ifdef __APPLE__
  if (cmCPackDragNDropGenerator::CanGenerate()) {
    this->RegisterGenerator("DragNDrop", "Mac OSX Drag And Drop",
                            cmCPackDragNDropGenerator::CreateGenerator);
  }
  if (cmCPackBundleGenerator::CanGenerate()) {
    this->RegisterGenerator("Bundle", "Mac OSX bundle",
                            cmCPackBundleGenerator::CreateGenerator);
  }
  if (cmCPackProductBuildGenerator::CanGenerate()) {
    this->RegisterGenerator("productbuild", "Mac OSX pkg",
                            cmCPackProductBuildGenerator::CreateGenerator);
  }
#endif
#if !defined(_WIN32) && !defined(__QNXNTO__) && !defined(__BEOS__) &&         \
  !defined(__HAIKU__)
  if (cmCPackRPMGenerator::CanGenerate()) {
    this->RegisterGenerator("RPM", "RPM packages",
                            cmCPackRPMGenerator::CreateGenerator);
  }
#endif
#ifdef HAVE_FREEBSD_PKG
  if (cmCPackFreeBSDGenerator::CanGenerate()) {
    this->RegisterGenerator("FREEBSD", "FreeBSD pkg(8) packages",
                            cmCPackFreeBSDGenerator::CreateGenerator);
  }
#endif
}

std::unique_ptr<cmCPackGenerator> cmCPackGeneratorFactory::NewGenerator(
  const std::string& name)
{
  auto it = this->GeneratorCreators.find(name);
  if (it == this->GeneratorCreators.end()) {
    return nullptr;
  }
  std::unique_ptr<cmCPackGenerator> gen(it->second());
  if (!gen) {
    return nullptr;
  }
  gen->SetLogger(this->Logger);
  return gen;
}

void cmCPackGeneratorFactory::RegisterGenerator(
  const std::string& name, const char* generatorDescription,
  CreateGeneratorCall* createGenerator)
{
  if (!createGenerator) {
    cmCPack_Log(this->Logger, cmCPackLog::LOG_ERROR,
                "Cannot register generator" << std::endl);
    return;
  }
  this->GeneratorCreators[name] = createGenerator;
  this->GeneratorDescriptions[name] = generatorDescription;
}
