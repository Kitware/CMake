# Pretend the ABI check failed in order to force the fall-back test to run.
set(CMAKE_HIP_ABI_COMPILED FALSE)
enable_language(HIP)
