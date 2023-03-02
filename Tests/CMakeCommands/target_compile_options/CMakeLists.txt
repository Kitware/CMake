
cmake_minimum_required(VERSION 3.5)

if(POLICY CMP0129)
  cmake_policy(SET CMP0129 NEW)
endif()

project(target_compile_options)

add_executable(target_compile_options
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
target_compile_options(target_compile_options
  PRIVATE $<$<CXX_COMPILER_ID:AppleClang,IBMClang,Clang,GNU,LCC>:-DMY_PRIVATE_DEFINE>
  PUBLIC $<$<COMPILE_LANG_AND_ID:CXX,GNU,LCC>:-DMY_PUBLIC_DEFINE>
  PUBLIC $<$<COMPILE_LANG_AND_ID:CXX,GNU,LCC,Clang,AppleClang,IBMClang>:-DMY_MUTLI_COMP_PUBLIC_DEFINE>
  INTERFACE $<$<CXX_COMPILER_ID:GNU,LCC>:-DMY_INTERFACE_DEFINE>
  INTERFACE $<$<CXX_COMPILER_ID:GNU,LCC,Clang,AppleClang,IBMClang>:-DMY_MULTI_COMP_INTERFACE_DEFINE>
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|LCC")
  target_compile_definitions(target_compile_options
    PRIVATE
      "DO_GNU_TESTS"
  )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_definitions(target_compile_options
    PRIVATE
      "DO_CLANG_TESTS"
  )
endif()

add_executable(consumer
  "${CMAKE_CURRENT_SOURCE_DIR}/consumer.cpp"
)

target_sources(consumer PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/consumer.c"
)
target_compile_options(consumer
  PRIVATE
    -DCONSUMER_LANG_$<COMPILE_LANGUAGE>
    -DLANG_IS_CXX=$<COMPILE_LANGUAGE:CXX>
    -DLANG_IS_C=$<COMPILE_LANGUAGE:C>
)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
  target_compile_definitions(consumer
    PRIVATE TEST_LANG_DEFINES_FOR_VISUAL_STUDIO
  )
endif()

target_compile_options(consumer
  PRIVATE $<$<CXX_COMPILER_ID:GNU,LCC,Clang,AppleClang,IBMClang>:$<TARGET_PROPERTY:target_compile_options,INTERFACE_COMPILE_OPTIONS>>
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|LCC")
  target_compile_definitions(consumer
    PRIVATE
      "DO_GNU_TESTS"
  )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_definitions(consumer
    PRIVATE
      "DO_CLANG_TESTS"
  )
endif()

# Test no items
target_compile_options(consumer
  PRIVATE
)
