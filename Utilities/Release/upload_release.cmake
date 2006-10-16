set(PROJECT_PREFIX cmake-)
set(VERSION 2.4)
file(GLOB FILES ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_PREFIX}*")
message("${FILES}")
set(UPLOAD_LOC
  "kitware@www.cmake.org:/projects/FTP/pub/cmake/v${VERSION}")
foreach(file ${FILES})
  if(NOT IS_DIRECTORY ${file})
    message("upload ${file}")
    execute_process(COMMAND 
      scp ${file} ${UPLOAD_LOC}
      RESULT_VARIABLE result)  
    if("${result}" GREATER 0)
      message(FATAL_ERROR "failed to upload file to ${UPLOAD_LOC}")
    endif("${result}" GREATER 0)
  endif(NOT IS_DIRECTORY ${file})
endforeach(file)
