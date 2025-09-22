set(bar "bar")
cmake_language(TRACE ON EXPAND)
  set(foo "${bar}")
cmake_language(TRACE OFF)
