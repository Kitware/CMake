#
# This module finds if Latex is installed and determines where the
# executables are. This code sets the following variables:
#
#  LATEX_COMPILE        = the full path to the LaTeX compiler
#  BIBTEX_COMPILE       = the full path to the BibTeX compiler
#  DVIPDF_COMPILE       = the full path to the DVIPDF converter
#  PS2PDF_COMPILE       = the full path to the PS2PDF converter
#  DVIPS_COMPILE        = the full path to the DVIPS converter
#  MAKEINDEX_COMPILE    = the full path to the MakeIndex compiler
#

FIND_PROGRAM(LATEX_COMPILE
  NAMES latex
)

FIND_PROGRAM(BIBTEX_COMPILE
  NAMES bibtex
)

FIND_PROGRAM(DVIPDF_COMPILE
  NAMES dvipdf
)

FIND_PROGRAM(DVIPS_COMPILE
  NAMES dvips
)

FIND_PROGRAM(MAKEINDEX_COMPILE
  NAMES makeindex
)

FIND_PROGRAM(PS2PDF_COMPILE
  NAMES ps2pdf
)

