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

#ifdef HAVE_X11_Xaw
#  include <X11/Intrinsic.h>
#  include <X11/Xaw/Box.h>

static void test_Xaw(void)
{
  XrmOptionDescRec opt_table[] = { { NULL } };

  Widget toplevel;
  toplevel =
    XtInitialize("test", "test", opt_table, XtNumber(opt_table), NULL, NULL);
  Widget box =
    XtCreateManagedWidget("testbox", boxWidgetClass, toplevel, NULL, 0);
  return;
}

#endif

#ifdef HAVE_X11_xcb
#  include <xcb/xcb.h>

static void test_xcb(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_composite
#  include <xcb/composite.h>
#  include <xcb/xcb.h>

static void test_xcb_composite(void)
{
  xcb_connection_t* connection = xcb_connect(NULL, NULL);
  xcb_composite_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_cursor
#  include <xcb/xcb.h>
#  include <xcb/xcb_cursor.h>

static void test_xcb_cursor(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_screen_iterator_t screens =
    xcb_setup_roots_iterator(xcb_get_setup(connection));
  xcb_cursor_context_t* ctx;
  xcb_cursor_context_new(connection, screens.data, &ctx);
  xcb_cursor_context_free(ctx);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_damage
#  include <xcb/damage.h>
#  include <xcb/xcb.h>

static void test_xcb_damage(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_damage_query_version_cookie_t cookie =
    xcb_damage_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_dpms
#  include <xcb/dpms.h>
#  include <xcb/xcb.h>

static void test_xcb_dpms(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_dpms_get_version_cookie_t cookie =
    xcb_dpms_get_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_dri2
#  include <xcb/dri2.h>
#  include <xcb/xcb.h>

static void test_xcb_dri2(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_dri2_query_version_cookie_t cookie =
    xcb_dri2_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_dri3
#  include <xcb/dri3.h>
#  include <xcb/xcb.h>

static void test_xcb_dri3(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_dri3_query_version_cookie_t cookie =
    xcb_dri3_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_errors
#  include <xcb/xcb.h>
#  include <xcb/xcb_errors.h>

static void test_xcb_errors(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_errors_context_t* context;
  xcb_errors_context_new(connection, &context);
  xcb_errors_context_free(context);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_ewmh
#  include <xcb/xcb.h>
#  include <xcb/xcb_ewmh.h>

static void test_xcb_ewmh(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_ewmh_connection_t ewmh_connection;
  xcb_intern_atom_cookie_t* cookie =
    xcb_ewmh_init_atoms(connection, &ewmh_connection);
  xcb_ewmh_init_atoms_replies(&ewmh_connection, cookie, NULL);
  xcb_ewmh_connection_wipe(&ewmh_connection);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_glx
#  include <xcb/glx.h>
#  include <xcb/xcb.h>

static void test_xcb_glx(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_glx_query_version_cookie_t cookie =
    xcb_glx_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_icccm
#  include <xcb/xcb.h>
#  include <xcb/xcb_icccm.h>

static void test_xcb_icccm(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_window_t root =
    xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root;
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_name(connection, root);
  xcb_icccm_get_text_property_reply_t reply;
  xcb_icccm_get_wm_name_reply(connection, cookie, &reply, NULL);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_image
#  include <xcb/xcb.h>
#  include <xcb/xcb_image.h>

static void test_xcb_image(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  // xcb_image is too convoluted/undocumented to make an
  // actually working example, apologies :)
  xcb_image_create(0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_keysyms
#  include <xcb/xcb.h>
#  include <xcb/xcb_keysyms.h>

static void test_xcb_keysyms(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_key_symbols_t* symbols = xcb_key_symbols_alloc(connection);
  if (symbols != NULL)
    xcb_key_symbols_free(symbols);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_present
#  include <xcb/present.h>
#  include <xcb/xcb.h>

static void test_xcb_present(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_present_query_version_cookie_t cookie =
    xcb_present_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_randr
#  include <xcb/randr.h>
#  include <xcb/xcb.h>

static void test_xcb_randr(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_randr_query_version_cookie_t cookie =
    xcb_randr_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_record
#  include <xcb/record.h>
#  include <xcb/xcb.h>

static void test_xcb_record(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_record_query_version_cookie_t cookie =
    xcb_record_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_render
#  include <xcb/render.h>
#  include <xcb/xcb.h>

static void test_xcb_render(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_render_query_version_cookie_t cookie =
    xcb_render_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_render_util
#  include <xcb/xcb.h>
#  include <xcb/xcb_renderutil.h>

static void test_xcb_render_util(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  const xcb_render_query_version_reply_t* cookie =
    xcb_render_util_query_version(connection);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_res
#  include <xcb/res.h>
#  include <xcb/xcb.h>

static void test_xcb_res(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_res_query_version_cookie_t cookie =
    xcb_res_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_screensaver
#  include <xcb/screensaver.h>
#  include <xcb/xcb.h>

static void test_xcb_screensaver(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_screensaver_query_version_cookie_t cookie =
    xcb_screensaver_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_shape
#  include <xcb/shape.h>
#  include <xcb/xcb.h>

static void test_xcb_shape(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_shape_query_version_cookie_t cookie =
    xcb_shape_query_version(connection);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_shm
#  include <xcb/shm.h>
#  include <xcb/xcb.h>

static void test_xcb_shm(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_shm_query_version_cookie_t cookie = xcb_shm_query_version(connection);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_sync
#  include <xcb/sync.h>
#  include <xcb/xcb.h>

static void test_xcb_sync(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_sync_initialize_cookie_t cookie = xcb_sync_initialize(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_util
#  include <xcb/xcb.h>
#  include <xcb/xcb_aux.h>

static void test_xcb_util(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_screen_t* screen = xcb_aux_get_screen(connection, screen_nbr);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_xf86dri
#  include <xcb/xcb.h>
#  include <xcb/xf86dri.h>

static void test_xcb_xf86dri(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_xf86dri_query_version_cookie_t cookie =
    xcb_xf86dri_query_version(connection);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_xfixes
#  include <xcb/xcb.h>
#  include <xcb/xfixes.h>

static void test_xcb_xfixes(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_xfixes_query_version(connection, 1, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_xinerama
#  include <xcb/xcb.h>
#  include <xcb/xinerama.h>

static void test_xcb_xinerama(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_xinerama_query_version_cookie_t cookie =
    xcb_xinerama_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_xinput
#  include <xcb/xcb.h>
#  include <xcb/xinput.h>

static void test_xcb_xinput(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_input_xi_query_version_cookie_t cookie =
    xcb_input_xi_query_version(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_xkb
#  include <xcb/xcb.h>
#  include <xcb/xkb.h>

static void test_xcb_xkb(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_xkb_use_extension_cookie_t cookie =
    xcb_xkb_use_extension(connection, 0, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_xrm
#  include <xcb/xcb.h>
#  include <xcb/xcb_xrm.h>

static void test_xcb_xrm(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_xrm_database_t* db = xcb_xrm_database_from_default(connection);
  xcb_xrm_database_free(db);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_xtest
#  include <xcb/xcb.h>
#  include <xcb/xtest.h>

static void test_xcb_xtest(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_test_get_version_unchecked(connection, 1, 0);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_xvmc
#  include <xcb/xcb.h>
#  include <xcb/xvmc.h>

static void test_xcb_xvmc(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_xvmc_query_version_cookie_t cookie = xcb_xvmc_query_version(connection);
  xcb_disconnect(connection);
}

#endif

#ifdef HAVE_X11_xcb_xv
#  include <xcb/xcb.h>
#  include <xcb/xv.h>

static void test_xcb_xv(void)
{
  int screen_nbr;
  xcb_connection_t* connection = xcb_connect(NULL, &screen_nbr);
  xcb_xv_query_extension_cookie_t cookie = xcb_xv_query_extension(connection);
  xcb_disconnect(connection);
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
#ifdef HAVE_X11_Xaw
    test_Xaw,
#endif
#ifdef HAVE_X11_xcb
    test_xcb,
#endif
#ifdef HAVE_X11_xcb_composite
    test_xcb_composite,
#endif
#ifdef HAVE_X11_xcb_cursor
    test_xcb_cursor,
#endif
#ifdef HAVE_X11_xcb_damage
    test_xcb_damage,
#endif
#ifdef HAVE_X11_xcb_dpms
    test_xcb_dpms,
#endif
#ifdef HAVE_X11_xcb_dri2
    test_xcb_dri2,
#endif
#ifdef HAVE_X11_xcb_dri3
    test_xcb_dri3,
#endif
#ifdef HAVE_X11_xcb_errors
    test_xcb_errors,
#endif
#ifdef HAVE_X11_xcb_ewmh
    test_xcb_ewmh,
#endif
#ifdef HAVE_X11_xcb_glx
    test_xcb_glx,
#endif
#ifdef HAVE_X11_xcb_icccm
    test_xcb_icccm,
#endif
#ifdef HAVE_X11_xcb_image
    test_xcb_image,
#endif
#ifdef HAVE_X11_xcb_keysyms
    test_xcb_keysyms,
#endif
#ifdef HAVE_X11_xcb_present
    test_xcb_present,
#endif
#ifdef HAVE_X11_xcb_randr
    test_xcb_randr,
#endif
#ifdef HAVE_X11_xcb_record
    test_xcb_record,
#endif
#ifdef HAVE_X11_xcb_render
    test_xcb_render,
#endif
#ifdef HAVE_X11_xcb_render_util
    test_xcb_render_util,
#endif
#ifdef HAVE_X11_xcb_res
    test_xcb_res,
#endif
#ifdef HAVE_X11_xcb_screensaver
    test_xcb_screensaver,
#endif
#ifdef HAVE_X11_xcb_shape
    test_xcb_shape,
#endif
#ifdef HAVE_X11_xcb_shm
    test_xcb_shm,
#endif
#ifdef HAVE_X11_xcb_sync
    test_xcb_sync,
#endif
#ifdef HAVE_X11_xcb_util
    test_xcb_util,
#endif
#ifdef HAVE_X11_xcb_xf86dri
    test_xcb_xf86dri,
#endif
#ifdef HAVE_X11_xcb_xfixes
    test_xcb_xfixes,
#endif
#ifdef HAVE_X11_xcb_xinerama
    test_xcb_xinerama,
#endif
#ifdef HAVE_X11_xcb_xinput
    test_xcb_xinput,
#endif
#ifdef HAVE_X11_xcb_xkb
    test_xcb_xkb,
#endif
#ifdef HAVE_X11_xcb_xrm
    test_xcb_xrm,
#endif
#ifdef HAVE_X11_xcb_xtest
    test_xcb_xtest,
#endif
#ifdef HAVE_X11_xcb_xvmc
    test_xcb_xvmc,
#endif
#ifdef HAVE_X11_xcb_xv
    test_xcb_xv,
#endif
    NULL,
  };

  // The code here is to convince the compiler to keep the static functions but
  // without calling them. This ends up always being "0" because `argc` is
  // always 1 in the test harness which always returns the sentinel at the end
  // of the array. The array logic is there to ensure that the contents of
  // `fptrs` is not optimized out.
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
  return (int)fptrs[(sizeof(fptrs) / sizeof(*fptrs)) - argc];
}
