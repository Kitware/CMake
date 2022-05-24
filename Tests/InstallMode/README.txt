This is an example superbuild project to demonstrate the use of the
CMAKE_INSTALL_MODE environment variable on.

The project hierarchy is like (B = Builds / D = Link Dependency):

+---------------------------------------------------------------------+
|                           Superbuild (Top)                          |
+---------------------------------------------------------------------+
        |                 |                 |                 |
        |                 |                 |                 |
       (B)               (B)               (B)               (B)
        |                 |                 |                 |
        v                 v                 v                 v
+---------------+ +---------------+ +---------------+ +---------------+
| A: Static Lib | | B: Shared Lib | | C: Nested     | | D: Executable |
|    Project    | |    Project    | |    Superbuild | |    Project    |
+---------------+ +---------------+ +---------------+ +---------------+
        ^               ^               |       |        |   |   |
        |               |              (B)     (B)       |   |   |
        |               |               |       |        |   |   |
        |               |               v       |        |   |   |
        |               |  +----------------+   |        |   |   |
        |               |  | C1: Static Lib |   |        |   |   |
        |               |  |     Project    |   |       (D) (D) (D)
        |               |  +----------------+   |        |   |   |
        |               |       ^               |        |   |   |
        |               |       |               v        |   |   |
        |               |      (D) +----------------+    |   |   |
        |               |       |  | C2: Static Lib |<---+   |   |
        |               |       +--|     Project    |        |   |
        |               |          +----------------+        |   |
        |               |                                    |   |
        |               +------------------------------------+   |
        |                                                        |
        +--------------------------------------------------------+

The superbuild system is built on top of ExternalProject_Add().

NOTE that the subprojects will configure, build and install
during the build phase ('make') of the top-level project.
There is no install target in the top-level project!
The CMAKE_INSTALL_PREFIX is therefore populated during the build
phase already.
