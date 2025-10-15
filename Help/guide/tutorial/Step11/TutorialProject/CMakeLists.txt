cmake_minimum_required(VERSION 3.23)

project(Tutorial
  VERSION 1.0.0
)

option(TUTORIAL_BUILD_UTILITIES "Build the Tutorial executable" ON)
option(TUTORIAL_USE_STD_SQRT "Use std::sqrt" OFF)
option(TUTORIAL_ENABLE_IPO "Check for and use IPO support" ON)
option(BUILD_TESTING "Enable testing and build tests" ON)

if(TUTORIAL_ENABLE_IPO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT result OUTPUT output)
  if(result)
    message("IPO is supported, enabling IPO")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  else()
    message(WARNING "IPO is not supported: ${output}")
  endif()
endif()

if(TUTORIAL_BUILD_UTILITIES)
  add_subdirectory(Tutorial)
endif()

if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(Tests)
endif()

add_subdirectory(MathFunctions)

include(GNUInstallDirs)

install(
  TARGETS MathFunctions OpAdd OpMul OpSub MathLogger SqrtTable
  EXPORT TutorialTargets
  FILE_SET HEADERS
)

install(
  EXPORT TutorialTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Tutorial
  NAMESPACE Tutorial::
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/TutorialConfigVersion.cmake
  COMPATIBILITY ExactVersion
)

install(
  FILES
    cmake/TutorialConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/TutorialConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Tutorial
)
