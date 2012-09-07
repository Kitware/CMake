set(PROCESSORS 2)
set(CMAKE_RELEASE_DIRECTORY /Users/kitware/CMakeReleaseDirectory)
set(USER_OVERRIDE "set(CMAKE_CXX_LINK_EXECUTABLE \\\"gcc  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>  -shared-libgcc -lstdc++-static\\\")")
set(INSTALL_PREFIX /)
set(HOST dashmacmini2)
set(MAKE_PROGRAM "make")
set(MAKE "${MAKE_PROGRAM} -j2")
set(CPACK_BINARY_GENERATORS "PackageMaker TGZ TZ")
set(INITIAL_CACHE "
CMAKE_BUILD_TYPE:STRING=Release
CMAKE_OSX_ARCHITECTURES:STRING=ppc;i386
CMAKE_USE_OPENSSL:BOOL=ON
OPENSSL_CRYPTO_LIBRARY:FILEPATH=/Users/kitware/openssl-1.0.1c-install/lib/libcrypto.a
OPENSSL_INCLUDE_DIR:PATH=/Users/kitware/openssl-1.0.1c-install/include
OPENSSL_SSL_LIBRARY:FILEPATH=/Users/kitware/openssl-1.0.1c-install/lib/libssl.a
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
CPACK_SYSTEM_NAME:STRING=Darwin-universal
BUILD_QtDialog:BOOL=TRUE
QT_QMAKE_EXECUTABLE:FILEPATH=/Users/kitware/Support/qt-4.8.0/install/bin/qmake
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
