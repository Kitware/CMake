set(CMAKE_RELEASE_DIRECTORY "/bench1/noibm34/CMakeReleaseDirectory")
set(FINAL_PATH /u/noibm34/cmake-release)
set(PROCESSORS 2)
set(HOST "sshserv.centers.ihost.com")
set(EXTRA_HOP "rsh p90n03")
set(MAKE_PROGRAM "make")
set(CC "xlc_r")
set(CXX "xlC_r")
set(FC "xlf")
set(INITIAL_CACHE "
CMAKE_BUILD_TYPE:STRING=Release
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
")
set(EXTRA_COPY "
rm -rf ~/cmake-release
mkdir ~/cmake-release
mv *.sh ~/cmake-release
mv *.Z ~/cmake-release
mv *.gz ~/cmake-release")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
