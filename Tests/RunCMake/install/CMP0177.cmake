enable_language(C)

add_library(foo1 STATIC obj1.c)
add_library(foo2 STATIC obj2.c)

set_target_properties(foo2 PROPERTIES HIDDEN_DOUBLE_DOT "..")

# All the shouldNotRemainX path components below should be normalized out when
# CMP0177 is set to NEW, and retained for OLD and WARN.

install(TARGETS foo1
  EXPORT pkg
  ARCHIVE DESTINATION shouldNotRemain1/../lib
)
install(TARGETS foo2
  EXPORT pkg
  ARCHIVE DESTINATION shouldNotRemain2/$<TARGET_PROPERTY:foo2,HIDDEN_DOUBLE_DOT>/lib
)
install(EXPORT pkg
  DESTINATION shouldNotRemain3/deeper/../.././lib/cmake/pkg
  FILE pkg-config.cmake
)
install(FILES obj1.c
  DESTINATION shouldNotRemain4/anotherSubdir/../../files
)
install(DIRECTORY dir
  # Trailing slash here is significant
  DESTINATION shouldNotRemain5/../dirs/more/../
)
