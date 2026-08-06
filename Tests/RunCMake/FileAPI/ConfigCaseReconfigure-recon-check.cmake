set(reply_dir "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/reply")

# Every reply file referenced from the index must still exist on disk after
# the reconfigure.  A dangling "jsonFile" reference is the bug this guards; it
# manifests only on a case-insensitive filesystem (Windows, default macOS).
file(GLOB reply_files "${reply_dir}/*.json")
set(dangling "")
foreach(reply_file IN LISTS reply_files)
  file(READ "${reply_file}" content)
  string(REGEX MATCHALL "\"jsonFile\"[ \t]*:[ \t]*\"[^\"]+\"" refs "${content}")
  foreach(ref IN LISTS refs)
    string(REGEX REPLACE "\"jsonFile\"[ \t]*:[ \t]*\"([^\"]+)\"" "\\1" name "${ref}")
    if(NOT EXISTS "${reply_dir}/${name}")
      get_filename_component(from "${reply_file}" NAME)
      string(APPEND dangling "\n  '${name}' referenced by ${from} is missing")
    endif()
  endforeach()
endforeach()
if(dangling)
  set(RunCMake_TEST_FAILED "Dangling File API reply references:${dangling}")
endif()
