# Pretend the ABI check failed in order to force the fall-back test to run.
set(CMAKE_C_ABI_COMPILED FALSE)
enable_language(C)
