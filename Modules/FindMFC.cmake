# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindMFC
-------

Find Microsoft Foundation Class Library (MFC) on Windows

Find the native MFC - i.e.  decide if an application can link to the
MFC libraries.

::

  MFC_FOUND - Was MFC support found

You don't need to include anything or link anything to use it.
#]=======================================================================]

# Assume no MFC support
set(MFC_FOUND "NO")

# Only attempt the try_compile call if it has a chance to succeed:
set(MFC_ATTEMPT_TRY_COMPILE 0)
if(WIN32 AND NOT UNIX AND NOT BORLAND AND NOT MINGW)
  set(MFC_ATTEMPT_TRY_COMPILE 1)
endif()

if(MFC_ATTEMPT_TRY_COMPILE)
  if(NOT DEFINED MFC_HAVE_MFC)
    set(CHECK_INCLUDE_FILE_VAR "afxwin.h")
    file(READ ${CMAKE_ROOT}/Modules/CheckIncludeFile.cxx.in _CIF_SOURCE_CONTENT)
    string(CONFIGURE "${_CIF_SOURCE_CONTENT}" _CIF_SOURCE_CONTENT)
    message(CHECK_START "Looking for MFC")
    # Try both shared and static as the root project may have set the /MT flag
    try_compile(MFC_HAVE_MFC
      SOURCE_FROM_VAR CheckIncludeFile.cxx _CIF_SOURCE_CONTENT
      CMAKE_FLAGS
      -DCMAKE_MFC_FLAG:STRING=2
      -DCOMPILE_DEFINITIONS:STRING=-D_AFXDLL
      OUTPUT_VARIABLE OUTPUT)
    if(NOT MFC_HAVE_MFC)
      try_compile(MFC_HAVE_MFC
        SOURCE_FROM_VAR CheckIncludeFile.cxx _CIF_SOURCE_CONTENT
        CMAKE_FLAGS
        -DCMAKE_MFC_FLAG:STRING=1
        OUTPUT_VARIABLE OUTPUT)
    endif()
    if(MFC_HAVE_MFC)
      message(CHECK_PASS "found")
      set(MFC_HAVE_MFC 1 CACHE INTERNAL "Have MFC?")
    else()
      message(CHECK_FAIL "not found")
      set(MFC_HAVE_MFC 0 CACHE INTERNAL "Have MFC?")
    endif()
  endif()

  if(MFC_HAVE_MFC)
    set(MFC_FOUND "YES")
  endif()
endif()
