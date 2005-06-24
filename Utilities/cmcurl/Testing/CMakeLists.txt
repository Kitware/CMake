SET(CURL_TESTS
    ftpget
    ftpgetresp
    ftpupload
    getinmemory
    persistant
    sepheaders
    simple
   )

CONFIGURE_FILE(${LIBCURL_SOURCE_DIR}/Testing/testconfig.h.in
               ${LIBCURL_BINARY_DIR}/Testing/testconfig.h)

INCLUDE_DIRECTORIES(${LIBCURL_BINARY_DIR}/Testing)

FOREACH(TEST ${CURL_TESTS})
  ADD_EXECUTABLE(${TEST} ${TEST}.c)
  TARGET_LINK_LIBRARIES(${TEST} cmcurl)
ENDFOREACH(TEST)
