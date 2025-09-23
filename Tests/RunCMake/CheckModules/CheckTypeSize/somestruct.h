#ifndef _CMAKE_SOMESTRUCT_H
#define _CMAKE_SOMESTRUCT_H

#ifdef __cplusplus
namespace ns {
#endif

struct somestruct
{
  int someint;
  void* someptr;
  char somechar;
  long somelong;
};

#ifdef __cplusplus
} /* namespace ns { */
#endif

#endif
