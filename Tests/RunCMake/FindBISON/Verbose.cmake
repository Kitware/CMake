include(${CMAKE_CURRENT_LIST_DIR}/check.cmake)
find_package(BISON REQUIRED)

bison_target(Verbose parser.y sub/parser1.c VERBOSE)
check_var(BISON_Verbose_OPTIONS "-d;--verbose")
check_var(BISON_Verbose_OUTPUTS
  "sub/parser1.c;${CMAKE_CURRENT_BINARY_DIR}/sub/parser1.output;sub/parser1.h")

bison_target(ReportFile parser.y parser2.c VERBOSE REPORT_FILE report2.txt)
check_var(BISON_ReportFile_OPTIONS "-d;--report-file=report2.txt;--verbose")
check_var(BISON_ReportFile_OUTPUTS
  "parser2.c;${CMAKE_CURRENT_BINARY_DIR}/report2.txt;parser2.h")

set(report "${CMAKE_CURRENT_BINARY_DIR}/reports/report3.txt")
bison_target(ReportFileAbsolute parser.y parser3.c VERBOSE REPORT_FILE "${report}")
check_var(BISON_ReportFileAbsolute_OPTIONS "-d;--report-file=${report};--verbose")
check_var(BISON_ReportFileAbsolute_OUTPUTS "parser3.c;${report};parser3.h")

# REPORT_FILE without VERBOSE passes the option but adds no output.
bison_target(ReportFileOnly parser.y parser4.c REPORT_FILE report4.txt)
check_var(BISON_ReportFileOnly_OPTIONS "-d;--report-file=report4.txt")
check_var(BISON_ReportFileOnly_OUTPUTS "parser4.c;parser4.h")

# Deprecated VERBOSE <file> form copies the report to <file>.
set(out "${CMAKE_CURRENT_BINARY_DIR}/parser5.c")
bison_target(VerboseFile parser.y "${out}" VERBOSE copy5.output)
check_var(BISON_VerboseFile_OPTIONS "-d;--verbose")
check_var(BISON_VerboseFile_OUTPUTS
  "${out};${CMAKE_CURRENT_BINARY_DIR}/parser5.output;${CMAKE_CURRENT_BINARY_DIR}/parser5.h;${CMAKE_CURRENT_BINARY_DIR}/copy5.output")
