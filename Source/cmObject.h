/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmObject_h
#define cmObject_h

#include <cmConfigure.h>

#include "cmStandardIncludes.h"

/** \class cmObject
 * \brief Superclass for all commands and other classes in CMake.
 *
 * cmObject is the base class for all classes in CMake. It defines some
 * methods such as GetNameOfClass, IsA, SafeDownCast.
 */
class cmObject
{
public:
  /**
   * Need virtual destructor to destroy real command type.
   */
  virtual ~cmObject() {}

  /**
   * The class name of the command.
   */
  virtual const char* GetNameOfClass() = 0;

  /**
   * Returns true if this class is the given class, or a subclass of it.
   */
  static bool IsTypeOf(const char* type) { return !strcmp("cmObject", type); }

  /**
   * Returns true if this object is an instance of the given class or
   * a subclass of it.
   */
  virtual bool IsA(const char* type) { return cmObject::IsTypeOf(type); }
};

#endif
