include(RunCPack)

set(env_PATH "$ENV{PATH}")

set(RunCPack_GENERATORS NuGet)

run_cpack(NuGetLib)
