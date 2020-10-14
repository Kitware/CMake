add_custom_command(
  OUTPUT top.txt
  COMMAND ${CMAKE_COMMAND} -DOUTFILE=top.txt -DINFILE=topdep.txt -DDEPFILE=top.txt.d -DSTAMPFILE=topstamp.txt -DDEPDIR= -P ${CMAKE_SOURCE_DIR}/WriteDepfile.cmake
  DEPFILE top.txt.d
  )
add_custom_target(top ALL DEPENDS top.txt)

add_subdirectory(Subdirectory)
