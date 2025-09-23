Regular expression debugging tool
=================================

A tool to help diagnose issues with ``RunCMake`` regular expressions by
offering an editor with live results matching the haystack (``content.txt``)
against the needle (``re.txt``).

This utility makes a few assumptions, but further improvement for other
workflows is welcome. One assumption is that it is run from this directory
(i.e., ``./run.sh``).

Requirements
------------

The tool currently assumes it is running inside of a ``tmux`` session and
offers a split which prints the results of matching the regular expression
against the content.

The ``EDITOR`` environment variable is used to detect the preferred editor,
defaulting to ``nano``. If the editor is detected as a Vi-alike (i.e., has
``vi`` in its name), both files are automatically opened in separate windows.
