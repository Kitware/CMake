if (NOT EXISTS ${v1}/postCMakeWorkflow.hook)
  set(RunCMake_TEST_FAILED "postCMakeWorkflow hook did not run\n")
endif()
