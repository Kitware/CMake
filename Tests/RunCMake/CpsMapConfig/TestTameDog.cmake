project(TestTameDog CXX)

set(ANIMAL_CONFIGS "TAME;DOG")
include(TestMappedConfig.cmake)

target_compile_definitions(test PRIVATE "EXPECTED=whine")
