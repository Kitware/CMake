export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

export CFLAGS=-fsanitize=fuzzer-no-link,address
export CXXFLAGS=-fsanitize=fuzzer-no-link,address
