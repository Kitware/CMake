/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <QtGlobal>

// The signed integer type that Qt uses for indexing.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using cm_qsizetype = qsizetype;
#else
using cm_qsizetype = int;
#endif
