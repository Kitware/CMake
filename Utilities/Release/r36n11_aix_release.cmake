set(CMAKE_RELEASE_DIRECTORY "/bench1/noibm34/CMakeReleaseDirectory" )
set(FINAL_PATH /u/noibm34/cmake-release)
set(PROCESSORS 2)
set(CVS_COMMAND /vol/local/bin/cvs)
set(HOST "sshserv.centers.ihost.com" )
set(EXTRA_HOP "ssh r36n11" )
set(MAKE_PROGRAM "make")
set(CC "xlc")
set(CXX "xlC")
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
