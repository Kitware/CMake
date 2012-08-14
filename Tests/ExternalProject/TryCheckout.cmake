find_package(CVS)
find_package(Subversion)


function(try_cvs_checkout repository module dir result_var)
  # Assume cvs checkouts will not work:
  set(${result_var} 0 PARENT_SCOPE)

  if(CVS_EXECUTABLE)
    message(STATUS "try_cvs_checkout")

    # Ensure directory exists so we can call cvs in it:
    file(MAKE_DIRECTORY "${dir}")

    # Try to do the cvs checkout command:
    execute_process(COMMAND ${CVS_EXECUTABLE} -d ${repository} co ${module}
      WORKING_DIRECTORY ${dir}
      TIMEOUT 30
      RESULT_VARIABLE rv)

    # If it worked, cvs checkouts will work:
    if(rv EQUAL 0)
      set(${result_var} 1 PARENT_SCOPE)
    endif()

    message(STATUS "try_cvs_checkout -- done")
  endif()
endfunction()


function(try_svn_checkout repository dir result_var)
  # Assume svn checkouts will not work:
  set(${result_var} 0 PARENT_SCOPE)

  if(Subversion_SVN_EXECUTABLE)
    message(STATUS "try_svn_checkout")

    # Ensure directory exists so we can call svn in it:
    file(MAKE_DIRECTORY "${dir}")

    # Try to do the svn checkout command:
    execute_process(COMMAND ${Subversion_SVN_EXECUTABLE} co ${repository} ${dir}
      WORKING_DIRECTORY ${dir}
      TIMEOUT 30
      RESULT_VARIABLE rv)

    # If it worked, svn checkouts will work:
    if(rv EQUAL 0)
      set(${result_var} 1 PARENT_SCOPE)
    endif()

    message(STATUS "try_svn_checkout -- done")
  endif()
endfunction()
