cmake_minimum_required(VERSION 3.23)

project(Tutorial)

# TODO1: Add a default ON option for a cache variable named:
#          TUTORIAL_BUILD_UTILITIES.
#        option() requires a doc string as its second argument, set this to
#        something like:
#          "Build the Tutorial executable"


# TODO2: Add a conditional statement around add_subdirectory(Tutorial). Only
#        build the Tutorial target if TUTORIAL_BUILD_UTILITIES is ON (or
#        otherwise truthy).

add_subdirectory(Tutorial)

add_subdirectory(MathFunctions)
