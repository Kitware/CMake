# Enable scanning by default for targets that explicitly use C++ 20.
cmake_policy(SET CMP0155 NEW)

enable_language(CXX)

# Hide any real scanning rule that may be available.
unset(CMAKE_CXX_SCANDEP_SOURCE)

# Explicitly enable C++20 for all targets.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Explicitly suppress scanning so that support is not required.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

add_executable(noscanning-variable main-no-use.cxx)

# Verify that CMAKE_CXX_SCAN_FOR_MODULES is propagated into the check.
# FIXME: Unset CMAKE_CXX_SCANDEP_SOURCE inside try_compile so this
# test can verify behavior on newer compilers too.
try_compile(result SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main-no-use.cxx)
