#
# bash-completion file for CMake
# Provided by Eric NOULARD - eric.noulard@gmail.com
#
# see http://bash-completion.alioth.debian.org/
# and http://www.cmake.org
#
# We will try to complete cmake commands options
# at 2 (or may be 3 levels)
#  [cmake|cpack|ctest] <level0> <level1> <level2>
#
#  level0 is top level cmake/cpack/ctest options
#  level1 is the first argument of level0 option
#  level2 is the seconf argument of level1 argument
#   FIXME: I don't know how to handle level2
#
# The file has been proposed for inclusion in the bash-completion package
# https://alioth.debian.org/tracker/?func=detail&atid=413095&aid=312632&group_id=100114
# In the meantime,
#   1) If you want to test bash completion for cmake/cpack/ctest
#      just source the current file at bash prompt
#      . ./cmake-completion
#
#   2) If you want to install it for good copy this file to
#      cp cmake-completion /etc/bash_completion.d/cmake
#

#
# cmake command
#
# have cmake &&
_cmake()
{
    local cur prev opts words cword
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    # seems to be only available on bash-completion 1.2
    #_get_comp_words_by_ref cur prev

    # cmake command line option we want to complete
    opts=`cmake --help | grep "^  \-.*=\ .*" | cut -d" " -f 3 | cut -d= -f 1 | cut -d[ -f 1`

    #
    #  Complete the arguments to some of
    #  the most commonly used commands (Level 1).
    #
    case "${prev}" in
        -E)
            local running=$(for x in `cmake -E |&  grep "^  " | cut -d" " -f 3`; do echo ${x} ; done )
            COMPREPLY=( $(compgen -W "${running}" -- ${cur}) )
            return 0
            ;;
        --help-command)
            local running=$(for x in `cmake --help-command-list`; do echo ${x} ; done )
            COMPREPLY=( $(compgen -W "${running}" -- ${cur}) )
            return 0
            ;;
        --help-module)
            local running=$(for x in `cmake --help-module-list`; do echo ${x} ; done )
            COMPREPLY=( $(compgen -W "${running}" -- ${cur}) )
            return 0
            ;;
         --help-policy)
            local running=$(for x in `cmake --help-policies | grep "^  CMP"`; do echo ${x} ; done )
            COMPREPLY=( $(compgen -W "${running}" -- ${cur}) )
            return 0
            ;;
         --help-property)
            local running=$(for x in `cmake --help-property-list`; do echo ${x} ; done )
            COMPREPLY=( $(compgen -W "${running}" -- ${cur}) )
            return 0
            ;;
         --help-variable)
            local running=$(for x in `cmake --help-variable-list`; do echo ${x} ; done )
            COMPREPLY=( $(compgen -W "${running}" -- ${cur}) )
            return 0
            ;;
        *)
            ;;
    esac

    #
    #  Complete the arguments to some of
    #  the most commonly used commands (Level 2).
    #   ?? How to do that ..

    #
    # Complete the option (Level 0 - right after cmake)
    #
    COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
} &&
complete -F _cmake -o default cmake

#
# cpack command
#
#have cpack &&
_cpack()
{
    local cur prev opts words cword
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    # seems to be only available on bash-completion 1.2
    #_get_comp_words_by_ref cur prev

    # cpack command line option we want to complete
    opts=`cpack --help | grep "^  \-.*=\ .*"  | cut -d" " -f 3 | cut -d= -f 1`
    opts="${opts} --help -V"

    #
    #  Complete the arguments to some of
    #  the most commonly used commands (Level 1).
    #
    case "${prev}" in
        -G)
            local running=$(for x in `cpack --help | grep "^  .*=\ .*" | grep -v "^  -" | cut -d" " -f 3`; do echo ${x} ; done )
            COMPREPLY=( $(compgen -W "${running}" -- ${cur}) )
            return 0
            ;;
        --config)
            COMPREPLY=( $(compgen -f ${cur}) )
            return 0
            ;;
        *)
            ;;
    esac

    #
    # Complete the option (Level 0 - right after cmake)
    #
    COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
} &&
complete -F _cpack -o default cpack

#
# cmake command
#
# have ctest &&
_ctest()
{
    local cur prev opts words cword
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    # seems to be only available on bash-completion 1.2
    #_get_comp_words_by_ref cur prev

    # cmake command line option we want to complete
    opts=`ctest --help | grep "\-\-.*" | cut -d" " -f 3 | sed s/,/\\\n/g`

    #
    #  Complete the arguments to some of
    #  the most commonly used commands (Level 1).
    #
    case "${prev}" in
        --help-command)
            local running=$(for x in `ctest --help-command-list`; do echo ${x} ; done )
            COMPREPLY=( $(compgen -W "${running}" -- ${cur}) )
            return 0
            ;;
        -R)
            local running=$(for x in `ctest -N 2> /dev/null | grep "^  Test" | cut -d: -f 2`; do echo ${x} ; done )
            COMPREPLY=( $(compgen -W "${running}" -- ${cur}) )
            return 0
            ;;
        *)
            ;;
    esac

    #
    #  Complete the arguments to some of
    #  the most commonly used commands (Level 2).
    #   ?? How to do that ..

    #
    # Complete the option (Level 0 - right after cmake)
    #
    COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
} &&
complete -F _ctest -o default ctest

# Local variables:
# mode: shell-script
# sh-basic-offset: 4
# sh-indent-comment: t
# indent-tabs-mode: nil
# End:
# ex: ts=4 sw=4 et filetype=sh