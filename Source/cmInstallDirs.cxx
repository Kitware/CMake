/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstallDirs.h"

#include <cmext/string_view>

#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"

namespace {
std::string GetDirectory(cmMakefile const* makefile,
                         std::string const& varName, std::string const& guess)
{
  cmValue value = makefile->GetDefinition(varName);
  if (!value.IsEmpty()) {
    return value;
  }
  return guess;
}
} // namespace

namespace cm {
namespace InstallDirs {
std::string GetRuntimeDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_BINDIR", "bin");
}

std::string GetSbinDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_SBINDIR", "sbin");
}

std::string GetArchiveDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_LIBDIR", "lib");
}

std::string GetLibraryDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_LIBDIR", "lib");
}

std::string GetIncludeDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_INCLUDEDIR", "include");
}

std::string GetSysconfDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_SYSCONFDIR", "etc");
}

std::string GetSharedStateDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_SHAREDSTATEDIR", "com");
}

std::string GetLocalStateDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_LOCALSTATEDIR", "var");
}

std::string GetRunStateDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_RUNSTATEDIR",
                      cmStrCat(GetLocalStateDirectory(makefile), "/run"));
}

std::string GetDataRootDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_DATAROOTDIR", "share");
}

std::string GetDataDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_DATADIR",
                      GetDataRootDirectory(makefile));
}

std::string GetInfoDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_INFODIR",
                      cmStrCat(GetDataRootDirectory(makefile), "/info"));
}

std::string GetLocaleDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_LOCALEDIR",
                      cmStrCat(GetDataRootDirectory(makefile), "/locale"));
}

std::string GetManDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_MANDIR",
                      cmStrCat(GetDataRootDirectory(makefile), "/man"));
}

std::string GetDocDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_DOCDIR",
                      cmStrCat(GetDataRootDirectory(makefile), "/doc"));
}

std::string GetLibExecDirectory(cmMakefile const* makefile)
{
  return GetDirectory(makefile, "CMAKE_INSTALL_LIBEXECDIR", "libexec");
}

std::string GetDirectoryForType(cmMakefile const* makefile,
                                cm::string_view type)
{
  if (type == "BIN"_s) {
    return GetRuntimeDirectory(makefile);
  }
  if (type == "SBIN"_s) {
    return GetSbinDirectory(makefile);
  }
  if (type == "SYSCONF"_s) {
    return GetSysconfDirectory(makefile);
  }
  if (type == "SHAREDSTATE"_s) {
    return GetSharedStateDirectory(makefile);
  }
  if (type == "LOCALSTATE"_s) {
    return GetLocalStateDirectory(makefile);
  }
  if (type == "RUNSTATE"_s) {
    return GetRunStateDirectory(makefile);
  }
  if (type == "LIB"_s) {
    return GetLibraryDirectory(makefile);
  }
  if (type == "INCLUDE"_s) {
    return GetIncludeDirectory(makefile);
  }
  if (type == "DATA"_s) {
    return GetDataDirectory(makefile);
  }
  if (type == "INFO"_s) {
    return GetInfoDirectory(makefile);
  }
  if (type == "LOCALE"_s) {
    return GetLocaleDirectory(makefile);
  }
  if (type == "MAN"_s) {
    return GetManDirectory(makefile);
  }
  if (type == "DOC"_s) {
    return GetDocDirectory(makefile);
  }
  if (type == "LIBEXEC"_s) {
    return GetLibExecDirectory(makefile);
  }
  return std::string{};
}

} // namespace InstallDirs
} // namespace cm
