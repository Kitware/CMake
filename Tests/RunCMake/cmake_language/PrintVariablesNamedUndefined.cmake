set(source_dir "src")
set(binary_dir "build")

# One undefined name -> printed inline as <NOTFOUND>.
cmake_language(PRINT_VARIABLES NAMED source_dir binary_dir NOT_SET)

# Multiple undefined names -> each printed as <NOTFOUND> in the order given.
cmake_language(PRINT_VARIABLES NAMED source_dir NOT_SET binary_dir ALSO_NOT_SET)
