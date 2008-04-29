ADD_DEFINITIONS(-DGENERATOR_EXTERN)

# add the executable which will be used for generating files
ADD_EXECUTABLE(generator_extern ../generator.cxx)
SET_TARGET_PROPERTIES(generator_extern PROPERTIES OUTPUT_NAME the_external_generator)

# add an executable which will be called from ADD_CUSTOM_COMMAND( ... POST_BUILD)
ADD_EXECUTABLE(dummy_generator ../generator.cxx)
