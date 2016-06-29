/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTypeMacro_h
#define cmTypeMacro_h

// All subclasses of cmCommand or cmCTestGenericHandler should
// invoke this macro.
#define cmTypeMacro(thisClass, superclass)                                    \
  const char* GetNameOfClass() CM_OVERRIDE { return #thisClass; }             \
  typedef superclass Superclass;                                              \
  static bool IsTypeOf(const char* type)                                      \
  {                                                                           \
    if (!strcmp(#thisClass, type)) {                                          \
      return true;                                                            \
    }                                                                         \
    return Superclass::IsTypeOf(type);                                        \
  }                                                                           \
  bool IsA(const char* type) CM_OVERRIDE                                      \
  {                                                                           \
    return thisClass::IsTypeOf(type);                                         \
  }                                                                           \
  static thisClass* SafeDownCast(cmObject* c)                                 \
  {                                                                           \
    if (c && c->IsA(#thisClass)) {                                            \
      return static_cast<thisClass*>(c);                                      \
    }                                                                         \
    return 0;                                                                 \
  }                                                                           \
  class cmTypeMacro_UseTrailingSemicolon

#endif
