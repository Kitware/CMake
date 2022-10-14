cmake -S Utilities/ClangTidyModule -B Utilities/ClangTidyModule/build
cmake --build Utilities/ClangTidyModule/build
ctest --test-dir Utilities/ClangTidyModule/build
