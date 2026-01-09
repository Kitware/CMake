project(TestTameCat CXX)

set(ANIMAL_CONFIGS "WILD;CAT")
include(TestMappedConfig.cmake)

target_compile_definitions(test PRIVATE "EXPECTED=roar")
