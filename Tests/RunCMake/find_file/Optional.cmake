set(CMAKE_FIND_REQUIRED ON)
find_file(FILE_doNotExists_Optional
  NAMES doNotExists.h
  OPTIONAL
)
find_file(FILE_doNotExists_OptionalRequired
  NAMES doNotExists.h
  OPTIONAL
  REQUIRED
)
find_file(FILE_doNotExists
  NAMES doNotExists.h
  REQUIRED
)
