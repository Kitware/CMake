
set(_compiler_id_version_compute "
  /* __IBMC__ = VRP */
# define @PREFIX@COMPILER_VERSION_MAJOR DEC(__IBMC__/100)
# define @PREFIX@COMPILER_VERSION_MINOR DEC(__IBMC__/10 % 10)
# define @PREFIX@COMPILER_VERSION_PATCH DEC(__IBMC__    % 10)")
