file(GLOB exeFiles "${dir}/*.exe")
foreach(exeFile IN LISTS exeFiles)
  file(REMOVE "${exeFile}")
endforeach()
