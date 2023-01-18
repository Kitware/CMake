
#pragma once

#ifdef _WIN32
#  define EXPORT __declspec(dllexport)
#  define IMPORT __declspec(dllimport)
#else
#  define EXPORT __attribute__((__visibility__("default")))
#  define IMPORT
#endif

struct result_type
{
  int input;
  int sum;
};
