set(CMAKE_PKG_CONFIG_PC_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageRoot/VersionPackages)

# Good = Should Succeed
# Bad  = Should Warn

cmake_pkg_config(EXTRACT a =aa)                   # Good
cmake_pkg_config(EXTRACT a >a)                    # Good
cmake_pkg_config(EXTRACT a <a)                    # Bad
cmake_pkg_config(EXTRACT a >aaa)                  # Bad
cmake_pkg_config(EXTRACT a <aaa)                  # Good

cmake_pkg_config(EXTRACT a !=bb)                  # Good
cmake_pkg_config(EXTRACT a >bb)                   # Bad
cmake_pkg_config(EXTRACT a <bb)                   # Good

cmake_pkg_config(EXTRACT a >1)                    # Bad
cmake_pkg_config(EXTRACT a <1)                    # Good

cmake_pkg_config(EXTRACT empty-key =)             # Good
cmake_pkg_config(EXTRACT empty-key !=)            # Bad
cmake_pkg_config(EXTRACT empty-key =0)            # Bad
cmake_pkg_config(EXTRACT empty-key !=0)           # Good

cmake_pkg_config(EXTRACT empty-key EXACT)         # Good

cmake_pkg_config(EXTRACT one =11)                 # Good
cmake_pkg_config(EXTRACT one >1)                  # Good
cmake_pkg_config(EXTRACT one <1)                  # Bad
cmake_pkg_config(EXTRACT one >111)                # Bad
cmake_pkg_config(EXTRACT one <111)                # Good

cmake_pkg_config(EXTRACT one !=22)                # Good
cmake_pkg_config(EXTRACT one >22)                 # Bad
cmake_pkg_config(EXTRACT one <22)                 # Good

cmake_pkg_config(EXTRACT one >a)                  # Good
cmake_pkg_config(EXTRACT one <a)                  # Bad

cmake_pkg_config(EXTRACT onedot 1.1.1)            # Good
cmake_pkg_config(EXTRACT onedot 01.01.01)         # Good
cmake_pkg_config(EXTRACT onedot =1.1.1)           # Good
cmake_pkg_config(EXTRACT onedot =01.01.01)        # Good
cmake_pkg_config(EXTRACT onedot <1.2.1)           # Good
cmake_pkg_config(EXTRACT onedot >1.2.1)           # Bad

cmake_pkg_config(EXTRACT onedot "< 1.2.1")        # Good
cmake_pkg_config(EXTRACT onedot "> 1.2.1")        # Bad

cmake_pkg_config(EXTRACT onedot 1.1.1 EXACT)      # Good
cmake_pkg_config(EXTRACT onedot =1.1.1 EXACT)     # Good
cmake_pkg_config(EXTRACT onedot =01.01.01 EXACT)  # Bad

cmake_pkg_config(EXTRACT pseudo-empty =~)         # Bad
cmake_pkg_config(EXTRACT pseudo-empty !=~)        # Good
cmake_pkg_config(EXTRACT pseudo-empty =~0)        # Good
cmake_pkg_config(EXTRACT pseudo-empty !=~0)       # Bad

cmake_pkg_config(EXTRACT tilde =~~1)              # Good
cmake_pkg_config(EXTRACT tilde <~1)               # Good
cmake_pkg_config(EXTRACT tilde >~1)               # Bad
cmake_pkg_config(EXTRACT tilde <~~~1)             # Bad
cmake_pkg_config(EXTRACT tilde >~~~1)             # Good

cmake_pkg_config(EXTRACT zeroone =1)              # Good
cmake_pkg_config(EXTRACT zeroone =001)            # Good
