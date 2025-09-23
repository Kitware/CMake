/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cm_sys_stat.h"

namespace cmFSPermissions {

// Table of permissions flags.
#if defined(_WIN32) && !defined(__CYGWIN__)
static mode_t const mode_owner_read = S_IREAD;
static mode_t const mode_owner_write = S_IWRITE;
static mode_t const mode_owner_execute = S_IEXEC;
static mode_t const mode_group_read = 040;
static mode_t const mode_group_write = 020;
static mode_t const mode_group_execute = 010;
static mode_t const mode_world_read = 04;
static mode_t const mode_world_write = 02;
static mode_t const mode_world_execute = 01;
static mode_t const mode_setuid = 04000;
static mode_t const mode_setgid = 02000;
#else
static mode_t const mode_owner_read = S_IRUSR;
static mode_t const mode_owner_write = S_IWUSR;
static mode_t const mode_owner_execute = S_IXUSR;
static mode_t const mode_group_read = S_IRGRP;
static mode_t const mode_group_write = S_IWGRP;
static mode_t const mode_group_execute = S_IXGRP;
static mode_t const mode_world_read = S_IROTH;
static mode_t const mode_world_write = S_IWOTH;
static mode_t const mode_world_execute = S_IXOTH;
static mode_t const mode_setuid = S_ISUID;
static mode_t const mode_setgid = S_ISGID;
#endif

bool stringToModeT(std::string const& arg, mode_t& permissions);

} // ns
