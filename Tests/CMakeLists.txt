# a macro for tests that have a simple format where the name matches the
# directory and project
macro(ADD_TEST_MACRO NAME)
  if(${ARGC} GREATER 1)
    set(_test_command --test-command ${ARGN})
  endif()
  string(REPLACE "." "/" dir "${NAME}")
  string(REGEX REPLACE "[^.]*\\." "" proj "${NAME}")
  add_test(NAME "${NAME}" COMMAND "${CMAKE_CTEST_COMMAND}"
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/${dir}"
    "${CMake_BINARY_DIR}/Tests/${dir}"
    --build-two-config
    ${build_generator_args}
    --build-project ${proj}
    ${${NAME}_CTEST_OPTIONS}
    --build-options
      ${${NAME}_BUILD_OPTIONS}
    ${_test_command})
  unset(_test_command)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${dir}")
endmacro()

include(${CMAKE_CURRENT_SOURCE_DIR}/CheckFortran.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/CheckSwift.cmake)

# Fake a user home directory to avoid polluting the real one.
if(NOT CTEST_NO_TEST_HOME AND (NOT WIN32 OR DEFINED ENV{HOME}))
  set(TEST_HOME "${CMake_BINARY_DIR}/Tests/CMakeFiles/TestHome")
  file(MAKE_DIRECTORY "${TEST_HOME}")
  file(WRITE "${TEST_HOME}/.cvspass" ":pserver:anoncvs@www.cmake.org:/cvsroot/KWSys A\n")
  set(TEST_HOME_ENV_CODE "# Fake a user home directory to avoid polluting the real one.
# But provide original ENV{HOME} value in ENV{CTEST_REAL_HOME} for tests that
# need access to the real HOME directory.
if(DEFINED ENV{HOME} AND NOT DEFINED ENV{CTEST_REAL_HOME})
  set(ENV{CTEST_REAL_HOME} \"\$ENV{HOME}\")
endif()
set(ENV{HOME} \"${TEST_HOME}\")
")
endif()

# Suppress generator deprecation warnings in test suite.
if(CMAKE_GENERATOR MATCHES "^Visual Studio 9 2008")
  set(TEST_WARN_VS_CODE "set(ENV{CMAKE_WARN_VS9} OFF)")
elseif(CMAKE_GENERATOR MATCHES "^Visual Studio 12 2013")
  set(TEST_WARN_VS_CODE "set(ENV{CMAKE_WARN_VS12} OFF)")
else()
  set(TEST_WARN_VS_CODE "")
endif()

# 3.9 or later provides a definitive answer to whether we are multi-config
# through a global property. Prior to 3.9, CMAKE_CONFIGURATION_TYPES being set
# is assumed to mean multi-config, but developers might modify it so it is
# technically not as reliable.
if(NOT CMAKE_VERSION VERSION_LESS 3.9)
  get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
elseif(CMAKE_CONFIGURATION_TYPES)
  set(_isMultiConfig True)
else()
  set(_isMultiConfig False)
endif()

# Choose a default configuration for CTest tests.
set(CTestTest_CONFIG Debug)
if(NOT _isMultiConfig AND CMAKE_BUILD_TYPE)
  set(CTestTest_CONFIG ${CMAKE_BUILD_TYPE})
endif()

configure_file(${CMake_SOURCE_DIR}/Tests/EnforceConfig.cmake.in
               ${CMake_BINARY_DIR}/Tests/EnforceConfig.cmake @ONLY)

# Testing
if(BUILD_TESTING)
  set(CMake_TEST_DEVENV "")
  if(CMAKE_VS_DEVENV_COMMAND)
    set(CMake_TEST_DEVENV "${CMAKE_VS_DEVENV_COMMAND}")
  elseif(CMAKE_GENERATOR MATCHES "Visual Studio 9 " AND
      NOT CMAKE_MAKE_PROGRAM MATCHES "[mM][sS][bB][uU][iI][lL][dD]\\.[eE][xX][eE]")
    set(CMake_TEST_DEVENV "${CMAKE_MAKE_PROGRAM}")
  endif()

  if(CMAKE_GENERATOR MATCHES "Visual Studio|Xcode")
    set(CMake_TEST_EXPLICIT_MAKE_PROGRAM "")
  else()
    set(CMake_TEST_EXPLICIT_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM}")
  endif()

  if(NOT CMake_TEST_EXTERNAL_CMAKE)
    if("${CMAKE_GENERATOR}" MATCHES "Unix Makefiles" OR ("${CMAKE_GENERATOR}" MATCHES Ninja AND NOT WIN32))
      set(TEST_CompileCommandOutput 1)
    endif()
  endif()

  set(MAKE_IS_GNU)
  if(CMAKE_MAKE_PROGRAM MATCHES make)
    execute_process(COMMAND ${CMAKE_MAKE_PROGRAM} no_such_target --version
      RESULT_VARIABLE res OUTPUT_VARIABLE out ERROR_VARIABLE out)
    if("${res}" STREQUAL "0")
      if("${out}" MATCHES "GNU")
        set(MAKE_IS_GNU 1)
      endif()
    endif()
  endif()

  # some old versions of make simply cannot handle spaces in paths
  if(MAKE_IS_GNU OR
      CMAKE_MAKE_PROGRAM MATCHES "nmake|gmake|wmake" OR
      CMAKE_GENERATOR MATCHES "Visual Studio|Xcode|Borland|Ninja")
    set(MAKE_SUPPORTS_SPACES 1)
  else()
    set(MAKE_SUPPORTS_SPACES 0)
  endif()

  # assume no resources building to test
  set(CMake_TEST_RESOURCES FALSE)
  # for windows and cygwin assume we have resources
  if(WIN32 OR CYGWIN)
    set(CMake_TEST_RESOURCES TRUE)
  endif()
  # For some Windows toolchains there is no resource support.
  if(WATCOM OR BORLAND OR CMAKE_C_COMPILER_ID STREQUAL "OrangeC")
    set(CMake_TEST_RESOURCES FALSE)
  endif()

  set(build_generator_args
    --build-generator ${CMAKE_GENERATOR}
    )
  if(CMAKE_GENERATOR_PLATFORM)
    list(APPEND build_generator_args
      --build-generator-platform ${CMAKE_GENERATOR_PLATFORM}
      )
  endif()
  if(CMAKE_GENERATOR_TOOLSET)
    list(APPEND build_generator_args
      --build-generator-toolset ${CMAKE_GENERATOR_TOOLSET}
      )
  endif()

  if(CMake_TEST_EXPLICIT_MAKE_PROGRAM)
    list(APPEND build_generator_args
      --build-makeprogram ${CMake_TEST_EXPLICIT_MAKE_PROGRAM}
      )
  endif()

  if(_isMultiConfig)
    set(test_options -C Debug)
  endif()

  # Look for git to use for tests.
  find_program(GIT_EXECUTABLE NAMES git)

  # Look for rpmbuild to use for tests.
  # The tool does not work with spaces in the path.
  if(NOT CMAKE_CURRENT_BINARY_DIR MATCHES " ")
    find_program(RPMBUILD_EXECUTABLE NAMES rpmbuild)
  else()
    set(RPMBUILD_EXECUTABLE "RPMBUILD_EXECUTABLE-NOTFOUND")
  endif()

  if(RPMBUILD_EXECUTABLE)
    set(CPACK_BINARY_RPM ON)
  else()
    set(CPACK_BINARY_RPM OFF)
  endif()

  # Look for dpkg to use for tests.
  find_program(DPKG_EXECUTABLE NAMES dpkg)

  if(DPKG_EXECUTABLE)
    set(CPACK_BINARY_DEB ON)
  else()
    set(CPACK_BINARY_DEB OFF)
  endif()

  # Look for NuGet to use for tests.
  find_program(NUGET_EXECUTABLE NAMES NuGet nuget)

  if(NUGET_EXECUTABLE)
    set(CPACK_BINARY_NUGET ON)
  else()
    set(CPACK_BINARY_NUGET OFF)
  endif()

  if(WIN32)
    # Macro to search for available Windows CE SDKs in the windows Registry
    macro(select_wince_sdk selected_reg selected_sdk)
      if(CMAKE_HOST_WIN32)
        execute_process(COMMAND reg QUERY "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows CE Tools\\SDKs"
                    OUTPUT_VARIABLE sdk_reg
                    ERROR_VARIABLE my_err)
        string(REGEX REPLACE "HKEY_LOCAL_MACHINE\\\\SOFTWARE\\\\Wow6432Node\\\\Microsoft\\\\Windows CE Tools\\\\SDKs\\\\"  ";" sdk_list "${sdk_reg}")
        list(LENGTH sdk_list sdk_list_len)
        if(${sdk_list_len} GREATER 1)
          list(GET sdk_list 1 _sdk) # The first entry is always empty due to the regex replace above
          string(STRIP ${_sdk} _sdk) # Make sure there is no newline in the SDK name
        endif()
        # Build a key to be used by get_filename_component that is pointing to the SDK directory
        set(_reg "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows CE Tools\\SDKs\\${_sdk}]")
        # Set return values
        set(${selected_reg} ${_reg})
        set(${selected_sdk} ${_sdk})
      endif(CMAKE_HOST_WIN32)
    endmacro(select_wince_sdk)

    set(reg_vs10 "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\10.0;InstallDir]")
    set(reg_vs11 "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\11.0;InstallDir]")
    set(reg_vs12 "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\12.0;InstallDir]")
    set(reg_vs14 "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\14.0;InstallDir]")
    set(reg_ws80 "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v8.0;InstallationFolder]")
    set(reg_ws81 "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v8.1;InstallationFolder]")
    set(reg_ws10_0 "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\14.0\\Setup\\Build Tools for Windows 10;srcPath]")
    set(reg_wp80 "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\WindowsPhone\\v8.0;InstallationFolder]")
    set(reg_wp81 "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\WindowsPhone\\v8.1;InstallationFolder]")
    select_wince_sdk(reg_wince wince_sdk)
    set(reg_tegra "[HKEY_LOCAL_MACHINE\\SOFTWARE\\NVIDIA Corporation\\Nsight Tegra;sdkRoot]")
    set(reg_nasm "[HKEY_CURRENT_USER\\SOFTWARE\\nasm]")
    foreach(reg IN ITEMS vs10 vs11 vs12 vs14 ws80 ws81 ws10_0 wp80 wp81 wince tegra nasm)
      get_filename_component(r "${reg_${reg}}" ABSOLUTE)
      if(IS_DIRECTORY "${r}" AND NOT "${r}" STREQUAL "/registry")
        set(${reg} 1)
      else()
        set(${reg} 0)
      endif()
    endforeach()
    if(CMAKE_HOST_WIN32 AND COMMAND cmake_host_system_information)
      set(info_vs15 "VS_15_DIR")
      set(info_vs16 "VS_16_DIR")
      set(info_vs17 "VS_17_DIR")
      set(vs_versions)
      if(WIN32)
        if(NOT CMAKE_VERSION VERSION_LESS 3.21.20210624)
          set(vs_versions vs15 vs16 vs17)
        elseif(NOT CMAKE_VERSION VERSION_LESS 3.14)
          set(vs_versions vs15 vs16)
        elseif(NOT CMAKE_VERSION VERSION_LESS 3.8)
          set(vs_versions vs15)
        endif()
      endif()
      foreach(info IN LISTS vs_versions)
        cmake_host_system_information(RESULT found QUERY "${info_${info}}")
        if(found)
          set(${info} 1)
        else()
          set(${info} 0)
        endif()
      endforeach()
    endif()
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND NOT DEFINED CMake_TEST_APPLE_SILICON)
    execute_process(COMMAND sysctl -q hw.optional.arm64
      OUTPUT_VARIABLE _sysctl_stdout
      ERROR_VARIABLE  _sysctl_stderr
      RESULT_VARIABLE _sysctl_result
      )
    if(_sysctl_result EQUAL 0 AND _sysctl_stdout MATCHES "hw.optional.arm64: 1")
      set(CMake_TEST_APPLE_SILICON 1)
    else()
      set(CMake_TEST_APPLE_SILICON 0)
    endif()
    unset(_sysctl_result)
    unset(_sysctl_stderr)
    unset(_sysctl_stdout)
  endif()

  #---------------------------------------------------------------------------
  # Add tests below here.

  if(NOT DEFINED CMake_TEST_Qt6)
    set(CMake_TEST_Qt6 1)
  endif()
  if(CMake_TEST_Qt6)
    find_package(Qt6 COMPONENTS Core Widgets QUIET NO_MODULE)
  endif()

  if(NOT DEFINED CMake_TEST_Qt5)
    set(CMake_TEST_Qt5 1)
  endif()
  if(CMake_TEST_Qt5)
    find_package(Qt5Widgets QUIET NO_MODULE)
  endif()

  # Collect a list of all test build directories.
  set(TEST_BUILD_DIRS)

  # Should the long tests be run?
  option(CMAKE_RUN_LONG_TESTS
    "Should the long tests be run (such as Bootstrap)." ON)
  mark_as_advanced(CMAKE_RUN_LONG_TESTS)

  if(CMAKE_RUN_LONG_TESTS)
    option(CTEST_TEST_CTEST
      "Should the tests that run a full sub ctest process be run?"
      OFF)
    mark_as_advanced(CTEST_TEST_CTEST)
  endif()

  option(CTEST_TEST_CPACK
    "Should the tests that use '--build-target package' be run?"
    ON)
  mark_as_advanced(CTEST_TEST_CPACK)
  set(CTEST_TEST_OSX_ARCH 0)
  set(CMake_TEST_XCODE_VERSION 0)
  if(APPLE)
    set(CTEST_TEST_OSX_ARCH 1)
    if(XCODE_VERSION)
      set(CMake_TEST_XCODE_VERSION "${XCODE_VERSION}")
    else()
      execute_process(
        COMMAND xcodebuild -version
        OUTPUT_VARIABLE _version ERROR_VARIABLE _version_err
        )
      if(_version MATCHES "^Xcode ([0-9]+(\\.[0-9]+)*)")
        set(CMake_TEST_XCODE_VERSION "${CMAKE_MATCH_1}")
      endif()
    endif()
    if(NOT CMake_TEST_XCODE_VERSION VERSION_LESS 10)
      # Since Xcode 10 we do not have two supported architectures for the host.
      set(CTEST_TEST_OSX_ARCH 0)
    endif()
    if(CMAKE_OSX_SYSROOT)
      execute_process(
        COMMAND xcodebuild -sdk ${CMAKE_OSX_SYSROOT} -version ProductName
        OUTPUT_VARIABLE _stdout
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE _stderr
        RESULT_VARIABLE _failed
        )
      if(NOT _failed)
        set(CMAKE_OSX_SDKPRODUCT "${_stdout}")
      endif()

      execute_process(
        COMMAND xcodebuild -sdk ${CMAKE_OSX_SYSROOT} -version SDKVersion
        OUTPUT_VARIABLE _stdout
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE _stderr
        RESULT_VARIABLE _failed
        )
      if(NOT _failed)
        set(CMAKE_OSX_SDKVERSION "${_stdout}")
      endif()
    endif()
  endif()

  if(CMake_TEST_XCODE_VERSION AND CMAKE_OSX_SDKVERSION AND CMAKE_OSX_SDKPRODUCT)
    if((NOT CMake_TEST_XCODE_VERSION VERSION_LESS 6.1) AND
      ((NOT CMAKE_OSX_SDKPRODUCT STREQUAL "Mac OS X") OR
      (NOT CMAKE_OSX_SDKVERSION VERSION_LESS 10.10)))
      if(CMAKE_GENERATOR STREQUAL "Xcode")
        set(CMake_TEST_XCODE_SWIFT 1)
      endif()
    endif()
  endif()
  if(NOT DEFINED CMake_TEST_Swift)
    if(CMAKE_Swift_COMPILER OR CMake_TEST_XCODE_SWIFT)
      set(CMake_TEST_Swift 1)
    endif()
  endif()

  if(NOT DEFINED CMake_TEST_OBJC)
    if(APPLE AND CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
      set(CMake_TEST_OBJC 1)
    endif()
  endif()

  # Use 1500 or CTEST_TEST_TIMEOUT for long test timeout value,
  # whichever is greater.
  set(CMAKE_LONG_TEST_TIMEOUT 1500)
  if(CTEST_TEST_TIMEOUT)
    set(CMAKE_LONG_TEST_TIMEOUT ${CTEST_TEST_TIMEOUT})
  endif()
  if(CMAKE_LONG_TEST_TIMEOUT LESS 1500)
    set(CMAKE_LONG_TEST_TIMEOUT 1500)
  endif()

  if(NOT CMake_TEST_EXTERNAL_CMAKE)
    add_subdirectory(CMakeLib)
  endif()
  add_subdirectory(CMakeOnly)
  add_subdirectory(RunCMake)

  add_subdirectory(FindPackageModeMakefileTest)

  add_test(NAME CMake.Copyright
    COMMAND ${CMAKE_CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/CMakeCopyright.cmake)

  # add a bunch of standard build-and-test style tests
  ADD_TEST_MACRO(CommandLineTest CommandLineTest)
  ADD_TEST_MACRO(FindPackageTest FindPackageTest)
  ADD_TEST_MACRO(StringFileTest StringFileTest)
  ADD_TEST_MACRO(TryCompile TryCompile)
  ADD_TEST_MACRO(SystemInformation SystemInformation)
  ADD_TEST_MACRO(MathTest MathTest)
  ADD_TEST_MACRO(CompileFeatures CompileFeatures)
  ADD_TEST_MACRO(CMakeCommands.target_compile_features)

  if(CMake_TEST_RESOURCES)
    ADD_TEST_MACRO(VSResource VSResource)
    if(CMAKE_GENERATOR MATCHES "Ninja")
      add_test_macro(VSResourceNinjaForceRSP VSResourceNinjaForceRSP)
    endif()
  endif()
  if(_isMultiConfig)
    set(MSManifest_CTEST_OPTIONS -C $<CONFIGURATION>)
  endif()
  ADD_TEST_MACRO(MSManifest ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>)
  ADD_TEST_MACRO(Simple Simple)
  ADD_TEST_MACRO(PreOrder PreOrder)
  ADD_TEST_MACRO(MissingSourceFile MissingSourceFile)
  set_tests_properties(MissingSourceFile PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Error at CMakeLists.txt:3 \\(add_executable\\):[ \r\n]*Cannot find source file:[ \r\n]*DoesNotExist/MissingSourceFile.c")
  if(CMake_TEST_Swift)
    ADD_TEST_MACRO(SwiftOnly SwiftOnly)
    ADD_TEST_MACRO(SwiftMixPCH SwiftMixPCH)
    if(CMake_TEST_XCODE_SWIFT)
      ADD_TEST_MACRO(SwiftMix SwiftMix)
    endif()
    if(CMAKE_Swift_COMPILER_VERSION VERSION_GREATER_EQUAL 5.1)
      ADD_TEST_MACRO(SwiftMixLib Swifty)
    endif()
  endif()
  if(CMAKE_Fortran_COMPILER)
    ADD_TEST_MACRO(FortranOnly FortranOnly)
    set_property(TEST FortranOnly APPEND PROPERTY LABELS "Fortran")
  endif()
  # test Visual Studio GNU Fortran mixing with cmake_add_fortran_subdirectory
  # run this project if we have a working fortran compiler or
  # the test is enabled with CMAKE_TEST_CMAKE_ADD_FORTRAN cache variable.
  # If you enable the test, CMake should find the MinGW fortran install,
  # or in some cases you might need to set the PATH so that cmake can find
  # the gfortran from mingw.
  if(CMAKE_Fortran_COMPILER OR CMAKE_TEST_CMAKE_ADD_FORTRAN)
    set(CMAKE_SKIP_VSGNUFortran FALSE)
    # disable test for apple builds using ifort if they are building
    # more than one architecture, as ifort does not support that.
    if(APPLE AND (CMAKE_Fortran_COMPILER MATCHES ifort))
      list(LENGTH CMAKE_OSX_ARCHITECTURES len)
      if("${len}" GREATER 1)
        message(STATUS "Skip VSGNUFortran for ifort dual cpu mac build")
        set(CMAKE_SKIP_VSGNUFortran TRUE)
      endif()
    endif()
    if(CMAKE_Fortran_COMPILER_ID STREQUAL LLVMFlang)
      # No DLLEXPORT for 'Tests/VSGNUFortran/subdir/fortran/world.f'.
      set(CMAKE_SKIP_VSGNUFortran TRUE)
    endif()
    if(CMAKE_Fortran_COMPILER_ID STREQUAL IntelLLVM)
      message(STATUS "Skip VSGNUFortran for ifx until DLLEXPORT support is implemented")
      set(CMAKE_SKIP_VSGNUFortran TRUE)
    endif()
    if((CMAKE_C_COMPILER MATCHES lsb)
        AND (CMAKE_Fortran_COMPILER MATCHES ifort))
      message(STATUS "Skip VSGNUFortran for ifort and lsb compilers")
      set(CMAKE_SKIP_VSGNUFortran TRUE)
    endif()
    if(NOT CMAKE_SKIP_VSGNUFortran)
      ADD_TEST_MACRO(VSGNUFortran ${CMAKE_CMAKE_COMMAND} -P runtest.cmake)
      set_property(TEST VSGNUFortran APPEND PROPERTY LABELS "Fortran")
    endif()
  endif()

  if(CMake_TEST_OBJC)
    add_subdirectory(ObjC)
    add_subdirectory(ObjCXX)
  endif()

  if(${CMAKE_GENERATOR} MATCHES "Visual Studio ([^9]|[9][0-9])")
    ADD_TEST_MACRO(CSharpOnly CSharpOnly)
    if(NOT CMAKE_VS_PLATFORM_NAME STREQUAL "ARM64")
      ADD_TEST_MACRO(CSharpLinkToCxx CSharpLinkToCxx)
      ADD_TEST_MACRO(CSharpLinkFromCxx CSharpLinkFromCxx)
    endif()
    ADD_TEST_MACRO(CSharpWin32GenEx CSharpWin32GenEx)
    set_tests_properties(CSharpWin32GenEx PROPERTIES
      PASS_REGULAR_EXPRESSION "Target \"CSharpWin32GenEx\" has a generator expression in its\n  WIN32_EXECUTABLE property\\.  This is not supported on managed executables\\."
      )
  endif()

  ADD_TEST_MACRO(COnly COnly)
  ADD_TEST_MACRO(CxxOnly CxxOnly)
  ADD_TEST_MACRO(CxxSubdirC CxxSubdirC)
  ADD_TEST_MACRO(OutDir runtime/OutDir)
  ADD_TEST_MACRO(OutName exe.OutName.exe)
  ADD_TEST_MACRO(ObjectLibrary UseCshared)
  ADD_TEST_MACRO(NewlineArgs NewlineArgs)
  ADD_TEST_MACRO(SetLang SetLangX)
  ADD_TEST_MACRO(EmptyProperty EmptyProperty)
  ADD_TEST_MACRO(ExternalOBJ ExternalOBJ)
  if(NOT CMake_TEST_EXTERNAL_CMAKE)
    ADD_TEST_MACRO(LoadCommand LoadedCommand)
  endif()
  ADD_TEST_MACRO(LinkDirectory bin/LinkDirectory)
  ADD_TEST_MACRO(LinkLanguage LinkLanguage)
  ADD_TEST_MACRO(LinkLine LinkLine)
  ADD_TEST_MACRO(MacroTest miniMacroTest)
  ADD_TEST_MACRO(FunctionTest miniFunctionTest)
  ADD_TEST_MACRO(ReturnTest ReturnTest)
  ADD_TEST_MACRO(Properties Properties)
  ADD_TEST_MACRO(Assembler HelloAsm)
  ADD_TEST_MACRO(SourceGroups SourceGroups)
  ADD_TEST_MACRO(Preprocess Preprocess)
  set(ExportImport_BUILD_OPTIONS -DCMake_TEST_NESTED_MAKE_PROGRAM:FILEPATH=${CMake_TEST_EXPLICIT_MAKE_PROGRAM}
                                 -DCMake_TEST_CUDA:BOOL=${CMake_TEST_CUDA}
                                 -DCMake_INSTALL_NAME_TOOL_BUG:BOOL=${CMake_INSTALL_NAME_TOOL_BUG}
                                 )
  ADD_TEST_MACRO(ExportImport ExportImport)
  set_property(TEST ExportImport APPEND
    PROPERTY LABELS "CUDA")
  ADD_TEST_MACRO(Unset Unset)
  ADD_TEST_MACRO(PolicyScope PolicyScope)
  ADD_TEST_MACRO(EmptyLibrary EmptyLibrary)
  ADD_TEST_MACRO(CompileDefinitions CompileDefinitions)

  if(_isMultiConfig)
    set(CompileOptions_CTEST_OPTIONS --build-config $<CONFIGURATION>)
  else()
    set(CompileOptions_BUILD_OPTIONS -DCMAKE_BUILD_TYPE=$<CONFIGURATION>)
  endif()
  if(CMAKE_Fortran_COMPILER)
    list(APPEND CompileOptions_BUILD_OPTIONS -DTEST_FORTRAN=1)
  endif()
  ADD_TEST_MACRO(CompileOptions CompileOptions)
  set_property(TEST CompileOptions APPEND PROPERTY LABELS "Fortran")

  ADD_TEST_MACRO(CompatibleInterface CompatibleInterface)
  ADD_TEST_MACRO(AliasTarget AliasTarget)
  ADD_TEST_MACRO(StagingPrefix StagingPrefix)
  ADD_TEST_MACRO(ImportedSameName ImportedSameName)
  ADD_TEST_MACRO(InterfaceLibrary InterfaceLibrary)
  if(NOT CMAKE_GENERATOR STREQUAL "Xcode")
    if(_isMultiConfig)
      set(ConfigSources_CTEST_OPTIONS --build-config $<CONFIGURATION>)
    else()
      set(ConfigSources_BUILD_OPTIONS -DCMAKE_BUILD_TYPE=$<CONFIGURATION>)
    endif()
    ADD_TEST_MACRO(ConfigSources ConfigSources)
  endif()
  ADD_TEST_MACRO(SourcesProperty SourcesProperty)
  ADD_TEST_MACRO(SourceFileProperty SourceFileProperty)
  if(NOT CMAKE_GENERATOR STREQUAL "Xcode")
    ADD_TEST_MACRO(SourceFileIncludeDirProperty SourceFileIncludeDirProperty)
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "LCC" OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
      AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7))
    set(runCxxDialectTest 1)
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL Clang
        AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.4 AND NOT "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
    if(NOT APPLE OR POLICY CMP0025)
      set(runCxxDialectTest 1)
    endif()
  endif()
  if(runCxxDialectTest)
    ADD_TEST_MACRO(CxxDialect CxxDialect)
  endif()
  set_tests_properties(EmptyLibrary PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Error: CMake can not determine linker language for target: test")
  ADD_TEST_MACRO(CrossCompile CrossCompile)
  set_tests_properties(CrossCompile PROPERTIES
    PASS_REGULAR_EXPRESSION "try_run.. invoked in cross-compiling mode")
  if("${CMAKE_GENERATOR}" MATCHES "Make")
    ADD_TEST_MACRO(Policy0002 Policy0002)
  endif()
  if(CTEST_TEST_OSX_ARCH)
    ADD_TEST_MACRO(Architecture Architecture)
    set_tests_properties(Architecture PROPERTIES
      PASS_REGULAR_EXPRESSION "(file is not of required architecture|does not match cputype|not the architecture being linked|but attempting to link with file built for)")
  endif()

  list(APPEND TEST_BUILD_DIRS ${CMake_TEST_INSTALL_PREFIX})

  if(NOT DEFINED CMake_TEST_Qt4)
    set(CMake_TEST_Qt4 1)
  endif()
  if(CMake_TEST_Qt4 AND NOT QT4_FOUND)
    find_package(Qt4 QUIET)
  endif()

  if(CMake_TEST_Qt4 AND QT4_FOUND)
    # test whether the Qt4 which has been found works, on some machines
    # which run nightly builds there were errors like "wrong file format"
    # for libQtCore.so. So first check it works, and only if it does add
    # the automoc test.
    include(CheckCXXSourceCompiles)
    set(_save_CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES}")
    set(_save_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")

    set(CMAKE_REQUIRED_INCLUDES ${QT_INCLUDES})
    set(CMAKE_REQUIRED_LIBRARIES ${QT_QTCORE_LIBRARIES})

    CHECK_CXX_SOURCE_COMPILES("#include <QCoreApplication>\n int main() {return (qApp == 0 ? 0 : 1); }\n"
                              QT4_WORKS)

    set(CMAKE_REQUIRED_INCLUDES "${_save_CMAKE_REQUIRED_INCLUDES}")
    set(CMAKE_REQUIRED_LIBRARIES "${_save_CMAKE_REQUIRED_LIBRARIES}")
  endif()

  # run test for BundleUtilities on supported platforms/compilers
  if(MSVC OR
     MINGW OR
     CMAKE_SYSTEM_NAME MATCHES "Linux" OR
     CMAKE_SYSTEM_NAME MATCHES "Darwin")
  if(NOT "${CMAKE_GENERATOR}" STREQUAL "Watcom WMake")

    add_test(BundleUtilities ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/BundleUtilities"
      "${CMake_BINARY_DIR}/Tests/BundleUtilities"
      ${build_generator_args}
      --build-project BundleUtilities
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BundleUtilities")

    # run test for DeployQt4 on supported platforms/compilers (which depends on BundleUtilities)
    # this test also depends on the existence of the standard qtiff plugin
    if(QT4_WORKS AND QT_QTSQL_FOUND)
      add_test(Qt4Deploy ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/Qt4Deploy"
        "${CMake_BINARY_DIR}/Tests/Qt4Deploy"
        ${build_generator_args}
        --build-project Qt4Deploy
        --build-options
          -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
          -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Qt4Deploy")
    endif()

  endif()
  endif()

  set(CMAKE_BUILD_TEST_SOURCE_DIR "${CMake_SOURCE_DIR}/Tests/COnly")
  set(CMAKE_BUILD_TEST_BINARY_DIR "${CMake_BINARY_DIR}/Tests/CMakeBuildCOnly")
  set(CMAKE_BUILD_TEST_EXE COnly)
  configure_file("${CMake_SOURCE_DIR}/Tests/CMakeBuildTest.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CMakeBuildTest.cmake" @ONLY)
  add_test(CMakeBuildTest ${CMAKE_CMAKE_COMMAND} -P
    "${CMake_BINARY_DIR}/Tests/CMakeBuildTest.cmake")
  list(APPEND TEST_BUILD_DIRS ${CMAKE_BUILD_TEST_BINARY_DIR})
  # now do it again for a project that has two project commands
  set(CMAKE_BUILD_TEST_SOURCE_DIR "${CMake_SOURCE_DIR}/Tests/DoubleProject")
  set(CMAKE_BUILD_TEST_BINARY_DIR "${CMake_BINARY_DIR}/Tests/DoubleProject")
  set(CMAKE_BUILD_TEST_EXE just_silly)
  configure_file("${CMake_SOURCE_DIR}/Tests/CMakeBuildTest.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CMakeBuildDoubleProjectTest.cmake" @ONLY)
  add_test(CMakeDoubleProject ${CMAKE_CMAKE_COMMAND} -P
    "${CMake_BINARY_DIR}/Tests/CMakeBuildDoubleProjectTest.cmake")
  list(APPEND TEST_BUILD_DIRS ${CMAKE_BUILD_TEST_BINARY_DIR})

  ADD_TEST_MACRO(Module.CheckTypeSize CheckTypeSize)

  set(Module.CheckIPOSupported-C_BUILD_OPTIONS -DCMake_TEST_IPO_WORKS_C=${CMake_TEST_IPO_WORKS_C})
  ADD_TEST_MACRO(Module.CheckIPOSupported-C CheckIPOSupported-C)

  set(Module.CheckIPOSupported-CXX_BUILD_OPTIONS -DCMake_TEST_IPO_WORKS_CXX=${CMake_TEST_IPO_WORKS_CXX})
  ADD_TEST_MACRO(Module.CheckIPOSupported-CXX CheckIPOSupported-CXX)

  if(CMake_TEST_CUDA)
    ADD_TEST_MACRO(Module.CheckIPOSupported-CUDA CheckIPOSupported-CUDA)
    set_property(TEST Module.CheckIPOSupported-CUDA APPEND PROPERTY LABELS "CUDA")
  endif()

  if(CMAKE_Fortran_COMPILER)
    set(Module.CheckIPOSupported-Fortran_BUILD_OPTIONS -DCMake_TEST_IPO_WORKS_Fortran=${CMake_TEST_IPO_WORKS_Fortran})
    ADD_TEST_MACRO(Module.CheckIPOSupported-Fortran CheckIPOSupported-Fortran)
    set_property(TEST Module.CheckIPOSupported-Fortran APPEND PROPERTY LABELS "Fortran")
  endif()

  add_test(Module.ExternalData ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Module/ExternalData"
    "${CMake_BINARY_DIR}/Tests/Module/ExternalData"
    ${build_generator_args}
    --build-project ExternalDataTest
    --build-noclean
    --force-new-ctest-process
    --build-options
      -DMAKE_SUPPORTS_SPACES=${MAKE_SUPPORTS_SPACES}
    --test-command ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE} -V
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Module/ExternalData")

  ADD_TEST_MACRO(Module.FindDependency FindDependency)

  ADD_TEST_MACRO(Module.WriteCompilerDetectionHeader WriteCompilerDetectionHeader)

  if(APPLE OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "LCC")
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-fPIE run_pic_test)
  else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "PGI"
        OR CMAKE_CXX_COMPILER_ID MATCHES "PathScale"
        OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
      set(run_pic_test 0)
    else()
      set(run_pic_test 1)
    endif()
  endif()

  if(run_pic_test)
    ADD_TEST_MACRO(PositionIndependentTargets PositionIndependentTargets)
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "LCC" OR
    ((CMAKE_CXX_COMPILER_ID MATCHES "GNU") AND
    (NOT "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 4.2) AND
    (CMAKE_SYSTEM_NAME MATCHES "Linux")))

    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(
      -fvisibility-inlines-hidden run_inlines_hidden_test)
  endif()

  if(run_inlines_hidden_test)
    add_test(Visibility ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Visibility"
      "${CMake_BINARY_DIR}/Tests/Visibility"
      ${build_generator_args}
      --build-project Visibility
    )
    list(APPEND TEST_BUILD_DIRS
      "${CMake_BINARY_DIR}/Tests/Visibility"
    )
  endif()

  add_test(LinkFlags-prepare
    ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkFlags"
    "${CMake_BINARY_DIR}/Tests/LinkFlags"
    ${build_generator_args}
    --build-project LinkFlags
    --build-target LinkFlags
    --build-options
      -DTEST_CONFIG=\${CTEST_CONFIGURATION_TYPE}
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LinkFlags")

  macro(ADD_LINK_FLAGS_TEST name depends)
    add_test(LinkFlags-${name}
      ${CMAKE_CMAKE_COMMAND} --build "${CMake_BINARY_DIR}/Tests/LinkFlags"
      --target LinkFlags_${name} --config \${CTEST_CONFIGURATION_TYPE}
      )
    set_tests_properties(LinkFlags-${name} PROPERTIES
      PASS_REGULAR_EXPRESSION "BADFLAG" DEPENDS LinkFlags-${depends})
  endmacro()
  ADD_LINK_FLAGS_TEST(lib prepare)
  ADD_LINK_FLAGS_TEST(dll lib)
  ADD_LINK_FLAGS_TEST(mod dll)
  ADD_LINK_FLAGS_TEST(exe mod)
  ADD_LINK_FLAGS_TEST(lib_config exe)
  ADD_LINK_FLAGS_TEST(dll_config lib_config)
  ADD_LINK_FLAGS_TEST(mod_config dll_config)
  ADD_LINK_FLAGS_TEST(exe_config mod_config)
  ADD_LINK_FLAGS_TEST(lib_flags exe_config)
  ADD_LINK_FLAGS_TEST(dll_flags lib_flags)
  ADD_LINK_FLAGS_TEST(mod_flags dll_flags)
  ADD_LINK_FLAGS_TEST(exe_flags mod_flags)
  ADD_LINK_FLAGS_TEST(lib_flags_config exe_flags)
  ADD_LINK_FLAGS_TEST(dll_flags_config lib_flags_config)
  ADD_LINK_FLAGS_TEST(mod_flags_config dll_flags_config)
  ADD_LINK_FLAGS_TEST(exe_flags_config mod_flags_config)

  # test for correct sub-project generation
  # not implemented in Xcode or Ninja
  if(NOT CMAKE_GENERATOR MATCHES "Xcode|Ninja")
    # run cmake and configure all of SubProject
    # but only build the independent executable car
    add_test(SubProject ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubProject"
      "${CMake_BINARY_DIR}/Tests/SubProject"
      --build-project SubProject
      ${build_generator_args}
      --build-target car
      --test-command car
      )

    # For stage 2, do not run cmake again.
    # Then build the foo sub project which should build
    # the bar library which should be referenced because
    # foo links to the static library bar, but bar is not
    # directly in the foo sub project
    if(CMake_TEST_EXPLICIT_MAKE_PROGRAM)
      set(SubProject-Stage2_BUILD_MAKEPROGRAM
        --build-makeprogram ${CMake_TEST_EXPLICIT_MAKE_PROGRAM}
        )
    endif()
    add_test(SubProject-Stage2  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubProject/foo"
      "${CMake_BINARY_DIR}/Tests/SubProject/foo"
      --build-generator ${CMAKE_GENERATOR}
      --build-generator-platform "${CMAKE_GENERATOR_PLATFORM}"
      --build-generator-toolset "${CMAKE_GENERATOR_TOOLSET}"
      ${SubProject-Stage2_BUILD_MAKEPROGRAM}
      --build-nocmake
      --build-project foo
      --build-target foo
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/SubProject/foo"
      --test-command foo
      )
    set_tests_properties(SubProject-Stage2 PROPERTIES DEPENDS SubProject)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SubProject")
  endif()

  # add tests with more complex invocations
  add_test(Framework ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Framework"
    "${CMake_BINARY_DIR}/Tests/Framework"
    --build-two-config
    ${build_generator_args}
    --build-project Framework
    --build-options
      -DMAKE_SUPPORTS_SPACES=${MAKE_SUPPORTS_SPACES}
    "-DCMAKE_INSTALL_PREFIX:PATH=${CMake_BINARY_DIR}/Tests/Framework/Install"
    --test-command bar)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Framework")

  add_test(TargetName ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TargetName"
    "${CMake_BINARY_DIR}/Tests/TargetName"
    --build-two-config
    ${build_generator_args}
    --build-project TargetName
    --test-command ${CMAKE_CMAKE_COMMAND} -E compare_files
    ${CMake_SOURCE_DIR}/Tests/TargetName/scripts/hello_world
    ${CMake_BINARY_DIR}/Tests/TargetName/scripts/hello_world)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TargetName")

  add_test(LibName ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LibName"
    "${CMake_BINARY_DIR}/Tests/LibName"
    --build-two-config
    ${build_generator_args}
    --build-project LibName
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/LibName/lib"
    --build-options
      -DMAKE_SUPPORTS_SPACES=${MAKE_SUPPORTS_SPACES}
    --test-command foobar
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LibName")

  add_test(CustComDepend ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/CustComDepend"
    "${CMake_BINARY_DIR}/Tests/CustComDepend"
    --build-two-config
    ${build_generator_args}
    --build-project CustComDepend
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/CustComDepend/bin"
    --test-command foo bar.c
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CustComDepend")

  add_test(ArgumentExpansion  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ArgumentExpansion"
    "${CMake_BINARY_DIR}/Tests/ArgumentExpansion"
    ${build_generator_args}
    --build-project ArgumentExpansion
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/ArgumentExpansion/bin"
    )
  set_tests_properties(ArgumentExpansion PROPERTIES
    FAIL_REGULAR_EXPRESSION "Unexpected: ")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ArgumentExpansion")

  add_test(GeneratorExpression
    ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GeneratorExpression"
    "${CMake_BINARY_DIR}/Tests/GeneratorExpression"
    ${build_generator_args}
    --build-project GeneratorExpression
    --build-options
      -DCMAKE_BUILD_TYPE=\${CTEST_CONFIGURATION_TYPE}
    --test-command ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE} -V
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/GeneratorExpression")

  add_test(CustomCommand  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/CustomCommand"
    "${CMake_BINARY_DIR}/Tests/CustomCommand"
    --build-two-config
    ${build_generator_args}
    --build-project CustomCommand
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/CustomCommand/bin"
    --build-options
    --test-command CustomCommand
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CustomCommand")

  ADD_TEST_MACRO(CustomCommandByproducts CustomCommandByproducts)

  ADD_TEST_MACRO(CommandLength CommandLength)

  ADD_TEST_MACRO(EmptyDepends ${CMAKE_CTEST_COMMAND})

  add_test(CustomCommandWorkingDirectory  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/CustomCommandWorkingDirectory"
    "${CMake_BINARY_DIR}/Tests/CustomCommandWorkingDirectory"
    --build-two-config
    ${build_generator_args}
    --build-project TestWorkingDir
    --test-command working
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CustomCommandWorkingDirectory")

  add_test(OutOfSource ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/OutOfSource"
    "${CMake_BINARY_DIR}/Tests/OutOfSource"
    ${build_generator_args}
    --build-project OutOfSource
    --build-two-config
    --test-command
    "${CMake_BINARY_DIR}/Tests/OutOfSource/SubDir/OutOfSourceSubdir/simple")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/OutOfSource")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/OutOfSourceDeep")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/OutOfBinary")

  add_test(BuildDepends ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/BuildDepends"
    "${CMake_BINARY_DIR}/Tests/BuildDepends"
    ${build_generator_args}
    --build-project BuildDepends
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BuildDepends")

  set(MissingInstallInstallDir
    "${CMake_BINARY_DIR}/Tests/MissingInstall/InstallDirectory")
  add_test(MissingInstall ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/MissingInstall"
    "${CMake_BINARY_DIR}/Tests/MissingInstall"
    ${build_generator_args}
    --build-project TestMissingInstall
    --build-two-config
    --build-options
    "-DCMAKE_INSTALL_PREFIX:PATH=${MissingInstallInstallDir}")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/MissingInstall")

  # By default, run the CPackComponents test if the CTEST_TEST_CPACK
  # option is ON:
  #
  set(CTEST_RUN_CPackComponents ${CTEST_TEST_CPACK})
  set(CTEST_package_X11_TEST ${CTEST_TEST_CPACK})
  set(CTEST_RUN_CPackComponentsForAll ${CTEST_TEST_CPACK})
  set(CTEST_RUN_CPackComponentsPrefix ${CTEST_TEST_CPACK})

  find_program(NSIS_MAKENSIS_EXECUTABLE NAMES makensis
    PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS]
    DOC "makensis program location"
    )

  # But on Windows, only run the CPackComponents test if the NSIS
  # installer builder is available:
  #
  if(WIN32)
    if(NSIS_MAKENSIS_EXECUTABLE)
      set(CTEST_RUN_CPackComponents ON)
    else()
      set(CTEST_RUN_CPackComponents OFF)
      set(CTEST_package_X11_TEST OFF)
    endif()
  endif()

  # On Windows run the CPackWiXGenerator test
  # if the WiX Toolset seems to be available
  if(WIN32)
    file(TO_CMAKE_PATH "$ENV{WIX}" WIX_ROOT)

    find_program(WIX_LIGHT_EXECUTABLE light
      PATHS "${WIX_ROOT}/bin"
      DOC "WiX Toolset light.exe location")

    if(WIX_LIGHT_EXECUTABLE)
      add_test(CPackWiXGenerator ${CMAKE_CTEST_COMMAND}
        -C \${CTEST_CONFIGURATION_TYPE}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/CPackWiXGenerator"
        "${CMake_BINARY_DIR}/Tests/CPackWiXGenerator"
        ${build_generator_args}
        --build-project CPackWiXGenerator
        --build-options
        --test-command ${CMAKE_CMAKE_COMMAND}
          "-DCPackWiXGenerator_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/CPackWiXGenerator"
          "-Dno_verify:BOOL=${CMake_TEST_WIX_NO_VERIFY}"
          "-Dconfig=\${CTEST_CONFIGURATION_TYPE}"
          -P "${CMake_SOURCE_DIR}/Tests/CPackWiXGenerator/RunCPackVerifyResult.cmake")

      set_property(TEST CPackWiXGenerator PROPERTY
        ATTACHED_FILES_ON_FAIL
        "${CMake_BINARY_DIR}/Tests/CPackWiXGenerator/_CPack_Packages/win32/WIX/wix.log")
    endif()
  endif()

  # On Windows run the CPackInnoSetupGenerator test
  if(WIN32 AND CMake_TEST_CPACK_INNOSETUP)
    add_test(CPackInnoSetupGenerator ${CMAKE_CTEST_COMMAND}
      -C \${CTEST_CONFIGURATION_TYPE}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackInnoSetupGenerator"
      "${CMake_BINARY_DIR}/Tests/CPackInnoSetupGenerator"
      ${build_generator_args}
      --build-project CPackInnoSetupGenerator
      --build-options
      --test-command ${CMAKE_CMAKE_COMMAND}
      "-DCPackInnoSetupGenerator_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/CPackInnoSetupGenerator"
      "-Dconfig=\${CTEST_CONFIGURATION_TYPE}"
      -P "${CMake_SOURCE_DIR}/Tests/CPackInnoSetupGenerator/RunCPackVerifyResult.cmake")

    set_property(TEST CPackInnoSetupGenerator PROPERTY
      ATTACHED_FILES_ON_FAIL
      "${CMake_BINARY_DIR}/Tests/CPackInnoSetupGenerator/_CPack_Packages/win32/INNOSETUP/ISCCOutput.log")

    set_property(TEST CPackInnoSetupGenerator PROPERTY
      ATTACHED_FILES
      "${CMake_BINARY_DIR}/Tests/CPackInnoSetupGenerator/_CPack_Packages/win32/INNOSETUP/ISScript.iss")
  endif()

  # On Windows run the CPackNSISGenerator test
  # if the nsis is available
  if(WIN32 AND NSIS_MAKENSIS_EXECUTABLE)
    add_test(CPackNSISGenerator ${CMAKE_CTEST_COMMAND}
      -C \${CTEST_CONFIGURATION_TYPE}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackNSISGenerator"
      "${CMake_BINARY_DIR}/Tests/CPackNSISGenerator"
      ${build_generator_args}
      --build-project CPackNSISGenerator
      --build-options
      --test-command ${CMAKE_CMAKE_COMMAND}
        "-DCPackNSISGenerator_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/CPackNSISGenerator"
        "-Dconfig=\${CTEST_CONFIGURATION_TYPE}"
        -P "${CMake_SOURCE_DIR}/Tests/CPackNSISGenerator/RunCPackVerifyResult.cmake")

    set_property(TEST CPackNSISGenerator PROPERTY
      ATTACHED_FILES_ON_FAIL
      "${CMake_BINARY_DIR}/Tests/CPackNSISGenerator/_CPack_Packages/win32/NSIS/NSISOutput.log")
  endif()

  if(CTEST_TEST_CPACK)
    add_test(CPackUseDefaultVersion ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackUseDefaultVersion"
      "${CMake_BINARY_DIR}/Tests/CPackUseDefaultVersion"
      ${build_generator_args}
      --build-project CPackUseDefaultVersion
      --build-two-config
      --build-options
        ${CPackUseDefaultVersion_BUILD_OPTIONS})
    set_tests_properties(CPackUseDefaultVersion PROPERTIES PASS_REGULAR_EXPRESSION "CPACK_PACKAGE_VERSION=0\\.1\\.1")
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackUseDefaultVersion")

    add_test(CPackUseProjectVersion ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackUseProjectVersion"
      "${CMake_BINARY_DIR}/Tests/CPackUseProjectVersion"
      ${build_generator_args}
      --build-project CPackUseProjectVersion
      --build-two-config
      --build-options
        ${CPackUseProjectVersion_BUILD_OPTIONS})
    set_tests_properties(CPackUseProjectVersion PROPERTIES PASS_REGULAR_EXPRESSION "CPACK_PACKAGE_VERSION=1\\.2\\.3")
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackUseProjectVersion")

    add_test(CPackUseShortProjectVersion ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackUseShortProjectVersion"
      "${CMake_BINARY_DIR}/Tests/CPackUseShortProjectVersion"
      ${build_generator_args}
      --build-project CPackUseShortProjectVersion
      --build-two-config
      --build-options
        ${CPackUseProjectVersion_BUILD_OPTIONS})
    set_tests_properties(CPackUseShortProjectVersion PROPERTIES PASS_REGULAR_EXPRESSION "CPACK_PACKAGE_VERSION=2")
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackUseShortProjectVersion")
  endif()

  if(CTEST_RUN_CPackComponents)
    set(CPackComponents_BUILD_OPTIONS)
    if(APPLE)
      set(CPackComponents_BUILD_OPTIONS -DCPACK_BINARY_DRAGNDROP:BOOL=ON)
      if(CMake_TEST_XCODE_VERSION VERSION_GREATER "4.6")
        set(CPackComponents_BUILD_OPTIONS ${CPackComponents_BUILD_OPTIONS}
          -DCPACK_BINARY_PRODUCTBUILD:BOOL=ON)
      endif()
    endif()
    if(NSIS_MAKENSIS_EXECUTABLE)
      execute_process(COMMAND ${NSIS_MAKENSIS_EXECUTABLE} "-VERSION" ERROR_QUIET OUTPUT_QUIET RESULT_VARIABLE NSIS_OK)
      if("${NSIS_OK}" STREQUAL "0")
        set(CPackComponents_BUILD_OPTIONS ${CPackComponents_BUILD_OPTIONS}
          -DCPACK_BINARY_NSIS:BOOL=ON)
      endif()
    endif()

    add_test(CPackComponents ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackComponents"
      "${CMake_BINARY_DIR}/Tests/CPackComponents"
      ${build_generator_args}
      --build-project CPackComponents
      --build-two-config
      --build-target package
      --build-options
        -DCPACK_BINARY_DEB:BOOL=${CPACK_BINARY_DEB}
        -DCPACK_BINARY_RPM:BOOL=${CPACK_BINARY_RPM}
        ${CPackComponents_BUILD_OPTIONS}
        --graphviz=CPackComponents.dot
      --test-command ${CMAKE_CMAKE_COMMAND}
        "-DCPackComponents_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/CPackComponents"
        -P "${CMake_SOURCE_DIR}/Tests/CPackComponents/VerifyResult.cmake")
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackComponents")
  endif()

  if(CTEST_RUN_CPackComponentsForAll)
    # Check whether if rpmbuild command is found
    # before adding RPM tests
    if(CPACK_BINARY_RPM)
      list(APPEND ACTIVE_CPACK_GENERATORS RPM)
    endif()
    # Check whether if dpkg command is found
    # before adding DEB tests
    if(CPACK_BINARY_DEB)
      list(APPEND ACTIVE_CPACK_GENERATORS DEB)
    endif()
    # Check whether if NuGet command is found
    # before adding NuGet tests
    if(CPACK_BINARY_NUGET)
      list(APPEND ACTIVE_CPACK_GENERATORS NUGET)
      set(CPACK_GENERATOR_STRING_NUGET NuGet)
    endif()

    # ACTIVE_CPACK_GENERATORS variable
    # now contains the list of 'active generators'
    set(CPackComponentsForAll_BUILD_OPTIONS)
    # set up list of CPack generators
    list(APPEND ACTIVE_CPACK_GENERATORS "ZIP")
    if(APPLE)
      list(APPEND ACTIVE_CPACK_GENERATORS "DragNDrop")
      if(CMake_TEST_XCODE_VERSION VERSION_GREATER "4.6")
        list(APPEND ACTIVE_CPACK_GENERATORS "productbuild")
      endif()
    endif()

    # set up list of component packaging ways
    list(APPEND CWAYLST "default")
    list(APPEND CWAYLST "OnePackPerGroup")
    list(APPEND CWAYLST "IgnoreGroup")
    list(APPEND CWAYLST "AllInOne")
    foreach(CPackGen IN LISTS ACTIVE_CPACK_GENERATORS)
      if(NOT DEFINED CPACK_GENERATOR_STRING_${CPackGen})
        set(CPACK_GENERATOR_STRING_${CPackGen} ${CPackGen})
      endif()
      set(CPackRun_CPackGen  "-DCPackGen=${CPACK_GENERATOR_STRING_${CPackGen}}")
      foreach(CPackComponentWay IN LISTS CWAYLST)
        set(CPackRun_CPackComponentWay "-DCPackComponentWay=${CPackComponentWay}")
        add_test(CPackComponentsForAll-${CPackGen}-${CPackComponentWay}
          ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
          --build-and-test
          "${CMake_SOURCE_DIR}/Tests/CPackComponentsForAll"
          "${CMake_BINARY_DIR}/Tests/CPackComponentsForAll/build${CPackGen}-${CPackComponentWay}"
          ${build_generator_args}
          --build-project CPackComponentsForAll
          --build-options
             -DCPACK_GENERATOR:STRING=${CPACK_GENERATOR_STRING_${CPackGen}}
             -DCPACK_BINARY_${CPackGen}:BOOL=ON
             ${CPackRun_CPackComponentWay}
             ${CPackComponentsForAll_BUILD_OPTIONS}
             --graphviz=CPackComponentsForAll.dot
          --test-command ${CMAKE_CMAKE_COMMAND}
            "-DCPackComponentsForAll_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/CPackComponentsForAll/build${CPackGen}-${CPackComponentWay}"
            "${CPackRun_CPackGen}"
            "${CPackRun_CPackComponentWay}"
            -P "${CMake_SOURCE_DIR}/Tests/CPackComponentsForAll/RunCPackVerifyResult.cmake")
        list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackComponentsForAll/build${CPackGen}-${CPackComponentWay}")
      endforeach()
    endforeach()

    # debian specific
    if(DPKG_EXECUTABLE)
      unset(CPackRun_CPackDEBConfiguration_ALL_CONFIGS)
      set(DEB_TEST_NAMES "CPackComponentsDEB")
      set(DEB_CONFIGURATIONS_TO_TEST "components-lintian-dpkgdeb-checks"
                                     "components-description1"
                                     "components-description2"
                                     "components-source"
                                     "components-shlibdeps1"
                                     "components-depend1"
                                     "compression")
      # Run additional tests if dpkg-shlibdeps is available (and is new enough version)
      find_program(SHLIBDEPS_EXECUTABLE NAMES dpkg-shlibdeps)
      if(SHLIBDEPS_EXECUTABLE)
        # Check version of the dpkg-shlibdeps tool
        execute_process(COMMAND ${CMAKE_COMMAND} -E env LC_ALL=C ${SHLIBDEPS_EXECUTABLE} --version
          OUTPUT_VARIABLE _TMP_VERSION
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(_TMP_VERSION MATCHES "dpkg-shlibdeps version ([0-9]+\\.[0-9]+\\.[0-9]+)")
          set(SHLIBDEPS_EXECUTABLE_VERSION "${CMAKE_MATCH_1}")
        else()
          unset(SHLIBDEPS_EXECUTABLE_VERSION)
        endif()
        if(NOT SHLIBDEPS_EXECUTABLE_VERSION VERSION_LESS 1.19 OR
          (NOT SHLIBDEPS_EXECUTABLE_VERSION VERSION_LESS 1.17 AND NOT CMAKE_BINARY_DIR MATCHES ".*[ ].*"))
            list(APPEND DEB_CONFIGURATIONS_TO_TEST "shlibdeps-with-private-lib-failure"
                                                   "shlibdeps-with-private-lib-success")
        endif()
        # Check if distro has symbols or shlibs data
        file(GLOB SHLIBS_FILES_EXIST "/var/lib/dpkg/info/*.shlibs" "/var/lib/dpkg/info/*.symbols")
        if(SHLIBS_FILES_EXIST)
          list(APPEND DEB_CONFIGURATIONS_TO_TEST "components-depend2")
        endif()
      endif()

      set(CPackGen "DEB")
      set(CPackRun_CPackGen "-DCPackGen=${CPackGen}")

      foreach(CPackDEBConfiguration IN LISTS DEB_CONFIGURATIONS_TO_TEST)
        set(CPackRun_CPackDEBConfiguration "-DCPackDEBConfiguration=${CPackDEBConfiguration}")
        add_test(NAME ${DEB_TEST_NAMES}-${CPackDEBConfiguration} COMMAND
          ${CMAKE_CTEST_COMMAND} -C $<CONFIG>
          --build-and-test
              "${CMake_SOURCE_DIR}/Tests/${DEB_TEST_NAMES}"
              "${CMake_BINARY_DIR}/Tests/${DEB_TEST_NAMES}/build${CPackGen}-${CPackDEBConfiguration}"
              ${build_generator_args}
          --build-project CPackComponentsDEB
          --build-options
              -DCPACK_GENERATOR:STRING=${CPackGen}
              -DCPACK_BINARY_${CPackGen}:BOOL=ON
              ${CPackRun_CPackDEBConfiguration}
              ${CPackRun_CPackDEBConfiguration_ALL_CONFIGS}
          --graphviz=${DEB_TEST_NAMES}.dot
          --test-command ${CMAKE_CMAKE_COMMAND}
              "-D${DEB_TEST_NAMES}_SOURCE_DIR:PATH=${CMake_SOURCE_DIR}/Tests/${DEB_TEST_NAMES}"
              "-D${DEB_TEST_NAMES}_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/${DEB_TEST_NAMES}/build${CPackGen}-${CPackDEBConfiguration}"
              "${CPackRun_CPackGen}"
              "${CPackRun_CPackDEBConfiguration}"
              "-DCONFIG=$<CONFIG>"
              -P "${CMake_SOURCE_DIR}/Tests/${DEB_TEST_NAMES}/RunCPackVerifyResult-${CPackDEBConfiguration}.cmake")
          list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${DEB_TEST_NAMES}/build${CPackGen}-${CPackDEBConfiguration}")
      endforeach()
    endif()

  endif()

  # By default, turn this test off (because it takes a long time...)
  #
  if(NOT DEFINED CTEST_RUN_CPackTestAllGenerators)
    set(CTEST_RUN_CPackTestAllGenerators OFF)

    # ...but: if it appears to be a coverage dashboard, or long tests are
    # on, then set it to the generic CTEST_TEST_CPACK setting.
    #
    if(CMAKE_CXX_FLAGS MATCHES "-ftest-coverage" OR
      NOT "$ENV{COVFILE}" STREQUAL "" OR
      CMAKE_RUN_LONG_TESTS)
      set(CTEST_RUN_CPackTestAllGenerators ${CTEST_TEST_CPACK})
    endif()
  endif()

  if(CTEST_RUN_CPackTestAllGenerators)
    add_test(CPackTestAllGenerators ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackTestAllGenerators"
      "${CMake_BINARY_DIR}/Tests/CPackTestAllGenerators"
      ${build_generator_args}
      --build-project CPackTestAllGenerators
      --test-command
      ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CPackTestAllGenerators
        -P ${CMake_SOURCE_DIR}/Tests/CPackTestAllGenerators/RunCPack.cmake
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackTestAllGenerators")
  endif()

  if(CTEST_RUN_CPackComponentsPrefix)
    set(CPackComponents_BUILD_OPTIONS)
    if(APPLE)
      set(CPackComponents_BUILD_OPTIONS -DCPACK_BINARY_DRAGNDROP:BOOL=ON)
      if(CMake_TEST_XCODE_VERSION VERSION_GREATER "4.6")
        set(CPackComponents_BUILD_OPTIONS ${CPackComponents_BUILD_OPTIONS}
          -DCPACK_BINARY_PRODUCTBUILD:BOOL=ON)
      endif()
    endif()
    if(NOT NSIS_MAKENSIS_EXECUTABLE)
      set(CPackComponents_BUILD_OPTIONS ${CPackComponents_BUILD_OPTIONS}
        -DCPACK_BINARY_NSIS:BOOL=OFF)
    endif()

    add_test(CPackComponentsPrefix ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackComponentsPrefix"
      "${CMake_BINARY_DIR}/Tests/CPackComponentsPrefix"
      ${build_generator_args}
      --build-project CPackComponentsPrefix
      --build-two-config
      --build-target package
      --build-options
        -DCPACK_BINARY_DEB:BOOL=${CPACK_BINARY_DEB}
        -DCPACK_BINARY_RPM:BOOL=${CPACK_BINARY_RPM}
        -DCPACK_BINARY_ZIP:BOOL=ON
        ${CPackComponents_BUILD_OPTIONS}
        )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackComponentsPrefix")
  endif()

  if(CTEST_package_X11_TEST)
    set(X11_build_target_arg --build-target package)
  else()
    set(X11_build_target_arg)
  endif()

  add_test(X11 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/X11"
    "${CMake_BINARY_DIR}/Tests/X11"
    ${build_generator_args}
    --build-project UseX11
    --build-two-config
    ${X11_build_target_arg}
    --test-command  UseX11)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/X11")

  if(NOT DEFINED CTEST_RUN_CMakeTestAllGenerators)
    set(CTEST_RUN_CMakeTestAllGenerators ON)
  endif()

  if(CTEST_RUN_CMakeTestAllGenerators)
    add_test(CMakeTestAllGenerators ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CMakeTestAllGenerators
        -D CMake_SOURCE_DIR=${CMake_SOURCE_DIR}
        -P ${CMake_SOURCE_DIR}/Tests/CMakeTestAllGenerators/RunCMake.cmake
      )
    list(APPEND TEST_BUILD_DIRS
      "${CMake_BINARY_DIR}/Tests/CMakeTestAllGenerators")
    # This test runs a lot of processes.  Do not make them compete
    # for resources with other tests.
    set_property(TEST CMakeTestAllGenerators PROPERTY RUN_SERIAL 1)
  endif()

  if(NOT CMake_TEST_EXTERNAL_CMAKE)
    add_test(LoadedCommandOneConfig  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/LoadCommandOneConfig"
      "${CMake_BINARY_DIR}/Tests/LoadCommandOneConfig"
      ${build_generator_args}
      --build-project LoadCommand
      --test-command  LoadedCommand
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LoadCommandOneConfig")
  endif()

  add_test(complex  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Complex"
    "${CMake_BINARY_DIR}/Tests/Complex"
    --build-two-config
    --build-config-sample "${CMAKE_CTEST_COMMAND}"
    ${build_generator_args}
    --build-project Complex
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Complex/bin"
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    --test-command complex
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Complex")

  add_test(complexOneConfig  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ComplexOneConfig"
    "${CMake_BINARY_DIR}/Tests/ComplexOneConfig"
    ${build_generator_args}
    --build-project Complex
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/ComplexOneConfig/bin"
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    --test-command complex)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ComplexOneConfig")
  # because of the registry write these tests depend on each other
  set_tests_properties(complex PROPERTIES DEPENDS complexOneConfig)

  add_test(Environment ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Environment"
    "${CMake_BINARY_DIR}/Tests/Environment"
    ${build_generator_args}
    --build-project EnvironmentProj
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Environment"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Environment")
  set_property(TEST Environment APPEND
    PROPERTY ENVIRONMENT
      "SET_FROM_AMBIENT_unset=base"
      "SET_FROM_AMBIENT_replace=base"
      "SET_FROM_AMBIENT_string=base"
      "SET_FROM_AMBIENT_path=base"
      "SET_FROM_AMBIENT_list=base")

  add_test(QtAutomocNoQt  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/QtAutomocNoQt"
    "${CMake_BINARY_DIR}/Tests/QtAutomocNoQt"
    ${build_generator_args}
    --build-project QtAutomocNoQt
    --build-options
      -DCMAKE_BUILD_TYPE=\${CTEST_CONFIGURATION_TYPE}
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/QtAutomocNoQt")

  if(CMake_TEST_Qt6 AND Qt6Widgets_FOUND)
    add_subdirectory(Qt6Autogen)
  endif()
  if(CMake_TEST_Qt5 AND Qt5Widgets_FOUND)
    add_subdirectory(Qt5Autogen)
  endif()
  if(QT4_WORKS AND QT_QTGUI_FOUND)
    add_subdirectory(Qt4Autogen)

    add_test(Qt4Targets ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Qt4Targets"
      "${CMake_BINARY_DIR}/Tests/Qt4Targets"
      ${build_generator_args}
      --build-project Qt4Targets
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Qt4Targets"
      --force-new-ctest-process
      --build-options
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
      --test-command ${CMAKE_CTEST_COMMAND} -V
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Qt4Targets")

    if(Qt5Widgets_FOUND AND NOT Qt5Widgets_VERSION VERSION_LESS 5.1.0)
      add_test(Qt4And5AutomocForward ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/Qt4And5Automoc"
        "${CMake_BINARY_DIR}/Tests/Qt4And5AutomocForward"
        ${build_generator_args}
        --build-project Qt4And5Automoc
        --build-exe-dir "${CMake_BINARY_DIR}/Tests/Qt4And5AutomocForward"
        --force-new-ctest-process
        --test-command ${CMAKE_CTEST_COMMAND} -V
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Qt4And5AutomocForward")
      add_test(Qt4And5AutomocReverse ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/Qt4And5Automoc"
        "${CMake_BINARY_DIR}/Tests/Qt4And5AutomocReverse"
        ${build_generator_args}
        --build-project Qt4And5Automoc
        --build-exe-dir "${CMake_BINARY_DIR}/Tests/Qt4And5AutomocReverse"
        --force-new-ctest-process
        --build-options -DQT_REVERSE_FIND_ORDER=1
        --test-command ${CMAKE_CTEST_COMMAND} -V
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Qt4And5AutomocReverse")
    endif()
  endif()

  # test for Find modules, simple cases
  foreach(
  _mod
  IN ITEMS
      ALSA
      BLAS
      Boost
      BZip2
      Cups
      CURL
      DevIL
      Doxygen
      EnvModules
      EXPAT
      Fontconfig
      Freetype
      GDAL
      GIF
      Git
      GLEW
      GLUT
      GnuTLS
      GSL
      GTK2
      HDF5
      Iconv
      ICU
      ImageMagick
      Intl
      Jasper
      JNI
      JPEG
      JsonCpp
      LAPACK
      LibArchive
      Libinput
      LibLZMA
      LibRHash
      LibUV
      LibXml2
      LibXslt
      LTTngUST
      MPI
      ODBC
      OpenACC
      OpenAL
      OpenCL
      OpenGL
      OpenMP
      OpenSP
      OpenSSL
      Patch
      PNG
      PostgreSQL
      Protobuf
      SDL
      SQLite3
      TIFF
      Vulkan
      wxWidgets
      X11
      XalanC
      XercesC
    )
    if(CMake_TEST_Find${_mod})
      add_subdirectory(Find${_mod})
    endif()
  endforeach()

  if(CMake_TEST_CUDA)
    add_subdirectory(Cuda)
    add_subdirectory(CudaOnly)
  endif()

  if(CMake_TEST_HIP)
    add_subdirectory(HIP)
  endif()

  if(CMake_TEST_ISPC)
    add_subdirectory(ISPC)
  endif()

  if(CMake_TEST_FindGTest)
    add_subdirectory(FindGTest)
    add_subdirectory(GoogleTest)
  endif()

  if(CMake_TEST_FindPython)
    set(CMake_TEST_FindPython2 TRUE)
    set(CMake_TEST_FindPython3 TRUE)
  endif()
  if(CMake_TEST_FindPython_SABIMOdule)
    set(CMake_TEST_FindPython2_SABIModule TRUE)
    set(CMake_TEST_FindPython3_SABIModule TRUE)
  endif()
  if(CMake_TEST_FindPython_NumPy)
    set(CMake_TEST_FindPython2_NumPyy TRUE)
    set(CMake_TEST_FindPython3_NumPy TRUE)
  endif()
  if(CMake_TEST_FindPython_Conda)
    set(CMake_TEST_FindPython3_Conda TRUE)
  endif()
  if(CMake_TEST_FindPython_IronPython)
    set(CMake_TEST_FindPython2_IronPython TRUE)
    set(CMake_TEST_FindPython3_IronPython TRUE)
  endif()
  if(CMake_TEST_FindPython_PyPy)
    set(CMake_TEST_FindPython2_PyPy TRUE)
    set(CMake_TEST_FindPython3_PyPy TRUE)
  endif()
  if(CMake_TEST_FindPython2 OR CMake_TEST_FindPython3
      OR CMake_TEST_FindPython2_SABIModule OR CMake_TEST_FindPython3_SABIModule
      OR CMake_TEST_FindPython2_NumPy OR CMake_TEST_FindPython3_NumPy
      OR CMake_TEST_FindPython3_Conda
      OR CMake_TEST_FindPython2_IronPython OR CMake_TEST_FindPython3_IronPython
      OR CMake_TEST_FindPython2_PyPy OR CMake_TEST_FindPython3_PyPy)
    if (CMake_TEST_FindPython2 AND CMAKE_SYSTEM_NAME MATCHES "Linux|Darwin")
      set(CMake_TEST_FindPython2_SABIModule TRUE)
    endif()
    if (CMake_TEST_FindPython3 AND CMAKE_SYSTEM_NAME MATCHES "Linux|Darwin")
      set(CMake_TEST_FindPython3_SABIModule TRUE)
    endif()
    add_subdirectory(FindPython)
  endif()

  if(CMake_TEST_UseSWIG)
    add_subdirectory(UseSWIG)
  endif()

  if(CMake_TEST_FindRuby)
    add_subdirectory(FindRuby)
  endif()

  add_subdirectory(FindThreads)

  # Matlab module
  # CMake_TEST_FindMatlab: indicates to look for Matlab (from PATH for Linux)
  # CMake_TEST_FindMatlab_ROOT_DIR: indicates an optional root directory for Matlab, allows to select a version.
  # CMake_TEST_FindMatlab_MCR: indicates the MCR is installed
  # CMake_TEST_FindMatlab_MCR_ROOT_DIR: indicates an optional root directory for the MCR, required on Linux
  if(CMake_TEST_FindMatlab OR CMake_TEST_FindMatlab_ROOT_DIR OR
     CMake_TEST_FindMatlab_MCR OR CMake_TEST_FindMatlab_MCR_ROOT_DIR)
    set(FindMatlab_additional_test_options)
    if(CMake_TEST_FindMatlab_MCR OR CMake_TEST_FindMatlab_MCR_ROOT_DIR)
      set(FindMatlab_additional_test_options -DIS_MCR=TRUE)
    endif()
    if(CMake_TEST_FindMatlab_ROOT_DIR)
      set(FindMatlab_additional_test_options ${FindMatlab_additional_test_options} "-DMatlab_ROOT_DIR=${CMake_TEST_FindMatlab_ROOT_DIR}")
    endif()
    if(CMake_TEST_FindMatlab_MCR_ROOT_DIR)
      set(FindMatlab_additional_test_options ${FindMatlab_additional_test_options} "-DMCR_ROOT:FILEPATH=${CMake_TEST_FindMatlab_MCR_ROOT_DIR}")
    endif()
    set(FindMatlab.basic_checks_BUILD_OPTIONS ${FindMatlab_additional_test_options})
    ADD_TEST_MACRO(FindMatlab.basic_checks      ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>)
    set_property(TEST FindMatlab.basic_checks APPEND PROPERTY LABELS "Matlab")
    set(FindMatlab.versions_checks_BUILD_OPTIONS ${FindMatlab_additional_test_options})
    ADD_TEST_MACRO(FindMatlab.versions_checks   ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>)
    set_property(TEST FindMatlab.versions_checks APPEND PROPERTY LABELS "Matlab")
    set(FindMatlab.components_checks_BUILD_OPTIONS ${FindMatlab_additional_test_options})
    ADD_TEST_MACRO(FindMatlab.components_checks ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>)
    set_property(TEST FindMatlab.components_checks APPEND PROPERTY LABELS "Matlab")
    set(FindMatlab.failure_reports_BUILD_OPTIONS ${FindMatlab_additional_test_options})
    ADD_TEST_MACRO(FindMatlab.failure_reports   ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>)
    set_property(TEST FindMatlab.failure_reports APPEND PROPERTY LABELS "Matlab")
    set(FindMatlab.r2018a_check_BUILD_OPTIONS ${FindMatlab_additional_test_options})
    ADD_TEST_MACRO(FindMatlab.r2018a_check   ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>)
    set_property(TEST FindMatlab.r2018a_check APPEND PROPERTY LABELS "Matlab")
    set(FindMatlab.targets_checks_BUILD_OPTIONS ${FindMatlab_additional_test_options})
    ADD_TEST_MACRO(FindMatlab.targets_checks      ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>)
    set_property(TEST FindMatlab.targets_checks APPEND PROPERTY LABELS "Matlab")
    set(FindMatlab.no_implicit_link_checks_BUILD_OPTIONS ${FindMatlab_additional_test_options})
    ADD_TEST_MACRO(FindMatlab.no_implicit_link_checks      ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>)
    set_property(TEST FindMatlab.no_implicit_link_checks APPEND PROPERTY LABELS "Matlab")
  endif()

  set(ExternalProject_BUILD_OPTIONS "")
  foreach(vcs IN ITEMS CVS SVN GIT HG)
    if(DEFINED CMake_TEST_ExternalProject_${vcs})
      list(APPEND ExternalProject_BUILD_OPTIONS -DEP_TEST_${vcs}=${CMake_TEST_ExternalProject_${vcs}})
    endif()
  endforeach()
  add_test(ExternalProject ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ExternalProject"
    "${CMake_BINARY_DIR}/Tests/ExternalProject"
    ${build_generator_args}
    --build-project ExternalProjectTest
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/ExternalProject"
    --force-new-ctest-process
    --build-options ${ExternalProject_BUILD_OPTIONS}
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ExternalProject")
  set_tests_properties(ExternalProject PROPERTIES
    ENVIRONMENT GIT_ALLOW_PROTOCOL=file
    RUN_SERIAL 1
    TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})

  add_test(NAME ExternalProjectSubdir
    COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ExternalProjectSubdir"
    "${CMake_BINARY_DIR}/Tests/ExternalProjectSubdir"
    ${build_generator_args}
    --build-project ExternalProjectSubdir
    --force-new-ctest-process
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ExternalProjectSubdir")

  add_test(NAME ExternalProjectSourceSubdir
    COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ExternalProjectSourceSubdir"
    "${CMake_BINARY_DIR}/Tests/ExternalProjectSourceSubdir"
    ${build_generator_args}
    --build-project ExternalProjectSourceSubdir
    --force-new-ctest-process
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ExternalProjectSourceSubdir")

  add_test(NAME ExternalProjectSourceSubdirNotCMake
    COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ExternalProjectSourceSubdirNotCMake"
    "${CMake_BINARY_DIR}/Tests/ExternalProjectSourceSubdirNotCMake"
    ${build_generator_args}
    --build-project ExternalProjectSourceSubdirNotCMake
    --force-new-ctest-process
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ExternalProjectSourceSubdirNotCMake")

  add_test(ExternalProjectLocal ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ExternalProjectLocal"
    "${CMake_BINARY_DIR}/Tests/ExternalProjectLocal"
    ${build_generator_args}
    --build-project ExternalProjectLocalTest
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/ExternalProjectLocal"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ExternalProjectLocal")
  set_tests_properties(ExternalProjectLocal PROPERTIES
    RUN_SERIAL 1
    TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})

  add_test(ExternalProjectUpdateSetup ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ExternalProjectUpdate"
    "${CMake_BINARY_DIR}/Tests/ExternalProjectUpdate"
    ${build_generator_args}
    --build-project ExternalProjectUpdateTest
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/ExternalProjectUpdate"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ExternalProjectUpdate")
  set_tests_properties(ExternalProjectUpdateSetup PROPERTIES
    RUN_SERIAL 1
    TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})

  add_test(NAME ExternalProjectUpdate
    COMMAND ${CMAKE_CMAKE_COMMAND}
    -DExternalProjectUpdate_SOURCE_DIR:PATH=${CMake_SOURCE_DIR}/Tests/ExternalProjectUpdate
    -DExternalProjectUpdate_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/ExternalProjectUpdate
    -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
    -DCMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
    -DCMAKE_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}
    -DCMAKE_CTEST_COMMAND=${CMAKE_CTEST_COMMAND}
    -P ${CMake_SOURCE_DIR}/Tests/ExternalProjectUpdate/ExternalProjectUpdateTest.cmake
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ExternalProjectUpdate")
  set_tests_properties(ExternalProjectUpdate PROPERTIES
    RUN_SERIAL 1
    TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT}
    WORKING_DIRECTORY ${CMake_SOURCE_DIR}/Tests/ExternalProjectUpdate
    DEPENDS ExternalProjectUpdateSetup
    )

  execute_process(
    COMMAND ${CMAKE_COMMAND}
    "-E" create_symlink
    "${CMake_SOURCE_DIR}/Tests/CMakeLists.txt"        # random source file that exists
    "${CMake_BINARY_DIR}/Tests/try_to_create_symlink" # random target file in existing directory
    RESULT_VARIABLE _symlink_result
    OUTPUT_VARIABLE _symlink_stdout
    ERROR_VARIABLE _symlink_stderr
    )
  if(_symlink_result EQUAL 0)
    file(REMOVE "${CMake_BINARY_DIR}/Tests/try_to_create_symlink")
    function(add_installmode_test _mode)
      set(ENV{CMAKE_INSTALL_MODE} _mode)
      set(_maybe_InstallMode_CTEST_OPTIONS)
      set(_maybe_BUILD_OPTIONS)
      if(_isMultiConfig)
        set(_maybe_CTEST_OPTIONS -C $<CONFIGURATION>)
      else()
        set(_maybe_BUILD_OPTIONS "-DCMAKE_BUILD_TYPE=$<CONFIGURATION>")
      endif()
      add_test(
        NAME "InstallMode-${_mode}"
        COMMAND
          ${CMAKE_CTEST_COMMAND} -V ${_maybe_CTEST_OPTIONS}
        --build-and-test
          "${CMake_SOURCE_DIR}/Tests/InstallMode"
          "${CMake_BINARY_DIR}/Tests/InstallMode-${_mode}"
        ${build_generator_args}
        --build-project superpro
        --build-exe-dir "${CMake_BINARY_DIR}/Tests/InstallMode-${_mode}"
        --force-new-ctest-process
        --build-options
          ${_maybe_BUILD_OPTIONS}
          "-DCMAKE_INSTALL_PREFIX:PATH=${CMake_BINARY_DIR}/Tests/InstallMode-${_mode}/install"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/InstallMode-${_mode}")
      unset(ENV{CMAKE_INSTALL_MODE})
    endfunction()

    add_installmode_test(COPY)
    add_installmode_test(REL_SYMLINK)
    add_installmode_test(REL_SYMLINK_OR_COPY)
    add_installmode_test(ABS_SYMLINK)
    add_installmode_test(ABS_SYMLINK_OR_COPY)
    add_installmode_test(SYMLINK)
    add_installmode_test(SYMLINK_OR_COPY)
  endif()


  # do each of the tutorial steps
  function(add_tutorial_test step_name use_mymath tutorial_arg pass_regex)
    set(tutorial_test_name Tutorial${step_name})
    set(tutorial_build_dir "${CMake_BINARY_DIR}/Tests/Tutorial/${step_name}")
    if(use_mymath)
      set(tutorial_build_options "")
    else()
      set(tutorial_test_name ${tutorial_test_name}_MYMATH)
      set(tutorial_build_dir "${tutorial_build_dir}_MYMATH")
      set(tutorial_build_options -DUSE_MYMATH:BOOL=OFF)
    endif()
    add_test(${tutorial_test_name} ${CMAKE_CTEST_COMMAND}
      -C "Release"
      --build-and-test
      "${CMake_SOURCE_DIR}/Help/guide/tutorial/${step_name}"
      ${tutorial_build_dir}_Build
      ${build_generator_args}
      --build-project Tutorial
      --build-options ${tutorial_build_options}
      --test-command Tutorial ${tutorial_arg})
    set_tests_properties(${tutorial_test_name} PROPERTIES
      PASS_REGULAR_EXPRESSION ${pass_regex})

    list(APPEND TEST_BUILD_DIRS "${tutorial_build_dir}_Build")
  endfunction()

  if(NOT CMake_TEST_EXTERNAL_CMAKE)
    foreach(STP RANGE 2 12)
      if(STP EQUAL 8)
        set(pass_regex ".*using log and exp")
      else()
        set(pass_regex "The square root of 25 is 5")
      endif()
      add_tutorial_test(Step${STP} TRUE 25 ${pass_regex})
    endforeach()
    set(pass_regex "The square root of 25 is 5")
    add_tutorial_test(Complete TRUE 25 ${pass_regex})
    foreach(STP RANGE 3 12)
      add_tutorial_test(Step${STP} FALSE 25 ${pass_regex})
    endforeach()
    add_tutorial_test(Complete FALSE 25 ${pass_regex})
  endif()

  function(add_importexport_test export_name import_name)
    set(install_dir
      "${CMake_BINARY_DIR}/Tests/ImportExport/Install${export_name}")
    set(export_build_dir "${CMake_BINARY_DIR}/Tests/ImportExport/${export_name}Build")
    set(export_test_name "Guide.ImportExport.${export_name}")
    add_test(${export_test_name} ${CMAKE_CTEST_COMMAND}
      -C "Release"
      --build-and-test
      "${CMake_SOURCE_DIR}/Help/guide/importing-exporting/${export_name}"
      "${export_build_dir}"
      ${build_generator_args}
      --build-project ${export_name}
      --build-target install
      --build-options
      "-DCMAKE_INSTALL_PREFIX:PATH=${install_dir}")
    list(APPEND TEST_BUILD_DIRS "${export_build_dir}")

    set(import_build_dir "${CMake_BINARY_DIR}/Tests/ImportExport/${import_name}Build")
    set(import_test_name "Guide.ImportExport.${import_name}")
    add_test(${import_test_name} ${CMAKE_CTEST_COMMAND}
      -C "Release"
      --build-and-test
      "${CMake_SOURCE_DIR}/Help/guide/importing-exporting/${import_name}"
      "${import_build_dir}"
      ${build_generator_args}
      --build-project ${import_name}
      --build-options
      "-DCMAKE_PREFIX_PATH:PATH=${install_dir}")
    set_tests_properties(${import_test_name} PROPERTIES DEPENDS ${export_test_name})
    list(APPEND TEST_BUILD_DIRS "${import_build_dir}")
  endfunction()

  if(NOT CMake_TEST_EXTERNAL_CMAKE)
    add_importexport_test("MyExe" "Importing")
    add_importexport_test("MathFunctions" "Downstream")
    add_importexport_test("MathFunctionsComponents" "DownstreamComponents")
  endif()

  add_test(testing ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Testing"
    "${CMake_BINARY_DIR}/Tests/Testing"
    ${build_generator_args}
    --build-project Testing
    --test-command ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    )
  set_tests_properties(testing PROPERTIES PASS_REGULAR_EXPRESSION "Passed")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Testing")

  add_test(wrapping  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Wrapping"
    "${CMake_BINARY_DIR}/Tests/Wrapping"
    ${build_generator_args}
    --build-project Wrapping
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --test-command wrapping
    )
  add_test(qtwrapping  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Wrapping"
    "${CMake_BINARY_DIR}/Tests/Wrapping"
    ${build_generator_args}
    --build-project Wrapping
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
      --test-command qtwrapping
      )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Wrapping")

  add_test(testdriver1 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver1"
    ${build_generator_args}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest test1
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TestDriver1")

  add_test(testdriver2 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver2"
    ${build_generator_args}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest test2
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TestDriver2")

  add_test(testdriver3  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver3"
    ${build_generator_args}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest subdir/test3
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TestDriver3")

  add_test(testdriver4  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver4"
    ${build_generator_args}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest -A test2
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TestDriver4")

  add_test(testdriver5  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver5"
    ${build_generator_args}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest -A test2
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TestDriver5")
  set_tests_properties(testdriver5 PROPERTIES
    PASS_REGULAR_EXPRESSION
    "TAP version 13\n1\\.\\.3.+ok 1 test1 # [0-9]+\\.[0-9]+.*All tests finished."
    )

  add_test(Dependency ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/Exec"
    ${build_generator_args}
    --build-project Dependency
    --test-command exec
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Dependency")

  if(CMAKE_SYSTEM_NAME MATCHES syllable)

# RPATH isn't supported under Syllable, so the tests don't
# find their libraries. In order to fix that LIBRARY_OUTPUT_DIR
# in the tests would have to be adjusted to ${EXECUTABLE_OUTPUT_DIR}/lib .
# For now we just require on Syllable that the user adjusts the DLL_PATH
# environment variable, so except the two tests below all other tests will succeed.

    set(_DLL_PATH "$ENV{DLL_PATH}")
    if(NOT "${_DLL_PATH}" MATCHES "^(.*:)?\\@bindir\\@/\\.(:.*)?$")
      message(FATAL_ERROR "In order to successfully run the CMake test suite on Syllable you need to add \"\\@bindir\\@/.\" to the DLL_PATH environment variable")
    endif()
    if(NOT "${_DLL_PATH}" MATCHES "^(.*:)?\\@bindir\\@/\\.\\./lib(:.*)?$")
      message(FATAL_ERROR "In order to successfully run the CMake test suite on Syllable you need to add \"\\@bindir\\@/../lib\" to the DLL_PATH environment variable")
    endif()

  else()

    add_test(JumpWithLibOut  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Jump"
      "${CMake_BINARY_DIR}/Tests/Jump/WithLibOut"
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Jump/WithLibOut/Executable"
      --build-project Jump
      ${build_generator_args}
      --build-options
        -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Jump/WithLibOut/Lib
      --test-command jumpExecutable
      )

    add_test(JumpNoLibOut  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Jump"
      "${CMake_BINARY_DIR}/Tests/Jump/NoLibOut"
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Jump/NoLibOut/Executable"
      --build-run-dir "${CMake_BINARY_DIR}/Tests/Jump/NoLibOut/Executable"
      --build-project Jump
      ${build_generator_args}
      --test-command jumpExecutable
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Jump")

    add_test(Plugin ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Plugin"
      "${CMake_BINARY_DIR}/Tests/Plugin"
      ${build_generator_args}
      --build-project Plugin
      --build-two-config
      --test-command bin/example)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Plugin")

    if(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG)
      ADD_TEST_MACRO(RuntimePath RuntimePath)
    endif()
  endif()

  if(APPLE AND "${DARWIN_MAJOR_VERSION}" GREATER 9)
    add_test(MacRuntimePath ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/MacRuntimePath"
      "${CMake_BINARY_DIR}/Tests/MacRuntimePath"
      ${build_generator_args}
      --build-project MacRuntimePath
      --build-options
        -DCMake_TEST_NESTED_MAKE_PROGRAM:FILEPATH=${CMake_TEST_EXPLICIT_MAKE_PROGRAM}
      )
  endif()

  if(CMake_TEST_XCODE_VERSION AND NOT CMake_TEST_XCODE_VERSION VERSION_LESS 5)
    if(NOT CMake_TEST_XCTest_DEPLOYMENT_TARGET)
      execute_process(
        COMMAND sw_vers -productVersion
        OUTPUT_VARIABLE OSX_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
      if(OSX_VERSION MATCHES "^([0-9]+\\.[0-9]+)")
        set(CMake_TEST_XCTest_DEPLOYMENT_TARGET "${CMAKE_MATCH_1}")
      endif()
    endif()
    if(CMake_TEST_XCTest_DEPLOYMENT_TARGET)
      set(XCTest_CTEST_OPTIONS --build-config $<CONFIGURATION>)
      set(XCTest_BUILD_OPTIONS -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMake_TEST_XCTest_DEPLOYMENT_TARGET} -DCMAKE_OSX_SYSROOT=macosx)
      ADD_TEST_MACRO(XCTest ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> -V)
    endif()
  endif()

  add_test(linkorder1 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLineOrder"
    "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    ${build_generator_args}
    --build-project LinkLineOrder
    --test-command Exec1
    )

  add_test(linkorder2  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLineOrder"
    "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    ${build_generator_args}
    --build-project LinkLineOrder
    --test-command Exec2
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LinkLineOrder")
  set_tests_properties(qtwrapping PROPERTIES DEPENDS wrapping)
  set_tests_properties(testdriver1 PROPERTIES DEPENDS qtwrapping)
  set_tests_properties(testdriver2 PROPERTIES DEPENDS testdriver1)
  set_tests_properties(testdriver3 PROPERTIES DEPENDS testdriver2)
  set_tests_properties(linkorder2 PROPERTIES DEPENDS linkorder1)

  # Test static linking on toolchains known to support it.
  if((CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "LCC")
      AND NOT APPLE AND NOT WIN32 AND NOT CYGWIN
      AND EXISTS "/usr/lib/libm.a")
    add_test(LinkStatic  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/LinkStatic"
      "${CMake_BINARY_DIR}/Tests/LinkStatic"
      ${build_generator_args}
      --build-project LinkStatic
      --build-options
        -DMATH_LIBRARY:FILEPATH=/usr/lib/libm.a
      --test-command LinkStatic
      )
  endif()

  if(MAKE_SUPPORTS_SPACES
      AND NOT CMAKE_GENERATOR STREQUAL "Xcode"
      AND NOT CMAKE_GENERATOR STREQUAL "Watcom WMake"
      )
    add_test(SubDirSpaces ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubDirSpaces"
      "${CMake_BINARY_DIR}/Tests/SubDirSpaces"
      --build-exe-dir
      "${CMake_BINARY_DIR}/Tests/SubDirSpaces/Executable Sources"
      ${build_generator_args}
      --build-project SUBDIR
      --test-command test
      "${CMake_BINARY_DIR}/Tests/SubDirSpaces/ShouldBeHere"
      "${CMake_BINARY_DIR}/Tests/SubDirSpaces/testfromsubdir.obj"
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SubDirSpaces")
  endif()

  if(WIN32)
    add_test(SubDir ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubDir"
      "${CMake_BINARY_DIR}/Tests/SubDir"
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/SubDir/Executable"
      ${build_generator_args}
      --build-project SUBDIR
      --test-command test
      "${CMake_BINARY_DIR}/Tests/SubDir/ShouldBeHere"
      "${CMake_BINARY_DIR}/Tests/SubDir/testfromsubdir.obj"
      )
  else()
    add_test(SubDir ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubDir"
      "${CMake_BINARY_DIR}/Tests/SubDir"
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/SubDir/Executable"
      ${build_generator_args}
      --build-project SUBDIR
      --test-command test
      "${CMake_BINARY_DIR}/Tests/SubDir/ShouldBeHere"
      "${CMake_BINARY_DIR}/Tests/SubDir/testfromsubdir.o"
      )
  endif()
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SubDir")

  if(MSVC OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND CMAKE_C_SIMULATE_ID STREQUAL "MSVC"))
    ADD_TEST_MACRO(PDBDirectoryAndName myexe)
    if(NOT CMAKE_C_COMPILER_ID STREQUAL "Clang" OR NOT "x${CMAKE_C_COMPILER_FRONTEND_VARIANT}" STREQUAL "xGNU")
      ADD_TEST_MACRO(ForceInclude foo)
    endif()
    if(NOT CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_C_COMPILER_ID STREQUAL "IntelLLVM")
      ADD_TEST_MACRO(PrecompiledHeader foo)
    endif()

    set(MSVCDebugInformationFormat_BUILD_OPTIONS -DCMake_TEST_CUDA=${CMake_TEST_CUDA})
    if(CMAKE_Fortran_COMPILER)
      list(APPEND MSVCDebugInformationFormat_BUILD_OPTIONS -DCMake_TEST_Fortran=1)
    endif()
    ADD_TEST_MACRO(MSVCDebugInformationFormat)
    set_property(TEST MSVCDebugInformationFormat APPEND
      PROPERTY LABELS "CUDA" "Fortran")

    set(MSVCRuntimeLibrary_BUILD_OPTIONS -DCMake_TEST_CUDA=${CMake_TEST_CUDA})
    ADD_TEST_MACRO(MSVCRuntimeLibrary)
    set_property(TEST MSVCRuntimeLibrary APPEND
      PROPERTY LABELS "CUDA")
    if(CMAKE_Fortran_COMPILER)
      ADD_TEST_MACRO(MSVCRuntimeLibrary.Fortran)
      set_property(TEST MSVCRuntimeLibrary.Fortran APPEND PROPERTY LABELS "Fortran")
    endif()
  endif()
  if(MSVC OR
      "${CMAKE_GENERATOR}" MATCHES "(MSYS|MinGW) Makefiles")
    ADD_TEST_MACRO(ModuleDefinition example_exe)
  endif()

  if(CMAKE_C_COMPILER_ID MATCHES "Watcom" AND WIN32)
    ADD_TEST_MACRO(WatcomRuntimeLibrary)
  endif()

  ADD_TEST_MACRO(CheckCompilerRelatedVariables CheckCompilerRelatedVariables)

  if("${CMAKE_GENERATOR}" MATCHES "Makefile" OR
     "${CMAKE_GENERATOR}" MATCHES "^Ninja$")
    add_test(MakeClean ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/MakeClean"
      "${CMake_BINARY_DIR}/Tests/MakeClean"
      ${build_generator_args}
      --build-project MakeClean
      --build-exe-dir "${CMake_BINARY_DIR}/MakeClean"
      --test-command check_clean
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/MakeClean")
  endif()

  if(CMake_TEST_MFC)
    add_test(MFC ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/MFC"
      "${CMake_BINARY_DIR}/Tests/MFC"
      --build-two-config
      ${build_generator_args}
      --build-project mfc_driver
      --test-command ${CMAKE_CTEST_COMMAND}
        -C \${CTEST_CONFIGURATION_TYPE} -VV)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/MFC")
  endif()

  if(MSVC AND NOT MSVC_VERSION LESS 1700
     AND (CMAKE_C_COMPILER_ARCHITECTURE_ID STREQUAL "ARM64")
      )
    ADD_TEST_MACRO(VSMARMASM VSMARMASM)
  endif()

  if(MSVC AND NOT MSVC_VERSION LESS 1310
     AND (NOT CMAKE_GENERATOR MATCHES "Visual Studio 9 "
          OR CMAKE_SIZEOF_VOID_P EQUAL 4)
     AND (NOT CMAKE_C_COMPILER_ARCHITECTURE_ID STREQUAL "ARM64")
      )
    ADD_TEST_MACRO(VSMASM VSMASM)
  endif()

  if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
    if(NOT MSVC60)
      ADD_TEST_MACRO(SBCS SBCS)
    endif()

    if(NOT "${CMAKE_GENERATOR}" MATCHES "Visual Studio 9 "
        AND NOT CMAKE_GENERATOR_TOOLSET STREQUAL "v90"
        AND NOT CMAKE_VS_PLATFORM_NAME STREQUAL "ARM64")
      ADD_TEST_MACRO(VSWindowsFormsResx VSWindowsFormsResx)
      ADD_TEST_MACRO(VSManagedCustomCommand)
    endif()

    add_test(VSExternalInclude ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/VSExternalInclude"
      "${CMake_BINARY_DIR}/Tests/VSExternalInclude"
      --build-two-config
      ${build_generator_args}
      --build-project VSExternalInclude
      --test-command VSExternalInclude)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSExternalInclude")

    add_test(VSMidl ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/VSMidl"
      "${CMake_BINARY_DIR}/Tests/VSMidl"
      --build-two-config
      ${build_generator_args}
      --build-project VSMidl
      --test-command VSMidl)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSMidl")

    if(CMake_TEST_DEVENV)
      # The test (and tested property) works with .sln files, so it's skipped when:
      # * cmake --build is set up to use MSBuild, since the MSBuild invocation does not use the .sln file
      set(_last_test "")
      foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
        add_test(NAME VSExcludeFromDefaultBuild-${config} COMMAND ${CMAKE_CTEST_COMMAND}
          --build-and-test
          "${CMake_SOURCE_DIR}/Tests/VSExcludeFromDefaultBuild"
          "${CMake_BINARY_DIR}/Tests/VSExcludeFromDefaultBuild"
          --build-config ${config}
          --build-two-config
          --build-generator ${CMAKE_GENERATOR}
          --build-makeprogram ${CMake_TEST_DEVENV}
          --build-generator-platform "${CMAKE_GENERATOR_PLATFORM}"
          --build-generator-toolset "${CMAKE_GENERATOR_TOOLSET}"
          --build-project VSExcludeFromDefaultBuild
          --build-target install
          --test-command ${CMAKE_COMMAND}
             -D "activeConfig=${config}"
             -D "allConfigs=${CMAKE_CONFIGURATION_TYPES}"
             -D "dir=${CMake_BINARY_DIR}/Tests/VSExcludeFromDefaultBuild"
             -P "${CMake_SOURCE_DIR}/Tests/VSExcludeFromDefaultBuild/ResultTest.cmake")
        if(_last_test)
          set_property(TEST VSExcludeFromDefaultBuild-${config} PROPERTY DEPENDS ${_last_test})
        endif()
        set(_last_test "VSExcludeFromDefaultBuild-${config}")
      endforeach()
      unset(_last_test)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSExcludeFromDefaultBuild")
    endif()

    if(CMAKE_GENERATOR MATCHES "Visual Studio ([0-5]|[6-9][0-9])")
      # This is Visual Studio 10 or above, so the default build tool is MSBuild.
      add_test(NAME VSProjectInSubdir COMMAND ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/VSProjectInSubdir"
        "${CMake_BINARY_DIR}/Tests/VSProjectInSubdir"
        --build-two-config
        --build-generator ${CMAKE_GENERATOR}
        --build-generator-platform "${CMAKE_GENERATOR_PLATFORM}"
        --build-generator-toolset "${CMAKE_GENERATOR_TOOLSET}"
        --build-project VSProjectInSubdir
        --build-target test)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSProjectInSubdir")
    endif()
  endif()

  get_filename_component(ntver "[HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion;CurrentVersion]" NAME)
  if(WIN32 AND ntver VERSION_GREATER 6.1) # Windows >= 8.0
    macro(add_test_VSWinStorePhone name generator systemName systemVersion architecture)
      add_test(NAME VSWinStorePhone.${name} COMMAND ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/VSWinStorePhone"
        "${CMake_BINARY_DIR}/Tests/VSWinStorePhone/${name}"
        --build-generator "${generator}"
        --build-generator-platform "${architecture}"
        --build-project VSWinStorePhone
        --build-config $<CONFIGURATION>
        --build-options -DCMAKE_SYSTEM_NAME=${systemName}
                        -DCMAKE_SYSTEM_VERSION=${systemVersion}
        --test-command
          ${CMAKE_CMAKE_COMMAND} -DAPP_PACKAGE_DIR="${CMake_BINARY_DIR}/Tests/VSWinStorePhone/${name}"
                                 -P "${CMake_SOURCE_DIR}/Tests/VSWinStorePhone/VerifyAppPackage.cmake"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSWinStorePhone/${name}")
    endmacro()

    if(vs12 AND ws81)
      add_test_VSWinStorePhone(vs12-store81-X86 "Visual Studio 12 2013" WindowsStore 8.1 Win32)
      add_test_VSWinStorePhone(vs12-store81-ARM "Visual Studio 12 2013" WindowsStore 8.1 ARM)
      add_test_VSWinStorePhone(vs12-store81-X64 "Visual Studio 12 2013" WindowsStore 8.1 x64)

      add_test(NAME VSXaml COMMAND ${CMAKE_CTEST_COMMAND}
        --build-and-test
          "${CMake_SOURCE_DIR}/Tests/VSXaml"
          "${CMake_BINARY_DIR}/Tests/VSXaml"
          --build-generator "Visual Studio 12 2013"
          --build-project VSXaml
          --build-config $<CONFIGURATION>
          --build-options -DCMAKE_SYSTEM_NAME=WindowsStore
                          -DCMAKE_SYSTEM_VERSION=8.1
        )
    endif()
    if(CMake_TEST_VSWinStorePhone_VS_2017 AND ws10_0)
      add_test_VSWinStorePhone(vs15-store10_0-X86 "Visual Studio 15 2017" WindowsStore 10.0 Win32)
      add_test_VSWinStorePhone(vs15-store10_0-ARM "Visual Studio 15 2017" WindowsStore 10.0 ARM)
      add_test_VSWinStorePhone(vs15-store10_0-X64 "Visual Studio 15 2017" WindowsStore 10.0 x64)
      add_test_VSWinStorePhone(vs15-store10_0-ARM64 "Visual Studio 15 2017" WindowsStore 10.0 ARM64)
    endif()
    if(vs14 AND ws10_0)
      add_test_VSWinStorePhone(vs14-store10_0-X86 "Visual Studio 14 2015" WindowsStore 10.0 Win32)
      add_test_VSWinStorePhone(vs14-store10_0-ARM "Visual Studio 14 2015" WindowsStore 10.0 ARM)
      add_test_VSWinStorePhone(vs14-store10_0-X64 "Visual Studio 14 2015" WindowsStore 10.0 x64)
    endif()
    if(vs12 AND wp81)
      add_test_VSWinStorePhone(vs12-phone81-X86 "Visual Studio 12 2013" WindowsPhone 8.1 Win32)
      add_test_VSWinStorePhone(vs12-phone81-ARM "Visual Studio 12 2013" WindowsPhone 8.1 ARM)
    endif()
  endif()

  if(WIN32 AND wince)
    macro(add_test_VSWinCE name generator systemName systemVersion generatorPlatform)
      # TODO: Fix the tutorial to make it work in cross compile
      # currently the MakeTable is build for target and can not be used on the host
      # This happens in part 5 so we build only through part 4 of the tutorial.
      foreach(STP RANGE 2 4)
        add_test(NAME "TutorialStep${STP}.${name}" COMMAND ${CMAKE_CTEST_COMMAND}
          --build-and-test
          "${CMake_SOURCE_DIR}/Help/guide/tutorial/Step${STP}"
          "${CMake_BINARY_DIR}/Tests/Tutorial/Step${STP}_${name}"
          --build-generator "${generator}"
          --build-project Tutorial
          --build-config $<CONFIGURATION>
          --build-options -DCMAKE_SYSTEM_NAME=${systemName}
                          -DCMAKE_SYSTEM_VERSION=${systemVersion}
                          -DCMAKE_GENERATOR_PLATFORM=${generatorPlatform})
        list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Tutorial/Step${STP}_${name}")
      endforeach()
    endmacro()

    if(vs12)
      add_test_VSWinCE(vs12-ce80-ARM "Visual Studio 12 2013" WindowsCE 8.0 ${wince_sdk})
    endif()
  endif()

  if(CMAKE_GENERATOR MATCHES "Visual Studio ([^9]|9[0-9])" AND nasm)
    ADD_TEST_MACRO(VSNASM VSNASM)
  endif()

  if(CMake_TEST_GreenHillsMULTI)
    macro(add_test_GhsMulti test_name test_dir bin_sub_dir build_opts)
      separate_arguments(_ghs_build_opts UNIX_COMMAND ${build_opts})
      separate_arguments(_ghs_toolset_extra UNIX_COMMAND ${ghs_toolset_extra})
      if(${ARGC} GREATER 4)
        set(_ghs_test_command --test-command ${ARGN})
      endif()
      if(ghs_config_name STREQUAL "__default__")
        set(_ghs_test_name "${test_name}")
      else()
        set(_ghs_test_name "${ghs_config_name}.${test_name}")
      endif()
      add_test(NAME GhsMulti.${_ghs_test_name}
        COMMAND ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/GhsMulti/${test_dir}"
        "${CMake_BINARY_DIR}/Tests/GhsMulti/${ghs_config_name}/${test_dir}/${bin_sub_dir}"
        --build-generator "Green Hills MULTI"
        --build-project test
        --build-config $<CONFIGURATION>
        --force-new-ctest-process
        --build-options ${ghs_target_arch} ${ghs_toolset_name} ${ghs_toolset_root} ${ghs_target_platform}
          ${ghs_os_root} ${ghs_os_dir} ${ghs_bsp_name} ${_ghs_build_opts} ${_ghs_toolset_extra}
          ${_ghs_test_command}
        )
        unset(_ghs_build_opts)
        unset(_ghs_toolset_extra)
        unset(_ghs_test_command)
        unset(_ghs_test_name)
    endmacro()
    macro(add_test_GhsMulti_rename_install test_name)
      add_test_GhsMulti( ${test_name} GhsMultiRenameInstall ${test_name}
        "-DCMAKE_INSTALL_PREFIX=. -DRUN_TEST=${test_name}" ${CMAKE_CMAKE_COMMAND} --build . --target INSTALL)
    endmacro()
    #unset ghs config variables
    unset(ghs_config_name)
    unset(ghs_target_arch)
    unset(ghs_toolset_root)
    unset(ghs_toolset_name)
    unset(ghs_os_root)
    unset(ghs_os_dir)
    unset(ghs_target_platform)
    unset(ghs_bsp_name)
    unset(ghs_toolset_extra)
    if(NOT CMake_TEST_GreenHillsMULTI_config)
      #if list of config settings not defined then just run once as default
      set(CMake_TEST_GreenHillsMULTI_config "__default__")
    endif()
    foreach(ghs_file IN LISTS CMake_TEST_GreenHillsMULTI_config)
      # source GHS tools config file
      if(NOT ghs_file STREQUAL "__default__")
        if(IS_ABSOLUTE ${ghs_file})
          include(${ghs_file})
        else()
          include(${CMAKE_BINARY_DIR}/${ghs_file})
        endif()
      endif()
      if(NOT ghs_config_name)
        set(ghs_config_name "__default__")
      endif()
      # test integrity build
      if(NOT ghs_skip_integrity AND (NOT ghs_target_platform OR ghs_target_platform MATCHES "integrity"))
        add_test_GhsMulti(integrityDDInt GhsMultiIntegrity/GhsMultiIntegrityDDInt "" "")
        add_test_GhsMulti(integrityMonolith GhsMultiIntegrity/GhsMultiIntegrityMonolith "" "")
        add_test_GhsMulti(integrityDD GhsMultiIntegrity/GhsMultiIntegrityDD "" "")
      endif()
      add_test_GhsMulti(duplicate_source_filenames GhsMultiDuplicateSourceFilenames "" "")
      add_test_GhsMulti_rename_install(SINGLE_EXEC)
      add_test_GhsMulti_rename_install(SINGLE_EXEC_RENAMED)
      add_test_GhsMulti_rename_install(EXEC_AND_LIB)
      add_test_GhsMulti(multiple_source_groups GhsMultiSrcGroups Default "")
      add_test_GhsMulti(multiple_source_groups_folders GhsMultiSrcGroups PropFolders "-DTEST_PROP=ON")
      add_test_GhsMulti(multiple_source_groups_all_folders GhsMultiSrcGroups AllFolders "-DCMAKE_GHS_NO_SOURCE_GROUP_FILE=ON")
      add_test_GhsMulti(unsupported_targets GhsMultiUnsupportedTargets "" "")
      add_test_GhsMulti(object_library GhsMultiObjectLibrary "" "")
      add_test_GhsMulti(exclude GhsMultiExclude "" ""
        ${CMAKE_CMAKE_COMMAND} -P ${CMake_SOURCE_DIR}/Tests/GhsMulti/GhsMultiExclude/verify.cmake)
      add_test_GhsMulti(interface GhsMultiInterface "" "")
      add_test_GhsMulti(transitive_link_test GhsMultiLinkTest TransitiveLink "-DRUN_TEST=NO_FLAGS")
      add_test_GhsMulti(flags_link_test GhsMultiLinkTest FlagsCheck "-DRUN_TEST=CHECK_FLAGS")
      add_test_GhsMulti(sub_link_test GhsMultiLinkTestSub "" "")
      add_test_GhsMulti(multiple_projects GhsMultiMultipleProjects "" ""
        ${CMAKE_CMAKE_COMMAND} -P ${CMake_SOURCE_DIR}/Tests/GhsMulti/GhsMultiMultipleProjects/verify.cmake)
      add_test_GhsMulti(compiler_options_none GhsMultiCompilerOptions None "-DRUN_TEST=RELEASE_FLAGS -DRUN_TEST_BUILD_TYPE=\"\"")
      add_test_GhsMulti(compiler_options_kernel GhsMultiCompilerOptions Kernel "-DRUN_TEST=KERNEL_FLAGS -DRUN_TEST_BUILD_TYPE=DEBUG")
      add_test_GhsMulti(try_compile_copy GhsMultiCopyFile "" "")
      add_test_GhsMulti(ghs_platform GhsMultiPlatform "" "")
      add_test_GhsMulti(custom_target GhsMultiCustomTarget "" "")
      add_test_GhsMulti(dep_order GhsMultiDepOrder "" "")
      add_test_GhsMulti(external_project GhsMultiExternalProject "" "")
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/GhsMulti/${ghs_config_name}")
      #unset ghs config variables
      unset(ghs_config_name)
      unset(ghs_target_arch)
      unset(ghs_toolset_root)
      unset(ghs_toolset_name)
      unset(ghs_os_root)
      unset(ghs_os_dir)
      unset(ghs_target_platform)
      unset(ghs_bsp_name)
      unset(ghs_toolset_extra)
    endforeach()
  endif()

  macro(add_test_VSAndroid name generator platform)
    add_test(NAME "VSAndroid.${name}.${platform}" COMMAND ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/VSAndroid"
      "${CMake_BINARY_DIR}/Tests/VSAndroid/${name}/${platform}"
      --build-generator "${generator}"
      --build-project VSAndroid
      --build-config $<CONFIGURATION>
      --build-options -DCMAKE_SYSTEM_NAME=Android "-A${platform}"
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSAndroid/${name}")
  endmacro()
  if(tegra AND NOT "${CMake_SOURCE_DIR};${CMake_BINARY_DIR}" MATCHES " ")
    if(vs12)
      add_test_VSAndroid(vs12 "Visual Studio 12 2013" "Tegra-Android")
    endif()
    if(vs14)
      add_test_VSAndroid(vs14 "Visual Studio 14 2015" "Tegra-Android")
    endif()
  endif()
  if(vs14 AND CMake_TEST_ANDROID_VS14)
    add_test_VSAndroid(vs14 "Visual Studio 14 2015" "ARM")
  endif()
  if(vs15 AND CMake_TEST_ANDROID_VS15)
    add_test_VSAndroid(vs15 "Visual Studio 15 2017" "ARM")
  endif()
  if(vs16 AND CMake_TEST_ANDROID_VS16)
    add_test_VSAndroid(vs16 "Visual Studio 16 2019" "ARM")
  endif()
  if(vs17 AND CMake_TEST_ANDROID_VS17)
    add_test_VSAndroid(vs17 "Visual Studio 17 2022" "ARM")
  endif()

  if(APPLE)
    if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
      set(BundleTestInstallDir
        "${CMake_BINARY_DIR}/Tests/BundleTest/InstallDirectory")
      add_test(BundleTest ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/BundleTest"
        "${CMake_BINARY_DIR}/Tests/BundleTest"
        --build-two-config
        ${build_generator_args}
        --build-project BundleTest
        --build-target install
#       --build-target package
        --build-options
        "-DCMAKE_INSTALL_PREFIX:PATH=${BundleTestInstallDir}"
        "-DCMake_SOURCE_DIR:PATH=${CMake_SOURCE_DIR}"
        --test-command
        ${BundleTestInstallDir}/Applications/SecondBundleExe.app/Contents/MacOS/SecondBundleExe)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BundleTest")

      add_test(NAME CFBundleTest COMMAND ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/CFBundleTest"
        "${CMake_BINARY_DIR}/Tests/CFBundleTest"
        --build-two-config
        ${build_generator_args}
        --build-project CFBundleTest
        --build-config $<CONFIGURATION>
        --test-command
        ${CMAKE_CMAKE_COMMAND} -DCTEST_CONFIGURATION_TYPE=$<CONFIGURATION>
        -Ddir=${CMake_BINARY_DIR}/Tests/CFBundleTest
        -Dgen=${CMAKE_GENERATOR}
        -P ${CMake_SOURCE_DIR}/Tests/CFBundleTest/VerifyResult.cmake)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CFBundleTest")
    endif()
  endif()

  if(APPLE AND CTEST_TEST_CPACK)
    add_test(BundleGeneratorTest ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/BundleGeneratorTest"
      "${CMake_BINARY_DIR}/Tests/BundleGeneratorTest"
      --build-two-config
      ${build_generator_args}
      --build-project BundleGeneratorTest
      --build-target package
      --build-options
        "-DCMAKE_INSTALL_PREFIX:PATH=${CMake_BINARY_DIR}/Tests/BundleGeneratorTest/InstallDirectory"
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BundleGeneratorTest")
  endif()

  add_test(WarnUnusedCliUnused ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/WarnUnusedCliUnused"
    "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUnused"
    ${build_generator_args}
    --build-project WarnUnusedCliUnused
    --build-options
      "-DUNUSED_CLI_VARIABLE=Unused")
  set_tests_properties(WarnUnusedCliUnused PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Warning:.*Manually-specified variables were not used by the project:.*  UNUSED_CLI_VARIABLE")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUnused")

  add_test(WarnUnusedCliUsed ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUsage"
    "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUsed"
    ${build_generator_args}
    --build-noclean
    --build-project WarnUnusedCliUsed
    --build-options
      "-DUSED_VARIABLE=Usage proven")
  set_tests_properties(WarnUnusedCliUsed PROPERTIES
    PASS_REGULAR_EXPRESSION "Usage proven")
  set_tests_properties(WarnUnusedCliUsed PROPERTIES
    FAIL_REGULAR_EXPRESSION "CMake Warning: The variable, 'USED_VARIABLE'")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUsed")

  add_test(WarnUninitialized ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUsage"
    "${CMake_BINARY_DIR}/Tests/WarnUninitialized"
    ${build_generator_args}
    --build-noclean
    --build-project WarnUninitialized
    --build-options
      "--warn-uninitialized")
  set_tests_properties(WarnUninitialized PROPERTIES
    PASS_REGULAR_EXPRESSION "uninitialized variable 'USED_VARIABLE'")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUninitialized")

  add_test(TestsWorkingDirectory
    ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestsWorkingDirectory"
    "${CMake_BINARY_DIR}/Tests/TestsWorkingDirectory"
    ${build_generator_args}
    --build-project TestsWorkingDirectoryProj
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/TestsWorkingDirectory"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V -C \${CTEST_CONFIGURATION_TYPE}
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TestsWorkingDirectory")

  # Make sure CTest can handle a test with no newline in output.
  add_test(CTest.NoNewline
    ${CMAKE_CMAKE_COMMAND} -E echo_append "This line has no newline!")

  # A simple test for ctest in script mode
  configure_file("${CMake_SOURCE_DIR}/Tests/CTestScriptMode/CTestTestScriptMode.cmake.in"
          "${CMake_BINARY_DIR}/Tests/CTestScriptMode/CTestTestScriptMode.cmake" @ONLY)
#  add_test(CTest.ScriptMode ${CMAKE_CTEST_COMMAND}
#        -S "${CMake_BINARY_DIR}/Tests/CTestScriptMode/CTestTestScriptMode.cmake"
#        )

  # Test CTest Update with Subversion
  if(NOT DEFINED CMake_TEST_CTestUpdate_SVN OR CMake_TEST_CTestUpdate_SVN)
    find_package(Subversion QUIET)
    if(Subversion_FOUND)
      get_filename_component(_Subversion_BIN_DIR
        ${Subversion_SVN_EXECUTABLE} PATH)
      find_program(Subversion_SVNADMIN_EXECUTABLE svnadmin
        HINTS ${_Subversion_BIN_DIR}
        )
      mark_as_advanced(Subversion_SVNADMIN_EXECUTABLE)
      if(NOT Subversion_SVNADMIN_EXECUTABLE)
        set(Subversion_FOUND FALSE)
      endif()
    endif()
  endif()
  if(NOT DEFINED CMake_TEST_CTestUpdate_SVN AND Subversion_FOUND)
    set(CMake_TEST_CTestUpdate_SVN 1)
  endif()
  if(CMake_TEST_CTestUpdate_SVN)
    if(NOT Subversion_FOUND)
      message(FATAL_ERROR "CMake_TEST_CTestUpdate_SVN enabled but Subversion is not found.")
    endif()
    set(CTestUpdateSVN_DIR "CTest UpdateSVN")
    configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateSVN.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestUpdateSVN.cmake" @ONLY)
    add_test(CTest.UpdateSVN ${CMAKE_CMAKE_COMMAND}
      -P "${CMake_BINARY_DIR}/Tests/CTestUpdateSVN.cmake"
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateSVN_DIR}")
  endif()

  # Test CTest Update with CVS
  if(NOT DEFINED CMake_TEST_CTestUpdate_CVS OR CMake_TEST_CTestUpdate_CVS)
    find_program(CVS_EXECUTABLE NAMES cvs)
    mark_as_advanced(CVS_EXECUTABLE)
  endif()
  if(NOT DEFINED CMake_TEST_CTestUpdate_CVS AND CVS_EXECUTABLE
      AND (UNIX OR NOT "${CVS_EXECUTABLE}" MATCHES "cygwin"))
    set(CMake_TEST_CTestUpdate_CVS 1)
  endif()
  if(CMake_TEST_CTestUpdate_CVS)
    if(NOT CVS_EXECUTABLE)
      message(FATAL_ERROR "CMake_TEST_CTestUpdate_CVS enabled but CVS_EXECUTABLE is not found.")
    endif()
    set(CTestUpdateCVS_DIR "CTest UpdateCVS")
    configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateCVS.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestUpdateCVS.cmake" @ONLY)
    add_test(CTest.UpdateCVS ${CMAKE_CMAKE_COMMAND}
      -P "${CMake_BINARY_DIR}/Tests/CTestUpdateCVS.cmake"
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateCVS_DIR}")
  endif()

  # Test CTest Update with BZR
  if(CMake_TEST_CTestUpdate_BZR)
    if(TEST_HOME)
      file(MAKE_DIRECTORY "${TEST_HOME}/.bazaar")
    endif()
    find_program(BZR_EXECUTABLE NAMES bzr)
    mark_as_advanced(BZR_EXECUTABLE)
    if(NOT BZR_EXECUTABLE)
      message(FATAL_ERROR "CMake_TEST_CTestUpdate_BZR enabled but BZR_EXECUTABLE is not found.")
    endif()
    set(CTestUpdateBZR_DIR "CTest UpdateBZR")
    configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateBZR.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestUpdateBZR.cmake" @ONLY)
    add_test(CTest.UpdateBZR ${CMAKE_CMAKE_COMMAND}
      -P "${CMake_BINARY_DIR}/Tests/CTestUpdateBZR.cmake"
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateBZR_DIR}")
    set(CTestUpdateBZR_DIR "CTest UpdateBZR_CLocale")
    configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateBZR.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestUpdateBZR_CLocale.cmake" @ONLY)
    add_test(CTest.UpdateBZR.CLocale ${CMAKE_CMAKE_COMMAND}
      -P "${CMake_BINARY_DIR}/Tests/CTestUpdateBZR_CLocale.cmake"
      )
    set_tests_properties(CTest.UpdateBZR.CLocale PROPERTIES ENVIRONMENT LC_ALL=C)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateBZR_DIR}")
  endif()

  # Test CTest Update with GIT
  if(NOT DEFINED CMake_TEST_CTestUpdate_GIT AND GIT_EXECUTABLE
      AND (UNIX OR NOT "${GIT_EXECUTABLE}" MATCHES "cygwin"))
    set(CMake_TEST_CTestUpdate_GIT 1)
  endif()
  if(CMake_TEST_CTestUpdate_GIT)
    if(NOT GIT_EXECUTABLE)
      message(FATAL_ERROR "CMake_TEST_CTestUpdate_GIT enabled but GIT_EXECUTABLE is not found.")
    endif()
    set(CTestUpdateGIT_DIR "CTest UpdateGIT")
    configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateGIT.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestUpdateGIT.cmake" @ONLY)
    add_test(CTest.UpdateGIT ${CMAKE_CMAKE_COMMAND}
      -P "${CMake_BINARY_DIR}/Tests/CTestUpdateGIT.cmake"
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateGIT_DIR}")
    set_property(TEST CTest.UpdateGIT PROPERTY ENVIRONMENT GIT_ALLOW_PROTOCOL=file)
  endif()

  # Test CTest Update with HG
  if(NOT DEFINED CMake_TEST_CTestUpdate_HG OR CMake_TEST_CTestUpdate_HG)
    find_program(HG_EXECUTABLE NAMES hg)
    mark_as_advanced(HG_EXECUTABLE)
  endif()
  if(NOT DEFINED CMake_TEST_CTestUpdate_HG AND HG_EXECUTABLE
      AND (UNIX OR NOT "${HG_EXECUTABLE}" MATCHES "cygwin"))
    execute_process(COMMAND "${HG_EXECUTABLE}" --version OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE HG_RV)
    if(HG_RV EQUAL 0)
      set(CMake_TEST_CTestUpdate_HG 1)
    endif()
  endif()
  if(CMake_TEST_CTestUpdate_HG)
    if(NOT HG_EXECUTABLE)
      message(FATAL_ERROR "CMake_TEST_CTestUpdate_HG enabled but HG_EXECUTABLE is not found.")
    endif()
    set(CTestUpdateHG_DIR "CTest UpdateHG")
    configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateHG.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestUpdateHG.cmake" @ONLY)
    add_test(CTest.UpdateHG ${CMAKE_CMAKE_COMMAND}
      -P "${CMake_BINARY_DIR}/Tests/CTestUpdateHG.cmake"
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateHG_DIR}")
  endif()

  # Test CTest Update with P4
  if(CMake_TEST_CTestUpdate_P4)
    find_program(P4_EXECUTABLE NAMES p4)
    find_program(P4D_EXECUTABLE NAMES p4d)
    mark_as_advanced(P4_EXECUTABLE P4D_EXECUTABLE)
    if(NOT P4_EXECUTABLE OR NOT P4D_EXECUTABLE)
      message(FATAL_ERROR "CMake_TEST_CTestUpdate_HG enabled but P4_EXECUTABLE and P4D_EXECUTABLE are not both not found.")
    endif()
    set(CTestUpdateP4_DIR "CTest UpdateP4")
    configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateP4.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestUpdateP4.cmake" @ONLY)
    add_test(CTest.UpdateP4 ${CMAKE_CMAKE_COMMAND}
      -P "${CMake_BINARY_DIR}/Tests/CTestUpdateP4.cmake"
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateP4_DIR}")
  endif()

  if(NOT CMake_TEST_NO_NETWORK)
    configure_file(
      "${CMake_SOURCE_DIR}/Tests/CTestTestUpload/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTestUpload/test.cmake"
      @ONLY ESCAPE_QUOTES)
    add_test(CTestTestUpload ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTestUpload/test.cmake" -V
      --output-log "${CMake_BINARY_DIR}/Tests/CTestTestUpload/testOut.log"
      )
    set_tests_properties(CTestTestUpload PROPERTIES
      PASS_REGULAR_EXPRESSION "Upload\\.xml")
  endif()

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestCoverageCollectGCOV/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestCoverageCollectGCOV/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestCoverageCollectGCOV ${CMAKE_CTEST_COMMAND}
    -C \${CTEST_CONFIGURATION_TYPE}
    -S "${CMake_BINARY_DIR}/Tests/CTestCoverageCollectGCOV/test.cmake" -VV
    --output-log "${CMake_BINARY_DIR}/Tests/CTestCoverageCollectGCOV/testOut.log"
    )

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestEmptyBinaryDirectory/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestEmptyBinaryDirectory/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestEmptyBinaryDirectory ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestEmptyBinaryDirectory/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestEmptyBinaryDirectory/testOut.log"
    )
  set_tests_properties(CTestTestEmptyBinaryDirectory PROPERTIES
    PASS_REGULAR_EXPRESSION "TEST_SUCCESS")

  # test coverage for mumps
  # create a MumpsCoverage dir in the binary tree under Testing to
  # avoid the .NoDartCoverage files in the cmake testing tree
  configure_file(
     "${CMake_SOURCE_DIR}/Tests/MumpsCoverage/DartConfiguration.tcl.in"
     "${CMake_BINARY_DIR}/Testing/MumpsCoverage/DartConfiguration.tcl")
  configure_file(
     "${CMake_SOURCE_DIR}/Tests/MumpsCoverage/gtm_coverage.mcov.in"
     "${CMake_BINARY_DIR}/Testing/MumpsCoverage/gtm_coverage.mcov")
  file(REMOVE_RECURSE "${CMake_BINARY_DIR}/Testing/MumpsCoverage/VistA-FOIA")
  file(COPY "${CMake_SOURCE_DIR}/Tests/MumpsCoverage/VistA-FOIA"
    DESTINATION "${CMake_BINARY_DIR}/Testing/MumpsCoverage")
  add_test(NAME CTestGTMCoverage
    COMMAND ${CMAKE_CMAKE_COMMAND} -E chdir
    ${CMake_BINARY_DIR}/Testing/MumpsCoverage
    $<TARGET_FILE:ctest> -T Coverage --debug)
  set_tests_properties(CTestGTMCoverage PROPERTIES
      PASS_REGULAR_EXPRESSION
      "Process file.*ZZCOVTST.m.*Total LOC:.*32.*Percentage Coverage: 81.25*"
      ENVIRONMENT COVFILE=)

  configure_file(
     "${CMake_SOURCE_DIR}/Tests/MumpsCoverage/DartConfiguration.cache.tcl.in"
     "${CMake_BINARY_DIR}/Testing/MumpsCacheCoverage/DartConfiguration.tcl")
  configure_file(
    "${CMake_SOURCE_DIR}/Tests/MumpsCoverage/cache_coverage.cmcov.in"
    "${CMake_BINARY_DIR}/Testing/MumpsCacheCoverage/cache_coverage.cmcov")
  file(REMOVE_RECURSE "${CMake_BINARY_DIR}/Testing/MumpsCacheCoverage/VistA-FOIA")
  file(COPY "${CMake_SOURCE_DIR}/Tests/MumpsCoverage/VistA-FOIA"
    DESTINATION "${CMake_BINARY_DIR}/Testing/MumpsCacheCoverage")
  add_test(NAME CTestCacheCoverage
    COMMAND ${CMAKE_CMAKE_COMMAND} -E chdir
    ${CMake_BINARY_DIR}/Testing/MumpsCacheCoverage
    $<TARGET_FILE:ctest> -T Coverage --debug)
  set_tests_properties(CTestCacheCoverage PROPERTIES
      PASS_REGULAR_EXPRESSION
      "Process file.*ZZCOVTST.m.*Total LOC:.*32.*Percentage Coverage: 87.50.*"
      ENVIRONMENT COVFILE=)

  # Adding a test case for Python Coverage
  configure_file(
     "${CMake_SOURCE_DIR}/Tests/PythonCoverage/coverage.xml.in"
     "${CMake_BINARY_DIR}/Testing/PythonCoverage/coverage.xml")
  configure_file(
     "${CMake_SOURCE_DIR}/Tests/PythonCoverage/DartConfiguration.tcl.in"
     "${CMake_BINARY_DIR}/Testing/PythonCoverage/DartConfiguration.tcl")
  file(COPY "${CMake_SOURCE_DIR}/Tests/PythonCoverage/coveragetest"
    DESTINATION "${CMake_BINARY_DIR}/Testing/PythonCoverage")
  add_test(NAME CTestPythonCoverage
    COMMAND ${CMAKE_CMAKE_COMMAND} -E chdir
    ${CMake_BINARY_DIR}/Testing/PythonCoverage
    $<TARGET_FILE:ctest> -T Coverage --debug)
  set_tests_properties(CTestPythonCoverage PROPERTIES
      PASS_REGULAR_EXPRESSION
      "Process file.*foo.py.*Total LOC:.*13.*Percentage Coverage: 84.62.*"
      ENVIRONMENT COVFILE=)

  # Adding a test case for non-python Cobertura Coverage
  configure_file(
     "${CMake_SOURCE_DIR}/Tests/CoberturaCoverage/DartConfiguration.tcl.in"
     "${CMake_BINARY_DIR}/Testing/CoberturaCoverage/DartConfiguration.tcl")
  configure_file(
     "${CMake_SOURCE_DIR}/Tests/CoberturaCoverage/coverage.xml.in"
     "${CMake_BINARY_DIR}/Testing/CoberturaCoverage/coverage.xml")
  file(COPY "${CMake_SOURCE_DIR}/Tests/CoberturaCoverage/src"
    DESTINATION "${CMake_BINARY_DIR}/Testing/CoberturaCoverage")
  add_test(NAME CTestCoberturaCoverage
    COMMAND ${CMAKE_CMAKE_COMMAND} -E chdir
    ${CMake_BINARY_DIR}/Testing/CoberturaCoverage
    $<TARGET_FILE:ctest> -T Coverage --debug)
  set_tests_properties(CTestCoberturaCoverage PROPERTIES
      PASS_REGULAR_EXPRESSION
      "Process file.*CoverageTest.java.*Total LOC:.*18.*Percentage Coverage: 72.22.*"
      ENVIRONMENT COBERTURADIR=${CMake_BINARY_DIR}/Testing/CoberturaCoverage
      ENVIRONMENT COVFILE=)


  # Adding a test case for JaCoCo Coverage
  configure_file(
     "${CMake_SOURCE_DIR}/Tests/JacocoCoverage/DartConfiguration.tcl.in"
     "${CMake_BINARY_DIR}/Testing/JacocoCoverage/DartConfiguration.tcl")
  file(COPY "${CMake_SOURCE_DIR}/Tests/JacocoCoverage/Coverage"
    DESTINATION "${CMake_BINARY_DIR}/Testing/JacocoCoverage")
  configure_file("${CMake_BINARY_DIR}/Testing/JacocoCoverage/Coverage/target/site/jacoco.xml.in"
    "${CMake_BINARY_DIR}/Testing/JacocoCoverage/Coverage/target/site/jacoco.xml")
  add_test(NAME CTestJacocoCoverage
    COMMAND ${CMAKE_CMAKE_COMMAND} -E chdir
    ${CMake_BINARY_DIR}/Testing/JacocoCoverage
    $<TARGET_FILE:ctest> -T Coverage --debug)
  set_tests_properties(CTestJacocoCoverage PROPERTIES
      PASS_REGULAR_EXPRESSION
      "Process file.*CoverageTest.java.*Total LOC:.*17.*Percentage Coverage: 76.47*"
      ENVIRONMENT COVFILE=)

  # Adding a test case for Javascript Coverage
  configure_file(
     "${CMake_SOURCE_DIR}/Tests/JavascriptCoverage/DartConfiguration.tcl.in"
     "${CMake_BINARY_DIR}/Testing/JavascriptCoverage/DartConfiguration.tcl")
  configure_file(
     "${CMake_SOURCE_DIR}/Tests/JavascriptCoverage/output.json.in"
     "${CMake_BINARY_DIR}/Testing/JavascriptCoverage/output.json")
  file(COPY "${CMake_SOURCE_DIR}/Tests/JavascriptCoverage/"
    DESTINATION "${CMake_BINARY_DIR}/Testing/JavascriptCoverage"
    FILES_MATCHING PATTERN "*.js")
  add_test(NAME CTestJavascriptCoverage
    COMMAND ${CMAKE_CMAKE_COMMAND} -E chdir
    ${CMake_BINARY_DIR}/Testing/JavascriptCoverage
    $<TARGET_FILE:ctest> -T Coverage --debug)
  set_tests_properties(CTestJavascriptCoverage PROPERTIES
      PASS_REGULAR_EXPRESSION
      "Process file.*test3.js.*Total LOC:.*49.*Percentage Coverage: 79.59*"
      ENVIRONMENT COVFILE=)

  # test coverage for Delphi-code-Coverage
  configure_file(
     "${CMake_SOURCE_DIR}/Tests/DelphiCoverage/DartConfiguration.tcl.in"
     "${CMake_BINARY_DIR}/Testing/DelphiCoverage/DartConfiguration.tcl")
  file(COPY "${CMake_SOURCE_DIR}/Tests/DelphiCoverage/src"
    DESTINATION "${CMake_BINARY_DIR}/Testing/DelphiCoverage")
  configure_file(
    "${CMake_SOURCE_DIR}/Tests/DelphiCoverage/UTCovTest(UTCovTest.pas).html.in"
    "${CMake_BINARY_DIR}/Testing/DelphiCoverage/UTCovTest(UTCovTest.pas).html")
  add_test(NAME CTestDelphiCoverage
    COMMAND ${CMAKE_CMAKE_COMMAND} -E chdir
    ${CMake_BINARY_DIR}/Testing/DelphiCoverage
    $<TARGET_FILE:ctest> -T Coverage --debug)
  set_tests_properties(CTestDelphiCoverage PROPERTIES
      PASS_REGULAR_EXPRESSION
      "Process file.*UTCovTest.pas.*Total LOC:.*20.*Percentage Coverage: 95.*"
      ENVIRONMENT COVFILE=)

  function(add_config_tests cfg)
    set(base "${CMake_BINARY_DIR}/Tests/CTestConfig")

    # Test -S script with a -C config arg to ctest:
    configure_file(
      "${CMake_SOURCE_DIR}/Tests/CTestConfig/script.cmake.in"
      "${base}/${cfg}-script.cmake"
      @ONLY ESCAPE_QUOTES)
    add_test(CTestConfig.Script.${cfg} ${CMAKE_CTEST_COMMAND}
      -C ${cfg}
      -S "${base}/${cfg}-script.cmake" -VV
      --output-log "${base}/${cfg}-script.log"
      )

    # Test -D dashboard with a -C config arg to ctest.
    # (Actual commands inside a cmake -P script because we need to be able to set
    #  the working directory reliably...)
    configure_file(
      "${CMake_SOURCE_DIR}/Tests/CTestConfig/dashboard.cmake.in"
      "${base}/${cfg}-dashboard.cmake"
      @ONLY ESCAPE_QUOTES)
    add_test(CTestConfig.Dashboard.${cfg} ${CMAKE_CMAKE_COMMAND}
      -P "${base}/${cfg}-dashboard.cmake" -VV
      )
  endfunction()

  add_config_tests(Debug)
  add_config_tests(MinSizeRel)
  add_config_tests(Release)
  add_config_tests(RelWithDebInfo)

  # Test -S script with some -D variable definition args to ctest:
  add_test(CTestConfig.ScriptWithArgs ${CMAKE_CTEST_COMMAND}
    -C "Release"
    -D arg1=this
    -D arg2=that
    -D "arg3=the other"
    "-Darg4=this is the fourth"
    -Darg5=the_fifth
    -Darg6:STRING=value-with-type
    -S "${CMake_SOURCE_DIR}/Tests/CTestConfig/ScriptWithArgs.cmake" -VV
    --output-log "${CMake_BINARY_DIR}/Tests/CTestConfig/ScriptWithArgs.log"
    )

  ADD_TEST_MACRO(CMakeCommands.add_compile_definitions add_compile_definitions)
  ADD_TEST_MACRO(CMakeCommands.add_compile_options add_compile_options)
  ADD_TEST_MACRO(CMakeCommands.target_link_libraries target_link_libraries)
  ADD_TEST_MACRO(CMakeCommands.target_include_directories target_include_directories)
  ADD_TEST_MACRO(CMakeCommands.target_compile_definitions target_compile_definitions)
  ADD_TEST_MACRO(CMakeCommands.target_compile_options target_compile_options)
  ADD_TEST_MACRO(CMakeCommands.target_sources target_sources)

  ADD_TEST_MACRO(CMakeCommands.add_link_options)
  ADD_TEST_MACRO(CMakeCommands.target_link_options)
  ADD_TEST_MACRO(CMakeCommands.link_directories)
  ADD_TEST_MACRO(CMakeCommands.target_link_directories)

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestCrash/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestCrash/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestCrash ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestCrash/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestCrash/testOutput.log"
    )
  # With Watcom and OrangeC the SEGFAULT is not found, it just fails
  if(CMAKE_GENERATOR MATCHES "Watcom WMake" OR CMAKE_C_COMPILER_ID STREQUAL "OrangeC")
    set_tests_properties(CTestTestCrash PROPERTIES
      PASS_REGULAR_EXPRESSION "Failed")
  else()
    set_tests_properties(CTestTestCrash PROPERTIES
      PASS_REGULAR_EXPRESSION "(Illegal|SegFault|Subprocess aborted|SIGTRAP)")
  endif()

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestBadExe/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestBadExe/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestBadExe ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestBadExe/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestBadExe/testOutput.log"
    )
  set(CTestTestBadExe_REGEX "BAD_COMMAND")
  # some cygwin can not be made to produce a BAD_COMMAND error
  # the best we can get from it is a failed test
  if(CYGWIN)
    set(CTestTestBadExe_REGEX "(\\*\\*\\*Failed)|BAD_COMMAND")
  endif()
  set_tests_properties(CTestTestBadExe PROPERTIES
    PASS_REGULAR_EXPRESSION "${CTestTestBadExe_REGEX}")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestBadGenerator/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestBadGenerator/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestBadGenerator ${CMAKE_CTEST_COMMAND}
    -C "\${CTestTest_CONFIG}"
    -S "${CMake_BINARY_DIR}/Tests/CTestTestBadGenerator/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestBadGenerator/testOutput.log"
    )
  set_property(TEST CTestTestBadGenerator PROPERTY
    PASS_REGULAR_EXPRESSION "could not create generator named \"Bad Generator\"")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestParallel/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestParallel/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestParallel ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestParallel/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestParallel/testOutput.log"
    )

  configure_file("${CMake_SOURCE_DIR}/Tests/CTestTestVerboseOutput/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestVerboseOutput/test.cmake" @ONLY ESCAPE_QUOTES)
  add_test(CTestTestVerboseOutput ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestVerboseOutput/test.cmake" -VV
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestVerboseOutput/testOutput.log"
    -C "\${CTestTest_CONFIG}"
    )
  set_property(TEST CTestTestVerboseOutput PROPERTY PASS_REGULAR_EXPRESSION
    "Test command:.*Working Directory:.*Environment variables:.*foo=bar.*this=that"
  )

  ADD_TEST_MACRO(CTestTestSerialInDepends ${CMAKE_CTEST_COMMAND} -j 4
    --output-on-failure -C "\${CTestTest_CONFIG}")

  ADD_TEST_MACRO(CTestTestMissingDependsExe ${CMAKE_CTEST_COMMAND}
    --output-on-failure -C "\${CTestTest_CONFIG}")
  set_tests_properties(CTestTestMissingDependsExe PROPERTIES
    PASS_REGULAR_EXPRESSION "\\*\\*\\*Not Run"
  )

  if(NOT BORLAND)
    set(CTestLimitDashJ_CTEST_OPTIONS --force-new-ctest-process)
    add_test_macro(CTestLimitDashJ ${CMAKE_CTEST_COMMAND} -j 4
      --output-on-failure -C "\${CTestTest_CONFIG}")
  endif()

  add_test(CTestTestPrintLabels ${CMAKE_CTEST_COMMAND} --print-labels)
  set_tests_properties(CTestTestPrintLabels PROPERTIES LABELS "Label1;Label2")
  set_tests_properties(CTestTestPrintLabels PROPERTIES PASS_REGULAR_EXPRESSION
    "All Labels:.*  Label1.*  Label2")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestLabelRegExp/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestLabelRegExp/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(NAME CTestTestLabelRegExp
    COMMAND ${CMAKE_CMAKE_COMMAND}
      -DSOURCE_DIR=${CMAKE_SOURCE_DIR}/Tests/CTestTestLabelRegExp
      -P ${CMAKE_BINARY_DIR}/Tests/CTestTestLabelRegExp/test.cmake
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Tests/CTestTestLabelRegExp
  )

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestScheduler/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestScheduler/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestScheduler ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestScheduler/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestScheduler/testOutput.log"
    )
  set_tests_properties(CTestTestScheduler PROPERTIES
    PASS_REGULAR_EXPRESSION "Start 1.*Start 2.*Start 3.*Start 4.*Start 4.*Start 3.*Start 2.*Start 1"
    RESOURCE_LOCK "CostData")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestCostSerial/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestCostSerial/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestCostSerial ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestCostSerial/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestCostSerial/testOutput.log"
    )
  set_tests_properties(CTestTestCostSerial PROPERTIES
    PASS_REGULAR_EXPRESSION "Start 2.*Start 3.*Start 1.*Start 2.*Start 3.*Start 1"
    RESOURCE_LOCK "CostData")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestStopTime/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestStopTime/test.cmake"
    @ONLY ESCAPE_QUOTES)
  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestStopTime/GetDate.cmake"
    "${CMake_BINARY_DIR}/Tests/CTestTestStopTime/GetDate.cmake"
    COPYONLY)
  add_test(CTestTestStopTime ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestStopTime/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestStopTime/testOutput.log"
    )
  set_tests_properties(CTestTestStopTime PROPERTIES
    PASS_REGULAR_EXPRESSION "The stop time has been passed")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestSubdir/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestSubdir/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestSubdir ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestSubdir/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestSubdir/testOutput.log"
    )
  #make sure all 3 subdirs were added
  set_tests_properties(CTestTestSubdir PROPERTIES
    PASS_REGULAR_EXPRESSION "0 tests failed out of 3")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestTimeout/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestTimeout/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestTimeout ${CMAKE_CTEST_COMMAND}
    -C "\${CTestTest_CONFIG}"
    -S "${CMake_BINARY_DIR}/Tests/CTestTestTimeout/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestTimeout/testOutput.log"
    )
  set_tests_properties(CTestTestTimeout PROPERTIES
    PASS_REGULAR_EXPRESSION "TestTimeout *\\.+ *\\*\\*\\*Timeout.*TestSleep *\\.+ *Passed.*timeout correctly killed child")

  add_test(
    NAME CTestTestRerunFailed
    COMMAND ${CMAKE_CTEST_COMMAND} --rerun-failed)
  set_tests_properties(CTestTestRerunFailed PROPERTIES
    PASS_REGULAR_EXPRESSION "1/1 Test #1: TestTimeout" DEPENDS CTestTestTimeout
    WORKING_DIRECTORY ${CMake_BINARY_DIR}/Tests/CTestTestTimeout)

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestDepends/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestDepends/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestDepends ${CMAKE_CTEST_COMMAND}
    -C "\${CTestTest_CONFIG}"
    -S "${CMake_BINARY_DIR}/Tests/CTestTestDepends/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestDepends/testOutput.log"
    )

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestCycle/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestCycle/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestCycle ${CMAKE_CTEST_COMMAND}
    -C "\${CTestTest_CONFIG}"
    -S "${CMake_BINARY_DIR}/Tests/CTestTestCycle/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestCycle/testOutput.log"
    )
  set_tests_properties(CTestTestCycle PROPERTIES
    PASS_REGULAR_EXPRESSION "a cycle exists in the test dependency graph")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestRunScript/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestRunScript/test.cmake"
    @ONLY ESCAPE_QUOTES)
  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestRunScript/hello.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestRunScript/hello.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestRunScript ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestRunScript/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestRunScript/testOutput.log"
    )

  add_test(CTestTestShowOnly ${CMAKE_CTEST_COMMAND} -N)

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestFdSetSize/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestFdSetSize/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestFdSetSize ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestFdSetSize/test.cmake" -j20 -V --timeout 120
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestFdSetSize/testOutput.log"
    )

  if(CMAKE_TESTS_CDASH_SERVER)
    set(regex "^([^:]+)://([^/]+)(.*)$")

    if("${CMAKE_TESTS_CDASH_SERVER}" MATCHES "${regex}")
      set(protocol "${CMAKE_MATCH_1}")
      set(server "${CMAKE_MATCH_2}")
      set(path "${CMAKE_MATCH_3}")
    else()
      set(protocol "http")
      set(server "open.cdash.org")
      set(path "")
      message("warning: CMAKE_TESTS_CDASH_SERVER does not match expected regex...")
      message("         ...using default url='${protocol}://${server}${path}' for CTestTest[23]")
    endif()
  endif()


  if(CTEST_TEST_CTEST AND CMAKE_RUN_LONG_TESTS AND CMAKE_TESTS_CDASH_SERVER)
    configure_file("${CMake_SOURCE_DIR}/Tests/CTestTest/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTest/test.cmake" @ONLY ESCAPE_QUOTES)
    add_test(CTestTest ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTest/test.cmake" -V
      --output-log "${CMake_BINARY_DIR}/Tests/CTestTest/testOutput.log"
      )

    if(NOT CMake_TEST_EXTERNAL_CMAKE)
      configure_file("${CMake_SOURCE_DIR}/Tests/CTestTest2/test.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestTest2/test.cmake" @ONLY ESCAPE_QUOTES)
      add_test(NAME CTestTest2 COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG>
        -S "${CMake_BINARY_DIR}/Tests/CTestTest2/test.cmake" -V
        --output-log "${CMake_BINARY_DIR}/Tests/CTestTest2/testOutput.log"
        )
    endif()

    if("${CMAKE_GENERATOR}" MATCHES "Makefiles" OR "${CMAKE_GENERATOR}" MATCHES "Ninja")
      configure_file("${CMake_SOURCE_DIR}/Tests/CTestTestLaunchers/test.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestTestLaunchers/test.cmake" @ONLY ESCAPE_QUOTES)
      add_test(CTestTestLaunchers ${CMAKE_CTEST_COMMAND}
        -S "${CMake_BINARY_DIR}/Tests/CTestTestLaunchers/test.cmake" -V
        --output-log "${CMake_BINARY_DIR}/Tests/CTestTestLaunchers/testOutput.log"
        )
      set_tests_properties(CTestTestLaunchers PROPERTIES
        PASS_REGULAR_EXPRESSION "CTEST_TEST_LAUNCHER_SUCCESS")
    endif()

    configure_file("${CMake_SOURCE_DIR}/Tests/CTestTestChecksum/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTestChecksum/test.cmake" @ONLY
      ESCAPE_QUOTES)
    add_test(CTestTestChecksum ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTestChecksum/test.cmake" -V
      --output-log
      "${CMake_BINARY_DIR}/Tests/CTestTestChecksum/testOutput.log"
      )
    set_tests_properties(CTestTestChecksum PROPERTIES PASS_REGULAR_EXPRESSION
      "md5 mismatch")

    # these tests take a long time, make sure they have it
    # if timeouts have not already been set
    get_test_property(CTestTest TIMEOUT PREVIOUS_TIMEOUT)
    if("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
      set_tests_properties(CTestTest
        PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
    endif()

    if(NOT CMake_TEST_EXTERNAL_CMAKE)
      get_test_property(CTestTest2 TIMEOUT PREVIOUS_TIMEOUT)
      if("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
        set_tests_properties(CTestTest2
          PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
      endif()
    endif()
  endif()

  if(NOT DEFINED CMake_TEST_BOOTSTRAP)
    if(CMAKE_RUN_LONG_TESTS
        AND NOT CMAKE_SKIP_BOOTSTRAP_TEST
        AND NOT CMake_TEST_EXTERNAL_CMAKE
        AND NOT CMAKE_GENERATOR MATCHES "Xcode"
        AND NOT EXISTS "${CMake_BINARY_DIR}/CMakeLists.txt"
        )
      set(CMake_TEST_BOOTSTRAP 1)
    else()
      set(CMake_TEST_BOOTSTRAP 0)
    endif()
  endif()
  set(bootstrap "")
  if(CMake_TEST_BOOTSTRAP)
    if(UNIX)
      set(bootstrap ${CMake_SOURCE_DIR}/bootstrap)
    elseif(MSYS)
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.bat.in
                     ${CMAKE_CURRENT_BINARY_DIR}/bootstrap.bat @ONLY)
      set(bootstrap ${CMAKE_CURRENT_BINARY_DIR}/bootstrap.bat)
    endif()
  endif()
  if(bootstrap)
    add_test(NAME BootstrapTest
      COMMAND ${CMAKE_CMAKE_COMMAND}
        -D "bootstrap=${bootstrap}"
        -D "bin_dir=${CMake_BINARY_DIR}/Tests/BootstrapTest"
        -D "generator=${CMAKE_GENERATOR}"
        -P ${CMAKE_CURRENT_SOURCE_DIR}/BootstrapTest.cmake
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BootstrapTest")
    # This test will use all processors.
    set_property(TEST BootstrapTest PROPERTY RUN_SERIAL 1)
    # This test may take a long time.
    if(NOT DEFINED CMake_TEST_BOOTSTRAP_TIMEOUT)
      set(CMake_TEST_BOOTSTRAP_TIMEOUT 5400)
    endif()
    set_property(TEST BootstrapTest PROPERTY TIMEOUT "${CMake_TEST_BOOTSTRAP_TIMEOUT}")
  endif()

  if(CMAKE_Fortran_COMPILER)
    add_test(Fortran ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Fortran"
      "${CMake_BINARY_DIR}/Tests/Fortran"
      ${build_generator_args}
      --build-project testf
      --build-two-config
      --test-command testf)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Fortran")
    set_property(TEST Fortran APPEND PROPERTY LABELS "Fortran")

    if(CMAKE_Fortran_COMPILER_SUPPORTS_F90)
      add_test(FortranModules ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/FortranModules"
        "${CMake_BINARY_DIR}/Tests/FortranModules"
        ${build_generator_args}
        --build-project FortranModules
        --build-options
          -DCMake_TEST_NESTED_MAKE_PROGRAM:FILEPATH=${CMake_TEST_EXPLICIT_MAKE_PROGRAM}
          -DCMake_TEST_Fortran_SUBMODULES:BOOL=${CMake_TEST_Fortran_SUBMODULES}
          ${CMake_TEST_FortranModules_BUILD_OPTIONS}
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/FortranModules")
      set_property(TEST FortranModules APPEND PROPERTY LABELS "Fortran")
    endif()

    # FortranCInterface tests.
    if(UNIX)
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/FortranC/Flags.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/FortranC/Flags.cmake @ONLY)
      add_test(FortranC.Flags ${CMAKE_CMAKE_COMMAND} -P
        ${CMAKE_CURRENT_BINARY_DIR}/FortranC/Flags.cmake)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/FortranC/Flags")
      set_property(TEST FortranC.Flags APPEND PROPERTY LABELS "Fortran")
    else()
      add_test(FortranC ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/FortranC"
        "${CMake_BINARY_DIR}/Tests/FortranC"
        ${build_generator_args}
        --build-project FortranC
        --build-two-config
        --test-command CMakeFiles/FortranCInterface/FortranCInterface)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/FortranC")
      set_property(TEST FortranC APPEND PROPERTY LABELS "Fortran")
    endif()
  endif()

  if(NOT DEFINED CMake_TEST_Java)
    if(APPLE OR MINGW)
      set(CMake_TEST_Java 0)
    else()
      set(CMake_TEST_Java 1)
    endif()
  endif()
  if(CMake_TEST_Java)
    find_package(Java COMPONENTS Development QUIET)
  endif()
  if(Java_JAVA_EXECUTABLE AND Java_JAVAC_EXECUTABLE AND Java_JAR_EXECUTABLE)

    set(JavaExportImport_BUILD_OPTIONS -DCMake_TEST_NESTED_MAKE_PROGRAM:FILEPATH=${CMake_TEST_EXPLICIT_MAKE_PROGRAM})
    ADD_TEST_MACRO(JavaExportImport JavaExportImport)

    get_filename_component(JAVACPATH ${Java_JAVAC_EXECUTABLE} REALPATH)
    get_filename_component(JNIPATH ${JAVACPATH} PATH)
    find_file(JNI_H jni.h
      "${JNIPATH}/../include"
      "${JNIPATH}/../java/include")
    if(JNI_H AND EXISTS "${JNI_H}") # in case jni.h is a broken symlink
      file(READ "${JNI_H}" JNI_FILE)
      if("${JNI_FILE}" MATCHES "JDK1_2")

        execute_process(
          COMMAND "${Java_JAVA_EXECUTABLE}" -version
          OUTPUT_VARIABLE _version ERROR_VARIABLE _version RESULT_VARIABLE _result
          )

        # E2K has broken Java RVM before 3.5.2
        if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "e2k" AND _result EQUAL 0)
          string(REGEX MATCH "RVM ([0-9.]+)" RVMVER "${_version}")
          # Consider empty match a broken version too
          if("${CMAKE_MATCH_1}" VERSION_LESS "3.5.2")
            set(BROKEN_RVM TRUE)
          endif()
        endif()

        if(NOT BROKEN_RVM)
          add_test(NAME Java.Jar COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG>
            --build-and-test
            "${CMake_SOURCE_DIR}/Tests/Java"
            "${CMake_BINARY_DIR}/Tests/JavaJar"
            ${build_generator_args}
            --build-project hello
            --build-run-dir "${CMake_BINARY_DIR}/Tests/JavaJar/"
            --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIG>)
          list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/JavaJar")

          # For next tests, java tool must have same architecture as toolchain
          math(EXPR _object_mode "${CMAKE_SIZEOF_VOID_P} * 8")
          if(_result EQUAL 0 AND _version MATCHES "${_object_mode}-Bit")
            ## next test is valid only if Java version is less than 1.10
            if("${Java_VERSION}" VERSION_LESS 1.10)
              add_test(NAME Java.Javah COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG>
                --build-and-test
                "${CMake_SOURCE_DIR}/Tests/JavaJavah"
                "${CMake_BINARY_DIR}/Tests/JavaJavah"
                ${build_generator_args}
                --build-project helloJavah
                --build-run-dir "${CMake_BINARY_DIR}/Tests/JavaJavah/"
                --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIG>)
              list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/JavaJavah")
            endif()
            ## next test is valid only if Java is, at least, version 1.8
            if(NOT "${Java_VERSION}" VERSION_LESS 1.8)
              add_test(NAME Java.NativeHeaders COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG>
                --build-and-test
                "${CMake_SOURCE_DIR}/Tests/JavaNativeHeaders"
                "${CMake_BINARY_DIR}/Tests/JavaNativeHeaders"
                ${build_generator_args}
                --build-project helloJavaNativeHeaders
                --build-run-dir "${CMake_BINARY_DIR}/Tests/JavaNativeHeaders/"
                --build-target install
                --build-options
                "-DCMAKE_INSTALL_PREFIX:PATH=${CMake_BINARY_DIR}/Tests/JavaNativeHeaders/Install"
                --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIG>)
              list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/JavaNativeHeaders")
            endif()
          endif()
        endif()
      endif()
    endif()
  endif()

  # add some cross compiler tests, for now only with makefile based generators
  if(CMAKE_GENERATOR MATCHES "Makefiles")

    # if sdcc is found, build the SimpleCOnly project with sdcc
    find_program(SDCC_EXECUTABLE sdcc)
    mark_as_advanced(SDCC_EXECUTABLE)
    if(SDCC_EXECUTABLE)
      add_test(SimpleCOnly_sdcc ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/SimpleCOnly"
        "${CMake_BINARY_DIR}/Tests/SimpleCOnly_sdcc"
        ${build_generator_args}
        --build-project SimpleC
        --build-options
          "-DCMAKE_SYSTEM_NAME=Generic"
          "-DCMAKE_C_COMPILER=${SDCC_EXECUTABLE}")
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SimpleCOnly_sdcc")
    endif()

    # If a Linux -> MinGW cross compiler is found then try it
    find_program(MINGW_CC_LINUX2WIN_EXECUTABLE i586-mingw32msvc-gcc)
    find_program(MINGW_CXX_LINUX2WIN_EXECUTABLE i586-mingw32msvc-g++)
    find_program(MINGW_RC_LINUX2WIN_EXECUTABLE i586-mingw32msvc-windres)
    mark_as_advanced(MINGW_CC_LINUX2WIN_EXECUTABLE MINGW_CXX_LINUX2WIN_EXECUTABLE MINGW_RC_LINUX2WIN_EXECUTABLE)
    if(MINGW_CC_LINUX2WIN_EXECUTABLE AND MINGW_CXX_LINUX2WIN_EXECUTABLE AND MINGW_RC_LINUX2WIN_EXECUTABLE)
      add_test(Simple_Mingw_Linux2Win ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/Simple"
        "${CMake_BINARY_DIR}/Tests/Simple_Mingw_Linux2Win"
        ${build_generator_args}
        --build-project Simple
        --build-options
          "-DCMAKE_SYSTEM_NAME=Windows"
          "-DCMAKE_C_COMPILER=${MINGW_CC_LINUX2WIN_EXECUTABLE}"
          "-DCMAKE_CXX_COMPILER=${MINGW_CXX_LINUX2WIN_EXECUTABLE}"
          "-DCMAKE_RC_COMPILER=${MINGW_RC_LINUX2WIN_EXECUTABLE}"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_Mingw_Linux2Win")
    endif()
  endif()

  if(CMAKE_TEST_PROJECT_CSE_DIR)
    set(script "${CMAKE_TEST_PROJECT_CSE_DIR}/BuildProjectCSE.cmake")
    if(NOT EXISTS "${script}")
      set(script "${CMAKE_TEST_PROJECT_CSE_DIR}/cse_build.cmake")
    endif()
    if(NOT EXISTS "${script}")
      message("warning: CMAKE_TEST_PROJECT_CSE_DIR set, but no build script found...")
    endif()

    add_test(BuildCSE ${CMAKE_CTEST_COMMAND} -V -S "${script}")
    set_tests_properties(BuildCSE PROPERTIES TIMEOUT 5400)
  endif()

  if(CMAKE_TEST_PLPLOT_DIR)
    add_test(plplot ${CMAKE_CTEST_COMMAND} -V -S ${CMAKE_TEST_PLPLOT_DIR}/../../EasyDashboardScripts/plplot.cmake)
    set_tests_properties ( plplot PROPERTIES TIMEOUT 5400)
  endif()

  if(CMAKE_TEST_CHICKEN_DIR)
    add_test(Chicken ${CMAKE_CTEST_COMMAND} -V -S ${CMAKE_TEST_CHICKEN_DIR}/../../EasyDashboardScripts/Chicken.cmake)
    set_tests_properties(Chicken PROPERTIES TIMEOUT 5400)
  endif()

  if(CMAKE_TEST_KDELIBS_ALPHA_1_DIR)
    add_test(KDELibsAlpha1 ${CMAKE_CTEST_COMMAND} -V -S ${CMAKE_TEST_KDELIBS_ALPHA_1_DIR}/../../EasyDashboardScripts/kdelibs.cmake)
    set_tests_properties(KDELibsAlpha1 PROPERTIES TIMEOUT 5400)
  endif()

  # Define a set of "contract" tests, each activated by a cache entry
  # named "CMake_TEST_CONTRACT_<project>".  For each Contract test,
  # the project should provide a directory with a CMakeLists.txt file
  # that uses ExternalProject to download and configure the project.
  # The directory should also contain a Configure.cmake file that
  # sets "CMake_TEST_CONTRACT_<project>_<var>" variables to configure
  # the code below.
  foreach(project IN ITEMS PLplot Trilinos VTK)
    if(CMake_TEST_CONTRACT_${project})
      include(Contracts/${project}/Configure.cmake)
      ADD_TEST_MACRO(Contracts.${project} ${CMake_TEST_CONTRACT_${project}_RUN_TEST})
      # The external projects may take a long time to build.
      if(DEFINED CMake_TEST_CONTRACT_${project}_TIMEOUT)
        set(timeout ${CMake_TEST_CONTRACT_${project}_TIMEOUT})
      elseif(CMake_TEST_CONTRACT_DEFAULT_TIMEOUT)
        set(timeout ${CMake_TEST_CONTRACT_DEFAULT_TIMEOUT})
      else()
        set(timeout 21600)
      endif()
      set_property(TEST Contracts.${project} PROPERTY TIMEOUT "${timeout}")
    endif()
  endforeach()

  if(TEST_CompileCommandOutput)
    set(CompileCommandOutput_BUILD_OPTIONS
      -DMAKE_SUPPORTS_SPACES=${MAKE_SUPPORTS_SPACES})
    ADD_TEST_MACRO(CompileCommandOutput
      "${CMake_BINARY_DIR}/Tests/CMakeLib/runcompilecommands")
  endif()

  add_test(IncludeDirectories ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/IncludeDirectories"
    "${CMake_BINARY_DIR}/Tests/IncludeDirectories"
    --build-two-config
    ${build_generator_args}
    --build-project IncludeDirectories
    --build-options
      -DMAKE_SUPPORTS_SPACES=${MAKE_SUPPORTS_SPACES}
    --test-command IncludeDirectories)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/IncludeDirectories")

  if(CMAKE_GENERATOR MATCHES "^((Unix|MSYS) Makefiles|Ninja)$" AND
     ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.4)
      OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
      OR (CMAKE_CXX_COMPILER_ID STREQUAL "LCC")
      OR (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")))
    add_test(IncludeDirectoriesCPATH ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/IncludeDirectoriesCPATH"
      "${CMake_BINARY_DIR}/Tests/IncludeDirectoriesCPATH"
      --build-two-config
      ${build_generator_args}
      --build-project IncludeDirectoriesCPATH)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/IncludeDirectoriesCPATH")
    set_tests_properties(IncludeDirectoriesCPATH
      PROPERTIES
        ENVIRONMENT "CPATH=${CMAKE_CURRENT_SOURCE_DIR}/IncludeDirectoriesCPATH/viacpath")
  endif()

  add_test(InterfaceLinkLibraries ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/InterfaceLinkLibraries"
    "${CMake_BINARY_DIR}/Tests/InterfaceLinkLibraries"
    --build-two-config
    ${build_generator_args}
    --build-project InterfaceLinkLibraries
    --test-command InterfaceLinkLibraries)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/InterfaceLinkLibraries")

  ADD_TEST_MACRO(InterfaceLinkLibrariesDirect)

  if(NOT CMake_TEST_EXTERNAL_CMAKE)
    add_subdirectory(CMakeTests)
  endif()

  if(BUILD_QtDialog AND CMake_TEST_GUI AND NOT CMake_TEST_EXTERNAL_CMAKE)
    add_subdirectory(CMakeGUI)
  endif()

  # Run CheckSourceTree as the very last test in the CMake/CTest/CPack test
  # suite. It detects if any changes have been made to the CMake source tree
  # by any previous configure, build or test steps.
  if(GIT_EXECUTABLE AND EXISTS "${CMake_SOURCE_DIR}/.git"
      AND NOT "${CMake_SOURCE_DIR}" STREQUAL "${CMake_BINARY_DIR}")
    add_subdirectory(CheckSourceTree)
  endif()

  # If this is not an in-source build, provide a target to wipe out
  # all the test build directories. This must come at the end after
  # all the above logic has finished adding to TEST_BUILD_DIRS
  if(NOT EXISTS "${CMake_BINARY_DIR}/CMakeLists.txt")
    configure_file(${CMake_SOURCE_DIR}/Tests/test_clean.cmake.in
                   ${CMake_BINARY_DIR}/Tests/test_clean.cmake @ONLY)
    add_custom_target(test_clean
      COMMAND ${CMAKE_COMMAND} -P ${CMake_BINARY_DIR}/Tests/test_clean.cmake
      COMMENT "Removing test build directories."
      )
  endif()
endif()
