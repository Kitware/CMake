set(ALL_FILES_GLOB "*.dmg")

function(getPackageContent FILE RESULT_VAR)
  get_filename_component(path_ "${FILE}" DIRECTORY)
  file(REMOVE_RECURSE "${path_}/content")
  file(MAKE_DIRECTORY "${path_}/content")
  execute_process(COMMAND ${HDIUTIL_EXECUTABLE} attach -mountroot ${path_}/content -nobrowse ${FILE}
          INPUT_FILE "${src_dir}/DragNDrop/Accept.txt"
          RESULT_VARIABLE attach_result_
          ERROR_VARIABLE attach_error_
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(attach_result_)
    message(FATAL_ERROR "Failed to attach DMG: '${attach_result_}';"
          " '${attach_error_}'.")
  endif()

  file(GLOB_RECURSE package_content_ LIST_DIRECTORIES true RELATIVE
      "${path_}/content" "${path_}/content/*")
  # Some versions of macOS have .Trashes, others do not.
  list(FILTER package_content_ EXCLUDE REGEX "/.Trashes$")
  set(${RESULT_VAR} "${package_content_}" PARENT_SCOPE)

  execute_process(COMMAND ${HDIUTIL_EXECUTABLE} detach ${path_}/content/volume-name
          RESULT_VARIABLE detach_result_
          ERROR_VARIABLE detach_error_
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(detach_result_)
    message(FATAL_ERROR "Failed to detach DMG: '${detach_result_}';"
          " '${detach_error_}'.")
  endif()
endfunction()

function(getPackageNameGlobexpr NAME COMPONENT VERSION REVISION FILE_NO RESULT_VAR)
  if(COMPONENT)
    set(COMPONENT "-${COMPONENT}")
  endif()

  set(${RESULT_VAR} "${NAME}-${VERSION}-Darwin${COMPONENT}.dmg" PARENT_SCOPE)
endfunction()

function(getPackageContentList FILE RESULT_VAR)
  getPackageContent("${FILE}" package_content_)

  set(${RESULT_VAR} "${package_content_}" PARENT_SCOPE)
endfunction()

function(toExpectedContentList FILE_NO CONTENT_VAR)
  set(prefix_ "volume-name")
  list(TRANSFORM ${CONTENT_VAR} PREPEND "${prefix_}" OUTPUT_VARIABLE prepared_)
  list(APPEND prepared_ "${prefix_}")

  set(${CONTENT_VAR} "${prepared_}" PARENT_SCOPE)
endfunction()
