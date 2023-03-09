cmake_minimum_required(VERSION 3.5)
project(FindImageMagick C CXX)
include(CTest)

find_package(ImageMagick REQUIRED COMPONENTS Magick++ MagickWand)

add_executable(test_magick++ main_magick++.cxx)
target_link_libraries(test_magick++ PRIVATE ImageMagick::Magick++)
add_test(NAME test_magick++ COMMAND test_magick++)

add_executable(test_magick_wand main_magick_wand.c)
target_link_libraries(test_magick_wand ImageMagick::MagickWand)
add_test(NAME test_magick_wand COMMAND test_magick_wand)
