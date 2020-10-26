add_custom_command(
  OUTPUT output1.txt
  COMMAND ${CMAKE_COMMAND} -E echo CustomCommand > output1.txt
  )
add_custom_target(CustomTarget ALL DEPENDS output1.txt)

add_custom_command(
  OUTPUT output2.txt
  COMMAND ${CMAKE_COMMAND} -E echo CustomCommand2 > output2.txt
  )
add_custom_target(CustomTarget2 ALL DEPENDS output2.txt)

add_custom_command(
  OUTPUT output3.txt
  COMMAND ${CMAKE_COMMAND} -E echo CustomCommand2 > output3.txt
  )
add_custom_target(CustomTarget3 ALL DEPENDS output3.txt)

add_custom_target(CustomTargetFail COMMAND DoesNotExist)
