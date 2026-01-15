file(WRITE "/tmp/cmake_fuzz_test.txt" "test content")
file(READ "/tmp/cmake_fuzz_test.txt" content)
file(REMOVE "/tmp/cmake_fuzz_test.txt")
file(GLOB files "/tmp/*.txt")
