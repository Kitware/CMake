include(RunCMake)

if (DEFINED with_qt_version)
  set(RunCMake_TEST_OPTIONS
    -Dwith_qt_version=${with_qt_version}
    "-DQt${with_qt_version}_DIR:PATH=${Qt${with_qt_version}_DIR}"
    "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
  )
  run_cmake(AutoMocIncludeDirectories)
  if (RunCMake_GENERATOR MATCHES "(Ninja|Makefiles|Visual Studio)")
    run_cmake(AutoMocIncludeDirectoriesShort)
  endif ()

  # Detect information from the toolchain:
  # - CMAKE_CXX_COMPILE_FEATURES
  # - CMAKE_MAKE_PROGRAM
  run_cmake(Inspect)
  include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

  # Building C++ module units at all needs a toolchain that can scan C++
  # module dependencies and a generator that can use the result.  The scandep
  # rule covers the compiler side: it is absent both for compilers that cannot
  # scan at all, such as AppleClang, and for versions that are too old.
  set(cxx_modules_supported 0)
  if (have_cxx_scandep AND "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    if (RunCMake_GENERATOR MATCHES "Ninja")
      execute_process(
        COMMAND "${CMAKE_MAKE_PROGRAM}" --version
        RESULT_VARIABLE _res
        OUTPUT_VARIABLE _ninja_version
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
      if (NOT _res AND _ninja_version VERSION_GREATER_EQUAL "1.11")
        set(cxx_modules_supported 1)
      endif ()
    elseif (RunCMake_GENERATOR MATCHES "Visual Studio")
      set(cxx_modules_supported 1)
    endif ()
  endif ()

  # AUTOMOC processes C++ module units only with Qt 6.13 or newer, whose moc
  # supports them.
  set(automoc_modules_supported 0)
  if (cxx_modules_supported AND QtCore_VERSION VERSION_GREATER_EQUAL "6.13")
    set(automoc_modules_supported 1)
  endif ()

  # A successful build IS the assertion: main.cpp uses the module classes, so
  # if AUTOMOC did not moc the module units into their own module-attached
  # translation units, the meta-object symbols would be undefined at link time.
  # This exercises moc's compiler-predefines (--include) plus module-declaration
  # handling, which only the C++-module-capable Clang/MSVC path reaches (GCC is
  # skipped in the project due to QTBUG-142513).
  if (automoc_modules_supported)
    block()
      set(RunCMake_TEST_SOURCE_DIR "${RunCMake_SOURCE_DIR}/cxx_modules")
      set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/CxxModules-build")
      run_cmake_with_options(CxxModules ${RunCMake_TEST_OPTIONS})
      set(RunCMake_TEST_NO_CLEAN 1)
      run_cmake_command(CxxModules-build ${CMAKE_COMMAND} --build . --config Debug)
    endblock()
  endif ()

  # Toggling a module unit's macro must not require a reconfigure: autogen's
  # depfile lists module-unit sources so ninja rescans them on rebuild.  Use
  # a writable copy of the sources so the edit never touches the tracked
  # test files (mirrors how Autogen_6's incremental test avoids that).
  if (automoc_modules_supported)
    block()
      set(incremental_src_dir "${RunCMake_BINARY_DIR}/CxxModulesIncremental-src")
      file(REMOVE_RECURSE "${incremental_src_dir}")
      file(COPY "${RunCMake_SOURCE_DIR}/cxx_modules/" DESTINATION "${incremental_src_dir}")

      set(RunCMake_TEST_SOURCE_DIR "${incremental_src_dir}")
      set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/CxxModulesIncremental-build")
      run_cmake_with_options(CxxModulesIncremental ${RunCMake_TEST_OPTIONS})
      set(RunCMake_TEST_NO_CLEAN 1)
      run_cmake_command(CxxModulesIncremental-build1 ${CMAKE_COMMAND} --build . --config Debug)

      file(WRITE "${incremental_src_dir}/mod-empty.cppm" [[
module;
#include <QObject>
export module Mod:Empty;

export class EmptyObject : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
signals:
    void emptySignal(int value);
};
]])

      run_cmake_command(CxxModulesIncremental-build2 ${CMAKE_COMMAND} --build . --config Debug)
    endblock()
  endif ()

  # moc rejects a meta-object macro in a module implementation unit, and
  # AUTOMOC does not work around that.  Such a unit is not a member of the
  # CXX_MODULES file set, so it goes through the ordinary source handling and
  # its ".moc" include is honored; moc then fails.
  if (automoc_modules_supported)
    block()
      set(RunCMake_TEST_SOURCE_DIR "${RunCMake_SOURCE_DIR}/cxx_modules_impl_unit")
      set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/CxxModulesImplUnit-build")
      run_cmake_with_options(CxxModulesImplUnit ${RunCMake_TEST_OPTIONS})
      set(RunCMake_TEST_NO_CLEAN 1)
      run_cmake_command(CxxModulesImplUnit-build ${CMAKE_COMMAND} --build . --config Debug)
    endblock()
  endif ()

  # A C++ module unit without a meta-object macro must not disturb AUTOMOC,
  # regardless of the Qt version: nothing is generated for the module unit,
  # while the ordinary header of the target is still moc'd.  The module unit
  # pulls in no Qt, so this also covers compilers that cannot yet compile
  # QObject into a module unit.
  if (cxx_modules_supported)
    block()
      set(RunCMake_TEST_SOURCE_DIR "${RunCMake_SOURCE_DIR}/cxx_modules_no_macro")
      set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/CxxModulesNoMacro-build")
      run_cmake_with_options(CxxModulesNoMacro ${RunCMake_TEST_OPTIONS})
      set(RunCMake_TEST_NO_CLEAN 1)
      run_cmake_command(CxxModulesNoMacro-build ${CMAKE_COMMAND} --build . --config Debug)
    endblock()
  endif ()

  # With older Qt, moc cannot process C++ module units at all.  A meta-object
  # macro in one must be reported as such, rather than left to fail later in
  # the compiler or the linker.  The project still configures; only the build
  # is expected to fail.
  if (cxx_modules_supported AND QtCore_VERSION VERSION_LESS "6.13")
    block()
      set(RunCMake_TEST_SOURCE_DIR "${RunCMake_SOURCE_DIR}/cxx_modules")
      set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/CxxModulesUnsupportedQt-build")
      run_cmake_with_options(CxxModulesUnsupportedQt ${RunCMake_TEST_OPTIONS})
      set(RunCMake_TEST_NO_CLEAN 1)
      run_cmake_command(CxxModulesUnsupportedQt-build ${CMAKE_COMMAND} --build . --config Debug)
    endblock()
  endif ()
endif()
