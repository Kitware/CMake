include(RunCMake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")

set(test_3_8_0 1)
if(CMake_TEST_LibArchive_VERSION AND CMake_TEST_LibArchive_VERSION VERSION_LESS 3.8.0)
  set(test_3_8_0 0)
endif()

function(external_command_test NAME)
  run_cmake_command(${NAME} ${CMAKE_COMMAND} -E ${ARGN})
endfunction()

external_command_test(without-files      tar cvf bad.tar)
external_command_test(bad-opt1           tar cvf bad.tar --bad)
external_command_test(bad-mtime1         tar cvf bad.tar --mtime=bad .)
external_command_test(bad-from1          tar cvf bad.tar --files-from=bad)
external_command_test(bad-from2          tar cvf bad.tar --files-from=.)
external_command_test(bad-from3          tar cvf bad.tar --files-from=${CMAKE_CURRENT_LIST_DIR}/bad-from3.txt)
external_command_test(bad-from4          tar cvf bad.tar --files-from=${CMAKE_CURRENT_LIST_DIR}/bad-from4.txt)
external_command_test(bad-from5          tar cvf bad.tar --files-from=${CMAKE_CURRENT_LIST_DIR}/bad-from5.txt)
external_command_test(bad-file           tar cf  bad.tar badfile.txt ${CMAKE_CURRENT_LIST_DIR}/test-file.txt)
external_command_test(bad-without-action tar f bad.tar ${CMAKE_CURRENT_LIST_DIR}/test-file.txt)
external_command_test(bad-wrong-flag     tar cvfq bad.tar ${CMAKE_CURRENT_LIST_DIR}/test-file.txt)
external_command_test(end-opt1           tar cvf bad.tar -- --bad)
external_command_test(end-opt2           tar cvf bad.tar --)
external_command_test(mtime              tar cvf bad.tar "--mtime=1970-01-01 00:00:00 UTC" ${CMAKE_CURRENT_LIST_DIR}/test-file.txt)
external_command_test(bad-format         tar cvf bad.tar "--format=bad-format" ${CMAKE_CURRENT_LIST_DIR}/test-file.txt)

run_cmake(7zip)
run_cmake(7zip-bz2)
run_cmake(7zip-gz)
run_cmake(7zip-lzma)
run_cmake(7zip-xz)
if (test_3_8_0)
  run_cmake(7zip-zstd)
else()
  external_command_test(7zip-zstd-unsupported tar cvf bad.7z --zstd --format=7zip ${CMAKE_CURRENT_LIST_DIR}/test-file.txt)
endif()
run_cmake(gnutar)
run_cmake(gnutar-gz)
run_cmake(pax)
run_cmake(pax-lzma)
run_cmake(pax-xz)
run_cmake(pax-zstd)
run_cmake(paxr)
run_cmake(paxr-bz2)
run_cmake(zip)
run_cmake(zip-gz)
if (test_3_8_0)
  run_cmake(zip-bz2)
  run_cmake(zip-lzma)
  run_cmake(zip-xz)
  run_cmake(zip-zstd)
else()
  external_command_test(zip-bz2-unsupported   tar cvjf bad.zip --format=zip ${CMAKE_CURRENT_LIST_DIR}/test-file.txt)
  external_command_test(zip-lzma-unsupported  tar cvf bad.zip --lzma --format=zip ${CMAKE_CURRENT_LIST_DIR}/test-file.txt)
  external_command_test(zip-xz-unsupported    tar cvJf bad.zip --format=zip ${CMAKE_CURRENT_LIST_DIR}/test-file.txt)
  external_command_test(zip-zstd-unsupported  tar cvf bad.zip --zstd --format=zip ${CMAKE_CURRENT_LIST_DIR}/test-file.txt)
endif()

# Check the --cmake-tar-threads option
external_command_test(bad-threads-not-a-number  tar cvf bad.tar --cmake-tar-threads=nan .)

run_cmake(threads-7zip)
run_cmake(threads-bz2)
run_cmake(threads-gz)
run_cmake(threads-xz)
run_cmake(threads-zstd)
run_cmake(threads-zip)

# Check the --cmake-tar-compression-level option
external_command_test(bad-compression-level-no-compression  tar cvf bad.tar --cmake-tar-compression-level=1 .)
external_command_test(bad-compression-level-not-a-number  tar cvjf bad.tar --cmake-tar-compression-level=nan .)
external_command_test(bad-compression-level-bz2  tar cvjf bad.tar --cmake-tar-compression-level=10 .)
external_command_test(bad-compression-level-gz  tar cvzf bad.tar --cmake-tar-compression-level=10 .)
external_command_test(bad-compression-level-lzma  tar cvf bad.tar --lzma --cmake-tar-compression-level=10 .)
external_command_test(bad-compression-level-xz  tar cvJf bad.tar --cmake-tar-compression-level=10 .)
external_command_test(bad-compression-level-zstd  tar cvf bad.tar --zstd --cmake-tar-compression-level=20 .)
external_command_test(bad-compression-level-7z  tar cvf bad.7z --format=7zip --cmake-tar-compression-level=10 .)
external_command_test(bad-compression-level-7z-zstd  tar cvf bad.7z --format=7zip --zstd --cmake-tar-compression-level=20 .)
external_command_test(bad-compression-level-zip  tar cvf bad.zip --format=zip --cmake-tar-compression-level=10 .)
external_command_test(bad-compression-level-zip-zstd  tar cvf bad.zip --format=zip --zstd --cmake-tar-compression-level=10 .)

run_cmake(compression-level-bz2)
run_cmake(compression-level-gz)
run_cmake(compression-level-xz)
run_cmake(compression-level-lzma)
run_cmake(compression-level-zstd)
run_cmake(compression-level-7z)
run_cmake(compression-level-zip)
if (test_3_8_0)
  run_cmake(compression-level-7z-zstd)
  run_cmake(compression-level-zip-zstd)
endif()

# Check the --cmake-tar-compression-method option
external_command_test(bad-compression-method-unknown
  tar cvf bad.tar "--cmake-tar-compression-method=bad-method"
    ${CMAKE_CURRENT_LIST_DIR}/test-file.txt
)
external_command_test(bad-compression-method-duplicate
  tar cvzf bad.tar "--cmake-tar-compression-method=gzip"
    ${CMAKE_CURRENT_LIST_DIR}/test-file.txt
)
external_command_test(bad-compression-method-different
  tar cvzf bad.tar "--cmake-tar-compression-method=none"
    ${CMAKE_CURRENT_LIST_DIR}/test-file.txt
)

run_cmake(compression-method-none)
run_cmake(compression-method-store)
run_cmake(compression-method-bzip2)
run_cmake(compression-method-gzip)
run_cmake(compression-method-deflate)
run_cmake(compression-method-xz)
run_cmake(compression-method-lzma2)
run_cmake(compression-method-lzma)
run_cmake(compression-method-zstd)
run_cmake(compression-method-ppmd)

# Extracting only selected files or directories
run_cmake(zip-filtered)

# Use the --mtime option to set the mtime when creating archive
run_cmake(set-mtime)

# Use the --touch option to avoid extracting the mtime
run_cmake(touch-mtime)
