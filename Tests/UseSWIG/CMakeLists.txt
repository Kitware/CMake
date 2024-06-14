find_package(SWIG QUIET)

add_test(NAME UseSWIG.LegacyPython COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/LegacyPython"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/LegacyPython"
  ${build_generator_args}
  --build-project TestLegacyPython
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
add_test(NAME UseSWIG.LegacyPerl COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/LegacyPerl"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/LegacyPerl"
  ${build_generator_args}
  --build-project TestLegacyPerl
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
add_test(NAME UseSWIG.LegacyPerl5 COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/LegacyPerl"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/LegacyPerl5"
  ${build_generator_args}
  --build-project TestLegacyPerl
  --build-options ${build_options} -Dlanguage=perl5
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

include(CheckLanguage)
check_language(CSharp)
if (CMAKE_CSharp_COMPILER)
  add_test(NAME UseSWIG.BasicCsharp COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/UseSWIG/BasicCsharp"
    "${CMake_BINARY_DIR}/Tests/UseSWIG/BasicCsharp"
    ${build_generator_args}
    --build-project TestBasicCsharp
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
endif()
add_test(NAME UseSWIG.NamespaceCsharp COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/NamespaceCsharp"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/NamespaceCsharp"
  ${build_generator_args}
  --build-project TestNamespaceCsharp
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

add_test(NAME UseSWIG.BasicPython COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/BasicPython"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/BasicPython"
  ${build_generator_args}
  --build-project TestBasicPython
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
add_test(NAME UseSWIG.BasicPerl COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/BasicPerl"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/BasicPerl"
  ${build_generator_args}
  --build-project TestBasicPerl
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
add_test(NAME UseSWIG.BasicPerl5 COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/BasicPerl"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/BasicPerl5"
  ${build_generator_args}
  --build-project TestBasicPerl
  --build-options ${build_options} -Dlanguage=perl5
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
if(SWIG_FOUND AND NOT SWIG_VERSION VERSION_LESS "4.0.2"
    AND CMAKE_GENERATOR MATCHES "Make|Ninja|Xcode|Visual Studio (1[1-9]|[2-9][0-9])")
  add_test(NAME UseSWIG.Depfile.BasicPython COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/UseSWIG/BasicPython"
    "${CMake_BINARY_DIR}/Tests/UseSWIG/BasicPython.Depfile"
    ${build_generator_args}
    --build-project TestBasicPython
    --build-options ${build_options} -DSWIG_USE_SWIG_DEPENDENCIES=ON
                                     "-DSWIG_OUTFILE_DIR=${CMake_BINARY_DIR}/Tests/UseSWIG/BasicPython.Depfile"
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME UseSWIG.Depfile.BasicPerl COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/UseSWIG/BasicPerl"
    "${CMake_BINARY_DIR}/Tests/UseSWIG/BasicPerl.Depfile"
    ${build_generator_args}
    --build-project TestBasicPerl
    --build-options ${build_options} -DSWIG_USE_SWIG_DEPENDENCIES=ON
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME UseSWIG.Depfile.BasicPerl5 COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/UseSWIG/BasicPerl"
    "${CMake_BINARY_DIR}/Tests/UseSWIG/BasicPerl5.Depfile"
    ${build_generator_args}
    --build-project TestBasicPerl
    --build-options ${build_options} -DSWIG_USE_SWIG_DEPENDENCIES=ON -Dlanguage=perl5
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
endif()

if (CMake_TEST_UseSWIG_Fortran)
  check_language(Fortran)
  if (CMAKE_Fortran_COMPILER)
    add_test(NAME UseSWIG.BasicFortran COMMAND
      ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/UseSWIG/BasicFortran"
      "${CMake_BINARY_DIR}/Tests/UseSWIG/BasicFortran"
      ${build_generator_args}
      --build-project TestBasicFortran
      --build-options ${build_options}
      --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
      )
    set_property(TEST UseSWIG.BasicFortran APPEND PROPERTY LABELS "Fortran")
  endif()
endif()

add_test(NAME UseSWIG.MultipleModules COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/MultipleModules"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/MultipleModules"
  ${build_generator_args}
  --build-project TestMultipleModules
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
if(CMake_TEST_FindPython2 AND CMake_TEST_FindPython3)
  add_test(NAME UseSWIG.MultiplePython COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/UseSWIG/MultiplePython"
    "${CMake_BINARY_DIR}/Tests/UseSWIG/MultiplePython"
    ${build_generator_args}
    --build-project TestMultiplePython
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
endif()
add_test(NAME UseSWIG.MultipleFiles COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/MultipleFiles"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/MultipleFiles"
  ${build_generator_args}
  --build-project TestMultipleFiles
  --build-options ${build_options}
)

if(CMake_TEST_FindPython2 OR CMake_TEST_FindPython3)
  add_test(NAME UseSWIG.ModuleVersion2 COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/UseSWIG/ModuleVersion2"
    "${CMake_BINARY_DIR}/Tests/UseSWIG/ModuleVersion2"
    ${build_generator_args}
    --build-project TestModuleVersion2
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
endif()

add_test(NAME UseSWIG.UseTargetINCLUDE_DIRECTORIES COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/UseTargetINCLUDE_DIRECTORIES"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/UseTargetINCLUDE_DIRECTORIES"
  ${build_generator_args}
  --build-project TestUseTargetINCLUDE_DIRECTORIES
  --build-options ${build_options}
  )


add_test(NAME UseSWIG.ModuleName COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/ModuleName"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/ModuleName"
  ${build_generator_args}
  --build-project TestModuleName
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )


add_test(NAME UseSWIG.SwigSrcFileExtension COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/SwigSrcFileExtension"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/SwigSrcFileExtension"
  ${build_generator_args}
  --build-project SwigSrcFileExtension
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )


add_test(NAME UseSWIG.AlternateLibraryName COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/AlternateLibraryName"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/AlternateLibraryName"
  ${build_generator_args}
  --build-project TestAlternateLibraryName
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )


add_test(NAME UseSWIG.SwigSrcOUTPUT_DIR COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/UseSWIG/SwigSrcOUTPUT_DIR"
  "${CMake_BINARY_DIR}/Tests/UseSWIG/SwigSrcOUTPUT_DIR"
  ${build_generator_args}
  --build-project TestSwigSrcOUTPUT_DIR
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
