enable_language(CXX)

get_filename_component(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}" DIRECTORY)
string(APPEND CMAKE_PREFIX_PATH "/FileSetAbsoluteInstallIncludeDirExport-build/install")

find_package(lib1 REQUIRED)

add_executable(exe main.cpp)
target_link_libraries(exe PRIVATE lib1::lib1)
