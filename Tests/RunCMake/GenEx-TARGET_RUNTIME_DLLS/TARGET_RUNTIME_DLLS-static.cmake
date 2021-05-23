enable_language(C)

add_library(static STATIC static.c)
set(condition)
get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(multi_config)
  set(condition CONDITION "$<CONFIG:Debug>")
endif()
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/dlls.txt" CONTENT "$<TARGET_RUNTIME_DLLS:static>" ${condition})
