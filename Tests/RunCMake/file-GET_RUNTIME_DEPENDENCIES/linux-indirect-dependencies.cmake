enable_language(C)
cmake_policy(SET CMP0095 NEW)

file(WRITE "${CMAKE_BINARY_DIR}/A.c" "void libA(void) {}\n")
file(WRITE "${CMAKE_BINARY_DIR}/C.c" "void libC(void) {}\n")
file(WRITE "${CMAKE_BINARY_DIR}/BUseAC.c" [[
extern void libA(void);
extern void libC(void);
void libB(void)
{
    libA();
    libC();
}
]])
file(WRITE "${CMAKE_BINARY_DIR}/mainABC.c" [[
extern void libA(void);
extern void libB(void);
extern void libC(void);

int main(void)
{
    libA();
    libB();
    libC();
    return 0;
}

]])

set(lib_dirExe "${CMAKE_BINARY_DIR}/Exe")
set(lib_dirA "${CMAKE_BINARY_DIR}/libA")
set(lib_dirB "${CMAKE_BINARY_DIR}/libB")
set(lib_dirC "${CMAKE_BINARY_DIR}/libC")
file(MAKE_DIRECTORY ${lib_dirExe})
file(MAKE_DIRECTORY ${lib_dirA})
file(MAKE_DIRECTORY ${lib_dirB})
file(MAKE_DIRECTORY ${lib_dirC})

add_library(A SHARED "${CMAKE_BINARY_DIR}/A.c")
set_property(TARGET A PROPERTY LIBRARY_OUTPUT_DIRECTORY ${lib_dirA})

add_library(C SHARED "${CMAKE_BINARY_DIR}/C.c")
set_property(TARGET C PROPERTY LIBRARY_OUTPUT_DIRECTORY ${lib_dirC})

# We doesn't need to set A as a dependency of B, because we don't need `RUNPATH` value set for B
add_library(B SHARED "${CMAKE_BINARY_DIR}/BUseAC.c")
target_link_libraries(B PRIVATE A C)
set_property(TARGET B PROPERTY LIBRARY_OUTPUT_DIRECTORY ${lib_dirB})

# We MUST have empty `RUNPATH` in A & B
set_target_properties(A B C PROPERTIES
    BUILD_WITH_INSTALL_RPATH 1
)

# The executable is really workable without `RUNPATH` in B
add_executable(exe "${CMAKE_BINARY_DIR}/mainABC.c")
target_link_libraries(exe A B C)
set_property(TARGET exe PROPERTY RUNTIME_OUTPUT_DIRECTORY ${lib_dirExe})

# We MUST have `RUNPATH` in exe, not `RPATH`
# Test will pass if we have `RPATH`, because of the inheritance
target_link_options(exe PRIVATE -Wl,--enable-new-dtags)

install(CODE [[
    # Work with non-installed binary, because of the RUNPATH values
    set(exeFile "$<TARGET_FILE:exe>")

    # Check executable is can be successfully finished
    execute_process(
        COMMAND "${exeFile}"
        COMMAND_ERROR_IS_FATAL ANY
    )

    # Check dependencies resolved
    file(GET_RUNTIME_DEPENDENCIES
        RESOLVED_DEPENDENCIES_VAR RESOLVED
        PRE_INCLUDE_REGEXES "^lib[ABC]\\.so$"
        PRE_EXCLUDE_REGEXES ".*"
        EXECUTABLES
            "${exeFile}"
    )
    message(STATUS "Resolved dependencies: ${RESOLVED}")
]])
