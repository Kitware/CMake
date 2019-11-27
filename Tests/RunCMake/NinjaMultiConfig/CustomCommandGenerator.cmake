enable_language(C)

add_library(generatorlib STATIC generatorlib.c)
add_library(generatorobj OBJECT generatorobj.c)
add_library(emptyobj OBJECT empty.c)
add_library(emptyobj2 OBJECT empty.c)

add_executable(generator generator.c $<TARGET_OBJECTS:generatorobj>)
target_link_libraries(generator PRIVATE generatorlib)

add_custom_command(OUTPUT generated.c COMMAND generator generated.c)
add_executable(generated ${CMAKE_BINARY_DIR}/generated.c $<TARGET_OBJECTS:generatorobj> $<TARGET_OBJECTS:emptyobj>)
target_link_libraries(generated PRIVATE generatorlib)

file(GENERATE OUTPUT include/genex/$<CONFIG>/genex_config.h CONTENT
"#ifndef GENEX_CONFIG_H
#define GENEX_CONFIG_H

#define GENEX_CONFIG_INCLUDE_DIR \"$<CONFIG>\"

#endif /* GENEX_CONFIG_H */
")
file(GENERATE OUTPUT include/intdir/$<CONFIG>/intdir_config.h CONTENT
"#ifndef INTDIR_CONFIG_H
#define INTDIR_CONFIG_H

#define INTDIR_CONFIG_INCLUDE_DIR \"$<CONFIG>\"

#endif /* INTDIR_CONFIG_H */
")

foreach(g generatorlib generatorobj generator generated)
  target_compile_definitions(${g} PRIVATE
    "GENEX_CONFIG_DEFINITION=\"$<CONFIG>\""
  # FIXME Get this working
  #  "INTDIR_CONFIG_DEFINITION=\"${CMAKE_CFG_INTDIR}\""
    )
  target_include_directories(${g} PRIVATE
    "${CMAKE_BINARY_DIR}/include/genex/$<CONFIG>"
  # FIXME Get this working
  #  "${CMAKE_BINARY_DIR}/include/intdir/${CMAKE_CFG_INTDIR}"
    )
endforeach()

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)
generate_output_files(generatorlib generatorobj emptyobj generator generated)

file(APPEND "${CMAKE_BINARY_DIR}/target_files.cmake" "set(GENERATED_FILES [==[${CMAKE_BINARY_DIR}/generated.c]==])\n")
set(genfiles)
foreach(cfg Debug Release MinSizeRel RelWithDebInfo)
  list(APPEND genfiles
    ${CMAKE_BINARY_DIR}/include/genex/${cfg}/genex_config.h
    ${CMAKE_BINARY_DIR}/include/intdir/${cfg}/intdir_config.h
    )
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/target_files.cmake" "set(CONFIG_FILES [==[${genfiles}]==])\n")
