# Link in QT


IF (WIN32)
  LINK_LIBRARIES( imm32.lib ws2_32.lib)
#Ensure that qt.lib is last
ENDIF (WIN32)


LINK_LIBRARIES( ${QT_QT_LIBRARY})
