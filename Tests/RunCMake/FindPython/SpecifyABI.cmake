enable_language(C)

set(${PYTHON}_FIND_ABI "ANY;ANY;ANY")

find_package(${PYTHON} REQUIRED COMPONENTS Development.Module)
