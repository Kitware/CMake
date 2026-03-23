
enable_language(C)

add_library(foo STATIC foo.c)


target_sources(foo PUBLIC FILE_SET HEADERS FILES foo.h)

get_property(srcs FILE_SET HEADERS TARGET foo PROPERTY SOURCES)
if(NOT srcs MATCHES ".*/foo.h$")
  message(SEND_ERROR "wrong sources: '${srcs}' instead of 'foo.h'")
endif()
get_property(srcs FILE_SET HEADERS TARGET foo PROPERTY INTERFACE_SOURCES)
if(NOT srcs MATCHES ".*/foo.h$")
  message(SEND_ERROR "wrong interface sources: '${srcs}' instead of 'foo.h'")
endif()


set_property(FILE_SET HEADERS TARGET foo PROPERTY SOURCES bar.h)

get_property(srcs FILE_SET HEADERS TARGET foo PROPERTY SOURCES)
if(NOT srcs MATCHES "[^;]*bar.h$")
  message(SEND_ERROR "wrong sources: '${srcs}' instead of 'bar.h'")
endif()
get_property(srcs FILE_SET HEADERS TARGET foo PROPERTY INTERFACE_SOURCES)
if(NOT srcs MATCHES "[^;]*bar.h$")
  message(SEND_ERROR "wrong interface sources: '${srcs}' instead of 'bar.h'")
endif()


set_property(FILE_SET HEADERS TARGET foo APPEND PROPERTY SOURCES foo.h)

get_property(srcs FILE_SET HEADERS TARGET foo PROPERTY SOURCES)
if(NOT srcs MATCHES "[^;]*bar.h;[^;]*foo.h$")
  message(SEND_ERROR "wrong sources: '${srcs}' instead of 'foo.h;bar.h'")
endif()
get_property(srcs FILE_SET HEADERS TARGET foo PROPERTY INTERFACE_SOURCES)
if(NOT srcs MATCHES "[^;]*bar.h;[^;]*foo.h$")
  message(SEND_ERROR "wrong interface sources: '${srcs}' instead of 'foo.h;bar.h'")
endif()


target_sources(foo PRIVATE FILE_SET foo TYPE HEADERS FILES h1.h)

get_property(srcs FILE_SET foo TARGET foo PROPERTY SOURCES)
if(NOT srcs MATCHES ".*/h1.h$")
  message(SEND_ERROR "wrong sources: '${srcs}' instead of 'h1.h'")
endif()
get_property(srcs FILE_SET foo TARGET foo PROPERTY INTERFACE_SOURCES)
if(srcs)
  message(SEND_ERROR "wrong interface sources: '${srcs}' instead of empty list")
endif()


set_property(FILE_SET foo TARGET foo PROPERTY SOURCES h2.h)

get_property(srcs FILE_SET foo TARGET foo PROPERTY SOURCES)
if(NOT srcs MATCHES "[^;]*h2.h$")
  message(SEND_ERROR "wrong sources: '${srcs}' instead of 'h2.h'")
endif()
get_property(srcs FILE_SET foo TARGET foo PROPERTY INTERFACE_SOURCES)
if(srcs)
  message(SEND_ERROR "wrong interface sources: '${srcs}' instead of empty list")
endif()


set_property(FILE_SET foo TARGET foo PROPERTY INTERFACE_SOURCES h2.h)

get_property(srcs FILE_SET foo TARGET foo PROPERTY SOURCES)
if(NOT srcs MATCHES "[^;]*h2.h$")
  message(SEND_ERROR "wrong sources: '${srcs}' instead of 'h2.h'")
endif()
get_property(srcs FILE_SET foo TARGET foo PROPERTY INTERFACE_SOURCES)
if(srcs)
  message(SEND_ERROR "wrong interface sources: '${srcs}' instead of empty list")
endif()


target_sources(foo INTERFACE FILE_SET bar TYPE HEADERS FILES foo.h)

get_property(srcs FILE_SET bar TARGET foo PROPERTY SOURCES)
if(srcs)
  message(SEND_ERROR "wrong sources: '${srcs}' instead of empty list")
endif()
get_property(srcs FILE_SET bar TARGET foo PROPERTY INTERFACE_SOURCES)
if(NOT srcs MATCHES "[^;]*/foo.h$")
  message(SEND_ERROR "wrong interface sources: '${srcs}' instead of 'foo.h'")
endif()


set_property(FILE_SET bar TARGET foo PROPERTY INTERFACE_SOURCES bar.h)

get_property(srcs FILE_SET bar TARGET foo PROPERTY SOURCES)
if(srcs)
  message(SEND_ERROR "wrong sources: '${srcs}' instead of empty list")
endif()
get_property(srcs FILE_SET bar TARGET foo PROPERTY INTERFACE_SOURCES)
if(NOT srcs MATCHES "[^;]*bar.h$")
  message(SEND_ERROR "wrong interface sources: '${srcs}' instead of 'bar.h'")
endif()


set_property(FILE_SET bar TARGET foo PROPERTY SOURCES foo.h)

get_property(srcs FILE_SET bar TARGET foo PROPERTY SOURCES)
if(srcs)
  message(SEND_ERROR "wrong sources: '${srcs}' instead of empty list")
endif()
get_property(srcs FILE_SET bar TARGET foo PROPERTY INTERFACE_SOURCES)
if(NOT srcs MATCHES "[^;]*bar.h$")
  message(SEND_ERROR "wrong interface sources: '${srcs}' instead of 'bar.h'")
endif()
