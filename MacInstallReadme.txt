DESCRIPTION
===========

This is an installer created using CPack (http://www.cmake.org).  CMake is a UNIX command line tool. On the Select a Destination page of the installer the destination folder will be empty, however, the default installation folder is actually /usr.  The installer will create files in /usr/bin, /usr/doc, /usr/man, /usr/share.  If you choose a different folder for the install, then the install tree will be in that folder under but still in the sub folder usr, your PATH will not be modified.  If you want to find CMake from the command line, you will have to modify your PATH to include the DESTINATION/usr/bin, where DESTINATION is the folder you choose to install into.  As long as bin, doc, man, and share are all share the same root folder they can be moved after installation. 

