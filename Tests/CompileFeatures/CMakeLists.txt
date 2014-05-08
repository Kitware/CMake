
cmake_minimum_required(VERSION 3.0)

project(CompileFeatures)

if (NOT CMAKE_CXX_COMPILE_FEATURES)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp"
    "int main(int,char**) { return 0; }\n"
  )
  add_executable(CompileFeatures "${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp")
  return()
endif()

macro(run_test feature lang)
  if (";${CMAKE_${lang}_COMPILE_FEATURES};" MATCHES ${feature})
    add_library(test_${feature} OBJECT ${feature})
    set_property(TARGET test_${feature}
      PROPERTY COMPILE_FEATURES "${feature}"
    )
  else()
    list(APPEND ${lang}_non_features ${feature})
  endif()
endmacro()

get_property(cxx_features GLOBAL PROPERTY CMAKE_CXX_KNOWN_FEATURES)
foreach(feature ${cxx_features})
  run_test(${feature} CXX)
endforeach()

if (CMAKE_CXX_COMPILE_FEATURES)
  include(CheckCXXSourceCompiles)
  foreach(feature ${CXX_non_features})
    check_cxx_source_compiles("#include \"${CMAKE_CURRENT_SOURCE_DIR}/${feature}.cpp\"\nint main() { return 0; }\n" ${feature}_works)
    if (${feature}_works)
      message(SEND_ERROR
        "Feature ${feature} expected not to work for ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}.  Update the supported features or blacklist it.")
    endif()
  endforeach()
endif()

add_executable(CompileFeatures main.cpp)
set_property(TARGET CompileFeatures
  PROPERTY COMPILE_FEATURES "cxx_auto_type"
)
set_property(TARGET CompileFeatures
  PROPERTY CXX_STANDARD_REQUIRED TRUE
)

add_executable(GenexCompileFeatures main.cpp)
set_property(TARGET GenexCompileFeatures
  PROPERTY COMPILE_FEATURES "$<1:cxx_auto_type>;$<0:not_a_feature>"
)

add_library(iface INTERFACE)
set_property(TARGET iface
  PROPERTY INTERFACE_COMPILE_FEATURES "cxx_auto_type"
)
add_executable(IfaceCompileFeatures main.cpp)
target_link_libraries(IfaceCompileFeatures iface)
