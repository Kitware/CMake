/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
#ifdef _WIN32
#  ifdef BUILDING_TestDynloadImpl
#    define DLIMPL_EXPORT __declspec(dllexport)
#  else
#    define DLIMPL_EXPORT __declspec(dllimport)
#  endif
#else
#  define DLIMPL_EXPORT
#endif

DLIMPL_EXPORT int TestDynamicLoaderImplData;

DLIMPL_EXPORT void TestDynamicLoaderImplSymbolPointer();
