include(RunCMake)

run_cmake(7zip)
run_cmake(gnutar)
run_cmake(gnutar-gz)
run_cmake(pax)
run_cmake(pax-xz)
run_cmake(pax-zstd)
run_cmake(paxr)
run_cmake(paxr-bz2)
run_cmake(zip)

# Extracting only selected files or directories
run_cmake(zip-filtered)

run_cmake(unsupported-format)
run_cmake(zip-with-bad-compression)
run_cmake(7zip-with-bad-compression)
