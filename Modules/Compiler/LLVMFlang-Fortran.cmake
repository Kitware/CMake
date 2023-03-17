set(CMAKE_Fortran_VERBOSE_FLAG "-v")

set(CMAKE_Fortran_SUBMODULE_SEP "-")
set(CMAKE_Fortran_SUBMODULE_EXT ".mod")

set(CMAKE_Fortran_PREPROCESS_SOURCE
    "<CMAKE_Fortran_COMPILER> -cpp <DEFINES> <INCLUDES> <FLAGS> -E <SOURCE> > <PREPROCESSED_SOURCE>")

set(CMAKE_Fortran_FORMAT_FIXED_FLAG "-ffixed-form")
set(CMAKE_Fortran_FORMAT_FREE_FLAG "-ffree-form")

set(CMAKE_Fortran_MODDIR_FLAG "-module-dir")

set(CMAKE_Fortran_COMPILE_OPTIONS_PREPROCESS_ON "-cpp")
set(CMAKE_Fortran_COMPILE_OPTIONS_PREPROCESS_OFF "-nocpp")
set(CMAKE_Fortran_POSTPROCESS_FLAG "-ffixed-line-length-72")
