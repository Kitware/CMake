file(GLOB _files ${ECOS_DIR}/*)

# remove all directories, which consist of lower-case letters only
# this skips e.g. CVS/ and .subversion/
foreach(_entry ${_files})
   if(IS_DIRECTORY ${_entry})
      get_filename_component(dir ${_entry} NAME)
      if(${dir} MATCHES "^[a-z]+$")
         file(REMOVE_RECURSE ${_entry})
      endif(${dir} MATCHES "^[a-z]+$")
   endif(IS_DIRECTORY ${_entry})
endforeach(_entry)
