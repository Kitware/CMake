
add_library(no_variadics INTERFACE)
target_include_directories(no_variadics INTERFACE no_variadics)
add_library(have_variadics INTERFACE)
target_include_directories(have_variadics INTERFACE have_variadics)

add_library(maybe_variadics INTERFACE)
target_link_libraries(maybe_variadics INTERFACE
  $<$<HAVE_COMPILER_FEATURE:CXX,cxx_variadic_templates>:have_variadics>
  $<$<NOT:$<HAVE_COMPILER_FEATURE:CXX,cxx_variadic_templates>>:no_variadics>
)

# Upstream publishes maybe_variadics, we use it below.

add_executable(template_user_with_variadics template_user.cpp)
target_compile_features(template_user_with_variadics PRIVATE cxx_variadic_templates)

target_link_libraries(template_user_with_variadics
  maybe_variadics
)

add_executable(template_user_no_variadics template_user.cpp)

target_link_libraries(template_user_no_variadics
  maybe_variadics
)
