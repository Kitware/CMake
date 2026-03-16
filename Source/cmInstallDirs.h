/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include <string>

#include <cm/string_view>

class cmMakefile;

namespace cm {

namespace InstallDirs {
std::string GetRuntimeDirectory(cmMakefile const*);
std::string GetSbinDirectory(cmMakefile const*);
std::string GetArchiveDirectory(cmMakefile const*);
std::string GetLibraryDirectory(cmMakefile const*);
std::string GetIncludeDirectory(cmMakefile const*);
std::string GetSysconfDirectory(cmMakefile const*);
std::string GetSharedStateDirectory(cmMakefile const*);
std::string GetLocalStateDirectory(cmMakefile const*);
std::string GetRunStateDirectory(cmMakefile const*);
std::string GetDataRootDirectory(cmMakefile const*);
std::string GetDataDirectory(cmMakefile const*);
std::string GetInfoDirectory(cmMakefile const*);
std::string GetLocaleDirectory(cmMakefile const*);
std::string GetManDirectory(cmMakefile const*);
std::string GetDocDirectory(cmMakefile const*);
std::string GetLibExecDirectory(cmMakefile const*);

std::string GetDirectoryForType(cmMakefile const*, cm::string_view type);
}
}
