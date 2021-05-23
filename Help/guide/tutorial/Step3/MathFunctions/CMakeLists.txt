add_library(MathFunctions mysqrt.cxx)

target_include_directories(MathFunctions
            INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
            )