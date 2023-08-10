cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0091 NEW)
project(TestFindVulkan C CXX)
include(CTest)

set(components
  glslang
  shaderc_combined
  SPIRV-Tools
  volk
  dxc
)

if(APPLE)
  list(APPEND components MoltenVK)
endif()
find_package(Vulkan REQUIRED
  COMPONENTS ${components})

add_executable(test_tgt main.c)
target_link_libraries(test_tgt Vulkan::Vulkan)
target_compile_features(test_tgt PRIVATE cxx_std_11)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${Vulkan_LIBRARIES})
target_compile_features(test_var PRIVATE cxx_std_11)
add_test(NAME test_var COMMAND test_var)

add_executable(test_tgt_dl main-dynamicVulkanLoading.cpp)
target_link_libraries(test_tgt_dl Vulkan::Headers ${CMAKE_DL_LIBS})
target_compile_features(test_tgt_dl PRIVATE cxx_std_11)
add_test(NAME test_tgt_dl COMMAND test_tgt_dl)

add_executable(test_var_dl main-dynamicVulkanLoading.cpp)
target_include_directories(test_var_dl PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(test_var_dl ${CMAKE_DL_LIBS})
target_compile_features(test_var_dl PRIVATE cxx_std_11)
add_test(NAME test_var_dl COMMAND test_var_dl)

add_executable(test_tgt_glslang main-glslang.cxx)
target_link_libraries(test_tgt_glslang Vulkan::glslang)
add_test(NAME test_tgt_glslang COMMAND test_tgt_glslang)

get_property(glslang_debug_location TARGET Vulkan::glslang PROPERTY IMPORTED_LOCATION_DEBUG)
if(NOT glslang_debug_location)
  set_property(TARGET test_tgt_glslang
    PROPERTY
      MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

add_executable(test_tgt_shaderc_combined main-shaderc_combined.cxx)
target_link_libraries(test_tgt_shaderc_combined Vulkan::shaderc_combined)
add_test(NAME test_tgt_shaderc_combined COMMAND test_tgt_shaderc_combined)

get_property(shaderc_combined_debug_location TARGET Vulkan::shaderc_combined PROPERTY IMPORTED_LOCATION_DEBUG)
if(NOT shaderc_combined_debug_location)
  set_property(TARGET test_tgt_shaderc_combined
    PROPERTY
      MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

add_executable(test_tgt_SPIRV-Tools main-SPIRV-Tools.c)
target_link_libraries(test_tgt_SPIRV-Tools Vulkan::SPIRV-Tools)
add_test(NAME test_tgt_SPIRV-Tools COMMAND test_tgt_SPIRV-Tools)

get_property(SPIRV-Tools_debug_location TARGET Vulkan::SPIRV-Tools PROPERTY IMPORTED_LOCATION_DEBUG)
if(NOT SPIRV-Tools_debug_location)
  set_property(TARGET test_tgt_SPIRV-Tools
    PROPERTY
      MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

if(APPLE)
  add_executable(test_tgt_MoltenVK main-MoltenVK.cxx)
  target_link_libraries(test_tgt_MoltenVK Vulkan::MoltenVK)
  add_test(NAME test_tgt_MoltenVK COMMAND test_tgt_MoltenVK)
endif()

add_executable(test_tgt_volk main-volk.cxx)
target_link_libraries(test_tgt_volk Vulkan::volk)
add_test(NAME test_tgt_volk COMMAND test_tgt_volk)

add_executable(test_tgt_dxc_lib main-dxc_lib.cxx)
target_link_libraries(test_tgt_dxc_lib Vulkan::dxc_lib)
add_test(NAME test_tgt_dxc_lib COMMAND test_tgt_dxc_lib)

if(Vulkan_GLSLC_EXECUTABLE)
  add_test(NAME test_glslc
    COMMAND ${CMAKE_COMMAND}
    "-DVULKAN_GLSLC_EXECUTABLE=${Vulkan_GLSLC_EXECUTABLE}"
    "-DVULKAN_GLSLC_EXECUTABLE_TARGET=$<TARGET_FILE:Vulkan::glslc>"
    -P "${CMAKE_CURRENT_LIST_DIR}/Run-glslc.cmake"
    )
endif()

if(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)
  add_test(NAME test_glslangValidator
    COMMAND ${CMAKE_COMMAND}
    "-DVULKAN_GLSLANG_VALIDATOR_EXECUTABLE=${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}"
    "-DVULKAN_GLSLANG_VALIDATOR_EXECUTABLE_TARGET=$<TARGET_FILE:Vulkan::glslangValidator>"
    -P "${CMAKE_CURRENT_LIST_DIR}/Run-glslangValidator.cmake"
    )
endif()

if(Vulkan_dxc_EXECUTABLE)
  add_test(NAME test_dxc_exe
    COMMAND ${CMAKE_COMMAND}
    "-DVULKAN_DXC_EXECUTABLE=${Vulkan_dxc_EXECUTABLE}"
    "-DVULKAN_DXC_EXECUTABLE_TARGET=$<TARGET_FILE:Vulkan::dxc_exe>"
    -P "${CMAKE_CURRENT_LIST_DIR}/Run-dxc_exe.cmake"
    )
endif()
