set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
add_library(foo INTERFACE)
target_compile_definitions(foo INTERFACE FOO_DEFINE)
