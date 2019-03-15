#include <libinput.h>
#include <stdio.h>

int main()
{
  struct libinput_interface interface;
  interface.open_restricted = 0;
  interface.close_restricted = 0;
  struct libinput* li;
  li = libinput_udev_create_context(&interface, NULL, NULL);
  printf("Found Libinput.\n");
  return 0;
}
