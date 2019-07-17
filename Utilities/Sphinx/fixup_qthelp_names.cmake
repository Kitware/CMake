
file(READ "${QTHELP_DIR}/CMake.qhcp" QHCP_CONTENT)

string(REPLACE
  "<homePage>qthelp://org.sphinx.cmake" "<homePage>qthelp://org.cmake"
  QHCP_CONTENT "${QHCP_CONTENT}"
)
string(REPLACE
  "<startPage>qthelp://org.sphinx.cmake" "<startPage>qthelp://org.cmake"
  QHCP_CONTENT "${QHCP_CONTENT}"
)

file(WRITE "${QTHELP_DIR}/CMake.qhcp" "${QHCP_CONTENT}")


file(READ "${QTHELP_DIR}/CMake.qhp" QHP_CONTENT)

string(REPLACE
  "<namespace>org.sphinx.cmake" "<namespace>org.cmake"
  QHP_CONTENT "${QHP_CONTENT}"
)

file(WRITE "${QTHELP_DIR}/CMake.qhp" "${QHP_CONTENT}")
