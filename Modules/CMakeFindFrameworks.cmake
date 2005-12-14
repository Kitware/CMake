# - helper module to find OSX frameworks

IF(NOT CMAKE_FIND_FRAMEWORKS_INCLUDED)
  SET(CMAKE_FIND_FRAMEWORKS_INCLUDED 1)
  MACRO(CMAKE_FIND_FRAMEWORKS fwk)
    SET(${fwk}_FRAMEWORKS)
    IF(APPLE)
      FOREACH(dir
          ~/Library/Frameworks/${fwk}.framework
          /Library/Frameworks/${fwk}.framework
          /System/Library/Frameworks/${fwk}.framework
          /Network/Library/Frameworks/${fwk}.framework)
        IF(EXISTS ${dir})
          SET(${fwk}_FRAMEWORKS ${${fwk}_FRAMEWORKS} ${dir})
        ENDIF(EXISTS ${dir})
      ENDFOREACH(dir)
    ENDIF(APPLE)
  ENDMACRO(CMAKE_FIND_FRAMEWORKS)
ENDIF(NOT CMAKE_FIND_FRAMEWORKS_INCLUDED)
