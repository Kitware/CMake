enable_language(Fortran)

# Each use_*.f90 finds its module only through a name split across lines,
# either by hand or by LLVMFlang wrapping its preprocessed output.
add_library(split_token STATIC
  FortranSplitToken/use_split.f90
  FortranSplitToken/use_long.f90
  FortranSplitToken/use_keyword.f90
  FortranSplitToken/use_dcolon.f90
  FortranSplitToken/mod_split.f90
  FortranSplitToken/mod_long.f90
  FortranSplitToken/mod_keyword.f90
  FortranSplitToken/mod_dcolon.f90
  )
