# create library
add_library(Addition STATIC Addition.cxx)

add_library(MathFunctions::Addition ALIAS Addition)

# add include directories
target_include_directories(Addition
                           PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# install the target and create export-set
install(TARGETS Addition
        EXPORT AdditionTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# install header file
install(FILES Addition.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# generate and install export file
install(EXPORT AdditionTargets
        FILE MathFunctionsAdditionTargets.cmake
        NAMESPACE MathFunctions::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MathFunctions
)
