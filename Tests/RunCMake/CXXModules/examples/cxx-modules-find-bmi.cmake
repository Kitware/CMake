function (check_for_bmi prefix destination name)
  set(found 0)
  foreach (ext IN ITEMS gcm ifc pcm)
    if (EXISTS "${prefix}/${destination}/${name}.${ext}")
      set(found 1)
      break ()
    endif ()
  endforeach ()

  if (NOT found)
    message(SEND_ERROR
      "Failed to find the ${name} BMI")
  endif ()
endfunction ()

function (check_for_interface prefix destination subdir name)
  set(found 0)
  if (NOT EXISTS "${prefix}/${destination}/${subdir}/${name}")
    message(SEND_ERROR
      "Failed to find the ${name} module interface")
  endif ()
endfunction ()

function (report_dirs prefix destination)
  message("prefix: ${prefix}")
  message("destination: ${destination}")
endfunction ()
