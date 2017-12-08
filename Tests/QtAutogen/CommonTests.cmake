# Autogen tests common for Qt4 and Qt5
ADD_AUTOGEN_TEST(MocOnly mocOnly)
ADD_AUTOGEN_TEST(MocOptions mocOptions)
if(QT_TEST_ALLOW_QT_MACROS)
  ADD_AUTOGEN_TEST(UicOnly uicOnly)
endif()
ADD_AUTOGEN_TEST(RccOnly rccOnly)
ADD_AUTOGEN_TEST(RccEmpty rccEmpty)
ADD_AUTOGEN_TEST(RccOffMocLibrary)
