include(RunCMake)

# Registration and initialization of the EMBT_RUNTIME_LIBRARY target property is
# generic and does not require the Embarcadero toolchain, so it runs on any host.
run_cmake(embt-properties)

# EMBT_RUNTIME_LIBRARY link selection: Dynamic links the shared libc++
# (libc++-380.dll import), Static links it statically (no import).  Builds real
# exes with bcc64x and checks imports; a no-op where bcc64x is absent.
run_cmake(runtime-library)

# Compiler-id regression: bcc64x -> EMBTClang, older drivers -> Embarcadero.
# The case finds the drivers itself and is a no-op where none are installed.
run_cmake(compiler-id-drivers)

# Binutils pinning: the linker and LLVM tools resolve to distinct absolute paths
# beside the compiler.  Finds bcc64x itself; a no-op where it is absent.
run_cmake(binutils-pinned)

# Subsystem macros: a console target compiles with __CONSOLE__ and a GUI target
# without it (compilation sees the macro, not just the link switch).
# Driver-gated.
run_cmake(subsystem-macros)

# Precompiled header reuse across target kinds: a static library's header is
# reused by shared and module libraries and console and GUI executables.
# Driver-gated.
run_cmake(pch-reuse-kinds)

# C++ module scanning is not supported: a C++20 target builds without being
# scanned, and C++ module sources are rejected.  Driver-gated.
run_cmake(cxx-module-scanning)
