#include <gdk-pixbuf/gdk-pixbuf.h>

int main(int argc, char* argv[])
{
  char const* version = gdk_pixbuf_version;
  guint const major = gdk_pixbuf_major_version;
  guint const minor = gdk_pixbuf_minor_version;
  guint const micro = gdk_pixbuf_micro_version;
  return 0;
}
