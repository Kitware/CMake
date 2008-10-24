#-----------------------------------------------------------------------------
# Function to run a child process and report output only on error.
function(run_child)
  execute_process(${ARGN}
    RESULT_VARIABLE FAILED
    OUTPUT_VARIABLE OUTPUT
    ERROR_VARIABLE OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
    )
  if(FAILED)
    string(REGEX REPLACE "\n" "\n  " OUTPUT "${OUTPUT}")
    message(FATAL_ERROR "Child failed.  Output is\n  ${OUTPUT}\n")
  endif(FAILED)
endfunction(run_child)

#-----------------------------------------------------------------------------
# Function to find the Update.xml file and check for expected entries.
function(check_updates build)
  # Find the Update.xml file for the given build tree
  set(PATTERN ${TOP}/${build}/Testing/*/Update.xml)
  file(GLOB UPDATE_XML_FILE RELATIVE ${TOP} ${PATTERN})
  string(REGEX REPLACE "//Update.xml$" "/Update.xml"
    UPDATE_XML_FILE "${UPDATE_XML_FILE}"
    )
  if(NOT UPDATE_XML_FILE)
    message(FATAL_ERROR "Cannot find Update.xml with pattern\n  ${PATTERN}")
  endif(NOT UPDATE_XML_FILE)
  message(" found ${UPDATE_XML_FILE}")

  # Read entries from the Update.xml file
  file(STRINGS ${TOP}/${UPDATE_XML_FILE} UPDATE_XML_ENTRIES
    REGEX "FullName"
    LIMIT_INPUT 4096
    )

  # Verify that expected entries exist
  set(MISSING)
  foreach(f ${ARGN})
    if(NOT "${UPDATE_XML_ENTRIES}" MATCHES "<FullName>${f}</FullName>")
      list(APPEND MISSING ${f})
    endif()
  endforeach(f)

  # Report the result
  if(MISSING)
    # List the missing entries
    set(MSG "Update.xml is missing an entry for:\n")
    foreach(f ${MISSING})
      set(MSG "${MSG}  ${f}\n")
    endforeach(f)

    # Provide the log file
    file(GLOB UPDATE_LOG_FILE
      ${TOP}/${build}/Testing/Temporary/LastUpdate*.log)
    if(UPDATE_LOG_FILE)
      file(READ ${UPDATE_LOG_FILE} UPDATE_LOG LIMIT 4096)
      string(REGEX REPLACE "\n" "\n  " UPDATE_LOG "${UPDATE_LOG}")
      set(MSG "${MSG}Update log:\n  ${UPDATE_LOG}")
    else(UPDATE_LOG_FILE)
      set(MSG "${MSG}No update log found!")
    endif(UPDATE_LOG_FILE)

    # Display the error message
    message(FATAL_ERROR "${MSG}")
  else(MISSING)
    # Success
    message(" no entries missing from Update.xml")
  endif(MISSING)
endfunction(check_updates)

#-----------------------------------------------------------------------------
# Function to create initial content.
function(create_content dir)
  file(MAKE_DIRECTORY ${TOP}/${dir})

  # An example CTest project configuration file.
  file(WRITE ${TOP}/${dir}/CTestConfig.cmake
    "# CTest Configuration File
set(CTEST_PROJECT_NAME TestProject)
set(CTEST_NIGHTLY_START_TIME \"21:00:00 EDT\")
")

  # Some other files.
  file(WRITE ${TOP}/${dir}/foo.txt "foo\n")
  file(WRITE ${TOP}/${dir}/bar.txt "bar\n")
endfunction(create_content)

#-----------------------------------------------------------------------------
# Function to update content.
function(update_content dir added_var removed_var)
  file(APPEND ${TOP}/${dir}/foo.txt "foo line 2\n")
  file(WRITE ${TOP}/${dir}/zot.txt "zot\n")
  file(REMOVE ${TOP}/${dir}/bar.txt)
  set(${added_var} zot.txt PARENT_SCOPE)
  set(${removed_var} bar.txt PARENT_SCOPE)
endfunction(update_content)

#-----------------------------------------------------------------------------
# Function to write CTestConfiguration.ini content.
function(create_build_tree src_dir bin_dir)
  file(MAKE_DIRECTORY ${TOP}/${bin_dir})
  file(WRITE ${TOP}/${bin_dir}/CTestConfiguration.ini
    "# CTest Configuration File
SourceDirectory: ${TOP}/${src_dir}
BuildDirectory: ${TOP}/${bin_dir}
Site: test.site
BuildName: user-test
")
endfunction(create_build_tree)

#-----------------------------------------------------------------------------
# Function to write the dashboard test script.
function(create_dashboard_script name custom_text)
  # Write the dashboard script.
  file(WRITE ${TOP}/dashboard.cmake
    "# CTest Dashboard Script
set(CTEST_DASHBOARD_ROOT \"${TOP}\")
set(CTEST_SITE test.site)
set(CTEST_BUILD_NAME dash-test)
set(CTEST_SOURCE_DIRECTORY \${CTEST_DASHBOARD_ROOT}/dash-source)
set(CTEST_BINARY_DIRECTORY \${CTEST_DASHBOARD_ROOT}/dash-binary)
${custom_text}
# Start a dashboard and run the update step
ctest_start(Experimental)
ctest_update(SOURCE \${CTEST_SOURCE_DIRECTORY})
")
endfunction(create_dashboard_script)

#-----------------------------------------------------------------------------
# Function to run the dashboard through the command line
function(run_dashboard_command_line bin_dir)
  run_child(
    WORKING_DIRECTORY ${TOP}/${bin_dir}
    COMMAND ${CMAKE_CTEST_COMMAND} -M Experimental -T Start -T Update
    )

  # Verify the updates reported by CTest.
  check_updates(${bin_dir} foo.txt bar.txt zot.txt)
endfunction(run_dashboard_command_line)

#-----------------------------------------------------------------------------
# Function to run the dashboard through a script
function(run_dashboard_script name)
  run_child(
    WORKING_DIRECTORY ${TOP}
    COMMAND ${CMAKE_CTEST_COMMAND} -S ${name} -V
    )

  # Verify the updates reported by CTest.
  check_updates(dash-binary foo.txt bar.txt zot.txt)
endfunction(run_dashboard_script)

#-----------------------------------------------------------------------------
# Function to initialize the testing directory.
function(init_testing)
  file(REMOVE_RECURSE ${TOP})
  file(MAKE_DIRECTORY ${TOP})
endfunction(init_testing)
