add_library(MathFunctions mysqrt.cxx)

# state that anybody linking to us needs to include the current source dir
# to find MathFunctions.h, while we don't.
target_include_directories(MathFunctions
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )

# link our compiler flags interface library
target_link_libraries(MathFunctions tutorial_compiler_flags)

# TODO 1: Include CheckCXXSourceCompiles

# TODO 2: Use check_cxx_source_compiles with simple C++ code to verify
# availability of:
# * std::log
# * std::exp
# Store the results in HAVE_LOG and HAVE_EXP respectively.

# Hint: Sample C++ code which uses log:
# #include <cmath>
# int main() {
#   std::log(1.0);
#   return 0;
# }

# TODO 3: Conditionally on HAVE_LOG and HAVE_EXP, add private compile
# definitions "HAVE_LOG" and "HAVE_EXP" to the MathFunctions target.

#Hint: Use target_compile_definitions()

# install libs
set(installable_libs MathFunctions tutorial_compiler_flags)
install(TARGETS ${installable_libs} DESTINATION lib)
# install include headers
install(FILES MathFunctions.h DESTINATION include)
