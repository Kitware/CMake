set(foo_preinst "^echo default_preinst$")
set(foo_prerm "^echo default_prerm$")
verifyDebControl("${FOUND_FILE_1}" "foo" "preinst;prerm")

set(bar_preinst "^echo bar_preinst$")
set(bar_prerm "^echo bar_prerm$")
verifyDebControl("${FOUND_FILE_2}" "bar" "preinst;prerm")

set(bas_preinst "^echo default_preinst$")
set(bas_prerm "^echo default_prerm$")
verifyDebControl("${FOUND_FILE_3}" "bas" "preinst;prerm")
