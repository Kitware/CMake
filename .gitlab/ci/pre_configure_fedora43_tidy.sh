cmake \
  -G Ninja \
  -S Utilities/ClangTidyModule \
  -B Utilities/ClangTidyModule/build \
  -DCMAKE_BUILD_TYPE=Release \
  -DRUN_TESTS=ON \
  -DCMAKE_CXX_COMPILER_LAUNCHER=sccache
cmake --build Utilities/ClangTidyModule/build
ctest --test-dir Utilities/ClangTidyModule/build --output-on-failure
