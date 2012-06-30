set(CURL_TESTS
    ftpget
    ftpgetresp
    ftpupload
    getinmemory
    persistant
    sepheaders
    simple
   )

configure_file(${LIBCURL_SOURCE_DIR}/Testing/testconfig.h.in
               ${LIBCURL_BINARY_DIR}/Testing/testconfig.h)

include_directories(${LIBCURL_BINARY_DIR}/Testing)

foreach(TEST ${CURL_TESTS})
  add_executable(${TEST} ${TEST}.c)
  target_link_libraries(${TEST} cmcurl)
endforeach(TEST)
