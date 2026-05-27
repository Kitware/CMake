# Verify the build system reused the owning target BMI when compatible and
# generated a synthetic target BMI only for the incompatible importer.
set(expected_consumers consumer20 consumer23)
set(linked_dir_keys "")
set(linked_dir_names "")

if (DEFINED RunCMake_TEST_CONFIG)
  set(config_dir "${RunCMake_TEST_CONFIG}")
else ()
  set(config_dir "Debug")
endif ()

# Get and check linked-target-dirs for each consumer
foreach (consumer IN LISTS expected_consumers)

  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(output_dir "${consumer}.dir/${config_dir}")
  else ()
    set(output_dir "${consumer}.dir")
  endif ()

  set(depend_info_file "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/${output_dir}/CXXDependInfo.json")
  if (NOT EXISTS "${depend_info_file}")
    list(APPEND RunCMake_TEST_FAILED
      "Could not find CXXDependInfo.json for consumer ${consumer}: checked ${depend_info_file}")
    continue()
  endif ()

  file(READ "${depend_info_file}" depend_info_json)

  # Extract linked-target-dirs array length and first element
  string(JSON linked_dirs_len LENGTH "${depend_info_json}" "linked-target-dirs")
  string(JSON linked_dirs GET "${depend_info_json}" "linked-target-dirs")

  if (consumer STREQUAL "consumer20")
    set(expected_linked_tgt_regex "^importable\.dir$")
    set(expected_linked_tgt_desc "owning target dir for 'importable'")
    set(expected_linked_tgt_has_bmi 0)
  elseif (consumer STREQUAL "consumer23")
    set(expected_linked_tgt_regex "^importable@synth_[A-Za-z0-9_]+\.dir$")
    set(expected_linked_tgt_desc "synthetic target dir for 'importable'")
    set(expected_linked_tgt_has_bmi 1)
  else ()
    list(APPEND RunCMake_TEST_FAILED
      "No linked target expectation defined for consumer '${consumer}'")
    continue()
  endif ()

  if (NOT linked_dirs_len GREATER 0)
    list(APPEND RunCMake_TEST_FAILED
      "Consumer '${consumer}' has no linked-target-dirs but expected ${expected_linked_tgt_desc}")
    continue()
  endif ()

  # For this test, expect exactly one linked target dir per consumer
  if (NOT linked_dirs_len EQUAL 1)
    list(APPEND RunCMake_TEST_FAILED
      "Expected 1 linked-target-dir for '${consumer}' but found ${linked_dirs_len}: ${linked_dirs}")
    continue()
  endif ()

  string(JSON linked_dir GET "${depend_info_json}" "linked-target-dirs" 0)

  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    cmake_path(GET linked_dir PARENT_PATH linked_tgt_root)
  else ()
    set(linked_tgt_root "${linked_dir}")
  endif ()

  cmake_path(GET linked_tgt_root FILENAME linked_tgt_name)

  if (NOT linked_tgt_name MATCHES "${expected_linked_tgt_regex}")
    list(APPEND RunCMake_TEST_FAILED
      "Consumer '${consumer}' should link to ${expected_linked_tgt_desc} but found ${linked_dir}")
    continue()
  endif ()

  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(linked_output_dir "${linked_dir}/${config_dir}")
  else ()
    set(linked_output_dir "${linked_dir}")
  endif ()

  # Verify the linked target dir exists and contains a BMI
  if (NOT EXISTS "${linked_dir}")
    list(APPEND RunCMake_TEST_FAILED
      "Consumer '${consumer}' links to target directory that does not exist: ${linked_dir}")
    continue()
  endif ()

  if (expected_linked_tgt_has_bmi)
    file(GLOB_RECURSE bmi_files "${linked_dir}/*.bmi")
    if (NOT bmi_files)
      list(APPEND RunCMake_TEST_FAILED
        "No BMI files found in target directory: ${linked_dir}")
      continue()
    endif ()
  endif ()

  if (linked_tgt_name MATCHES "^importable@synth_[A-Za-z0-9_]+\.dir$")
    # Record the consumers of each synthetic dir to verify uniqueness.
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" linked_dir_key "${linked_tgt_name}")
    set(linked_dir_consumers_var "linked_dir_consumers_${linked_dir_key}")
    if (NOT DEFINED ${linked_dir_consumers_var})
      list(APPEND linked_dir_keys "${linked_dir_key}")
      list(APPEND linked_dir_names "${linked_tgt_name}")
    endif ()

    list(APPEND ${linked_dir_consumers_var} ${consumer})
  endif ()

endforeach ()

# Verify each synthetic target is consumed exactly once.
foreach (linked_dir_key IN LISTS linked_dir_keys)
  set(consumers "${linked_dir_consumers_${linked_dir_key}}")
  list(LENGTH consumers linked_dir_consumer_count)

  if (linked_dir_consumer_count GREATER 1)
    list(FIND linked_dir_keys ${linked_dir_key} linked_dir_index)
    list(GET linked_dir_names ${linked_dir_index} linked_tgt_name)

    string(JOIN ", " linked_dir_consumers_joined ${consumers})
    list(APPEND RunCMake_TEST_FAILED
      "Expected per-compatibility synthetic BMI generation, but '${linked_tgt_name}' is linked to by multiple targets: ${linked_dir_consumers_joined}")
  endif ()
endforeach ()

string(REPLACE ";" "\n  " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
