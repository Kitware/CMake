# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__WINDOWS_EMBTCLANG)
  return()
endif()
set(__WINDOWS_EMBTCLANG 1)

include(Platform/Windows-Clang)

# CMAKE_EMBT_RUNTIME_LIBRARY is forwarded into try_compile() by cmCoreTryCompile,
# so probes compile with the right runtime.  The derived RTL link directory and
# RAD Studio include base are not, so forward them too: a probe for a project
# that adds -tJ to link the RAD Studio RTL, or that includes the RAD headers,
# then resolves rtl.lib/rtl.bpi/rtle.lib and <System.hpp> from these paths.
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
  CMAKE_EMBT_RTL_LIBRARY_DIR CMAKE_EMBT_INCLUDE_DIR)

macro(__windows_compiler_embtclang lang)
  __windows_compiler_clang(${lang})

  # The GNU-driver Clang path gives Debug builds only "-g"; the RAD Studio
  # headers select debug runtime behavior on _DEBUG, which "-g" does not
  # define.  Add it so a Debug build matches the debug runtime.
  string(APPEND CMAKE_${lang}_FLAGS_DEBUG_INIT " -D_DEBUG")

  # Drive the WIN32_EXECUTABLE property through the bcc64x subsystem switches
  # instead of the inherited MSVC-style linker options.  These are link rules.
  # At compile -tC only defines __CONSOLE__, which the RAD Studio headers
  # branch on, and -tW defines nothing; but -tC is recorded in precompiled
  # headers, so the compile side defines the macro instead.  Keep the two
  # separate: a CREATE_*_EXE link rule may carry link-only syntax (LINKER:)
  # that is meaningless on a compile command.
  set(CMAKE_${lang}_CREATE_WIN32_EXE "-tW")
  set(CMAKE_${lang}_CREATE_CONSOLE_EXE "-tC")
  set(CMAKE_${lang}_COMPILE_OPTIONS_EMBT_GUI     "")
  set(CMAKE_${lang}_COMPILE_OPTIONS_EMBT_CONSOLE -D__CONSOLE__=1)

  # The bcc64x toolchain uses MSVC-style naming (no "lib" prefix, ".lib"
  # import/static, ".dll" shared), not the GNU/MinGW names inherited from
  # Windows-Clang's non-MSVC path.
  set(CMAKE_STATIC_LIBRARY_PREFIX "")
  set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
  set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
  set(CMAKE_SHARED_MODULE_PREFIX "")
  set(CMAKE_SHARED_MODULE_SUFFIX ".dll")
  set(CMAKE_IMPORT_LIBRARY_PREFIX "")
  set(CMAKE_IMPORT_LIBRARY_SUFFIX ".lib")
  # Find RAD Studio-named libraries first, then the MinGW names the toolchain's
  # own libraries use: static before import, as the driver links by default.
  set(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".a" ".dll.a")

  # Disable the GNU link-type switches (-Bstatic/-Bdynamic) inherited above.
  # Static and import libraries share the ".lib" name, so they cannot select
  # between them, and the driver sets the link type itself to match the
  # runtime library, which a switch left at the end of the line would undo.
  foreach(__embt_type IN ITEMS EXE SHARED_LIBRARY SHARED_MODULE)
    set(CMAKE_${__embt_type}_LINK_STATIC_${lang}_FLAGS "")
    set(CMAKE_${__embt_type}_LINK_DYNAMIC_${lang}_FLAGS "")
  endforeach()
  unset(__embt_type)

  set(CMAKE_${lang}_OUTPUT_EXTENSION ".o")
  # Append (foo.c.o), do not replace (foo.o): replacing the source extension
  # collides when a target has two sources sharing a stem (e.g. test.c +
  # test.cxx), producing "multiple rules generate <stem>.o"
  set(CMAKE_${lang}_OUTPUT_EXTENSION_REPLACE 0)

  # The default archive command (qc) does not write a symbol index.  The
  # ARCHIVE_FINISH step runs ranlib to add one, but llvm-ranlib may not be
  # present.  Use 's' so llvm-ar embeds the index in the same pass; lld
  # requires the index to resolve symbols from a static archive.
  set(CMAKE_${lang}_ARCHIVE_CREATE "<CMAKE_AR> -qcDs <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_${lang}_ARCHIVE_APPEND "<CMAKE_AR> -qDs  <TARGET> <LINK_FLAGS> <OBJECTS>")

  # The EMBT_RUNTIME_LIBRARY property selects the runtime library the same way as
  # MSVC_RUNTIME_LIBRARY / WATCOM_RUNTIME_LIBRARY: a default activates it and the
  # property overrides.  Static relies on the driver's -tC/-tW/-tD default, which
  # links libc++ statically; Dynamic adds -tR for the shared libc++ and defines
  # USEPACKAGES so the RAD Studio headers import RTL symbols.
  set(CMAKE_EMBT_RUNTIME_LIBRARY_DEFAULT "Static")
  set(CMAKE_${lang}_COMPILE_OPTIONS_EMBT_RUNTIME_LIBRARY_Static  "")
  set(CMAKE_${lang}_COMPILE_OPTIONS_EMBT_RUNTIME_LIBRARY_Dynamic -tR -DUSEPACKAGES)

  # A shared or module library is a plain DLL: -tD at link.  At compile -tD
  # only defines __DLL__, but it is recorded in precompiled headers, which
  # then cannot be reused by other target kinds; define the macro instead.
  set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-shared -tD")
  set(CMAKE_SHARED_MODULE_CREATE_${lang}_FLAGS "-shared -tD")
  set(CMAKE_${lang}_COMPILE_OPTIONS_EMBT_DLL -D__DLL__=1)

  # When a project adds -tJ to link against the RAD Studio RTL, the driver pulls
  # the RTL import libraries (rtl.lib/rtl.bpi and rtle.lib) and startup object
  # from the link search path.  They sit next to the compiler at
  # <compiler>/../lib/win64x/<config>.  Put that directory on the standard link
  # path so a user-added -tJ resolves them.  Override with
  # CMAKE_EMBT_RTL_LIBRARY_DIR.
  if(NOT DEFINED CMAKE_EMBT_RTL_LIBRARY_DIR)
    get_filename_component(__embt_bindir "${CMAKE_${lang}_COMPILER}" DIRECTORY)
    # Use the debug RTL only for a Debug build -- the config that also defines
    # _DEBUG, matching the debug runtime; use release otherwise, including when
    # no build type is set.  Keyed on CMAKE_BUILD_TYPE, so this assumes a
    # single-config generator; multi-config would need a $<CONFIG:Debug> genex.
    if(CMAKE_BUILD_TYPE MATCHES "^[Dd][Ee][Bb][Uu][Gg]$")
      set(__embt_rtl_config debug)
    else()
      set(__embt_rtl_config release)
    endif()
    get_filename_component(CMAKE_EMBT_RTL_LIBRARY_DIR
      "${__embt_bindir}/../lib/win64x/${__embt_rtl_config}" ABSOLUTE)
    unset(__embt_bindir)
    unset(__embt_rtl_config)
  endif()
  if(EXISTS "${CMAKE_EMBT_RTL_LIBRARY_DIR}")
    list(APPEND CMAKE_${lang}_STANDARD_LINK_DIRECTORIES
      "${CMAKE_EMBT_RTL_LIBRARY_DIR}")
  endif()

  # The bcc64x driver does not put the RAD Studio C++ headers on the default
  # include path (the IDE adds them).  Do the same so <System.hpp> and the
  # framework headers resolve.  They live at <compiler>/../include/windows/<set>.
  # Override the base with CMAKE_EMBT_INCLUDE_DIR.
  if(NOT DEFINED CMAKE_EMBT_INCLUDE_DIR)
    get_filename_component(__embt_bindir "${CMAKE_${lang}_COMPILER}" DIRECTORY)
    get_filename_component(CMAKE_EMBT_INCLUDE_DIR
      "${__embt_bindir}/../include/windows" ABSOLUTE)
    unset(__embt_bindir)
  endif()
  foreach(__embt_inc IN ITEMS sdk rtl vcl fmx)
    if(EXISTS "${CMAKE_EMBT_INCLUDE_DIR}/${__embt_inc}")
      list(APPEND CMAKE_${lang}_STANDARD_INCLUDE_DIRECTORIES
        "${CMAKE_EMBT_INCLUDE_DIR}/${__embt_inc}")
    endif()
  endforeach()
  unset(__embt_inc)
endmacro()
