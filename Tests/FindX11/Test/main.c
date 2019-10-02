#ifdef HAVE_X11_ICE
#  include <X11/ICE/ICElib.h>

static Status test_ICE(void)
{
  return IceInitThreads();
}
#endif

#ifdef HAVE_X11_SM
#  include <X11/SM/SMlib.h>
#  include <stdlib.h>

static void test_SM(void)
{
  SmcProtocolVersion(NULL);
}
#endif

#ifdef HAVE_X11_X11
#  include <X11/Xlib.h>

static Status test_X11(void)
{
  return XInitThreads();
}
#endif

#ifdef HAVE_X11_Xau
#  include <X11/Xauth.h>

static char* test_Xau(void)
{
  return XauFileName();
}
#endif

#ifdef HAVE_X11_Xcomposite
#  include <X11/extensions/Xcomposite.h>

static int test_Xcomposite(void)
{
  return XCompositeVersion();
}
#endif

#ifdef HAVE_X11_Xdamage
#  include <X11/extensions/Xdamage.h>

static Bool test_Xdamage(void)
{
  Display* dpy = XOpenDisplay(NULL);
  int ev_base;
  int err_base;
  Bool ret = XDamageQueryExtension(dpy, &ev_base, &err_base);
  XCloseDisplay(dpy);
  return ret;
}
#endif

#ifdef HAVE_X11_Xdmcp
#  include <X11/Xdmcp.h>

static int test_Xdmcp(void)
{
  BYTE data[1024];
  XdmcpBuffer buf = { data, sizeof(data), 0, 0 };
  return XdmcpReadRemaining(&buf);
}
#endif

#ifdef HAVE_X11_Xext
#  include <X11/Xlib.h>
#  include <X11/extensions/Xext.h>

static int test_Xext(void)
{
  Display* dpy = XOpenDisplay(NULL);
  int ret = XMissingExtension(dpy, "cmake");
  XCloseDisplay(dpy);
  return ret;
}
#endif

#ifdef HAVE_X11_Xxf86misc
#  include <X11/Xlib.h>
#  include <X11/extensions/xf86misc.h>

static Bool test_Xxf86misc(void)
{
  Display* dpy = XOpenDisplay(NULL);
  Bool ret = XF86MiscSetClientVersion(dpy);
  XCloseDisplay(dpy);
  return ret;
}
#endif

#ifdef HAVE_X11_Xxf86vm
#  include <X11/Xlib.h>
#  include <X11/extensions/xf86vmode.h>

static Bool test_Xxf86vm(void)
{
  Display* dpy = XOpenDisplay(NULL);
  Bool ret = XF86VidModeSetClientVersion(dpy);
  XCloseDisplay(dpy);
  return ret;
}
#endif

#ifdef HAVE_X11_Xfixes
#  include <X11/extensions/Xfixes.h>

static Bool test_Xfixes(void)
{
  Display* dpy = XOpenDisplay(NULL);
  int ev_base;
  int err_base;
  Bool ret = XFixesQueryExtension(dpy, &ev_base, &err_base);
  XCloseDisplay(dpy);
  return ret;
}
#endif

#ifdef HAVE_X11_Xft
#  include <X11/Xft/Xft.h>

static FcBool test_Xft(void)
{
  return XftInitFtLibrary();
}
#endif

#ifdef HAVE_X11_Xi
#  include <X11/extensions/XInput.h>

static XExtensionVersion* test_Xi(void)
{
  Display* dpy = XOpenDisplay(NULL);
  XExtensionVersion* ret = XGetExtensionVersion(dpy, "cmake");
  XCloseDisplay(dpy);
  return ret;
}
#endif

#ifdef HAVE_X11_Xinerama
#  include <X11/extensions/Xinerama.h>

static Bool test_Xinerama(void)
{
  Display* dpy = XOpenDisplay(NULL);
  int ev_base;
  int err_base;
  Bool ret = XineramaQueryExtension(dpy, &ev_base, &err_base);
  XCloseDisplay(dpy);
  return ret;
}
#endif

#ifdef HAVE_X11_Xkb
#  include <X11/XKBlib.h>

static Bool test_Xkb(void)
{
  return XkbIgnoreExtension(0);
}
#endif

#ifdef HAVE_X11_xkbfile
// clang-format off
#  include <stdio.h>
#  include <X11/XKBlib.h>
#  include <X11/extensions/XKBfile.h>
#  include <stdlib.h>
// clang-format on

static void test_xkbfile(void)
{
  Display* dpy = XOpenDisplay(NULL);
  XkbInitAtoms(dpy);
  XCloseDisplay(dpy);
}
#endif

#ifdef HAVE_X11_Xmu
#  include <X11/Xmu/Xmu.h>
#  include <stdlib.h>

static Bool test_Xmu(void)
{
  return XmuValidArea(NULL);
}
#endif

#ifdef HAVE_X11_Xpm
#  include <X11/xpm.h>

static int test_Xpm(void)
{
  return XpmAttributesSize();
}
#endif

#ifdef HAVE_X11_Xtst
#  include <X11/extensions/XTest.h>

static Status test_Xtst(void)
{
  Display* dpy = XOpenDisplay(NULL);
  Status ret = XTestDiscard(dpy);
  XCloseDisplay(dpy);
  return ret;
}
#endif

#ifdef HAVE_X11_Xrandr
#  include <X11/extensions/Xrandr.h>

static Bool test_Xrandr(void)
{
  Display* dpy = XOpenDisplay(NULL);
  int ev_base;
  int err_base;
  Bool ret = XRRQueryExtension(dpy, &ev_base, &err_base);
  XCloseDisplay(dpy);
  return ret;
}
#endif

#ifdef HAVE_X11_Xrender
#  include <X11/extensions/Xrender.h>

static Bool test_Xrender(void)
{
  Display* dpy = XOpenDisplay(NULL);
  int ev_base;
  int err_base;
  Bool ret = XRenderQueryExtension(dpy, &ev_base, &err_base);
  XCloseDisplay(dpy);
  return ret;
}
#endif

#ifdef HAVE_X11_XRes
#  include <X11/Xlib.h>
#  include <X11/extensions/XRes.h>

static Bool test_XRes(void)
{
  Display* dpy = XOpenDisplay(NULL);
  int ev_base;
  int err_base;
  Bool ret = XResQueryExtension(dpy, &ev_base, &err_base);
  XCloseDisplay(dpy);
  return ret;
}
#endif

#ifdef HAVE_X11_Xss
#  include <X11/extensions/scrnsaver.h>

static Bool test_Xss(void)
{
  Display* dpy = XOpenDisplay(NULL);
  int ev_base;
  int err_base;
  Bool ret = XScreenSaverQueryExtension(dpy, &ev_base, &err_base);
  XCloseDisplay(dpy);
  return ret;
}
#endif

#ifdef HAVE_X11_Xt
#  include <X11/Intrinsic.h>

static void test_Xt(void)
{
  return XtToolkitInitialize();
}
#endif

#ifdef HAVE_X11_Xutil
#  include <X11/Xutil.h>

static int test_Xutil(void)
{
  Region r = XCreateRegion();
  return XDestroyRegion(r);
}
#endif

#ifdef HAVE_X11_Xv
#  include <X11/Xlib.h>
#  include <X11/extensions/Xvlib.h>

static int test_Xv(void)
{
  Display* dpy = XOpenDisplay(NULL);
  unsigned int version;
  unsigned int revision;
  unsigned int req_base;
  unsigned int ev_base;
  unsigned int err_base;
  int ret =
    XvQueryExtension(dpy, &version, &revision, &req_base, &ev_base, &err_base);
  XCloseDisplay(dpy);
  return ret;
}
#endif

#include <stddef.h>

int main(int argc, char* argv[])
{
  (void)argv;
  void* fptrs[] = {
#ifdef HAVE_X11_ICE
    test_ICE,
#endif
#ifdef HAVE_X11_SM
    test_SM,
#endif
#ifdef HAVE_X11_X11
    test_X11,
#endif
#ifdef HAVE_X11_Xau
    test_Xau,
#endif
#ifdef HAVE_X11_Xcomposite
    test_Xcomposite,
#endif
#ifdef HAVE_X11_Xdamage
    test_Xdamage,
#endif
#ifdef HAVE_X11_Xdmcp
    test_Xdmcp,
#endif
#ifdef HAVE_X11_Xext
    test_Xext,
#endif
#ifdef HAVE_X11_Xxf86misc
    test_Xxf86misc,
#endif
#ifdef HAVE_X11_Xxf86vm
    test_Xxf86vm,
#endif
#ifdef HAVE_X11_Xfixes
    test_Xfixes,
#endif
#ifdef HAVE_X11_Xft
    test_Xft,
#endif
#ifdef HAVE_X11_Xi
    test_Xi,
#endif
#ifdef HAVE_X11_Xinerama
    test_Xinerama,
#endif
#ifdef HAVE_X11_Xkb
    test_Xkb,
#endif
#ifdef HAVE_X11_xkbfile
    test_xkbfile,
#endif
#ifdef HAVE_X11_Xmu
    test_Xmu,
#endif
#ifdef HAVE_X11_Xpm
    test_Xpm,
#endif
#ifdef HAVE_X11_Xtst
    test_Xtst,
#endif
#ifdef HAVE_X11_Xrandr
    test_Xrandr,
#endif
#ifdef HAVE_X11_Xrender
    test_Xrender,
#endif
#ifdef HAVE_X11_XRes
    test_XRes,
#endif
#ifdef HAVE_X11_Xss
    test_Xss,
#endif
#ifdef HAVE_X11_Xt
    test_Xt,
#endif
#ifdef HAVE_X11_Xutil
    test_Xutil,
#endif
#ifdef HAVE_X11_Xv
    test_Xv,
#endif
    NULL,
  };

  // The code here is to convince the compiler to keep the static functions but
  // without calling them. This ends up always being "0" because `argc` is
  // always 1 in the test harness which always returns the sentinel at the end
  // of the array. The array logic is there to ensure that the contents of
  // `fptrs` is not optimized out.
  return (int)fptrs[(sizeof(fptrs) / sizeof(*fptrs)) - argc];
}
