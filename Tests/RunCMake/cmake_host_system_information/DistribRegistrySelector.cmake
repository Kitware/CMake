# FROM_SYSROOT before QUERY's content makes WINDOWS_REGISTRY no longer the
# registry signature, so it is rejected as an unknown <key>.  This guards the
# registry-first dispatch: registry queries never see FROM_SYSROOT.
cmake_host_system_information(RESULT r QUERY FROM_SYSROOT ON WINDOWS_REGISTRY "HKLM")
