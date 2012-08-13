# - Find Latex
# This module finds if Latex is installed and determines where the
# executables are. This code sets the following variables:
#
#  LATEX_COMPILER:       path to the LaTeX compiler
#  PDFLATEX_COMPILER:    path to the PdfLaTeX compiler
#  BIBTEX_COMPILER:      path to the BibTeX compiler
#  MAKEINDEX_COMPILER:   path to the MakeIndex compiler
#  DVIPS_CONVERTER:      path to the DVIPS converter
#  PS2PDF_CONVERTER:     path to the PS2PDF converter
#  LATEX2HTML_CONVERTER: path to the LaTeX2Html converter
#

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if (WIN32)

  # Try to find the MikTex binary path (look for its package manager).

  find_path(MIKTEX_BINARY_PATH mpm.exe
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MiK\\MiKTeX\\CurrentVersion\\MiKTeX;Install Root]/miktex/bin"
    DOC
    "Path to the MikTex binary directory."
  )
  mark_as_advanced(MIKTEX_BINARY_PATH)

  # Try to find the GhostScript binary path (look for gswin32).

  get_filename_component(GHOSTSCRIPT_BINARY_PATH_FROM_REGISTERY_8_00
     "[HKEY_LOCAL_MACHINE\\SOFTWARE\\AFPL Ghostscript\\8.00;GS_DLL]" PATH
  )

  get_filename_component(GHOSTSCRIPT_BINARY_PATH_FROM_REGISTERY_7_04
     "[HKEY_LOCAL_MACHINE\\SOFTWARE\\AFPL Ghostscript\\7.04;GS_DLL]" PATH
  )

  find_path(GHOSTSCRIPT_BINARY_PATH gswin32.exe
    ${GHOSTSCRIPT_BINARY_PATH_FROM_REGISTERY_8_00}
    ${GHOSTSCRIPT_BINARY_PATH_FROM_REGISTERY_7_04}
    DOC "Path to the GhostScript binary directory."
  )
  mark_as_advanced(GHOSTSCRIPT_BINARY_PATH)

  find_path(GHOSTSCRIPT_LIBRARY_PATH ps2pdf13.bat
    "${GHOSTSCRIPT_BINARY_PATH}/../lib"
    DOC "Path to the GhostScript library directory."
  )
  mark_as_advanced(GHOSTSCRIPT_LIBRARY_PATH)

endif ()

find_program(LATEX_COMPILER
  NAMES latex
  PATHS ${MIKTEX_BINARY_PATH}
        /usr/bin
)

find_program(PDFLATEX_COMPILER
  NAMES pdflatex
  PATHS ${MIKTEX_BINARY_PATH}
        /usr/bin
)

find_program(BIBTEX_COMPILER
  NAMES bibtex
  PATHS ${MIKTEX_BINARY_PATH}
        /usr/bin
)

find_program(MAKEINDEX_COMPILER
  NAMES makeindex
  PATHS ${MIKTEX_BINARY_PATH}
        /usr/bin
)

find_program(DVIPS_CONVERTER
  NAMES dvips
  PATHS ${MIKTEX_BINARY_PATH}
        /usr/bin
)

find_program(DVIPDF_CONVERTER
  NAMES dvipdfm dvipdft dvipdf
  PATHS ${MIKTEX_BINARY_PATH}
        /usr/bin
)

if (WIN32)
  find_program(PS2PDF_CONVERTER
    NAMES ps2pdf14.bat
    PATHS ${GHOSTSCRIPT_LIBRARY_PATH}
  )
else ()
  find_program(PS2PDF_CONVERTER
    NAMES ps2pdf14 ps2pdf
  )
endif ()

find_program(LATEX2HTML_CONVERTER
  NAMES latex2html
  PATHS ${MIKTEX_BINARY_PATH}
        /usr/bin
)


mark_as_advanced(
  LATEX_COMPILER
  PDFLATEX_COMPILER
  BIBTEX_COMPILER
  MAKEINDEX_COMPILER
  DVIPS_CONVERTER
  DVIPDF_CONVERTER
  PS2PDF_CONVERTER
  LATEX2HTML_CONVERTER
)
