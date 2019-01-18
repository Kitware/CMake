#include "INTEGRITY.h"
#include "boottable.h"

void main()
{
  Exit(0);
}

/* This global table will be filled in during the Integrate phase with */
/* information about the AddressSpaces, Tasks, and Objects that are to be */
/* created.  If you do not plan to use Integrate, you may omit this file from
 */
/* the kernel, and the boot table code will then not be included. */

GlobalTable TheGlobalTable = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
