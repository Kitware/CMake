/* gethostname.c: minimal substitute for missing gethostname() function
 * created 2000-Mar-02 jmk
 * requires SVR4 uname() and -lc
 *
 * by Jim Knoble <jmknoble@pobox.com>
 * Copyright ? 2000 Jim Knoble
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This software is provided "as is", without warranty of any kind,
 * express or implied, including but not limited to the warranties of
 * merchantability, fitness for a particular purpose and
 * noninfringement. In no event shall the author(s) be liable for any
 * claim, damages or other liability, whether in an action of contract,
 * tort or otherwise, arising from, out of or in connection with the
 * software or the use or other dealings in the software.
 */

#include <string.h>
#include <sys/utsname.h>

int gethostname(char *name, size_t len)
{
   struct utsname u;
   int status = uname(&u);
   if (-1 != status) {
      strncpy(name, u.nodename, len);
      name[len - 1] = '\0';
   }
   return(status);
}

