# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include(CMakeLanguageInformation)

set(CMAKE_Rust_OUTPUT_EXTENSION .rlib)

# Other values are supported to generate various outputs (LLVM bitcode, or IR,
# crate metadata, Rust MIR). However, CMake cannot do anything with those
# outputs, so we list output which can be reused in later stages of the build.
# See: https://doc.rust-lang.org/rustc/command-line-arguments.html#--emit-specifies-the-types-of-output-files-to-generate
set(CMAKE_Rust_EMIT_VALUES link obj asm)

# The output extension for each supported emit value.
set(CMAKE_Rust_EMIT_link_OUTPUT_EXTENSION .rlib)
set(CMAKE_Rust_EMIT_asm_OUTPUT_EXTENSION .s)
# Might be switched to .obj on Windows when using MSVC target triple, see:
# https://github.com/rust-lang/rust/issues/37207
set(CMAKE_Rust_EMIT_obj_OUTPUT_EXTENSION .o)

set(CMAKE_Rust_LIBRARY_PATH_FLAG "-L ")
set(CMAKE_Rust_LINK_LIBRARY_FILE_FLAG "-C link-arg=")
set(CMAKE_EXECUTABLE_RUNTIME_Rust_FLAG "-C link-arg=-Wl,-rpath,")
set(CMAKE_EXECUTABLE_RUNTIME_Rust_FLAG_SEP ",")

set(CMAKE_Rust_FLAGS_DEBUG_INIT "-C opt-level=0 -g")
set(CMAKE_Rust_FLAGS_RELEASE_INIT "-O")
set(CMAKE_Rust_FLAGS_RELWITHDEBINFO_INIT "-O -g")
set(CMAKE_Rust_FLAGS_MINSIZEREL_INIT "-C opt-level=z")

block(
    PROPAGATE
    CMAKE_Rust_LINK_PIE_SUPPORTED
    CMAKE_Rust_LINK_NO_PIE_SUPPORTED
    CMAKE_Rust_COMPILE_OPTIONS_PIE
    CMAKE_Rust_COMPILE_OPTIONS_PIC
    CMAKE_Rust_LINK_OPTIONS_PIE
    CMAKE_Rust_LINK_OPTIONS_NO_PIE
  )
  execute_process(
    COMMAND "${CMAKE_Rust_COMPILER}" --print relocation-models
    OUTPUT_VARIABLE RUSTC_OUTPUT
    ERROR_VARIABLE RUSTC_ERROR
    RESULT_VARIABLE RUSTC_EXITCODE
  )
  if(RUSTC_EXITCODE EQUAL "0")
    string(REPLACE "\n" ";" RUSTC_OUTPUT_LINES "${RUSTC_OUTPUT}")
    list(TRANSFORM RUSTC_OUTPUT_LINES STRIP)
    if("pic" IN_LIST RUSTC_OUTPUT_LINES)
      set(CMAKE_Rust_COMPILE_OPTIONS_PIC -C relocation-model=pic)
    endif()
    if("pie" IN_LIST RUSTC_OUTPUT_LINES)
      set(CMAKE_Rust_LINK_PIE_SUPPORTED TRUE)
      set(CMAKE_Rust_COMPILE_OPTIONS_PIE -C relocation-model=pie)
      set(CMAKE_Rust_LINK_OPTIONS_PIE -C relocation-model=pie)
    else()
      set(CMAKE_Rust_LINK_PIE_SUPPORTED FALSE)
    endif()
    if("static" IN_LIST RUSTC_OUTPUT_LINES)
      set(CMAKE_Rust_LINK_NO_PIE_SUPPORTED TRUE)
      set(CMAKE_Rust_LINK_OPTIONS_NO_PIE -C relocation-model=static)
    else()
      set(CMAKE_Rust_LINK_NO_PIE_SUPPORTED FALSE)
    endif()
  else()
    string(REPLACE "\n" "\n  " RUSTC_ERROR "  ${RUSTC_ERROR}")
    message(FATAL_ERROR "Failed to check PIC/PIE support in rustc:\n${RUSTC_ERROR}")
  endif()
endblock()

cmake_initialize_per_config_variable(CMAKE_Rust_FLAGS "Flags used by the Rust compiler")

if(NOT CMAKE_Rust_COMPILE_OBJECT)
  set(CMAKE_Rust_COMPILE_OBJECT "<CMAKE_Rust_COMPILER> --crate-type=rlib <FLAGS> --emit=<RUST_EMIT>,dep-info=<DEP_FILE> -o <OBJECT> <SOURCE>")
endif()

if(NOT CMAKE_Rust_CREATE_STATIC_LIBRARY)
  set(CMAKE_Rust_CREATE_STATIC_LIBRARY "${CMAKE_Rust_COMPILER} <LANGUAGE_COMPILE_FLAGS> --crate-type=staticlib --emit=link,dep-info=<DEP_FILE> <RUST_MAIN_CRATE_ROOT> -o <TARGET> <RUST_LINK_CRATES> <RUST_NATIVE_OBJECTS>")
endif()

if(NOT CMAKE_Rust_CREATE_SHARED_LIBRARY)
  set(CMAKE_Rust_CREATE_SHARED_LIBRARY "${CMAKE_Rust_COMPILER} <LANGUAGE_COMPILE_FLAGS> --crate-type=cdylib --emit=link,dep-info=<DEP_FILE> <RUST_MAIN_CRATE_ROOT> -o <TARGET> <RUST_LINK_CRATES> <RUST_NATIVE_OBJECTS> <LINK_FLAGS> <LINK_LIBRARIES>")
endif()

if(NOT CMAKE_Rust_LINK_EXECUTABLE)
  set(CMAKE_Rust_LINK_EXECUTABLE "${CMAKE_Rust_COMPILER} <FLAGS> --crate-type=bin --emit=link,dep-info=<DEP_FILE> <RUST_MAIN_CRATE_ROOT> -o <TARGET> <RUST_LINK_CRATES> <RUST_NATIVE_OBJECTS> <LINK_FLAGS> <LINK_LIBRARIES>")
endif()

set(CMAKE_Rust_INFORMATION_LOADED 1)
