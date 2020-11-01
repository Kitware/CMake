# Don't have targets
set(Boost_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

set(Boost_DATE_TIME_FOUND 1)
set(Boost_DATE_TIME_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/lib/libboost_date_time.a")

set(Boost_LIBRARIES ${Boost_DATE_TIME_LIBRARY})
