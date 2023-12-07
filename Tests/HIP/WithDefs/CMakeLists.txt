
cmake_minimum_required(VERSION 3.18)
project (WithDefs HIP)

set(release_compile_defs DEFREL)

#Goal for this example:
#Build an executable that needs to be passed a complex define through add_definitions.
#Verify we can pass C++ style attributes to the HIP compiler.
add_definitions("-DPACKED_DEFINE=[[gnu::packed]]")

add_executable(HIPOnlyWithDefs main.hip.cpp)
set_source_files_properties(main.hip.cpp PROPERTIES LANGUAGE HIP)

target_compile_features(HIPOnlyWithDefs PRIVATE hip_std_17)

target_compile_options(HIPOnlyWithDefs
  PRIVATE
    -DFLAG_COMPILE_LANG_$<COMPILE_LANGUAGE>
    -DFLAG_LANG_IS_HIP=$<COMPILE_LANGUAGE:HIP>
  )

target_compile_definitions(HIPOnlyWithDefs
  PRIVATE
    $<$<CONFIG:RELEASE>:$<BUILD_INTERFACE:${release_compile_defs}>>
    -DDEF_COMPILE_LANG_$<COMPILE_LANGUAGE>
    -DDEF_LANG_IS_HIP=$<COMPILE_LANGUAGE:HIP>
    -DDEF_HIP_COMPILER=$<HIP_COMPILER_ID>
    -DDEF_HIP_COMPILER_VERSION=$<HIP_COMPILER_VERSION>
  )

target_include_directories(HIPOnlyWithDefs
  PRIVATE
    $<$<COMPILE_LANGUAGE:HIP>:${CMAKE_CURRENT_SOURCE_DIR}/inc_hip>
)
