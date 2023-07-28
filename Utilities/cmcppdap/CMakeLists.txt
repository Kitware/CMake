# Disable warnings to avoid changing 3rd party code.
if(CMAKE_CXX_COMPILER_ID MATCHES
    "^(GNU|LCC|Clang|AppleClang|IBMClang|XLClang|XL|VisualAge|SunPro|HP|Intel|IntelLLVM|NVHPC)$")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "PathScale")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -woffall")
endif()

add_library(cmcppdap STATIC
  src/content_stream.cpp
  src/io.cpp
  src/jsoncpp_json_serializer.cpp
  src/network.cpp
  src/null_json_serializer.cpp
  src/protocol_events.cpp
  src/protocol_requests.cpp
  src/protocol_response.cpp
  src/protocol_types.cpp
  src/session.cpp
  src/socket.cpp
  src/typeinfo.cpp
  src/typeof.cpp
)

target_compile_definitions(cmcppdap PRIVATE CPPDAP_JSON_JSONCPP=1)
target_include_directories(cmcppdap PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_property(TARGET cmcppdap PROPERTY CXX_CLANG_TIDY "")
set_property(TARGET cmcppdap PROPERTY CXX_INCLUDE_WHAT_YOU_USE "")

target_link_libraries(cmcppdap PRIVATE JsonCpp::JsonCpp)
if(WIN32)
  target_link_libraries(cmcppdap PRIVATE ws2_32)
elseif(NOT APPLE)
  target_link_libraries(cmcppdap PRIVATE Threads::Threads)
endif()
if(CMake_HAVE_CXX_ATOMIC_LIB)
  target_link_libraries(cmcppdap PRIVATE atomic)
endif()

if(CMake_BUILD_PCH)
  target_precompile_headers(cmcppdap PRIVATE "include/dap/protocol.h")
endif()

install(FILES NOTICE DESTINATION ${CMAKE_DOC_DIR}/cmcppdap)
