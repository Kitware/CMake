set(REGEX_TO_MATCH "
.*.UseRelativePaths_Experimental = true.*
.*.UseDeterministicPaths_Experimental = true.*
.*.SourceMapping_Experimental = '/new/root'.*
.*.AllowResponseFile = true.*
.*.ExtraFiles =.*
.*{
.*'file1',
.*'file2'
.*}
")
include(${RunCMake_SOURCE_DIR}/check.cmake)
