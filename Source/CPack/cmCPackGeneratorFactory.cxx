/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCPackGeneratorFactory.h"

#include <ostream>
#include <utility>

#include "IFW/cmCPackIFWGenerator.h"
#if ENABLE_BUILD_FREEBSD_PKG
#  include "cmCPackFreeBSDGenerator.h"
#endif
#include "cmCPackArchiveGenerator.h"
#include "cmCPackDebGenerator.h"
#include "cmCPackExternalGenerator.h"
#include "cmCPackGenerator.h"
#include "cmCPackInnoSetupGenerator.h"
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

#if ENABLE_BUILD_WIX_GENERATOR
#  include "WiX/cmCPackWIXGenerator.h"
#endif

#ifdef __linux__
#  include "cmCPackAppImageGenerator.h"
#endif

cmCPackGeneratorFactory::cmCPackGeneratorFactory()
{
  if (cmCPackArchiveGenerator::CanGenerate()) {
    this->RegisterGenerator(
      "7Z", "7-Zip file format with default compression algorithm",
      cmCPackArchiveGenerator::Create7ZGenerator);
    this->RegisterGenerator("7Z_STORE",
                            "7-Zip file format without compression",
                            cmCPackArchiveGenerator::Create7ZStoreGenerator);
    this->RegisterGenerator("7Z_DEFLATE",
                            "7-Zip file format with Deflate compression",
                            cmCPackArchiveGenerator::Create7ZDeflateGenerator);
    this->RegisterGenerator("7Z_BZ2",
                            "7-Zip file format with BZip2 compression",
                            cmCPackArchiveGenerator::Create7ZBzip2Generator);
    this->RegisterGenerator("7Z_LZMA",
                            "7-Zip file format with LZMA compression",
                            cmCPackArchiveGenerator::Create7ZLzmaGenerator);
    this->RegisterGenerator("7Z_LZMA2",
                            "7-Zip file format with LZMA2 compression",
                            cmCPackArchiveGenerator::Create7ZLzma2Generator);
    this->RegisterGenerator("7Z_ZSTD",
                            "7-Zip file format with Zstandard compression",
                            cmCPackArchiveGenerator::Create7ZZstdGenerator);
    this->RegisterGenerator("7Z_PPMD",
                            "7-Zip file format with PPMd compression",
                            cmCPackArchiveGenerator::Create7ZPPMdGenerator);
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
    this->RegisterGenerator("TAR", "Tar no compression",
                            cmCPackArchiveGenerator::CreateTarGenerator);
    this->RegisterGenerator(
      "ZIP", "ZIP file format with default compression algorithm",
      cmCPackArchiveGenerator::CreateZIPGenerator);
    this->RegisterGenerator("ZIP_STORE", "ZIP file format without compression",
                            cmCPackArchiveGenerator::CreateZipStoreGenerator);
    this->RegisterGenerator(
      "ZIP_DEFLATE", "ZIP file format with Deflate compression",
      cmCPackArchiveGenerator::CreateZipDeflateGenerator);
    this->RegisterGenerator("ZIP_BZ2",
                            "ZIP file format with BZip2 compression",
                            cmCPackArchiveGenerator::CreateZipBzip2Generator);
    this->RegisterGenerator("ZIP_LZMA",
                            "ZIP file format with LZMA compression",
                            cmCPackArchiveGenerator::CreateZipLzmaGenerator);
    this->RegisterGenerator("ZIP_LZMA2",
                            "ZIP file format with LZMA2 compression",
                            cmCPackArchiveGenerator::CreateZipLzma2Generator);
    this->RegisterGenerator("ZIP_ZSTD",
                            "ZIP file format with Zstandard compression",
                            cmCPackArchiveGenerator::CreateZipZstdGenerator);
  }
  if (cmCPackSTGZGenerator::CanGenerate()) {
    this->RegisterGenerator("STGZ", "Self extracting Tar GZip compression",
                            cmCPackSTGZGenerator::CreateGenerator);
  }
  if (cmCPackInnoSetupGenerator::CanGenerate()) {
    this->RegisterGenerator("INNOSETUP", "Inno Setup packages",
                            cmCPackInnoSetupGenerator::CreateGenerator);
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
#if ENABLE_BUILD_WIX_GENERATOR
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
#if ENABLE_BUILD_FREEBSD_PKG
  if (cmCPackFreeBSDGenerator::CanGenerate()) {
    this->RegisterGenerator("FREEBSD", "FreeBSD pkg(8) packages",
                            cmCPackFreeBSDGenerator::CreateGenerator);
  }
#endif
#ifdef __linux__
  if (cmCPackAppImageGenerator::CanGenerate()) {
    this->RegisterGenerator("AppImage", "AppImage packages",
                            cmCPackAppImageGenerator::CreateGenerator);
  }
#endif
}

std::unique_ptr<cmCPackGenerator> cmCPackGeneratorFactory::NewGenerator(
  std::string const& name)
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
  std::string const& name, char const* generatorDescription,
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
