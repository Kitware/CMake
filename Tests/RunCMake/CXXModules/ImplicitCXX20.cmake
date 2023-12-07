# Enable scanning by default for targets that explicitly use C++ 20.
cmake_policy(SET CMP0155 NEW)

# Force CMAKE_CXX_STANDARD_DEFAULT to be C++ 20.
set(ENV{CXXFLAGS} "$ENV{CXXFLAGS} ${CMAKE_CXX20_STANDARD_COMPILE_OPTION}")
enable_language(CXX)

# Hide any real scanning rule that may be available.
unset(CMAKE_CXX_SCANDEP_SOURCE)

# Create a target that does not explicitly use C++ 20 to verify it works
# without any scanning rule available.
add_executable(cmp0155-new sources/module-use.cxx)
