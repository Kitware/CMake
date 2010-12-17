INCLUDE_DIRECTORIES(${Dependency_BINARY_DIR}/Two)
ADD_LIBRARY( Four FourSrc.c )
TARGET_LINK_LIBRARIES( Four One Two NoDepA )

# TwoCustom must build before Four.
ADD_DEPENDENCIES(Four TwoCustom)
