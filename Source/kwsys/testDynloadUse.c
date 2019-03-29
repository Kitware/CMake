/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
#include "testDynloadImpl.h"

#ifdef _WIN32
#  define DL_EXPORT __declspec(dllexport)
#else
#  define DL_EXPORT
#endif

DL_EXPORT int TestLoad()
{
  TestDynamicLoaderImplSymbolPointer();
  return TestDynamicLoaderImplData;
}
