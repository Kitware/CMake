/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
// .NAME cmDynamicLoader - class interface to system dynamic libraries
// .SECTION Description
// cmDynamicLoader provides a portable interface to loading dynamic
// libraries into a process.

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmsys/DynamicLoader.hxx" // IWYU pragma: export

class cmDynamicLoader
{
public:
  // Description:
  // Load a dynamic library into the current process.
  // The returned cmsys::DynamicLoader::LibraryHandle can be used to access
  // the symbols in the library.
  static cmsys::DynamicLoader::LibraryHandle OpenLibrary(const char*);

  // Description:
  // Flush the cache of dynamic loader.
  static void FlushCache();

protected:
  cmDynamicLoader() = default;
  ~cmDynamicLoader() = default;
};
