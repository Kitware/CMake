set(foo_preinst "^echo default_preinst$")
# NOTE: optional dot at the end of permissions regex is for SELinux enabled systems
set(foo_preinst_permissions "-rwxr-xr-x")
set(foo_prerm "^echo default_prerm$")
set(foo_prerm_permissions "-rwxr-xr-x")
verifyDebControl("${FOUND_FILE_1}" "foo" "preinst;prerm")

set(bar_preinst "^echo bar_preinst$")
set(bar_preinst_permissions "-rwx------")
set(bar_prerm "^echo bar_prerm$")
set(bar_prerm_permissions "-rwx------")
verifyDebControl("${FOUND_FILE_2}" "bar" "preinst;prerm")

set(bas_preinst "^echo default_preinst$")
set(bas_preinst_permissions "-rwxr-xr-x")
set(bas_prerm "^echo default_prerm$")
set(bas_prerm_permissions "-rwxr-xr-x")
verifyDebControl("${FOUND_FILE_3}" "bas" "preinst;prerm")
