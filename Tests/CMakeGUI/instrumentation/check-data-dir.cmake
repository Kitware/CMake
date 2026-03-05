set(instrumentation_v1 "${instrumentation_build_dir}/.cmake/instrumentation/v1")

file(GLOB_RECURSE queries LIST_DIRECTORIES false ${instrumentation_v1}/query/*)
list(LENGTH queries n_queries)

if (NOT n_queries EQUAL 1)
  message(FATAL_ERROR "Expected one instrumentation query, got: ${n_queries}")
endif()

file(GLOB snippets_configure ${instrumentation_v1}/data/configure-*)
list(LENGTH snippets_configure n_snippets_configure)

if (NOT n_snippets_configure EQUAL 2)
  message(FATAL_ERROR "Expected two configure snippets, got: ${n_snippets_configure}")
endif()

file(GLOB snippets_generate ${instrumentation_v1}/data/generate-*)
list(LENGTH snippets_generate n_snippets_generate)

if (NOT n_snippets_generate EQUAL 1)
  message(FATAL_ERROR "Expected one generate snippet, got: ${n_snippets_generate}")
endif()

file(GLOB content ${instrumentation_v1}/data/content/*)
list(LENGTH content n_content)
if (NOT n_content EQUAL 1)
  message(FATAL_ERROR "Expected one content file, got: ${n_content}")
endif()

# The earliest configure snippet should not have an associated content file,
# because we immediately re-configured, so the corresponding generation never
# occurred.
list(GET snippets_configure 0 first_configure)
file(READ "${first_configure}" configure_json)
string(JSON configure_content_file
  ERROR_VARIABLE configure_content_error
  GET "${configure_json}" "cmakeContent")
if (configure_content_error)
  message(FATAL_ERROR "Failed to get cmakeContent from ${first_configure}: ${configure_content_error}")
endif()
if (NOT configure_content_file STREQUAL "")
  message(FATAL_ERROR "Expected null for cmakeContent in ${first_configure}, got: ${configure_content_file}")
endif()

execute_process(
  COMMAND
    "${CMAKE_CTEST_COMMAND}" --collect-instrumentation
      "${instrumentation_build_dir}"
)
if (NOT EXISTS "${instrumentation_build_dir}/callback-count.txt")
  message(FATAL_ERROR "Expected a callback output file at: ${instrumentation_build_dir}/callback-count.txt")
endif()
file(STRINGS "${instrumentation_build_dir}/callback-count.txt" callback_lines)
list(LENGTH callback_lines n_callbacks)
if (NOT n_callbacks EQUAL 1)
  message(FATAL_ERROR "Expected one callback execution, got: ${n_callbacks}")
endif()
