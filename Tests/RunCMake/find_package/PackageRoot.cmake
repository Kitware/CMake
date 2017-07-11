cmake_policy(SET CMP0057 NEW)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/PackageRoot)
set(PackageRoot_BASE ${CMAKE_CURRENT_SOURCE_DIR}/PackageRoot)

macro(CleanUpPackageRootTest)
  unset(Foo_ROOT)
  unset(ENV{Foo_ROOT})
  unset(Bar_ROOT)
  unset(ENV{Bar_ROOT})
  unset(FOO_TEST_FILE_FOO)
  unset(FOO_TEST_PATH_FOO)
  unset(FOO_TEST_PROG_FOO)
  unset(BAR_TEST_FILE_FOO)
  unset(BAR_TEST_FILE_BAR)
  unset(BAR_TEST_PATH_FOO)
  unset(BAR_TEST_PATH_BAR)
  unset(BAR_TEST_PROG_FOO)
  unset(BAR_TEST_PROG_BAR)
  unset(FOO_TEST_FILE_FOO CACHE)
  unset(FOO_TEST_PATH_FOO CACHE)
  unset(FOO_TEST_PROG_FOO CACHE)
  unset(BAR_TEST_FILE_FOO CACHE)
  unset(BAR_TEST_FILE_BAR CACHE)
  unset(BAR_TEST_PATH_FOO CACHE)
  unset(BAR_TEST_PATH_BAR CACHE)
  unset(BAR_TEST_PROG_FOO CACHE)
  unset(BAR_TEST_PROG_BAR CACHE)
endmacro()

macro(RunPackageRootTest)
  set(orig_foo_cmake_root ${Foo_ROOT})
  set(orig_foo_env_root   $ENV{Foo_ROOT})
  set(orig_bar_cmake_root ${Bar_ROOT})
  set(orig_bar_env_root   $ENV{Bar_ROOT})

  find_package(Foo)
  message("Foo_ROOT      :${Foo_ROOT}")
  message("ENV{Foo_ROOT} :$ENV{Foo_ROOT}")
  message("FOO_TEST_FILE_FOO :${FOO_TEST_FILE_FOO}")
  message("FOO_TEST_FILE_ZOT :${FOO_TEST_FILE_ZOT}")
  message("FOO_TEST_PATH_FOO :${FOO_TEST_PATH_FOO}")
  message("FOO_TEST_PATH_ZOT :${FOO_TEST_PATH_ZOT}")
  message("FOO_TEST_PROG_FOO :${FOO_TEST_PROG_FOO}")
  CleanUpPackageRootTest()
  message("")

  set(Foo_ROOT      ${orig_foo_cmake_root})
  set(ENV{Foo_ROOT} ${orig_foo_env_root})
  set(Bar_ROOT      ${orig_bar_cmake_root})
  set(ENV{Bar_ROOT} ${orig_bar_env_root})

  find_package(Foo COMPONENTS Bar)
  message("Foo_ROOT      :${Foo_ROOT}")
  message("ENV{Foo_ROOT} :$ENV{Foo_ROOT}")
  message("Bar_ROOT      :${Bar_ROOT}")
  message("ENV{Bar_ROOT} :$ENV{Bar_ROOT}")
  message("FOO_TEST_FILE_FOO :${FOO_TEST_FILE_FOO}")
  message("FOO_TEST_PATH_FOO :${FOO_TEST_PATH_FOO}")
  message("FOO_TEST_PROG_FOO :${FOO_TEST_PROG_FOO}")
  message("BAR_TEST_FILE_FOO :${BAR_TEST_FILE_FOO}")
  message("BAR_TEST_FILE_BAR :${BAR_TEST_FILE_BAR}")
  message("BAR_TEST_FILE_ZOT :${BAR_TEST_FILE_ZOT}")
  message("BAR_TEST_PATH_FOO :${BAR_TEST_PATH_FOO}")
  message("BAR_TEST_PATH_BAR :${BAR_TEST_PATH_BAR}")
  message("BAR_TEST_PATH_ZOT :${BAR_TEST_PATH_ZOT}")
  message("BAR_TEST_PROG_FOO :${BAR_TEST_PROG_FOO}")
  message("BAR_TEST_PROG_BAR :${BAR_TEST_PROG_BAR}")
  CleanUpPackageRootTest()
  message("")

  unset(orig_foo_cmake_root)
  unset(orig_foo_env_root)
  unset(orig_bar_cmake_root)
  unset(orig_bar_env_root)
endmacro()

RunPackageRootTest()

set(Foo_ROOT      ${PackageRoot_BASE}/foo/cmake_root)
RunPackageRootTest()

set(ENV{Foo_ROOT} ${PackageRoot_BASE}/foo/env_root)
RunPackageRootTest()

set(Foo_ROOT      ${PackageRoot_BASE}/foo/cmake_root)
set(ENV{Foo_ROOT} ${PackageRoot_BASE}/foo/env_root)
RunPackageRootTest()

##

set(Bar_ROOT      ${PackageRoot_BASE}/bar/cmake_root)
RunPackageRootTest()

set(ENV{Bar_ROOT} ${PackageRoot_BASE}/bar/env_root)
RunPackageRootTest()

set(Bar_ROOT      ${PackageRoot_BASE}/bar/cmake_root)
set(ENV{Bar_ROOT} ${PackageRoot_BASE}/bar/env_root)
RunPackageRootTest()

##

set(Foo_ROOT      ${PackageRoot_BASE}/foo/cmake_root)
set(Bar_ROOT      ${PackageRoot_BASE}/bar/cmake_root)
RunPackageRootTest()

set(ENV{Foo_ROOT} ${PackageRoot_BASE}/foo/env_root)
set(Bar_ROOT      ${PackageRoot_BASE}/bar/cmake_root)
RunPackageRootTest()

set(Foo_ROOT      ${PackageRoot_BASE}/foo/cmake_root)
set(ENV{Foo_ROOT} ${PackageRoot_BASE}/foo/env_root)
set(Bar_ROOT      ${PackageRoot_BASE}/bar/cmake_root)
RunPackageRootTest()

##

set(Foo_ROOT      ${PackageRoot_BASE}/foo/cmake_root)
set(ENV{Bar_ROOT} ${PackageRoot_BASE}/bar/env_root)
RunPackageRootTest()

set(ENV{Foo_ROOT} ${PackageRoot_BASE}/foo/env_root)
set(ENV{Bar_ROOT} ${PackageRoot_BASE}/bar/env_root)
RunPackageRootTest()

set(Foo_ROOT      ${PackageRoot_BASE}/foo/cmake_root)
set(ENV{Foo_ROOT} ${PackageRoot_BASE}/foo/env_root)
set(ENV{Bar_ROOT} ${PackageRoot_BASE}/bar/env_root)
RunPackageRootTest()

##

set(Foo_ROOT      ${PackageRoot_BASE}/foo/cmake_root)
set(Bar_ROOT      ${PackageRoot_BASE}/bar/cmake_root)
set(ENV{Bar_ROOT} ${PackageRoot_BASE}/bar/env_root)
RunPackageRootTest()

set(ENV{Foo_ROOT} ${PackageRoot_BASE}/foo/env_root)
set(Bar_ROOT      ${PackageRoot_BASE}/bar/cmake_root)
set(ENV{Bar_ROOT} ${PackageRoot_BASE}/bar/env_root)
RunPackageRootTest()

set(Foo_ROOT      ${PackageRoot_BASE}/foo/cmake_root)
set(ENV{Foo_ROOT} ${PackageRoot_BASE}/foo/env_root)
set(Bar_ROOT      ${PackageRoot_BASE}/bar/cmake_root)
set(ENV{Bar_ROOT} ${PackageRoot_BASE}/bar/env_root)
RunPackageRootTest()
