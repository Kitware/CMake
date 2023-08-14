cmake_minimum_required(VERSION 3.5)
project(TestFindDevIL C)
include(CTest)

find_package(DevIL)

#FIXME: check version too.
# add_definitions(
#  -DCMAKE_EXPECTED_SDL_VERSION_MAJOR=${SDL_VERSION_MAJOR}
#  -DCMAKE_EXPECTED_SDL_VERSION_MINOR=${SDL_VERSION_MINOR}
#  -DCMAKE_EXPECTED_SDL_VERSION_PATCH=${SDL_VERSION_PATCH})

add_executable(test_devil_var main.c)
target_include_directories(test_devil_var PRIVATE ${IL_INCLUDE_DIRS})
target_link_libraries(test_devil_var PRIVATE ${IL_LIBRARIES})
add_test(NAME test_devil_var COMMAND test_devil_var)

add_executable(test_devil_il_tgt main.c)
target_link_libraries(test_devil_il_tgt DevIL::IL)
add_test(NAME test_devil_il_tgt COMMAND test_devil_il_tgt)

add_executable(test_devil_ilu_tgt main_ilu.c)
target_link_libraries(test_devil_ilu_tgt DevIL::ILU)

add_executable(test_devil_ilu_var main_ilu.c)
target_include_directories(test_devil_ilu_var PRIVATE ${IL_INCLUDE_DIRS} ${ILU_INCLUDE_DIRS})
target_link_libraries(test_devil_ilu_var ${IL_LIBRARIES} ${ILU_LIBRARIES})

#FIXME: Check DevIL::ILUT target
