set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
# Pretend the ABI check failed in order to force the fall-back test to run.
set(CMAKE_SYCL_ABI_COMPILED FALSE)
enable_language(SYCL)
