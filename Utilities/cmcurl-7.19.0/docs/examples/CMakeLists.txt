set(CURL_EXAMPLES
  10-at-a-time 
  anyauthput 
  cookie_interface 
  debug 
  fileupload 
  fopen 
  ftpget 
  ftpgetresp 
  ftpupload 
  getinfo 
  getinmemory 
  http-post 
  httpcustomheader 
  https 
  post-callback 
  postit2 
  sendrecv 
  simple 
  simplepost 
  simplessl)

if(NOT WIN32)
  set(CURL_EXAMPLES ${CURL_EXAMPLES}
  httpput 
  multi-debugcallback 
  multi-double 
  multi-post 
  multi-single 
  persistant 
  sepheaders 
  multi-app )
endif(NOT WIN32)

foreach(EXAMPLE ${CURL_EXAMPLES})
  add_executable(${EXAMPLE} ${EXAMPLE}.c)
  target_link_libraries(${EXAMPLE} cmcurl)
endforeach(EXAMPLE)
