include(${RunCMake_SOURCE_DIR}/CheckDashboard.cmake)
check_dashboard_files(Update.xml Configure.xml Notes.xml)
check_dashboard_tests(Test.xml)
check_dashboard_content(Coverage.xml "<File Name=\"fails.cxx\"")
check_dashboard_content(Build.xml "<Commands>")
check_dashboard_submitted("Step5: instrumentation"
    Update.xml Configure.xml Build.xml Test.xml Notes.xml Done.xml Coverage.xml CoverageLog-0.xml
)
