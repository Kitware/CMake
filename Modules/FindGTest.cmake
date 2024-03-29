# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindGTest
---------

Locate the Google C++ Testing Framework.

.. versionadded:: 3.20
  Upstream ``GTestConfig.cmake`` is used if possible.

Imported targets
^^^^^^^^^^^^^^^^

.. versionadded:: 3.20
  This module defines the following :prop_tgt:`IMPORTED` targets:

``GTest::gtest``
  The Google Test ``gtest`` library, if found; adds Thread::Thread
  automatically
``GTest::gtest_main``
  The Google Test ``gtest_main`` library, if found

.. versionadded:: 3.23

``GTest::gmock``
  The Google Mock ``gmock`` library, if found; adds Thread::Thread
  automatically
``GTest::gmock_main``
  The Google Mock ``gmock_main`` library, if found

.. deprecated:: 3.20
  For backwards compatibility, this module defines additionally the
  following deprecated :prop_tgt:`IMPORTED` targets (available since 3.5):

``GTest::GTest``
  The Google Test ``gtest`` library, if found; adds Thread::Thread
  automatically
``GTest::Main``
  The Google Test ``gtest_main`` library, if found

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``GTest_FOUND``
  Found the Google Testing framework
``GTEST_INCLUDE_DIRS``
  the directory containing the Google Test headers

The library variables below are set as normal variables.  These
contain debug/optimized keywords when a debugging library is found.

``GTEST_LIBRARIES``
  The Google Test ``gtest`` library; note it also requires linking
  with an appropriate thread library
``GTEST_MAIN_LIBRARIES``
  The Google Test ``gtest_main`` library
``GTEST_BOTH_LIBRARIES``
  Both ``gtest`` and ``gtest_main``

Cache variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``GTEST_ROOT``
  The root directory of the Google Test installation (may also be
  set as an environment variable)
``GTEST_MSVC_SEARCH``
  If compiling with MSVC, this variable can be set to ``MT`` or
  ``MD`` (the default) to enable searching a GTest build tree


Example usage
^^^^^^^^^^^^^

::

    enable_testing()
    find_package(GTest REQUIRED)

    add_executable(foo foo.cc)
    target_link_libraries(foo GTest::gtest GTest::gtest_main)

    add_test(AllTestsInFoo foo)


Deeper integration with CTest
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

See :module:`GoogleTest` for information on the :command:`gtest_add_tests`
and :command:`gtest_discover_tests` commands.

.. versionchanged:: 3.9
  Previous CMake versions defined :command:`gtest_add_tests` macro in this
  module.
#]=======================================================================]

include(${CMAKE_CURRENT_LIST_DIR}/GoogleTest.cmake)

function(__gtest_append_debugs _endvar _library)
    if(${_library} AND ${_library}_DEBUG)
        set(_output optimized ${${_library}} debug ${${_library}_DEBUG})
    else()
        set(_output ${${_library}})
    endif()
    set(${_endvar} ${_output} PARENT_SCOPE)
endfunction()

function(__gtest_find_library _name)
    find_library(${_name}
        NAMES ${ARGN}
        HINTS
            ENV GTEST_ROOT
            ${GTEST_ROOT}
        PATH_SUFFIXES ${_gtest_libpath_suffixes}
    )
    mark_as_advanced(${_name})
endfunction()

macro(__gtest_determine_windows_library_type _var)
    if(EXISTS "${${_var}}")
        file(TO_NATIVE_PATH "${${_var}}" _lib_path)
        get_filename_component(_name "${${_var}}" NAME_WE)
        cmake_policy(PUSH)
        cmake_policy(SET CMP0159 NEW) # file(STRINGS) with REGEX updates CMAKE_MATCH_<n>
        file(STRINGS "${${_var}}" _match REGEX "${_name}\\.dll" LIMIT_COUNT 1)
        cmake_policy(POP)
        if(NOT _match STREQUAL "")
            set(${_var}_TYPE SHARED PARENT_SCOPE)
        else()
            set(${_var}_TYPE UNKNOWN PARENT_SCOPE)
        endif()
        return()
    endif()
endmacro()

function(__gtest_determine_library_type _var)
    if(WIN32)
        # For now, at least, only Windows really needs to know the library type
        __gtest_determine_windows_library_type(${_var})
        __gtest_determine_windows_library_type(${_var}_RELEASE)
        __gtest_determine_windows_library_type(${_var}_DEBUG)
    endif()
    # If we get here, no determination was made from the above checks
    set(${_var}_TYPE UNKNOWN PARENT_SCOPE)
endfunction()

function(__gtest_import_library _target _var _config)
    if(_config)
        set(_config_suffix "_${_config}")
    else()
        set(_config_suffix "")
    endif()

    set(_lib "${${_var}${_config_suffix}}")
    if(EXISTS "${_lib}")
        if(_config)
            set_property(TARGET ${_target} APPEND PROPERTY
                IMPORTED_CONFIGURATIONS ${_config})
        endif()
        set_target_properties(${_target} PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES${_config_suffix} "CXX")
        if(WIN32 AND ${_var}_TYPE STREQUAL SHARED)
            set_target_properties(${_target} PROPERTIES
                IMPORTED_IMPLIB${_config_suffix} "${_lib}")
        else()
            set_target_properties(${_target} PROPERTIES
                IMPORTED_LOCATION${_config_suffix} "${_lib}")
        endif()
    endif()
endfunction()

function(__gtest_define_backwards_compatible_library_targets)
    set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} PARENT_SCOPE)

    # Add targets mapping the same library names as defined in
    # older versions of CMake's FindGTest
    if(NOT TARGET GTest::GTest)
        add_library(GTest::GTest INTERFACE IMPORTED)
        target_link_libraries(GTest::GTest INTERFACE GTest::gtest)
    endif()
    if(NOT TARGET GTest::Main)
        add_library(GTest::Main INTERFACE IMPORTED)
        target_link_libraries(GTest::Main INTERFACE GTest::gtest_main)
    endif()
endfunction()

#

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

# first specifically look for the CMake version of GTest
find_package(GTest QUIET NO_MODULE)

# if we found the GTest cmake package then we are done, and
# can print what we found and return.
if(GTest_FOUND)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTest HANDLE_COMPONENTS CONFIG_MODE)

    set(GTEST_LIBRARIES      GTest::gtest)
    set(GTEST_MAIN_LIBRARIES GTest::gtest_main)

    __gtest_define_backwards_compatible_library_targets()

    return()
endif()

if(NOT DEFINED GTEST_MSVC_SEARCH)
    set(GTEST_MSVC_SEARCH MD)
endif()

set(_gtest_libpath_suffixes lib)
if(MSVC)
    if(GTEST_MSVC_SEARCH STREQUAL "MD")
        list(APPEND _gtest_libpath_suffixes
            msvc/gtest-md/Debug
            msvc/gtest-md/Release
            msvc/x64/Debug
            msvc/x64/Release
            msvc/2010/gtest-md/Win32-Debug
            msvc/2010/gtest-md/Win32-Release
            msvc/2010/gtest-md/x64-Debug
            msvc/2010/gtest-md/x64-Release
            )
    elseif(GTEST_MSVC_SEARCH STREQUAL "MT")
        list(APPEND _gtest_libpath_suffixes
            msvc/gtest/Debug
            msvc/gtest/Release
            msvc/x64/Debug
            msvc/x64/Release
            msvc/2010/gtest/Win32-Debug
            msvc/2010/gtest/Win32-Release
            msvc/2010/gtest/x64-Debug
            msvc/2010/gtest/x64-Release
            )
    endif()
endif()


find_path(GTEST_INCLUDE_DIR gtest/gtest.h
    HINTS
        $ENV{GTEST_ROOT}/include
        ${GTEST_ROOT}/include
)
mark_as_advanced(GTEST_INCLUDE_DIR)

if(MSVC AND GTEST_MSVC_SEARCH STREQUAL "MD")
    # The provided /MD project files for Google Test add -md suffixes to the
    # library names.
    __gtest_find_library(GTEST_LIBRARY            gtest-md  gtest)
    __gtest_find_library(GTEST_LIBRARY_DEBUG      gtest-mdd gtestd)
    __gtest_find_library(GTEST_MAIN_LIBRARY       gtest_main-md  gtest_main)
    __gtest_find_library(GTEST_MAIN_LIBRARY_DEBUG gtest_main-mdd gtest_maind)
    __gtest_find_library(GMOCK_LIBRARY            gmock-md  gmock)
    __gtest_find_library(GMOCK_LIBRARY_DEBUG      gmock-mdd gmockd)
    __gtest_find_library(GMOCK_MAIN_LIBRARY       gmock_main-md  gmock_main)
    __gtest_find_library(GMOCK_MAIN_LIBRARY_DEBUG gmock_main-mdd gmock_maind)
else()
    __gtest_find_library(GTEST_LIBRARY            gtest)
    __gtest_find_library(GTEST_LIBRARY_DEBUG      gtestd)
    __gtest_find_library(GTEST_MAIN_LIBRARY       gtest_main)
    __gtest_find_library(GTEST_MAIN_LIBRARY_DEBUG gtest_maind)
    __gtest_find_library(GMOCK_LIBRARY            gmock)
    __gtest_find_library(GMOCK_LIBRARY_DEBUG      gmockd)
    __gtest_find_library(GMOCK_MAIN_LIBRARY       gmock_main)
    __gtest_find_library(GMOCK_MAIN_LIBRARY_DEBUG gmock_maind)
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTest DEFAULT_MSG GTEST_LIBRARY GTEST_INCLUDE_DIR GTEST_MAIN_LIBRARY)

if(GMOCK_LIBRARY AND GMOCK_MAIN_LIBRARY)
    set(GMock_FOUND True)
else()
    set(GMock_FOUND False)
endif()

if(GTest_FOUND)
    set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIR})
    __gtest_append_debugs(GTEST_LIBRARIES      GTEST_LIBRARY)
    __gtest_append_debugs(GTEST_MAIN_LIBRARIES GTEST_MAIN_LIBRARY)

    find_package(Threads QUIET)

    if(NOT TARGET GTest::gtest)
        __gtest_determine_library_type(GTEST_LIBRARY)
        add_library(GTest::gtest ${GTEST_LIBRARY_TYPE} IMPORTED)
        if(TARGET Threads::Threads)
            set_target_properties(GTest::gtest PROPERTIES
                INTERFACE_LINK_LIBRARIES Threads::Threads)
        endif()
        if(GTEST_LIBRARY_TYPE STREQUAL "SHARED")
            set_target_properties(GTest::gtest PROPERTIES
                INTERFACE_COMPILE_DEFINITIONS "GTEST_LINKED_AS_SHARED_LIBRARY=1")
        endif()
        if(GTEST_INCLUDE_DIRS)
            set_target_properties(GTest::gtest PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIRS}")
        endif()
        __gtest_import_library(GTest::gtest GTEST_LIBRARY "")
        __gtest_import_library(GTest::gtest GTEST_LIBRARY "RELEASE")
        __gtest_import_library(GTest::gtest GTEST_LIBRARY "DEBUG")
    endif()
    if(NOT TARGET GTest::gtest_main)
        __gtest_determine_library_type(GTEST_MAIN_LIBRARY)
        add_library(GTest::gtest_main ${GTEST_MAIN_LIBRARY_TYPE} IMPORTED)
        set_target_properties(GTest::gtest_main PROPERTIES
            INTERFACE_LINK_LIBRARIES "GTest::gtest")
        __gtest_import_library(GTest::gtest_main GTEST_MAIN_LIBRARY "")
        __gtest_import_library(GTest::gtest_main GTEST_MAIN_LIBRARY "RELEASE")
        __gtest_import_library(GTest::gtest_main GTEST_MAIN_LIBRARY "DEBUG")
    endif()

    __gtest_define_backwards_compatible_library_targets()
endif()

if(GMock_FOUND AND GTest_FOUND)
    if(NOT TARGET GTest::gmock)
        __gtest_determine_library_type(GMOCK_LIBRARY)
        add_library(GTest::gmock ${GMOCK_LIBRARY_TYPE} IMPORTED)
        set(_gmock_link_libraries "GTest::gtest")
        if(TARGET Threads::Threads)
            list(APPEND _gmock_link_libraries Threads::Threads)
        endif()
        set_target_properties(GTest::gmock PROPERTIES
            INTERFACE_LINK_LIBRARIES "${_gmock_link_libraries}")
        if(GMOCK_LIBRARY_TYPE STREQUAL "SHARED")
            set_target_properties(GTest::gmock PROPERTIES
                INTERFACE_COMPILE_DEFINITIONS "GMOCK_LINKED_AS_SHARED_LIBRARY=1")
        endif()
        if(GTEST_INCLUDE_DIRS)
            set_target_properties(GTest::gmock PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIRS}")
        endif()
        __gtest_import_library(GTest::gmock GMOCK_LIBRARY "")
        __gtest_import_library(GTest::gmock GMOCK_LIBRARY "RELEASE")
        __gtest_import_library(GTest::gmock GMOCK_LIBRARY "DEBUG")
    endif()
    if(NOT TARGET GTest::gmock_main)
        __gtest_determine_library_type(GMOCK_MAIN_LIBRARY)
        add_library(GTest::gmock_main ${GMOCK_MAIN_LIBRARY_TYPE} IMPORTED)
        set_target_properties(GTest::gmock_main PROPERTIES
            INTERFACE_LINK_LIBRARIES "GTest::gmock")
        __gtest_import_library(GTest::gmock_main GMOCK_MAIN_LIBRARY "")
        __gtest_import_library(GTest::gmock_main GMOCK_MAIN_LIBRARY "RELEASE")
        __gtest_import_library(GTest::gmock_main GMOCK_MAIN_LIBRARY "DEBUG")
    endif()
endif()
