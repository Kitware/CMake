include(RunCMake)

run_cmake(Example)
run_cmake(VERSION-SKIP)
# run_cmake(VERSION-KEEP) # Enable when 3.0.0 is released
run_cmake(PROPERTY-SKIP)
run_cmake(PROPERTY-KEEP)
run_cmake(ARGUMENT-SKIP)
run_cmake(ARGUMENT-KEEP)
