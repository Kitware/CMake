
do_build(buildsystem2)

set(content "cmake_minimum_required(VERSION 3.4)\n\nproject(buildsystem2)\n\nset(var1 123)\n\nset(var2 345)\n\nadd_executable(main main.cpp)\n\nadd_executable(m_other main.cpp)\n\nadd_library(foo foo.cpp)\n\nfunction(f1)\nendfunction()\n\ntarget_\nset(var3 345)\n\n\nadd_subdirectory(subdir)\n")

set(request
  {\"type\":\"code_complete\",\"file_path\":\"${CMAKE_SOURCE_DIR}/buildsystem2/CMakeLists.txt\",\"file_content\":\"${content}\",\"file_line\":18,\"file_column\":7}
)

set(response
  [==[{"completion":{"commands":["target_compile_definitions","target_compile_features","target_compile_options","target_include_directories","target_link_libraries","target_sources"],"matcher":"target_"}}
  ]==]
)

do_test(buildsystem2 "${request}" "${response}")

set(content "cmake_minimum_required(VERSION 3.4)\n\nproject(buildsystem2)\n\nset(var1 123)\n\nset(var2 345)\n\nadd_executable(main main.cpp)\n\nadd_executable(m_other main.cpp)\n\nadd_library(foo foo.cpp)\n\nfunction(f1)\nendfunction()\n\ntarget_include_directories(\nset(var3 345)\n\nadd_subdirectory(subdir)\n")

set(request
  {\"type\":\"code_complete\",\"file_path\":\"${CMAKE_SOURCE_DIR}/buildsystem2/CMakeLists.txt\",\"file_content\":\"${content}\",\"file_line\":18,\"file_column\":28}
)

set(response
  [==[{"completion":{"targets":["foo","m_other","main", "ooo"],"matcher":""}}
  ]==]
)

do_test(buildsystem2 "${request}" "${response}")

set(content "cmake_minimum_required(VERSION 3.4)\n\nproject(buildsystem2)\n\nset(var1 123)\n\nset(var2 345)\n\nadd_executable(main main.cpp)\n\nadd_executable(m_other main.cpp)\n\nadd_library(foo foo.cpp)\n\nfunction(f1)\nendfunction()\n\ntarget_include_directories(m\nset(var3 345)\n\nadd_subdirectory(subdir)\n")

set(request
  {\"type\":\"code_complete\",\"file_path\":\"${CMAKE_SOURCE_DIR}/buildsystem2/CMakeLists.txt\",\"file_content\":\"${content}\",\"file_line\":18,\"file_column\":29}
)

set(response
  [==[{"completion":{"targets":["m_other","main"],"matcher":"m"}}
  ]==]
)

do_test(buildsystem2 "${request}" "${response}")


set(content "cmake_minimum_required(VERSION 3.4)\n\nproject(buildsystem2)\n\nset(var1 123)\n\nset(var2 345)\n\nadd_executable(main main.cpp)\n\nadd_executable(m_other main.cpp)\n\nadd_library(foo foo.cpp)\n\nfunction(f1)\nendfunction()\n\ntarget_include_directories(main \nset(var3 345)\n\nadd_subdirectory(subdir)\n")

set(request
  {\"type\":\"code_complete\",\"file_path\":\"${CMAKE_SOURCE_DIR}/buildsystem2/CMakeLists.txt\",\"file_content\":\"${content}\",\"file_line\":18,\"file_column\":33}
)

set(response
  [==[{"completion":{"keywords":["INTERFACE","PRIVATE","PUBLIC"],"matcher":""}}
  ]==]
)

do_test(buildsystem2 "${request}" "${response}")



set(content "cmake_minimum_required(VERSION 3.4)\n\nproject(buildsystem2)\n\nset(var1 123)\n\nset(var2 345)\n\nadd_executable(main main.cpp)\n\nadd_executable(m_other main.cpp)\n\nadd_library(foo foo.cpp)\n\nfunction(f1)\nendfunction()\n\ntarget_include_directories(main P\nset(var3 345)\n\nadd_subdirectory(subdir)\n")

set(request
  {\"type\":\"code_complete\",\"file_path\":\"${CMAKE_SOURCE_DIR}/buildsystem2/CMakeLists.txt\",\"file_content\":\"${content}\",\"file_line\":18,\"file_column\":34}
)

set(response
  [==[{"completion":{"keywords":["PRIVATE","PUBLIC"],"matcher":"P"}}
  ]==]
)

do_test(buildsystem2 "${request}" "${response}")



set(request
  {\"type\":\"content\",\"file_path\":\"${CMAKE_SOURCE_DIR}/buildsystem2/subdir/CMakeLists.txt\",\"file_line\":2,\"matcher\":\"bar\"}
)

set(response
  [==[{"content":{"bar4":"something"}}
  ]==]
)

do_test(buildsystem2 "${request}" "${response}")

set(request
  {\"type\":\"content\",\"file_path\":\"${CMAKE_SOURCE_DIR}/buildsystem2/CMakeLists.txt\",\"file_line\":6,\"matcher\":\"var\"}
)

set(response
  [==[{"content":{"var1":"123"}}
  ]==]
)

do_test(buildsystem2 "${request}" "${response}")
