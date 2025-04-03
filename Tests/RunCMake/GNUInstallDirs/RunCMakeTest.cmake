include(RunCMake)

if(SYSTEM_NAME STREQUAL "FreeBSD")
  set(variant "-FreeBSD")
elseif(SYSTEM_NAME MATCHES "^(([^k].*)?BSD|DragonFly)$")
  set(variant "-BSD")
elseif(EXISTS "/etc/debian_version")
  set(variant "-Debian")
else()
  set(variant "")
endif()

foreach(case
    Opt
    Root
    Usr
    UsrLocal
    )
  set(RunCMake-stderr-file ${case}${variant}-stderr.txt)
  run_cmake(${case})
  unset(RunCMake-stderr-file)
endforeach()

block()
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/Reconfigure-build")
  set(RunCMake-stderr-file UsrLocal${variant}-stderr.txt)
  run_cmake_with_options(Reconfigure)
  set(RunCMake_TEST_NO_CLEAN 1)
  foreach(case
      Opt
      Root
      Usr
      UsrLocal
  )
    set(RunCMake-stderr-file ${case}${variant}-stderr.txt)
    run_cmake_with_options(Reconfigure -Dcase=${case})
  endforeach()
endblock()

run_cmake(GetAbs)
run_cmake(NoSystem)
