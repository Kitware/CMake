cmake_minimum_required(VERSION 3.10)
project(TestFindProtobuf CXX)
include(CTest)

find_package(Protobuf REQUIRED)

add_executable(test_tgt main.cxx)
target_link_libraries(test_tgt protobuf::libprotobuf)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.cxx)
target_include_directories(test_var PRIVATE ${Protobuf_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${Protobuf_LIBRARIES})
add_test(NAME test_var COMMAND test_var)

add_executable(test_tgt_lite main.cxx)
target_link_libraries(test_tgt_lite protobuf::libprotobuf-lite)
add_test(NAME test_tgt_lite COMMAND test_tgt_lite)

add_executable(test_var_lite main.cxx)
target_include_directories(test_var_lite PRIVATE ${Protobuf_INCLUDE_DIRS})
target_link_libraries(test_var_lite PRIVATE ${Protobuf_LITE_LIBRARIES})
add_test(NAME test_var_lite COMMAND test_var_lite)

add_executable(test_tgt_protoc main-protoc.cxx)
target_link_libraries(test_tgt_protoc protobuf::libprotoc)
add_test(NAME test_tgt_protoc COMMAND test_tgt_protoc)

add_executable(test_var_protoc main-protoc.cxx)
target_include_directories(test_var_protoc PRIVATE ${Protobuf_INCLUDE_DIRS})
target_link_libraries(test_var_protoc PRIVATE ${Protobuf_PROTOC_LIBRARIES})
target_compile_features(test_var_protoc PRIVATE cxx_std_11)
add_test(NAME test_var_protoc COMMAND test_var_protoc)

add_test(NAME test_tgt_protoc_version COMMAND protobuf::protoc --version)

set(Protobuf_IMPORT_DIRS ${Protobuf_INCLUDE_DIRS})
protobuf_generate_cpp(PROTO_SRC PROTO_HEADER msgs/example.proto)
protobuf_generate_cpp(DESC_PROTO_SRC DESC_PROTO_HEADER DESCRIPTORS DESC_PROTO_DESC msgs/example_desc.proto)
add_library(msgs ${PROTO_SRC} ${PROTO_HEADER})
target_compile_features(msgs PRIVATE cxx_std_11)

add_executable(test_generate main-generate.cxx ${PROTO_SRC})
target_include_directories(test_generate PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(test_generate msgs ${Protobuf_LIBRARIES})
target_compile_features(test_generate PRIVATE cxx_std_11)
add_test(NAME test_generate COMMAND test_generate)

add_executable(test_desc main-desc.cxx ${DESC_PROTO_SRC})
target_compile_features(test_desc PRIVATE cxx_std_11)
target_include_directories(test_desc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(test_desc msgs ${Protobuf_LIBRARIES})
target_compile_features(test_desc PRIVATE cxx_std_11)
add_test(NAME test_desc COMMAND test_desc ${DESC_PROTO_DESC})

if(CMake_TEST_FindProtobuf_gRPC)
  find_program(gRPC_CPP_PLUGIN grpc_cpp_plugin)
  add_library(msgs_grpc msgs/grpc/example_service.proto)
  # NOTE: by default generated files will be placed under ${CMAKE_CURRENT_BINARY_DIR}/msgs/grpc/
  target_include_directories(msgs_grpc PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/msgs/grpc/)
  target_link_libraries(msgs_grpc PUBLIC ${Protobuf_LIBRARIES})
  protobuf_generate(TARGET msgs_grpc LANGUAGE cpp)
  protobuf_generate(TARGET msgs_grpc LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${gRPC_CPP_PLUGIN}")

  add_executable(test_generate_grpc main-generate-grpc.cxx)
  target_link_libraries(test_generate_grpc PRIVATE msgs_grpc)
  add_test(NAME test_generate_grpc COMMAND test_generate_grpc)

  add_library(msgs_grpc_IMPORT_DIRS msgs/grpc/example_service.proto)
  # NOTE: with IMPORT_DIRS msgs/, generated files will be placed under ${CMAKE_CURRENT_BINARY_DIR}/grpc/
  target_include_directories(msgs_grpc_IMPORT_DIRS PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/grpc/)
  target_link_libraries(msgs_grpc_IMPORT_DIRS PUBLIC ${Protobuf_LIBRARIES})
  protobuf_generate(TARGET msgs_grpc_IMPORT_DIRS LANGUAGE cpp IMPORT_DIRS msgs/ PROTOC_EXE ${Protobuf_PROTOC_EXECUTABLE})
  protobuf_generate(TARGET msgs_grpc_IMPORT_DIRS LANGUAGE grpc IMPORT_DIRS msgs/ GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${gRPC_CPP_PLUGIN}")
  add_executable(test_generate_grpc_IMPORT_DIRS main-generate-grpc.cxx)
  target_link_libraries(test_generate_grpc_IMPORT_DIRS PRIVATE msgs_grpc_IMPORT_DIRS)
  add_test(NAME test_generate_grpc_IMPORT_DIRS COMMAND test_generate_grpc_IMPORT_DIRS)

  add_library(msgs_grpc_APPEND_PATH msgs/grpc/example_service.proto)
  # NOTE: with APPEND_PATH, generated files will be placed under ${CMAKE_CURRENT_BINARY_DIR}/
  target_include_directories(msgs_grpc_APPEND_PATH PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries(msgs_grpc_APPEND_PATH PUBLIC ${Protobuf_LIBRARIES})
  protobuf_generate(TARGET msgs_grpc_APPEND_PATH LANGUAGE cpp APPEND_PATH)
  protobuf_generate(TARGET msgs_grpc_APPEND_PATH LANGUAGE grpc APPEND_PATH GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${gRPC_CPP_PLUGIN}")
  add_executable(test_generate_grpc_APPEND_PATH main-generate-grpc.cxx)
  target_link_libraries(test_generate_grpc_APPEND_PATH PRIVATE msgs_grpc_APPEND_PATH)
  add_test(NAME test_generate_grpc_APPEND_PATH COMMAND test_generate_grpc_APPEND_PATH)
endif()
