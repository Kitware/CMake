
set(_compiler_id_version_compute "
  /* __IBMCPP__ = VRP */
# define @PREFIX@COMPILER_VERSION_MAJOR DEC(__IBMCPP__/100)
# define @PREFIX@COMPILER_VERSION_MINOR DEC(__IBMCPP__/10 % 10)
# define @PREFIX@COMPILER_VERSION_PATCH DEC(__IBMCPP__    % 10)")
