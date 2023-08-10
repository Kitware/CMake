install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/empty3.cmake ${CMAKE_CURRENT_SOURCE_DIR}/empty4.cmake
  DESTINATION .
  )
install(
  SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/install_script.cmake"
  CODE "write_empty_file(empty2.txt)"
  SCRIPT "$<INSTALL_PREFIX>/empty3.cmake"
  CODE [[include($<INSTALL_PREFIX>/empty4.cmake)]]
  )
