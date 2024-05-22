# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindATL
-------

Find Microsoft Advanced Template Library (ATL) on Windows

Find the native ATL - i.e.  decide if an application can link to the
ATL libraries.

::

  ATL_FOUND - Was ATL support found

You don't need to include anything or link anything to use it.
#]=======================================================================]

# Assume no ATL support
set(ATL_FOUND "NO")

# Only attempt the try_compile call if it has a chance to succeed:
set(ATL_ATTEMPT_TRY_COMPILE 0)
if(WIN32 AND NOT UNIX AND NOT BORLAND AND NOT MINGW)
  set(ATL_ATTEMPT_TRY_COMPILE 1)
endif()

if(ATL_ATTEMPT_TRY_COMPILE)
  if(NOT DEFINED ATL_HAVE_ATL)
    set(CHECK_INCLUDE_FILE_VAR "atlbase.h")
    file(READ ${CMAKE_ROOT}/Modules/CheckIncludeFile.cxx.in _CIF_SOURCE_CONTENT)
    string(CONFIGURE "${_CIF_SOURCE_CONTENT}" _CIF_SOURCE_CONTENT)
    message(CHECK_START "Looking for ATL")
    # Try both shared and static as the root project may have set the /MT flag
    try_compile(ATL_HAVE_ATL
      SOURCE_FROM_VAR CheckIncludeFile.cxx _CIF_SOURCE_CONTENT
      CMAKE_FLAGS
      -DCMAKE_ATL_FLAG:STRING=2
      OUTPUT_VARIABLE OUTPUT)
    if(NOT ATL_HAVE_ATL)
      try_compile(ATL_HAVE_ATL
        SOURCE_FROM_VAR CheckIncludeFile.cxx _CIF_SOURCE_CONTENT
        CMAKE_FLAGS
        -DCMAKE_ATL_FLAG:STRING=1
        OUTPUT_VARIABLE OUTPUT)
    endif()
    if(ATL_HAVE_ATL)
      message(CHECK_PASS "found")
      set(ATL_HAVE_ATL 1 CACHE INTERNAL "Have ATL?")
    else()
      message(CHECK_FAIL "not found")
      set(ATL_HAVE_ATL 0 CACHE INTERNAL "Have ATL?")
    endif()
  endif()

  if(ATL_HAVE_ATL)
    set(ATL_FOUND "YES")
  endif()
endif()
