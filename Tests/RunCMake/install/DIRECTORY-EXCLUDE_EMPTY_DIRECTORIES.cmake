set(CMAKE_INSTALL_MESSAGE "ALWAYS")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/prefix")

set(DIR_TO_INSTALL "${CMAKE_BINARY_DIR}/dir_to_install")
file(MAKE_DIRECTORY ${DIR_TO_INSTALL})

file(TOUCH ${DIR_TO_INSTALL}/empty.txt)

# make an empty folder
file(MAKE_DIRECTORY ${DIR_TO_INSTALL}/empty_folder)
# make empty subfolders under the empty folder
file(MAKE_DIRECTORY ${DIR_TO_INSTALL}/empty_folder/empty_subfolder1)
file(MAKE_DIRECTORY ${DIR_TO_INSTALL}/empty_folder/empty_subfolder2)

if(UNIX)
  # make an folder with a symlink
  file(MAKE_DIRECTORY ${DIR_TO_INSTALL}/folder_with_symlink)
  file(CREATE_LINK ${DIR_TO_INSTALL}/empty.txt
    ${DIR_TO_INSTALL}/folder_with_symlink/symlink_to_empty.txt
    SYMBOLIC
  )
endif()

install(DIRECTORY ${DIR_TO_INSTALL}
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    EXCLUDE_EMPTY_DIRECTORIES
)
