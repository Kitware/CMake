include(${RunCMake_SOURCE_DIR}/CheckDashboard.cmake)
check_dashboard_files(Update.xml Configure.xml Build.xml)
check_dashboard_tests(Test.xml)
check_dashboard_content(Coverage.xml "<File Name=\"fails.cxx\"")
check_dashboard_submitted("Step9: command line"
  Configure.xml Update.xml Build.xml Test.xml Coverage.xml CoverageLog-0.xml Done.xml
)
