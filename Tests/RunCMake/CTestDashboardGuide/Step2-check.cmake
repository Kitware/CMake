include(${RunCMake_SOURCE_DIR}/CheckDashboard.cmake)
check_dashboard_files(Update.xml Configure.xml Notes.xml Test.xml)
check_dashboard_content(Build.xml "<Failure type=\"Error\">.*undeclared_function")
check_dashboard_submitted("Step2: launchers"
    Update.xml Configure.xml Build.xml Test.xml Notes.xml Done.xml
)
