add_custom_command(
  OUTPUT sub.txt
  COMMAND ${CMAKE_COMMAND} -DOUTFILE=sub.txt -DINFILE=subdep.txt -DDEPFILE=sub.txt.d -DSTAMPFILE=substamp.txt -DDEPDIR=${depdir} -P ${CMAKE_SOURCE_DIR}/WriteDepfile.cmake
  DEPFILE ${depdir}sub.txt.d
  )
add_custom_target(sub ALL DEPENDS sub.txt)
