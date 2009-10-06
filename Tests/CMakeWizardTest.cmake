message("CTEST_FULL_OUTPUT (Avoid ctest truncation of output)")

message(STATUS "build_dir='${build_dir}'")

message(STATUS "source_dir='${source_dir}'")


execute_process(COMMAND ${CMAKE_COMMAND} -E
  remove_directory ${build_dir}
  TIMEOUT 5)

execute_process(COMMAND ${CMAKE_COMMAND} -E
  make_directory ${build_dir}
  TIMEOUT 5)

execute_process(COMMAND ${CMAKE_COMMAND} -E
  copy_directory ${source_dir} ${build_dir}/src
  TIMEOUT 5)

execute_process(COMMAND ${CMAKE_COMMAND} -E
  make_directory ${build_dir}/build
  TIMEOUT 5)

# This is enough to answer 32 questions with "the default answer is ok"...
#
file(WRITE ${build_dir}/input.txt
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n")


message(STATUS "running wizard mode (cmake -i)...")

execute_process(COMMAND ${CMAKE_COMMAND} -i ../src
  INPUT_FILE ${build_dir}/input.txt
  WORKING_DIRECTORY ${build_dir}/build
  TIMEOUT 5
  )


message(STATUS "building...")

execute_process(COMMAND ${CMAKE_COMMAND} --build .
  WORKING_DIRECTORY ${build_dir}/build
  TIMEOUT 5
  )


message(STATUS "testing...")

execute_process(COMMAND ${CMAKE_CTEST_COMMAND}
  WORKING_DIRECTORY ${build_dir}/build
  TIMEOUT 5
  )
