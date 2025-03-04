/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "xyz.h"

#include <stdio.h>

Xyz::Xyz()
  : QObject()
{
}

void Xyz::doXyz()
{
  printf("This is xyz !\n");
}
