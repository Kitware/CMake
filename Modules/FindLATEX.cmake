#
# This module finds if Latex is installed and determines where the
# executables are. This code sets the following variables:
#
#  LATEX_COMPILE        = the full path to the LaTeX compiler
#  BIBTEX_COMPILE       = the full path to the BibTeX compiler
#  DVIPDF_COMPILE       = the full path to the DVIPDF converter
#  DVIPS_COMPILE        = the full path to the DVIPS converter
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

