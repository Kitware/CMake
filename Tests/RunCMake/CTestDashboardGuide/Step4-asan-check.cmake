include(${RunCMake_SOURCE_DIR}/CheckDashboard.cmake)
check_dashboard_files(Update.xml Configure.xml Build.xml Notes.xml)
check_dashboard_content(DynamicAnalysis.xml "<Defect type=\"heap-use-after-free\">")
check_dashboard_submitted("Step4: AddressSanitizer"
    Update.xml Configure.xml Build.xml DynamicAnalysis.xml DynamicAnalysis-Test.xml Notes.xml Done.xml
)
