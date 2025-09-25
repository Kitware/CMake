set(CMAKE_FIND_REQUIRED ON)
find_path(PATH_doNotExists_Optional
  NAMES doNotExists.h
  OPTIONAL
)
find_path(PATH_doNotExists_OptionalRequired
  NAMES doNotExists.h
  OPTIONAL
  REQUIRED
)
find_path(PATH_doNotExists
  NAMES doNotExists.h
)
