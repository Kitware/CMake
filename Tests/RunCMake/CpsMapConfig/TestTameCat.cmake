project(TestTameCat CXX)

set(ANIMAL_CONFIGS "TAME;CAT")
include(TestMappedConfig.cmake)

target_compile_definitions(test PRIVATE "EXPECTED=meow")
