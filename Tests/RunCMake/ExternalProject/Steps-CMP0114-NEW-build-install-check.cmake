check_steps_present(proj0 download configure build)
check_steps_present(proj1 download patch configure build install)
check_steps_missing(proj1 test)
