#
# A more complex test case
#
SET(CMAKE_BACKWARDS_COMPATIBILITY 1.4)
PROJECT (Complex)

# Try setting a new policy.  The IF test is for coverage.
IF(POLICY CMP0003)
  CMAKE_POLICY(SET CMP0003 NEW)

  CMAKE_POLICY(GET CMP0003 P3)
  IF(NOT "${P3}" STREQUAL "NEW")
    MESSAGE(FATAL_ERROR "CMAKE_POLICY(GET) did not report NEW!")
  ENDIF(NOT "${P3}" STREQUAL "NEW")
ENDIF(POLICY CMP0003)

# Test building without per-rule echo lines in Makefiles.
SET_PROPERTY(GLOBAL PROPERTY RULE_MESSAGES OFF)

SET(CPACK_SOURCE_IGNORE_FILES "~$;\\.cvsignore$;^C:/hoffman/My Builds/testcase.*/CVS/;^C:/hoffman/My Builds/testcase.*/\\.svn/;^C:/hoffman/My Builds/testcase.*/sweigart/;^C:/hoffman/My Builds/testcase/www/eospaper/solution/eos2001/;^C:/hoffman/My Builds/testcase/www/eospaper/solution/opal_tables_new/;^C:/hoffman/My Builds/testcase/COPYING$;^C:/hoffman/My Builds/testcase/INSTALL$;^C:/hoffman/My Builds/testcase/Makefile$;^C:/hoffman/My Builds/testcase/Makefile\\.in$;^C:/hoffman/My Builds/testcase/.*\\.lo$;^C:/hoffman/My Builds/testcase/.*\\.la$;^C:/hoffman/My Builds/testcase/mkinstalldirs$;^C:/hoffman/My Builds/testcase/missing$;^C:/hoffman/My Builds/testcase/ltmain\\.sh$;^C:/hoffman/My Builds/testcase/libtool$;^C:/hoffman/My Builds/testcase/install-sh$;^C:/hoffman/My Builds/testcase/configure$;^C:/hoffman/My Builds/testcase/config\\.sub$;^C:/hoffman/My Builds/testcase/config\\.status$;^C:/hoffman/My Builds/testcase/config\\.log$;^C:/hoffman/My Builds/testcase/config\\.guess$;^C:/hoffman/My Builds/testcase/autom4te\\.cache$;^C:/hoffman/My Builds/testcase/aclocal\\.m4$;^C:/hoffman/My Builds/testcase/depcomp$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/0\\.075\\.model_cassisi_eos1_10$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/0\\.075\\.model_cassisi_eos1_10_corr$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/0\\.1\\.model_cassisi_eos1$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/0\\.1\\.model_cassisi_scvh$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/0\\.1\\.modelc$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/0\\.3\\.modelc$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/1\\.0\\.modelc$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/1\\.0\\.rgbtip\\.modelc$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/1\\.0\\.zahb\\.modelc$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/0\\.1\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/1\\.0\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/0\\.3\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/0\\.085\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/.*\\.ps$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange\\.mem$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange\\.aux$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange\\.log$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange\\.dvi$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange\\.tex\\.bak$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/head\\.tmp$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/body\\.tmp$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/prior-dvi\\.aux$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/j10\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/j12\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/j16\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/j20\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/j22\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/j26\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/j30\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/j32\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/j36\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/k10\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/k12\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/k20\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/k22\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/k30\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/k32\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/1_exchange_dgamma1\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/1_exchange_dlnp\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/2_exchange_dgamma1\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/2_exchange_dlnp\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/linear_exchange_dgamma1\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/linear_exchange_dlnp\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/noexchange_dgamma1\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/noexchange_dlnp\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/nr_exchange_dgamma1\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/nr_exchange_dlnp\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/series_exchange_dgamma1\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/series_exchange_dlnp\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_JNR_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_Jseries_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_KNR_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_Kseries_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check34_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check35_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check36_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check43_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check44_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check45_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check46_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check47_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check48_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/tc_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/.*\\.pyc$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/context\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/make\\.out.*$;^C:/hoffman/My Builds/testcase/www/Makefile$;^C:/hoffman/My Builds/testcase/www/Makefile\\.in$;^C:/hoffman/My Builds/testcase/src/.*\\.flc$;^C:/hoffman/My Builds/testcase/src/Makefile$;^C:/hoffman/My Builds/testcase/src/Makefile\\.in$;^C:/hoffman/My Builds/testcase/src/\\.deps$;^C:/hoffman/My Builds/testcase/src/\\.libs$;^C:/hoffman/My Builds/testcase/src/.*\\.la$;^C:/hoffman/My Builds/testcase/src/.*\\.lo$;^C:/hoffman/My Builds/testcase/src/make\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/statef.*\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/0\\.1\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/0\\.1\\.model_13$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/0\\.1\\.model_23$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/0\\.3\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/0\\.3\\.model_13$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/0\\.3\\.model_23$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/1\\.0\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/1\\.0\\.model_13$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/1\\.0\\.model_15$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/1\\.0\\.model_23$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/1\\.0\\.model_rel$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/hot_post_agb\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/rgb_tip\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/rgbtip\\.1\\.0\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/rgbtip\\.1\\.0\\.model_13$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/rgbtip\\.1\\.0\\.model_23$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/start_shellflash\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/white_dwarf\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/zahb\\.1\\.0\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/zahb\\.1\\.0\\.model_13$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/zahb\\.1\\.0\\.model_23$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/model-loci/zahb\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/dh/dgamma1$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/dh/dlnp$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/dh/statef_compare\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/fermi_dirac_approx/15gamma1\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/fermi_dirac_approx/15lnp\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/fermi_dirac_approx/23gamma1\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/fermi_dirac_approx/23lnp\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/thermodynamic_consistency/.*\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/thermodynamic_consistency/.*\\.results$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/pteh/dgamma1$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/pteh/dlnp$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/pteh/statef_compare\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/newversion_grid/.*\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/newversion_grid/.*\\.err$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/.*\\.ps$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/.*\\.pyc$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_fit\\.aux$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_fit\\.dvi$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_fit\\.log$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/body\\.tmp$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/head\\.tmp$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/prior-dvi\\.aux$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/3order_data\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/5order_data\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/8order_data\\.tex$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_check8_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_check1_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_check3_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_check5_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/effo_check3_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/effoo_check3_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/fda15gamma1\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/fda15lnp\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/fda23gamma1\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/fda23lnp\\.yplot$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/tc_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/make\\.out.*$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/thermodynamic_consistency/statef_compare\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/thermodynamic_consistency/tc\\.results$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/opal_solar/opal_compare_model\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/opal_solar/opal_compare_solar\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/opal_solar/opal_solar\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/opal_solar/opal_solar_1995\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/opal_solar/statef_opal_model\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/opal_solar/statef_opal_model_1995\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/purehe_newversion_grid/.*\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/purehe_newversion_grid/.*\\.err$;^C:/hoffman/My Builds/testcase/include/Makefile\\.in$;^C:/hoffman/My Builds/testcase/include/Makefile$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/model-loci/0\\.1\\.model_pteh$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/model-loci/1\\.0\\.model_eos1a-eos1$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/model-loci/1\\.0\\.model_pteh$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/model-loci/statef_model_0\\.1\\.model_pteh\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/model-loci/statef_model_1\\.0\\.model_eos1a-eos1\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/model-loci/statef_model_1\\.0\\.model_pteh\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/context/contour\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/context/eos_grid\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/context/statef_grid\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/thermodynamic_consistency/fort\\.91$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/thermodynamic_consistency/statef_compare\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/nocoulomb/dgamma1$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/nocoulomb/dlnp$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/nocoulomb/statef_compare\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/context$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/oldversion_grid$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/2005Ap&SS\\.298\\.\\.135S\\.pdf$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/2007Ap&SS\\.307\\.\\.263C\\.pdf$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/astro-ph\\.9909168_eprint_submitted_to_High_Press\\.Res\\.16,331\\.pdf$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/.*ps.*$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/.*\\.pyc$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/convergence\\.aux$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/convergence\\.dvi$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/convergence\\.log$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/body\\.tmp$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/head\\.tmp$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/prior-dvi\\.aux$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/statef_grid-newversion$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/context\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/pureh_context\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/purehe_context\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/old$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/make\\.out.*$;^C:/hoffman/My Builds/testcase/utils/.*\\.flc$;^C:/hoffman/My Builds/testcase/utils/Makefile$;^C:/hoffman/My Builds/testcase/utils/Makefile\\.in$;^C:/hoffman/My Builds/testcase/utils/\\.deps$;^C:/hoffman/My Builds/testcase/utils/\\.libs$;^C:/hoffman/My Builds/testcase/utils/.*\\.la$;^C:/hoffman/My Builds/testcase/utils/.*\\.lo$;^C:/hoffman/My Builds/testcase/utils/free_eos_test$;^C:/hoffman/My Builds/testcase/utils/test_rosenbrock$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_check/eff_check1\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_check/eff_check3\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_check/eff_check5\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_check/eff_check8\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_check/eff_checknr\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_check/effo_check3\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/eff_check/effoo_check3\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence_20070613$;^C:/hoffman/My Builds/testcase/www/eospaper/text$;^C:/hoffman/My Builds/testcase/www/eospaper/cassisi_book_fig$;^C:/hoffman/My Builds/testcase/www/eospaper/1\\.1\\.0$;^C:/hoffman/My Builds/testcase/www/eospaper/2\\.0\\.0$;^C:/hoffman/My Builds/testcase/www/eospaper/1\\.2\\.0$;^C:/hoffman/My Builds/testcase/www/eospaper/1\\.3\\.0$;^C:/hoffman/My Builds/testcase/www/eospaper/1\\.4\\.0$;^C:/hoffman/My Builds/testcase/www/eospaper/1\\.5\\.0$;^C:/hoffman/My Builds/testcase/www/eospaper/1\\.6\\.0$;^C:/hoffman/My Builds/testcase/www/eospaper/figures$;^C:/hoffman/My Builds/testcase/www/eospaper/old$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/.*\\.ps.*$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/coulomb\\.aux$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/coulomb\\.dvi$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/coulomb\\.log$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/.*\\.pyc$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/body\\.tmp$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/head\\.tmp$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/prior-dvi\\.aux$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/context\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/dh_dgamma1_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/dh_dlnp_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/dhtau_dgamma1_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/dhtau_dlnp_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/nocoulomb_dgamma1_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/nocoulomb_dlnp_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/pteh_dgamma1_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/pteh_dlnp_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/make\\.out.*$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/exchange_JNR\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/exchange_Jseries\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/exchange_KNR\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/exchange_Kseries\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/exchange_check34\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/exchange_check35\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/exchange_check36\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/exchange_check44\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/exchange_check45\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/exchange_check46\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/statef_compare_1_exchange\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/statef_compare_2_exchange\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/statef_compare_linear_exchange\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/statef_compare_noexchange\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/statef_compare_nr_exchange\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/statef_compare_series_exchange\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/1_exchange_dgamma1\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/noexchange_dlnp\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/nr_exchange_dgamma1\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/series_exchange_dlnp\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/series_exchange_dgamma1\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/linear_exchange_dlnp\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/2_exchange_dgamma1\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/nr_exchange_dlnp\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/linear_exchange_dgamma1\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/noexchange_dgamma1\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/1_exchange_dlnp\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/exchange_check/2_exchange_dlnp\\.gnuplot$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/pureh_newversion_grid/.*\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/convergence/pureh_newversion_grid/.*\\.err$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/dhtau/dgamma1$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/dhtau/dlnp$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/dhtau/statef_compare\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/statef_model_0\\.1\\.model\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/statef_model_0\\.3\\.model\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/statef_model_1\\.0\\.model\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/statef_model_1\\.0\\.model_linear\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/statef_model_1\\.0\\.model_noexchange\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/statef_model_1\\.0\\.model_nr\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/statef_model_1\\.0\\.rgbtip\\.model\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/statef_model_1\\.0\\.zahb\\.model\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/1\\.0\\.zahb\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/1\\.0\\.rgbtip\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/1\\.0\\.model_linear$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/1\\.0\\.model_noexchange$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/1\\.0\\.model_nr$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/0\\.1\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/1\\.0\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/model-loci/0\\.3\\.model$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/context/contour\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/context/eos_grid\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/exchange/context/statef_grid\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/gong/delta\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/gong/m0085eos1gong\\.ascii$;^C:/hoffman/My Builds/testcase/www/eospaper/eff_fit/rho-T-loci/gong/m0085eos2gong\\.ascii$;^C:/hoffman/My Builds/testcase/www/eospaper/coulomb/coulomb_adjust/coulomb_adjust\\.out$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/.*\\.ps$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/.*\\.pyc$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/head\\.tmp$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/body\\.tmp$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/prior-dvi\\.aux$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/solution\\.aux$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/solution\\.log$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/solution\\.dvi$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/rtc_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/tc_yplot\\.in$;^C:/hoffman/My Builds/testcase/www/eospaper/solution/make\\.out.*$")

#
# Define a macro
#
MACRO(ASSERT value msg)
  IF (NOT ${value})
    MESSAGE ("Assertion failure:" ${msg} )
  ENDIF (NOT ${value})
ENDMACRO(ASSERT)
 
# invoke the macro
ASSERT(Complex_BINARY_DIR "The PROJECT command is broken")

#
# Define a var args macro, it must take two or four args
#
MACRO(TEST_ARGC value1 value2)
  ADD_DEFINITIONS(${value1} ${value2})
  IF (${ARGC} MATCHES 4)
    ADD_DEFINITIONS(${ARGV2} ${ARGV3})
  ENDIF (${ARGC} MATCHES 4)
ENDMACRO(TEST_ARGC)
 
# invoke the macro
TEST_ARGC(-DCMAKE_ARGV1 -DCMAKE_ARGV2 -DCMAKE_ARGV3 -DCMAKE_ARGV4)

MACRO(TEST_VAR_ARG fa)
  IF("${ARGV}" MATCHES "^1;2;3$")
    MESSAGE(STATUS "ARGV works")
  ELSE("${ARGV}" MATCHES "^1;2;3$")
    MESSAGE(FATAL_ERROR "ARGV does not work; got \"${ARGV}\" instead of \"1;2;3\"")
  ENDIF("${ARGV}" MATCHES "^1;2;3$")
  IF("${ARGN}" MATCHES "^2;3$")
    MESSAGE(STATUS "ARGN works")
  ELSE("${ARGN}" MATCHES "^2;3$")
    MESSAGE(FATAL_ERROR "ARGV does not work; got \"${ARGN}\" instead of \"2;3\"")
  ENDIF("${ARGN}" MATCHES "^2;3$")
ENDMACRO(TEST_VAR_ARG)

TEST_VAR_ARG(1 2 3)

# Floating-point comparison test.
IF(2.4 LESS 2.4)
  MESSAGE(FATAL_ERROR "Failed: 2.4 LESS 2.4")
ENDIF(2.4 LESS 2.4)
IF(2.4 GREATER 2.4)
  MESSAGE(FATAL_ERROR "Failed: 2.4 GREATER 2.4")
ENDIF(2.4 GREATER 2.4)
IF(NOT 2.4 EQUAL 2.4)
  MESSAGE(FATAL_ERROR "Failed: NOT 2.4 EQUAL 2.4")
ENDIF(NOT 2.4 EQUAL 2.4)

IF(CMAKE_SYSTEM MATCHES "OSF1-V.*")
  IF(NOT CMAKE_COMPILER_IS_GNUCXX)
     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -timplicit_local -no_implicit_include ")
  ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)
ENDIF(CMAKE_SYSTEM MATCHES "OSF1-V.*")


ADD_DEFINITIONS(-DCMAKE_IS_FUN)
ADD_DEFINITIONS(-DCMAKE_IS_REALLY_FUN)
SET_PROPERTY(DIRECTORY
  PROPERTY COMPILE_DEFINITIONS_RELEASE
  CMAKE_IS_FUN_IN_RELEASE_MODE
  )

SET(TEST_SEP "a b c")
SEPARATE_ARGUMENTS(TEST_SEP)


#
# Include vars from a file and from a cache
#
IF (EXISTS ${Complex_SOURCE_DIR}/VarTests.cmake)
  INCLUDE(${Complex_SOURCE_DIR}/VarTests.cmake)
ENDIF (EXISTS ${Complex_SOURCE_DIR}/VarTests.cmake)
INCLUDE(fileshouldnotbehere OPTIONAL)
LOAD_CACHE(${Complex_SOURCE_DIR}/Cache
           EXCLUDE
             CACHE_TEST_VAR_EXCLUDED
           INCLUDE_INTERNALS
             CACHE_TEST_VAR_INTERNAL)

LOAD_CACHE(${Complex_SOURCE_DIR}/Cache READ_WITH_PREFIX foo CACHE_TEST_VAR2)
IF(${fooCACHE_TEST_VAR2} MATCHES bar)
  MESSAGE("Load cache worked: ${fooCACHE_TEST_VAR2}")
ELSE(${fooCACHE_TEST_VAR2} MATCHES bar)
  MESSAGE(FATAL_ERROR "Load cache with prefix failed: ${fooCACHE_TEST_VAR2}")
ENDIF(${fooCACHE_TEST_VAR2} MATCHES bar)



#
# Specify include and lib dirs 
# (BEFORE is for coverage)
#
INCLUDE_DIRECTORIES(
  Library
)

INCLUDE_DIRECTORIES(BEFORE
  ${Complex_BINARY_DIR}
)
INCLUDE_DIRECTORIES(SYSTEM Library/SystemDir)

INCLUDE_REGULAR_EXPRESSION("^(cmTest|file|sharedFile|test).*$" "^cmMissing")

LINK_DIRECTORIES(
  ${Complex_BINARY_DIR}/Library
)

#
# check for SET CACHE FORCE
#
SET(FORCE_TEST 1 CACHE STRING "a test")
SET(FORCE_TEST 0 CACHE STRING "a test" FORCE)

#
# Lib and exe path
#
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${Complex_BINARY_DIR}/lib/static")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${Complex_BINARY_DIR}/lib")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${Complex_BINARY_DIR}/bin")

MESSAGE (Test " " escape " " semi-colon " " \; \;)
#
# Exec program (TODO: test a result)
# Increase coverage.
#
MESSAGE("\nIgnore this message")
OPTION(NO_EXEC_PROGRAM "Do not test EXEC_PROGRAM" 0)
IF (NOT NO_EXEC_PROGRAM)
  EXEC_PROGRAM(${CMAKE_COMMAND} ARGS -E echo NO_EXEC_PROGRAM "${Complex_BINARY_DIR}")
ELSE (NOT NO_EXEC_PROGRAM) 
  MESSAGE("Set this option ON")
ENDIF (NOT NO_EXEC_PROGRAM) 

MARK_AS_ADVANCED(NO_EXEC_PROGRAM)
MARK_AS_ADVANCED(CLEAR NO_EXEC_PROGRAM)

# Execute a process.  Add coverage for this command.
EXECUTE_PROCESS(
  COMMAND ${CMAKE_COMMAND} -E echo "ABCDEFG"
  OUTPUT_VARIABLE TEST_OUT
  )
IF("${TEST_OUT}" MATCHES "^ABCDEFG\n$")
ELSE("${TEST_OUT}" MATCHES "^ABCDEFG\n$")
  MESSAGE(SEND_ERROR "EXECUTE_PROCESS output test failed: [${TEST_OUT}]")
ENDIF("${TEST_OUT}" MATCHES "^ABCDEFG\n$")

# This test has some problems on UNIX systems.  Disabling for now.
#
# EXECUTE_PROCESS(
#   COMMAND ${CMAKE_COMMAND} -E echo "ABCDEFG"
#   COMMAND /process/does/not/exist
#   OUTPUT_QUIET
#   ERROR_QUIET
#   RESULT_VARIABLE RESULT
#   )
# IF("${RESULT}" MATCHES "^0$")
#   MESSAGE(SEND_ERROR
#     "EXECUTE_PROCESS result test failed with RESULT=[${RESULT}]")
# ELSE("${RESULT}" MATCHES "^0$")
#   MESSAGE(STATUS "EXECUTE_PROCESS result test passed with RESULT=[${RESULT}]")
# ENDIF("${RESULT}" MATCHES "^0$")

#
# Create directory.
# The 'complex' executable will then test if this dir exists,
# sadly it won't be able to remove it.
#
MAKE_DIRECTORY("${Complex_BINARY_DIR}/make_dir")

#
# Test FIND_LIBARY
# Create a dummy empty lib
#
CONFIGURE_FILE(
  ${Complex_SOURCE_DIR}/Library/dummy
  ${Complex_BINARY_DIR}/Library/dummylib.lib
  COPYONLY IMMEDIATE)
FOREACH (ext ${CMAKE_SHLIB_SUFFIX};.so;.a;.sl)
  CONFIGURE_FILE(
    ${Complex_SOURCE_DIR}/Library/dummy
    ${Complex_BINARY_DIR}/Library/libdummylib${ext}
    COPYONLY IMMEDIATE)
ENDFOREACH (ext)

FIND_LIBRARY(FIND_DUMMY_LIB
             dummylib
             PATHS
             ${Complex_BINARY_DIR}/Library DOC "find dummy lib")

FIND_LIBRARY(FIND_DUMMY_LIB
             NAMES dummylib dummylib2
             PATHS
             ${Complex_BINARY_DIR}/Library DOC "find dummy lib")

#
# Test SET_SOURCE_FILES_PROPERTIES 
#
SET_SOURCE_FILES_PROPERTIES(nonexisting_file2 
  GENERATED 
  ABSTRACT 
  WRAP_EXCLUDE 
  COMPILE_FLAGS "-foo -bar")

GET_SOURCE_FILE_PROPERTY(FILE_HAS_ABSTRACT nonexisting_file2 ABSTRACT)
GET_SOURCE_FILE_PROPERTY(FILE_HAS_WRAP_EXCLUDE nonexisting_file2 WRAP_EXCLUDE)
GET_SOURCE_FILE_PROPERTY(FILE_COMPILE_FLAGS nonexisting_file2 COMPILE_FLAGS)

SET_SOURCE_FILES_PROPERTIES(nonexisting_file3 PROPERTIES
  GENERATED 1
  ABSTRACT 1
  WRAP_EXCLUDE 1
  COMPILE_FLAGS "-foo -bar")
GET_SOURCE_FILE_PROPERTY(FILE_HAS_ABSTRACT nonexisting_file3 ABSTRACT)
GET_SOURCE_FILE_PROPERTY(FILE_HAS_WRAP_EXCLUDE nonexisting_file3 WRAP_EXCLUDE)
GET_SOURCE_FILE_PROPERTY(FILE_COMPILE_FLAGS nonexisting_file3 COMPILE_FLAGS)

#
# Test registry (win32)
# Create a file, put its path in a registry key, try to find the file in that
# path using that registry key, then remove the file and the key
#
IF (WIN32)
  IF (NOT UNIX)
    SET(dir "${Complex_BINARY_DIR}/registry_dir")
    SET(file "registry_test_dummy")
    SET(hkey "HKEY_CURRENT_USER\\Software\\Kitware\\CMake\\Tests\\Complex;registry_test")
    CONFIGURE_FILE(
      ${Complex_SOURCE_DIR}/Library/dummy
      "${dir}/${file}"
      COPYONLY IMMEDIATE)
    EXEC_PROGRAM(${CMAKE_COMMAND} ARGS "-E write_regv \"${hkey}\" \"${dir}\"")
    FIND_PATH(REGISTRY_TEST_PATH
      ${file}
      "[${hkey}]" DOC "Registry_Test_Path")
    EXEC_PROGRAM(${CMAKE_COMMAND} ARGS "-E delete_regv \"${hkey}\"")
    EXEC_PROGRAM(${CMAKE_COMMAND} ARGS "-E remove \"${dir}/${file}\"")
  ENDIF (NOT UNIX)
ENDIF (WIN32)

#
# Test a set and a remove
#
SET(REMOVE_STRING a b c d e f)
SET(removeVar1 c e)
REMOVE(REMOVE_STRING ${removeVar1} f)

#
# Test an IF inside a FOREACH.
#
FOREACH(x "a")
  IF(${x} MATCHES "a")
    # Should always execute.
    SET(IF_INSIDE_FOREACH_THEN_EXECUTED 1)
  ELSE(${x} MATCHES "a")
    # Should never execute.
    SET(IF_INSIDE_FOREACH_ELSE_EXECUTED 1)
  ENDIF(${x} MATCHES "a")
ENDFOREACH(x)

# test WHILE command
SET (while_var 1)
WHILE (while_var LESS 1000)
  SET(while_var ${while_var}0)
ENDWHILE(while_var LESS 1000)

SET(SHOULD_BE_ZERO )
SET(SHOULD_BE_ONE 1)

# test elseif functionality, the mess below tries to catch problem 
# of clauses being executed early or late etc
set (RESULT 3)
if (RESULT EQUAL 1)
  if (RESULT EQUAL 2)
    set (ELSEIF_RESULT 1)
  elseif (RESULT EQUAL 3)
    set (ELSEIF_RESULT 1)
  endif (RESULT EQUAL 2)  
elseif (RESULT EQUAL 2)
  set (ELSEIF_RESULT 1)
elseif (RESULT EQUAL 3)
  if (RESULT EQUAL 2)
    set (ELSEIF_RESULT 1)
  elseif (RESULT EQUAL 3)
    if (NOT ELSEIF_RESULT EQUAL 1)
      set (ELSEIF_RESULT 2)
    endif (NOT ELSEIF_RESULT EQUAL 1)      
  endif (RESULT EQUAL 2)  
elseif (RESULT EQUAL 4)
  if (RESULT EQUAL 2)
    set (ELSEIF_RESULT 1)
  elseif (RESULT EQUAL 3)
    set (ELSEIF_RESULT 1)
  endif (RESULT EQUAL 2)  
else (RESULT  EQUAL 1)
  if (RESULT EQUAL 2)
    set (ELSEIF_RESULT 1)
  elseif (RESULT EQUAL 3)
    set (ELSEIF_RESULT 1)
  endif (RESULT EQUAL 2)  
endif (RESULT EQUAL 1)

if (NOT ELSEIF_RESULT EQUAL 2)
  set (ELSEIF_RESULT 0)
endif (NOT ELSEIF_RESULT EQUAL 2)

# test handling of parenthetical groups in conditionals
if (2 GREATER 1 AND (4 LESS 3 OR 5 LESS 6) AND NOT (7 GREATER 8))
  set(CONDITIONAL_PARENTHESES 1)
endif()


#
# Configure file 
# (plug vars to #define so that they can be tested)
#
CONFIGURE_FILE(
  ${Complex_SOURCE_DIR}/cmTestConfigure.h.in
  ${Complex_BINARY_DIR}/cmTestConfigure.h)

SET(STRING_WITH_QUOTES "\"hello world\"")
# test CONFIGURE_FILE with ESCAPE_QUOTES on
CONFIGURE_FILE(
  ${Complex_SOURCE_DIR}/cmTestConfigureEscape.h.in
  ${Complex_BINARY_DIR}/cmTestConfigureEscape.h ESCAPE_QUOTES)

# Test regular expression commands.
STRING(REGEX MATCH "b" RESULT "abc")
IF(NOT RESULT)
  MESSAGE(SEND_ERROR "STRING(REGEX MATCH ... ) test failed.")
ENDIF(NOT RESULT)
STRING(REGEX MATCHALL "b" RESULT "abcb")
IF(NOT RESULT)
  MESSAGE(SEND_ERROR "STRING(REGEX MATCHALL ... ) test failed.")
ENDIF(NOT RESULT)
STRING(REGEX REPLACE ".([bd])." "[\\1]" RESULT "a(b)c(d)e")
IF("x${RESULT}" MATCHES "^xa\\[b\\]c\\[d\\]e$")
  SET(STRING_REGEX_PASSED 1)
ENDIF("x${RESULT}" MATCHES "^xa\\[b\\]c\\[d\\]e$")
IF(NOT STRING_REGEX_PASSED)
  MESSAGE(SEND_ERROR
    "STRING(REGEX REPLACE ... ) test failed (\"${RESULT}\" v. \"a[b]c[d]e\")")
ENDIF(NOT STRING_REGEX_PASSED)


#
# Create the libs and the main exe
#
ADD_SUBDIRECTORY(Library)
ADD_SUBDIRECTORY(Executable)
SUBDIR_DEPENDS(Executable Library)
EXPORT_LIBRARY_DEPENDENCIES(${Complex_BINARY_DIR}/ComplexLibraryDepends.cmake)
INCLUDE(${Complex_BINARY_DIR}/ComplexLibraryDepends.cmake OPTIONAL)
