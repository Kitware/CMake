GET_FILENAME_COMPONENT(_CMAKE_INSTALL_DIR "${CMAKE_ROOT}" PATH)
GET_FILENAME_COMPONENT(_CMAKE_INSTALL_DIR "${_CMAKE_INSTALL_DIR}" PATH)

# the /bin, /lib and /include dirs are mainly for mingw cross compiler users 
# under Linux who use CMAKE_FIND_ROOT_PATH
SET(CMAKE_SYSTEM_INCLUDE_PATH ${CMAKE_SYSTEM_INCLUDE_PATH}  
    "$ENV{ProgramFiles}" "${CMAKE_INSTALL_PREFIX}/include" "${_CMAKE_INSTALL_DIR}/include" /include)

# mingw can also link against dlls which can also be in /bin, so list this too
SET(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH} 
   "$ENV{ProgramFiles}" 
   "${CMAKE_INSTALL_PREFIX}/lib" "${CMAKE_INSTALL_PREFIX}/bin" 
   "${_CMAKE_INSTALL_DIR}/lib" "${_CMAKE_INSTALL_DIR}/bin"
   /lib /bin )

SET(CMAKE_SYSTEM_PROGRAM_PATH ${CMAKE_SYSTEM_PROGRAM_PATH} 
   "$ENV{ProgramFiles}" "${CMAKE_INSTALL_PREFIX}/bin" "${_CMAKE_INSTALL_DIR}/bin" /bin)
