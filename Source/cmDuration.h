/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <chrono>
#include <ratio>

using cmDuration = std::chrono::duration<double, std::ratio<1>>;

/*
 * This function will return number of seconds in the requested type T.
 *
 * A duration_cast from duration<double> to duration<T> will not yield what
 * one might expect if the double representation does not fit into type T.
 * This function aims to safely convert, by clamping the double value between
 * the permissible valid values for T.
 */
template <typename T>
T cmDurationTo(cmDuration const& duration);

#ifndef CMDURATION_CPP
extern template int cmDurationTo<int>(cmDuration const&);
extern template unsigned int cmDurationTo<unsigned int>(cmDuration const&);
#endif
