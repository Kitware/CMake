file(READ "${RunCMake_TEST_BINARY_DIR}/list-conversion-generated.txt" content)

set(reference "<ARGS>AA,BB</ARGS>|<ARGS>CC,DD</ARGS>|<ARGS>EE,FF,GG</ARGS>")
if(NOT content MATCHES "${reference}")
  set(RunCMake_TEST_FAILED "actual content:\n [[${content}]]\nbut expected:\n [[${reference}]]")
endif()
