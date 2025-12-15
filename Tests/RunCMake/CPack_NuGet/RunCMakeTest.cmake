include(RunCPack) # Uses sample projects from `../RunCPack/*`

set(env_PATH "$ENV{PATH}")

set(RunCPack_GENERATORS NuGet)

run_cpack(NuGetLib)
run_cpack(NuGetSymbol)
