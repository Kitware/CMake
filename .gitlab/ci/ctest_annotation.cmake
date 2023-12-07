function (ctest_annotation_report file)
  set(label "")

  if (EXISTS "${file}")
    file(READ "${file}" json)
  else ()
    set(json "{\"CDash\": []}")
  endif ()

  foreach (arg IN LISTS ARGN)
    if (NOT label)
      set(label "${arg}")
      continue ()
    endif ()

    set(item "{\"external_link\":{\"label\":\"${label}\",\"url\":\"${arg}\"}}")
    set(label "")

    string(JSON length LENGTH "${json}" "CDash")
    string(JSON json SET "${json}" "CDash" "${length}" "${item}")
  endforeach ()

  file(WRITE "${file}" "${json}")
endfunction ()

if (NOT DEFINED build_id)
  include("${CTEST_BINARY_DIRECTORY}/cdash-build-id" OPTIONAL)
endif ()
function (store_build_id build_id)
  file(WRITE "${CTEST_BINARY_DIRECTORY}/cdash-build-id"
    "set(build_id \"${build_id}\")\n")
endfunction ()
