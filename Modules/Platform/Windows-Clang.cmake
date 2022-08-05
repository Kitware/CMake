# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__WINDOWS_CLANG)
  return()
endif()
set(__WINDOWS_CLANG 1)

set(__pch_header_C "c-header")
set(__pch_header_CXX "c++-header")
set(__pch_header_OBJC "objective-c-header")
set(__pch_header_OBJCXX "objective-c++-header")

macro(__windows_compiler_clang_gnu lang)
  set(CMAKE_LIBRARY_PATH_FLAG "-L")
  set(CMAKE_LINK_LIBRARY_FLAG "-l")

  set(CMAKE_IMPORT_LIBRARY_PREFIX "")
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
  set(CMAKE_SHARED_MODULE_PREFIX  "")
  set(CMAKE_STATIC_LIBRARY_PREFIX "")
  set(CMAKE_EXECUTABLE_SUFFIX     ".exe")
  set(CMAKE_IMPORT_LIBRARY_SUFFIX ".lib")
  set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
  set(CMAKE_SHARED_MODULE_SUFFIX  ".dll")
  set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")
  if(NOT "${lang}" STREQUAL "ASM")
    set(CMAKE_DEPFILE_FLAGS_${lang} "-MD -MT <DEP_TARGET> -MF <DEP_FILE>")
  endif()

  set(CMAKE_FIND_LIBRARY_PREFIXES "lib" "")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a" ".a" ".lib")
  set(CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1)
  set (CMAKE_LINK_DEF_FILE_FLAG "-Xlinker /DEF:")

  set(CMAKE_${lang}_LINKER_WRAPPER_FLAG "-Xlinker" " ")
  set(CMAKE_${lang}_LINKER_WRAPPER_FLAG_SEP)

  set(CMAKE_${lang}_LINKER_MANIFEST_FLAG " -Xlinker /MANIFESTINPUT:")
  set(CMAKE_${lang}_COMPILE_OPTIONS_WARNING_AS_ERROR "-Werror")

  if("${CMAKE_${lang}_SIMULATE_VERSION}" MATCHES "^([0-9]+)\\.([0-9]+)")
    math(EXPR MSVC_VERSION "${CMAKE_MATCH_1}*100 + ${CMAKE_MATCH_2}")
  endif()

  # No -fPIC on Windows
  set(CMAKE_${lang}_COMPILE_OPTIONS_PIC "")
  set(CMAKE_${lang}_COMPILE_OPTIONS_PIE "")
  set(_CMAKE_${lang}_PIE_MAY_BE_SUPPORTED_BY_LINKER NO)
  set(CMAKE_${lang}_LINK_OPTIONS_PIE "")
  set(CMAKE_${lang}_LINK_OPTIONS_NO_PIE "")
  set(CMAKE_SHARED_LIBRARY_${lang}_FLAGS "")

  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_OBJECTS 1)
  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_INCLUDES 1)

  if(CMAKE_${lang}_COMPILER_VERSION VERSION_GREATER_EQUAL 3.9)
    set(CMAKE_${lang}_COMPILE_OPTIONS_IPO "-flto=thin")
  else()
    set(CMAKE_${lang}_COMPILE_OPTIONS_IPO "-flto")
  endif()

  set(_CMAKE_${lang}_IPO_SUPPORTED_BY_CMAKE YES)
  set(_CMAKE_${lang}_IPO_MAY_BE_SUPPORTED_BY_COMPILER YES)
  set(CMAKE_${lang}_ARCHIVE_CREATE_IPO "<CMAKE_AR> qc <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_${lang}_ARCHIVE_APPEND_IPO "<CMAKE_AR> q <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_${lang}_ARCHIVE_FINISH_IPO "<CMAKE_RANLIB> <TARGET>")

  # Create archiving rules to support large object file lists for static libraries.
  set(CMAKE_${lang}_ARCHIVE_CREATE "<CMAKE_AR> qc <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_${lang}_ARCHIVE_APPEND "<CMAKE_AR> q <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_${lang}_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")
  set(CMAKE_${lang}_CREATE_SHARED_LIBRARY
    "<CMAKE_${lang}_COMPILER> -fuse-ld=lld-link -nostartfiles -nostdlib <CMAKE_SHARED_LIBRARY_${lang}_FLAGS> <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS> -o <TARGET> ${CMAKE_GNULD_IMAGE_VERSION} -Xlinker /MANIFEST:EMBED -Xlinker /implib:<TARGET_IMPLIB> -Xlinker /pdb:<TARGET_PDB> -Xlinker /version:<TARGET_VERSION_MAJOR>.<TARGET_VERSION_MINOR> <OBJECTS> <LINK_LIBRARIES> <MANIFESTS>")
  set(CMAKE_${lang}_CREATE_SHARED_MODULE ${CMAKE_${lang}_CREATE_SHARED_LIBRARY})
  set(CMAKE_${lang}_LINK_EXECUTABLE
    "<CMAKE_${lang}_COMPILER> -fuse-ld=lld-link -nostartfiles -nostdlib <FLAGS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> -Xlinker /MANIFEST:EMBED -Xlinker /implib:<TARGET_IMPLIB> -Xlinker /pdb:<TARGET_PDB> -Xlinker /version:<TARGET_VERSION_MAJOR>.<TARGET_VERSION_MINOR> ${CMAKE_GNULD_IMAGE_VERSION} <LINK_LIBRARIES> <MANIFESTS>")

  set(CMAKE_${lang}_CREATE_WIN32_EXE "-Xlinker /subsystem:windows")
  set(CMAKE_${lang}_CREATE_CONSOLE_EXE "-Xlinker /subsystem:console")

  if(NOT "${lang}" STREQUAL "ASM")
    set(CMAKE_${lang}_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreaded         -Xclang -flto-visibility-public-std -D_MT -Xclang --dependent-lib=libcmt)
    set(CMAKE_${lang}_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreadedDLL      -D_DLL -D_MT -Xclang --dependent-lib=msvcrt)
    set(CMAKE_${lang}_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreadedDebug    -D_DEBUG -Xclang -flto-visibility-public-std -D_MT -Xclang --dependent-lib=libcmtd)
    set(CMAKE_${lang}_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreadedDebugDLL -D_DEBUG -D_DLL -D_MT -Xclang --dependent-lib=msvcrtd)

    if(CMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT)
      set(__ADDED_FLAGS "")
      set(__ADDED_FLAGS_DEBUG "")
    else()
      set(__ADDED_FLAGS_DEBUG "-D_DEBUG -D_DLL -D_MT -Xclang --dependent-lib=msvcrtd")
      set(__ADDED_FLAGS "-D_DLL -D_MT -Xclang --dependent-lib=msvcrt")
    endif()

    string(APPEND CMAKE_${lang}_FLAGS_DEBUG_INIT " -g -Xclang -gcodeview -O0 ${__ADDED_FLAGS_DEBUG}")
    string(APPEND CMAKE_${lang}_FLAGS_MINSIZEREL_INIT " -Os -DNDEBUG ${__ADDED_FLAGS}")
    string(APPEND CMAKE_${lang}_FLAGS_RELEASE_INIT " -O3 -DNDEBUG ${__ADDED_FLAGS}")
    string(APPEND CMAKE_${lang}_FLAGS_RELWITHDEBINFO_INIT " -O2 -g -DNDEBUG -Xclang -gcodeview ${__ADDED_FLAGS}")
  endif()
  set(CMAKE_INCLUDE_SYSTEM_FLAG_${lang} "-isystem ")
  set(CMAKE_${lang}_LINKER_SUPPORTS_PDB ON)

  set(CMAKE_PCH_EXTENSION .pch)
  set(CMAKE_PCH_PROLOGUE "#pragma clang system_header")
  set(CMAKE_${lang}_COMPILE_OPTIONS_USE_PCH -Xclang -include-pch -Xclang <PCH_FILE> -Xclang -include -Xclang <PCH_HEADER>)
  set(CMAKE_${lang}_COMPILE_OPTIONS_CREATE_PCH -Xclang -emit-pch -Xclang -include -Xclang <PCH_HEADER> -x ${__pch_header_${lang}})

  unset(__ADDED_FLAGS)
  unset(__ADDED_FLAGS_DEBUG)
  string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
  set(CMAKE_${lang}_STANDARD_LIBRARIES_INIT "-lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32 -loldnames")

  # Features for LINK_LIBRARY generator expression
  if(MSVC_VERSION GREATER "1900")
    ## WHOLE_ARCHIVE: Force loading all members of an archive
    set(CMAKE_${lang}_LINK_LIBRARY_USING_WHOLE_ARCHIVE "LINKER:/WHOLEARCHIVE:<LIBRARY>")
    set(CMAKE_${lang}_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED TRUE)
  endif()

  enable_language(RC)
endmacro()

macro(__enable_llvm_rc_preprocessing clang_option_prefix extra_pp_flags)
  # Feed the preprocessed rc file to llvm-rc
  if(CMAKE_RC_COMPILER_INIT MATCHES "llvm-rc" OR CMAKE_RC_COMPILER MATCHES "llvm-rc")
    if(DEFINED CMAKE_C_COMPILER_ID)
      set(CMAKE_RC_PREPROCESSOR CMAKE_C_COMPILER)
    elseif(DEFINED CMAKE_CXX_COMPILER_ID)
      set(CMAKE_RC_PREPROCESSOR CMAKE_CXX_COMPILER)
    endif()
    if(DEFINED CMAKE_RC_PREPROCESSOR)
      set(CMAKE_DEPFILE_FLAGS_RC "${clang_option_prefix}-MD ${clang_option_prefix}-MF ${clang_option_prefix}<DEP_FILE>")
      # The <FLAGS> are passed to the preprocess and the resource compiler to pick
      # up the eventual -D / -C options passed through the CMAKE_RC_FLAGS.
      set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_COMMAND> -E cmake_llvm_rc <SOURCE> <OBJECT>.pp <${CMAKE_RC_PREPROCESSOR}> <DEFINES> -DRC_INVOKED <INCLUDES> <FLAGS> ${extra_pp_flags} -E -- <SOURCE> ++ <CMAKE_RC_COMPILER> <DEFINES> -I <SOURCE_DIR> <INCLUDES> <FLAGS> /fo <OBJECT> <OBJECT>.pp")
      if(CMAKE_GENERATOR MATCHES "Ninja")
        set(CMAKE_NINJA_CMCLDEPS_RC 0)
        set(CMAKE_NINJA_DEP_TYPE_RC gcc)
      endif()
      unset(CMAKE_RC_PREPROCESSOR)
    endif()
  endif()
endmacro()


if("x${CMAKE_C_SIMULATE_ID}" STREQUAL "xMSVC"
    OR "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")

  if ( DEFINED CMAKE_C_COMPILER_ID AND DEFINED CMAKE_CXX_COMPILER_ID
       AND NOT "x${CMAKE_C_COMPILER_ID}" STREQUAL "x${CMAKE_CXX_COMPILER_ID}")
    message(FATAL_ERROR "The current configuration mixes Clang and MSVC or "
            "some other CL compatible compiler tool. This is not supported. "
            "Use either clang or MSVC as both C and C++ compilers.")
  endif()

  if ( DEFINED CMAKE_C_COMPILER_FRONTEND_VARIANT AND DEFINED CMAKE_CXX_COMPILER_FRONTEND_VARIANT
       AND NOT "x${CMAKE_C_COMPILER_FRONTEND_VARIANT}" STREQUAL "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")
    message(FATAL_ERROR "The current configuration uses the Clang compiler "
            "tool with mixed frontend variants, both the GNU and in MSVC CL "
            "like variants. This is not supported. Use either clang/clang++ "
            "or clang-cl as both C and C++ compilers.")
  endif()

  if(NOT CMAKE_RC_COMPILER_INIT)
    # Check if rc is already in the path
    # This may happen in cases where the user is already in a visual studio environment when CMake is invoked
    find_program(__RC_COMPILER_PATH NAMES rc)

    # Default to rc if it's available, otherwise fall back to llvm-rc
    if(__RC_COMPILER_PATH)
      set(CMAKE_RC_COMPILER_INIT rc)
    else()
      find_program(__RC_COMPILER_PATH NAMES llvm-rc)
      if(__RC_COMPILER_PATH)
        set(CMAKE_RC_COMPILER_INIT llvm-rc)
      endif()
    endif()

    unset(__RC_COMPILER_PATH CACHE)
  endif()

  if ( "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC" OR "x${CMAKE_C_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC" )
    include(Platform/Windows-MSVC)
    # Set the clang option forwarding prefix for clang-cl usage in the llvm-rc processing stage
    __enable_llvm_rc_preprocessing("-clang:" "")
    macro(__windows_compiler_clang_base lang)
      set(_COMPILE_${lang} "${_COMPILE_${lang}_MSVC}")
      __windows_compiler_msvc(${lang})
      set(CMAKE_${lang}_COMPILE_OPTIONS_WARNING_AS_ERROR "-WX")
      set(CMAKE_INCLUDE_SYSTEM_FLAG_${lang} "-imsvc")
    endmacro()
  else()
    cmake_policy(GET CMP0091 __WINDOWS_CLANG_CMP0091)
    if(__WINDOWS_CLANG_CMP0091 STREQUAL "NEW")
      set(CMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
      set(CMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT "")
    endif()
    unset(__WINDOWS_CLANG_CMP0091)

    set(CMAKE_BUILD_TYPE_INIT Debug)

    __enable_llvm_rc_preprocessing("" "-x c")
    macro(__windows_compiler_clang_base lang)
      __windows_compiler_clang_gnu(${lang})
    endmacro()
  endif()

else()
  include(Platform/Windows-GNU)
  __enable_llvm_rc_preprocessing("" "-x c")
  macro(__windows_compiler_clang_base lang)
    __windows_compiler_gnu(${lang})
  endmacro()
endif()

macro(__windows_compiler_clang lang)
  if(CMAKE_${lang}_COMPILER_VERSION VERSION_LESS 3.4.0)
    set(CMAKE_${lang}_COMPILE_OPTIONS_TARGET "-target ")
  else()
    set(CMAKE_${lang}_COMPILE_OPTIONS_TARGET "--target=")
  endif()
  __windows_compiler_clang_base(${lang})
endmacro()
