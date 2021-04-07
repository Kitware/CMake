include(RunCMake)

set(ENV{CFLAGS} "$ENV{CFLAGS} -Denv=\"a\\b\"")
run_cmake(C)

set(ENV{CXXFLAGS} "$ENV{CXXFLAGS} -Denv=\"a\\b\"")
run_cmake(CXX)
