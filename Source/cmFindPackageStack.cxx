/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#define cmFindPackageStack_cxx
#include "cmFindPackageStack.h"

#include "cmStack.tcc" // IWYU pragma: keep
template class cmStack<cmFindPackageCall const, cmFindPackageStack,
                       cmStackType::Const>;
