cmake_minimum_required(VERSION 3.5)
project(TestFindSDL C)
include(CTest)

find_package(SDL)

add_definitions(
  -DCMAKE_EXPECTED_SDL_VERSION_MAJOR=${SDL_VERSION_MAJOR}
  -DCMAKE_EXPECTED_SDL_VERSION_MINOR=${SDL_VERSION_MINOR}
  -DCMAKE_EXPECTED_SDL_VERSION_PATCH=${SDL_VERSION_PATCH})

add_executable(test_sdl_tgt main.c)
target_link_libraries(test_sdl_tgt SDL::SDL)
add_test(NAME test_sdl_tgt COMMAND test_sdl_tgt)

add_executable(test_sdl_var main.c)
target_include_directories(test_sdl_var PRIVATE ${SDL_INCLUDE_DIRS})
target_link_libraries(test_sdl_var PRIVATE ${SDL_LIBRARIES})
add_test(NAME test_sdl_var COMMAND test_sdl_var)
