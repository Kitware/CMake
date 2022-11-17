# Pretend the ABI check failed in order to force the fall-back test to run.
set(CMAKE_OBJC_ABI_COMPILED FALSE)
enable_language(OBJC)
