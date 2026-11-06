include(${RunCMake_SOURCE_DIR}/CheckDashboard.cmake)
check_dashboard_files(Update.xml Configure.xml Build.xml Notes.xml)
check_dashboard_tests(Test.xml)
check_dashboard_submitted("Step1: introduction"
    Update.xml Configure.xml Build.xml Test.xml Notes.xml Done.xml
)
