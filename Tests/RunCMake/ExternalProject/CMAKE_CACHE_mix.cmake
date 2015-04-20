include(ExternalProject)

ExternalProject_Add(FOO TMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmp"
                        DOWNLOAD_COMMAND ""
                        CMAKE_CACHE_ARGS "-DFOO:STRING=BAR"
                        CMAKE_CACHE_DEFAULT_ARGS "-DBAR:STRING=BAZ")
