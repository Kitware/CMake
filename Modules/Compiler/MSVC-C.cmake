# MSVC has no specific options to set C language standards, but set them as
# empty strings anyways so the feature test infrastructure can at least check
# to see if they are defined.
set(CMAKE_C90_STANDARD_COMPILE_OPTION "")
set(CMAKE_C90_EXTENSION_COMPILE_OPTION "")
set(CMAKE_C99_STANDARD_COMPILE_OPTION "")
set(CMAKE_C99_EXTENSION_COMPILE_OPTION "")
set(CMAKE_C11_STANDARD_COMPILE_OPTION "")
set(CMAKE_C11_EXTENSION_COMPILE_OPTION "")

# There is no meaningful default for this
set(CMAKE_C_STANDARD_DEFAULT "")

# There are no C compiler modes so we hard-code the known compiler supported
# features. Override the default macro for this special case.  Pretend that
# all language standards are available so that at least compilation
# can be attempted.
macro(cmake_record_c_compile_features)
  list(APPEND CMAKE_C_COMPILE_FEATURES
    c_std_90
    c_std_99
    c_std_11
    c_function_prototypes
    c_variadic_macros
    )
  list(APPEND CMAKE_C90_COMPILE_FEATURES c_std_90 c_function_prototypes)
  list(APPEND CMAKE_C99_COMPILE_FEATURES c_std_99 c_variadic_macros)
  list(APPEND CMAKE_C11_COMPILE_FEATURES c_std_11)
endmacro()
