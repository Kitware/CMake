# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(test C)

add_library(sharedLib SHARED file.c)

add_library(moduleLib MODULE file.c)
