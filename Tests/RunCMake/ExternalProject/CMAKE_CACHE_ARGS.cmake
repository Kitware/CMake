include(ExternalProject)

ExternalProject_Add(FOO TMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmp"
                        DOWNLOAD_COMMAND ""
                        CMAKE_CACHE_ARGS "-DFOO:STRING=BAR")
