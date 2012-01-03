# - Functions to help assemble a standalone Qt4 executable.
# A collection of CMake utility functions useful for deploying
# Qt4 executables.
#
# The following functions are provided by this module:
#   write_qt4_conf
#   resolve_qt4_paths
#   fixup_qt4_executable
#   install_qt4_plugin_path
#   install_qt4_plugin
#   install_qt4_executable
# Requires CMake 2.6 or greater because it uses function and
# PARENT_SCOPE. Also depends on BundleUtilities.cmake.
#
#  WRITE_QT4_CONF(<qt_conf_dir> <qt_conf_contents>)
# Writes a qt.conf file with the <qt_conf_contents> into <qt_conf_dir>.
#
#  RESOLVE_QT4_PATHS(<paths_var> [<executable_path>])
# Loop through <paths_var> list and if any don't exist resolve them
# relative to the <executable_path> (if supplied) or the CMAKE_INSTALL_PREFIX.
#
#  FIXUP_QT4_EXECUTABLE(<executable> [<qtplugins> <libs> <dirs> <plugins_dir> <request_qt_conf>])
# Copies Qt plugins, writes a Qt configuration file (if needed) and fixes up a
# Qt4 executable using BundleUtilities so it is standalone and can be
# drag-and-drop copied to another machine as long as all of the system
# libraries are compatible.
#
# <executable> should point to the executable to be fixed-up.
#
# <qtplugins> should contain a list of the names or paths of any Qt plugins
# to be installed.
#
# <libs> will be passed to BundleUtilities and should be a list of any already
# installed plugins, libraries or executables to also be fixed-up.
#
# <dirs> will be passed to BundleUtilities and should contain and directories
# to be searched to find library dependencies.
#
# <plugins_dir> allows an custom plugins directory to be used.
#
# <request_qt_conf> will force a qt.conf file to be written even if not needed.
#
#  INSTALL_QT4_PLUGIN_PATH(plugin executable copy installed_plugin_path_var <plugins_dir> <component> <configurations>)
# Install (or copy) a resolved <plugin> to the default plugins directory
# (or <plugins_dir>) relative to <executable> and store the result in
# <installed_plugin_path_var>.
#
# If <copy> is set to TRUE then the plugins will be copied rather than
# installed. This is to allow this module to be used at CMake time rather than
# install time.
#
# If <component> is set then anything installed will use this COMPONENT.
#
#  INSTALL_QT4_PLUGIN(plugin executable copy installed_plugin_path_var <plugins_dir> <component>)
# Install (or copy) an unresolved <plugin> to the default plugins directory
# (or <plugins_dir>) relative to <executable> and store the result in
# <installed_plugin_path_var>. See documentation of INSTALL_QT4_PLUGIN_PATH.
#
#  INSTALL_QT4_EXECUTABLE(<executable> [<qtplugins> <libs> <dirs> <plugins_dir> <request_qt_conf> <component>])
# Installs Qt plugins, writes a Qt configuration file (if needed) and fixes up
# a Qt4 executable using BundleUtilities so it is standalone and can be
# drag-and-drop copied to another machine as long as all of the system
# libraries are compatible. The executable will be fixed-up at install time.
# <component> is the COMPONENT used for bundle fixup and plugin installation.
# See documentation of FIXUP_QT4_BUNDLE.

#=============================================================================
# Copyright 2011 Mike McQuaid <mike@mikemcquaid.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# The functions defined in this file depend on the fixup_bundle function
# (and others) found in BundleUtilities.cmake

include(BundleUtilities)
set(DeployQt4_cmake_dir "${CMAKE_CURRENT_LIST_DIR}")

function(write_qt4_conf qt_conf_dir qt_conf_contents)
        set(qt_conf_path "${qt_conf_dir}/qt.conf")
        message(STATUS "Writing ${qt_conf_path}")
        file(WRITE "${qt_conf_path}" "${qt_conf_contents}")
endfunction()

function(resolve_qt4_paths paths_var)
        set(executable_path ${ARGV1})

        set(paths_resolved)
        foreach(path ${${paths_var}})
                if(EXISTS "${path}")
                        list(APPEND paths_resolved "${path}")
                else()
                        if(${executable_path})
                                list(APPEND paths_resolved "${executable_path}/${path}")
                        else()
                                list(APPEND paths_resolved "\${CMAKE_INSTALL_PREFIX}/${path}")
                        endif()
                endif()
        endforeach()
        set(${paths_var} ${paths_resolved} PARENT_SCOPE)
endfunction()

function(fixup_qt4_executable executable)
        set(qtplugins ${ARGV1})
        set(libs ${ARGV2})
        set(dirs ${ARGV3})
        set(plugins_dir ${ARGV4})
        set(request_qt_conf ${ARGV5})

        message(STATUS "fixup_qt4_executable")
        message(STATUS "  executable='${executable}'")
        message(STATUS "  qtplugins='${qtplugins}'")
        message(STATUS "  libs='${libs}'")
        message(STATUS "  dirs='${dirs}'")
        message(STATUS "  plugins_dir='${plugins_dir}'")
        message(STATUS "  request_qt_conf='${request_qt_conf}'")

        if(QT_LIBRARY_DIR)
                list(APPEND dirs "${QT_LIBRARY_DIR}")
        endif()

        if(APPLE)
                set(qt_conf_dir "${executable}/Contents/Resources")
                set(executable_path "${executable}")
                set(write_qt_conf TRUE)
        else()
                get_filename_component(executable_path "${executable}" PATH)
                if(NOT executable_path)
                        set(executable_path ".")
                endif()
                set(qt_conf_dir "${executable_path}")
                set(write_qt_conf ${request_qt_conf})
        endif()

        foreach(plugin ${qtplugins})
                set(installed_plugin_path "")
                install_qt4_plugin("${plugin}" "${plugins_dir}" "${executable}" 1 installed_plugin_path)
                list(APPEND libs ${installed_plugin_path})
        endforeach()

        foreach(lib ${libs})
                if(NOT EXISTS "${lib}")
                        message(FATAL_ERROR "Library does not exist: ${lib}")
                endif()
        endforeach()

        resolve_qt4_paths(libs "${executable_path}")

        if(write_qt_conf)
                set(qt_conf_contents "[Paths]\nPlugins = ${plugins_dir}")
                write_qt4_conf("${qt_conf_dir}" "${qt_conf_contents}")
        endif()

        fixup_bundle("${executable}" "${libs}" "${dirs}")
endfunction()

function(install_qt4_plugin_path plugin executable copy installed_plugin_path_var)
        set(plugins_dir ${ARGV4})
        set(component ${ARGV5})
        set(configurations ${ARGV6})
        if(EXISTS "${plugin}")
                if(plugins_dir)
                        set(plugins_dir "${plugins_dir}")
                else()
                        if(APPLE)
                                set(plugins_dir "PlugIns")
                        else()
                                set(plugins_dir "plugins")
                        endif()
                endif()
                if(APPLE)
                        set(plugins_path "${executable}/Contents/${plugins_dir}")
                else()
                        get_filename_component(executable_path "${executable}" PATH)
                        if(NOT executable_path)
                                set(executable_path ".")
                        endif()
                        set(plugins_path "${executable_path}/${plugins_dir}")
                endif()

                set(plugin_group "")

                get_filename_component(plugin_path "${plugin}" PATH)
                get_filename_component(plugin_parent_path "${plugin_path}" PATH)
                get_filename_component(plugin_parent_dir_name "${plugin_parent_path}" NAME)
                get_filename_component(plugin_name "${plugin}" NAME)
                string(TOLOWER "${plugin_parent_dir_name}" plugin_parent_dir_name)

                if("${plugin_parent_dir_name}" STREQUAL "plugins")
                        get_filename_component(plugin_group "${plugin_path}" NAME)
                        set(${plugin_group_var} "${plugin_group}")
                endif()
                set(plugins_path "${plugins_path}/${plugin_group}")

                if(${copy})
                        file(MAKE_DIRECTORY "${plugins_path}")
                        file(COPY "${plugin}" DESTINATION "${plugins_path}")
                else()
                        if(configurations AND (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE))
                                set(configurations CONFIGURATIONS ${configurations})
                        else()
                                unset(configurations)
                        endif()
                        install(FILES "${plugin}" DESTINATION "${plugins_path}" ${configurations} ${component})
                endif()
                set(${installed_plugin_path_var} ${${installed_path_var}} "${plugins_path}/${plugin_name}" PARENT_SCOPE)
        endif()
endfunction()

function(install_qt4_plugin plugin executable copy installed_plugin_path_var)
        set(plugins_dir ${ARGV4})
        set(component ${ARGV5})
        if(EXISTS "${plugin}")
                install_qt4_plugin_path("${plugin}" "${executable}" "${copy}" "${installed_plugin_path_var}" "${plugins_dir}" "${component}")
        else()
                if(QT_IS_STATIC)
                        string(TOUPPER "QT_${plugin}_LIBRARY" plugin_var)
                else()
                        string(TOUPPER "QT_${plugin}_PLUGIN" plugin_var)
                endif()
                set(plugin_release_var "${plugin_var}_RELEASE")
                set(plugin_debug_var "${plugin_var}_DEBUG")
                set(plugin_release "${${plugin_release_var}}")
                set(plugin_debug "${${plugin_debug_var}}")
                if(DEFINED "${plugin_release_var}" AND DEFINED "${plugin_debug_var}" AND NOT EXISTS "${plugin_release}" AND NOT EXISTS "${plugin_debug}")
                        message(WARNING "Qt plugin \"${plugin}\" not recognized or found.")
                endif()
                install_qt4_plugin_path("${plugin_release}" "${executable}" "${copy}" "${installed_plugin_path_var}" "${plugins_dir}" "${component}" "Release|RelWithDebInfo|MinSizeRel")
                install_qt4_plugin_path("${plugin_debug}" "${executable}" "${copy}" "${installed_plugin_path_var}" "${plugins_dir}" "${component}" "Debug")
        endif()
        set(installed_plugin_path_var "${installed_plugin_path_var}" PARENT_SCOPE)
endfunction()

function(install_qt4_executable executable)
        set(qtplugins ${ARGV1})
        set(libs ${ARGV2})
        set(dirs ${ARGV3})
        set(plugins_dir ${ARGV4})
        set(request_qt_conf ${ARGV5})
        set(component ${ARGV6})
        if(QT_LIBRARY_DIR)
                list(APPEND dirs "${QT_LIBRARY_DIR}")
        endif()
        if(component)
                set(component COMPONENT ${component})
        else()
                unset(component)
        endif()

        get_filename_component(executable_absolute "${executable}" ABSOLUTE)
        if(EXISTS "${QT_QTCORE_LIBRARY_RELEASE}")
            gp_file_type("${executable_absolute}" "${QT_QTCORE_LIBRARY_RELEASE}" qtcore_type)
        elseif(EXISTS "${QT_QTCORE_LIBRARY_DEBUG}")
            gp_file_type("${executable_absolute}" "${QT_QTCORE_LIBRARY_DEBUG}" qtcore_type)
        endif()
        if(qtcore_type STREQUAL "system")
                set(qt_plugins_dir "")
        endif()

        if(NOT qtplugins AND QT_LIBRARIES_PLUGINS)
                set(qtplugins "${QT_LIBRARIES_PLUGINS}")
        endif()

        foreach(plugin ${qtplugins})
                set(installed_plugin_paths "")
                install_qt4_plugin("${plugin}" "${executable}" 0 installed_plugin_paths "${plugins_dir}" "${component}")
                list(APPEND libs ${installed_plugin_paths})
        endforeach()

        resolve_qt4_paths(libs)

        install(CODE
  "INCLUDE(\"${DeployQt4_cmake_dir}/DeployQt4.cmake\")
  SET(BU_CHMOD_BUNDLE_ITEMS TRUE)
  FIXUP_QT4_EXECUTABLE(\"\${CMAKE_INSTALL_PREFIX}/${executable}\" \"\" \"${libs}\" \"${dirs}\" \"${plugins_dir}\" \"${request_qt_conf}\")"
                ${component}
        )
endfunction()
