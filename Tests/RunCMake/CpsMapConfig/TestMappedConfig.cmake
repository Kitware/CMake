macro(set_config_map CONFIG)
  string(TOUPPER "${CONFIG}" CONFIG_UPPER)
  set(CMAKE_MAP_IMPORTED_CONFIG_${CONFIG_UPPER} "${ANIMAL_CONFIGS}")
  message(STATUS "CMAKE_MAP_IMPORTED_CONFIG_${CONFIG_UPPER} => \"${ANIMAL_CONFIGS}\"")
endmacro()

# Set up configuration maps
if(DEFINED CMAKE_BUILD_TYPE)
  set_config_map(${CMAKE_BUILD_TYPE})
else()
  foreach(CONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
    set_config_map(${CONFIG})
  endforeach()
endif()

# Find the test package
set(animal_DIR ${CMAKE_CURRENT_SOURCE_DIR})

find_package(animal REQUIRED)

# Set up the test executable
set(CMAKE_CXX_STANDARD 11)

add_executable(test test.cpp)
target_link_libraries(test animal::animal)
