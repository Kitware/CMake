# Our CI machines do not consistently have Java installed, so a build may
# detect that Java is available and working, but a test machine then not have a
# working Java installed. To work around this, just act as if Java is not
# available on any CI machine.
set(Java_JAVA_EXECUTABLE "" CACHE FILEPATH "")
set(Java_JAVAC_EXECUTABLE "" CACHE FILEPATH "")
set(Java_JAR_EXECUTABLE "" CACHE FILEPATH "")

set(BUILD_QtDialog ON CACHE BOOL "")
