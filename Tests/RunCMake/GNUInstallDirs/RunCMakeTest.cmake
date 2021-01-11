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

run_cmake(GetAbs)
run_cmake(NoSystem)
