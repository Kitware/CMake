project(TestTameDog CXX)

set(ANIMAL_CONFIGS "WILD;DOG")
include(TestMappedConfig.cmake)

target_compile_definitions(test PRIVATE "EXPECTED=growl")
