install(CODE [[cmake_language(EXIT 7)]] COMPONENT comp_fail)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt TYPE DATA RENAME ok.txt
        COMPONENT comp_ok)
