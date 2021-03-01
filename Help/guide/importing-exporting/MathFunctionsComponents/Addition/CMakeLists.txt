# create library
add_library(Addition STATIC Addition.cxx)

add_library(MathFunctions::Addition ALIAS Addition)

# add include directories
target_include_directories(Addition
                           PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           $<INSTALL_INTERFACE:include>
)

# install the target and create export-set
install(TARGETS Addition
        EXPORT AdditionTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

# install header file
install(FILES Addition.h DESTINATION include)

# generate and install export file
install(EXPORT AdditionTargets
        FILE MathFunctionsAdditionTargets.cmake
        NAMESPACE MathFunctions::
        DESTINATION lib/cmake/MathFunctions
)
