/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#define cmFindPackageStack_cxx
#include "cmFindPackageStack.h"

#include "cmStack.tcc" // IWYU pragma: keep
template class cmStack<cmFindPackageCall, cmFindPackageStack>;

template cmFindPackageCall&
cmStack<cmFindPackageCall, cmFindPackageStack>::Top<true>();

cmFindPackageCall const& cmFindPackageStack::Top() const
{
  return this->cmStack::Top();
}
