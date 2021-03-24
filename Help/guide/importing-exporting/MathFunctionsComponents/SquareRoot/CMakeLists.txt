# create library
add_library(SquareRoot STATIC SquareRoot.cxx)

add_library(MathFunctions::SquareRoot ALIAS SquareRoot)

# add include directories
target_include_directories(SquareRoot
                           PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# install the target and create export-set
install(TARGETS SquareRoot
        EXPORT SquareRootTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# install header file
install(FILES SquareRoot.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# generate and install export file
install(EXPORT SquareRootTargets
        FILE MathFunctionsSquareRootTargets.cmake
        NAMESPACE MathFunctions::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MathFunctions
)
