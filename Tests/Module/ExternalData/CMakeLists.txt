cmake_minimum_required(VERSION 2.8.10.20130115)
project(ExternalDataTest NONE)

include(CTest)

include(ExternalData)

if(NOT "${CMAKE_CURRENT_SOURCE_DIR}" MATCHES "^/")
  set(slash /)
endif()
set(ExternalData_URL_TEMPLATES
  "file://${slash}${CMAKE_CURRENT_SOURCE_DIR}/%(algo)/%(hash)"
  )
set(ExternalData_BINARY_ROOT "${CMAKE_CURRENT_BINARY_DIR}/ExternalData")
file(REMOVE_RECURSE ${ExternalData_BINARY_ROOT}) # clean test

if(MAKE_SUPPORTS_SPACES)
  set(Data1CheckSpaces -D "DataSpace=DATA{Data Space.dat}")
endif()

ExternalData_Add_Test(Data1
  NAME Data1Check
  COMMAND ${CMAKE_COMMAND}
    -D Data=DATA{Data.dat}
    ${Data1CheckSpaces}
    -D SeriesA=DATA{SeriesA.dat,:}
    -D SeriesB=DATA{SeriesB.dat,:}
    -D SeriesC=DATA{SeriesC.dat,:}
    -D SeriesD=DATA{SeriesD.dat,:}
    -D SeriesAn=DATA{SeriesAn1.dat,:}
    -D SeriesBn=DATA{SeriesBn_1.dat,:}
    -D SeriesCn=DATA{SeriesCn.1.dat,:}
    -D SeriesDn=DATA{SeriesDn-1.dat,:}
    -D SeriesMixed=DATA{SeriesMixed.1.dat,:}
    -D Paired=DATA{PairedA.dat,PairedB.dat}
    -D Meta=DATA{MetaTop.dat,REGEX:Meta[ABC].dat}
    -D Directory=DATA{Directory/,A.dat,REGEX:[BC].dat}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/Data1Check.cmake
  )
ExternalData_Add_Target(Data1)

add_subdirectory(Data2)
add_subdirectory(Data3)
