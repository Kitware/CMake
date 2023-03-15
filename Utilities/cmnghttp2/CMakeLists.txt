# Disable warnings to avoid changing 3rd party code.
if(CMAKE_C_COMPILER_ID MATCHES
    "^(GNU|LCC|Clang|AppleClang|IBMClang|XLClang|XL|VisualAge|SunPro|HP|Intel|IntelLLVM|NVHPC)$")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
elseif(CMAKE_C_COMPILER_ID STREQUAL "PathScale")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -woffall")
endif()

# Re-use some check result cache entries from cmcurl:
# * HAVE_ARPA_INET_H    (referenced in cmakeconfig.h.in)
# * HAVE_NETINET_IN_H   (referenced in cmakeconfig.h.in)
# * HAVE_SIZEOF_SSIZE_T (referenced here)
if(NOT HAVE_SIZEOF_SSIZE_T)
  set(ssize_t KWIML_INT_intptr_t)
endif()
configure_file(cmakeconfig.h.in config.h)

add_library(cmnghttp2 STATIC
  lib/nghttp2_buf.c
  lib/nghttp2_callbacks.c
  lib/nghttp2_debug.c
  lib/nghttp2_extpri.c
  lib/nghttp2_frame.c
  lib/nghttp2_hd.c
  lib/nghttp2_hd_huffman.c
  lib/nghttp2_hd_huffman_data.c
  lib/nghttp2_helper.c
  lib/nghttp2_http.c
  lib/nghttp2_map.c
  lib/nghttp2_mem.c
  lib/nghttp2_npn.c
  lib/nghttp2_option.c
  lib/nghttp2_outbound_item.c
  lib/nghttp2_pq.c
  lib/nghttp2_priority_spec.c
  lib/nghttp2_queue.c
  lib/nghttp2_rcbuf.c
  lib/nghttp2_session.c
  lib/nghttp2_stream.c
  lib/nghttp2_submit.c
  lib/nghttp2_version.c
  )

target_compile_definitions(cmnghttp2
  PUBLIC NGHTTP2_STATICLIB
  PRIVATE HAVE_CONFIG_H
  )
target_include_directories(cmnghttp2 PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/includes
  )

install(FILES COPYING DESTINATION ${CMAKE_DOC_DIR}/cmnghttp2)
