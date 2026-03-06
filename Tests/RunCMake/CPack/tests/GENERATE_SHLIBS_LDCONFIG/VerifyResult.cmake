find_program(DPKG_EXECUTABLE dpkg)
set(_dpkg_is_old FALSE)
if(DPKG_EXECUTABLE)
  execute_process(
    COMMAND "${DPKG_EXECUTABLE}" --version
    OUTPUT_VARIABLE DPKG_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(_dpkg_version_re "Debian 'dpkg' package management program version ([0-9]+\.[0-9]+\.[0-9]+).*")
  if(DPKG_VERSION MATCHES "${_dpkg_version_re}")
    set(DPKG_VERSION "${CMAKE_MATCH_1}")
    if(DPKG_VERSION VERSION_LESS 1.18.3)
      set(_dpkg_is_old TRUE)
    endif()
  else()
    set(_dpkg_is_old TRUE)
  endif()
else()
    set(_dpkg_is_old TRUE)
endif()

set(expected_files shlibs)
set(shlibs_shlibs "^libtest_lib 0\\.8 generate_shlibs_ldconfig \\(>\\= 0\\.1\\.1\\)\n$")
# NOTE: optional dot at the end of permissions regex is for SELinux enabled systems
set(shlibs_shlibs_permissions_regex "-rw-r--r--\.? .*")
if(_dpkg_is_old)
  list(APPEND expected_files postinst postrm)
  set(shlibs_postinst ".*ldconfig.*")
  set(shlibs_postinst_permissions_regex "-rwxr-xr-x\.? .*")

  set(shlibs_postrm ".*ldconfig.*")
  set(shlibs_postrm_permissions_regex "-rwxr-xr-x\.? .*")
else()
  list(APPEND expected_files triggers)
  set(triggers_postrm ".*ldconfig.*")
  set(triggers_postrm_permissions_regex "-rw-r--r--\.? .*")
endif()
verifyDebControl("${FOUND_FILE_1}" "shlibs" "${expected_files}")
