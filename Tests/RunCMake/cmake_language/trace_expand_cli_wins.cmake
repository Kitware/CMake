set(bar "bar")
cmake_language(TRACE ON)
set(foo "${bar}")  # NOTE Will be expanded
cmake_language(TRACE OFF)
set(foo "${bar}")
