/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmDiagnosticContext.h"

#include <array>

#include "cmStateSnapshot.h"

void cmDiagnosticContext::RecordDiagnostic(cmDiagnosticCategory category,
                                           cmStateSnapshot const& state)
{
  this->DiagnosticState[category] = state.GetDiagnostic(category);
  this->HasState = true;
}
