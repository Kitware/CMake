set(failures "")

foreach(f explicit_root_sbom explicit_subdir_sbom)
  if(NOT EXISTS "${RunCMake_TEST_INSTALL_DIR}/${f}.spdx.json")
    list(APPEND failures "expected explicit SBOM ${f}.spdx.json to exist")
  endif()
endforeach()

foreach(f implicit_root implicit_subdir)
  if(NOT EXISTS "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/test_project/${f}.spdx.json")
    list(APPEND failures "expected autogen SBOM ${f}.spdx.json to exist")
  endif()
endforeach()

foreach(f explicit_root explicit_subdir)
  if(EXISTS "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/test_project/${f}.spdx.json")
    list(APPEND failures "autogen wrongly produced ${f}.spdx.json (should be suppressed by explicit install(SBOM))")
  endif()
endforeach()

if(failures)
  string(REPLACE ";" "\n  " msg "${failures}")
  set(RunCMake_TEST_FAILED "${msg}")
endif()
