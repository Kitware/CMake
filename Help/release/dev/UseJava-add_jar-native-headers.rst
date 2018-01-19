UseJava-add_jar-native-headers
------------------------------

* | The command add_jar from :module:`UseJava` module learns how to generate native
    headers files using option -h of javac tool.
  | This capability requires, at least, version 1.8 of Javac tool.
  | Command create_javah will no longer be supported due to the
    `suppression of javah tool <http://openjdk.java.net/jeps/313>`_ in the version 1.10
    of the JDK, so ``add_jar(GENERATE_NATIVE_HEADERS)`` must be used instead.
