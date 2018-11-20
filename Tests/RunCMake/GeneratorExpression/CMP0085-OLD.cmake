cmake_policy(SET CMP0070 NEW)
file(GENERATE OUTPUT CMP0085-OLD-generated.txt CONTENT
  "$<IN_LIST:,>$<IN_LIST:,a>$<IN_LIST:,;a>$<IN_LIST:a,>$<IN_LIST:a,a>$<IN_LIST:a,;a>"
  )
