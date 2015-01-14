ctest_submit
------------

Submit results to a dashboard server.

::

  ctest_submit([PARTS ...] [FILES ...]
               [RETRY_COUNT count]
               [RETRY_DELAY delay]
               [RETURN_VALUE res]
               )

By default all available parts are submitted if no PARTS or FILES are
specified.  The PARTS option lists a subset of parts to be submitted.
Valid part names are:

::

  Start      = nothing
  Update     = ctest_update results, in Update.xml
  Configure  = ctest_configure results, in Configure.xml
  Build      = ctest_build results, in Build.xml
  Test       = ctest_test results, in Test.xml
  Coverage   = ctest_coverage results, in Coverage.xml
  MemCheck   = ctest_memcheck results, in DynamicAnalysis.xml
  Notes      = Files listed by CTEST_NOTES_FILES, in Notes.xml
  ExtraFiles = Files listed by CTEST_EXTRA_SUBMIT_FILES
  Upload     = Files prepared for upload by ctest_upload(), in Upload.xml
  Submit     = nothing

The FILES option explicitly lists specific files to be submitted.
Each individual file must exist at the time of the call.

The RETRY_DELAY option specifies how long in seconds to wait after a
timed-out submission before attempting to re-submit.

The RETRY_COUNT option specifies how many times to retry a timed-out
submission.

::

  ctest_submit([CDASH_UPLOAD file]
               [CDASH_UPLOAD_TYPE type_string])

This second signature is used to upload files to CDash via the CDash
file upload API. The api first sends a request to upload to CDash along
with the md5 sum of the file. If CDash does not already have the file,
then it is uploaded. Along with the file, a CDash type string is specified
to tell CDash which handler to use to process the data.
