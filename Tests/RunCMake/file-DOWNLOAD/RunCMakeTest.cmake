include(RunCMake)

# We do not contact any real URLs, but do try a bogus one.
# Remove any proxy configuration that may change behavior.
unset(ENV{http_proxy})
unset(ENV{https_proxy})

run_cmake(hash-mismatch)
run_cmake(unused-argument)
run_cmake(httpheader-not-set)
run_cmake(netrc-bad)
run_cmake(tls-cainfo-not-set)
run_cmake(tls-verify-not-set)
run_cmake(pass-not-set)
run_cmake(no-save-hash)

run_cmake(basic)
run_cmake(EXPECTED_HASH)
run_cmake(file-without-path)
run_cmake(no-file)
run_cmake(range)
run_cmake(SHOW_PROGRESS)

if(NOT CMake_TEST_NO_NETWORK)
  run_cmake(bad-hostname)
endif()

if(CMake_TEST_TLS_VERIFY_URL)
  run_cmake(TLS_VERIFY-bad)
  run_cmake_with_options(TLS_VERIFY-good -Durl=${CMake_TEST_TLS_VERIFY_URL})
endif()
