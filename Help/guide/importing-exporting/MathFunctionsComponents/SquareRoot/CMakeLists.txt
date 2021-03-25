# create library
add_library(SquareRoot STATIC SquareRoot.cxx)

add_library(MathFunctions::SquareRoot ALIAS SquareRoot)

# add include directories
target_include_directories(SquareRoot
                           PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:include>"
)

# install the target and create export-set
install(TARGETS SquareRoot
        EXPORT SquareRootTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

# install header file
install(FILES SquareRoot.h DESTINATION include)

# generate and install export file
install(EXPORT SquareRootTargets
        FILE MathFunctionsSquareRootTargets.cmake
        NAMESPACE MathFunctions::
        DESTINATION lib/cmake/MathFunctions
)
