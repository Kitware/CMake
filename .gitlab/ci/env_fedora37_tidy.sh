cmake \
  -S Utilities/ClangTidyModule \
  -B Utilities/ClangTidyModule/build \
  -DCMAKE_BUILD_TYPE=Release \
  -DRUN_TESTS=ON
cmake --build Utilities/ClangTidyModule/build
ctest --test-dir Utilities/ClangTidyModule/build --output-on-failure
