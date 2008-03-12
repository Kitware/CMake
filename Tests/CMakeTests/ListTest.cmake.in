MACRO(TEST command expected)
  IF("x${result}" STREQUAL "x${expected}")
    #MESSAGE("TEST \"${command}\" success: \"${result}\" expected: \"${expected}\"")
  ELSE("x${result}" STREQUAL "x${expected}")
    MESSAGE(SEND_ERROR "${CMAKE_CURRENT_LIST_LINE}: TEST \"${command}\" failed: \"${result}\" expected: \"${expected}\"")
  ENDIF("x${result}" STREQUAL "x${expected}")
ENDMACRO(TEST command expected)

SET(mylist andy bill ken brad)

LIST(LENGTH mylist result)
TEST("LENGTH mylist result" "4")
LIST(LENGTH "mylist" result)
TEST("LENGTH \"mylist\" result" "4")

LIST(LENGTH "nonexiting_list1" result)
TEST("LENGTH \"nonexiting_list1\" result" "0")

LIST(GET mylist 3 2 1 0 result)
TEST("GET mylist 3 2 1 0 result" "brad;ken;bill;andy")

LIST(GET mylist 0 item0)
LIST(GET mylist 1 item1)
LIST(GET mylist 2 item2)
LIST(GET mylist 3 item3)
SET(result "${item3}" "${item0}" "${item1}" "${item2}")
TEST("GET individual 3 2 1 0 result" "brad;andy;bill;ken")

LIST(GET mylist -1 -2 -3 -4 result)
TEST("GET mylist -1 -2 -3 -4 result" "brad;ken;bill;andy")

LIST(GET mylist -1 2 -3 0 result)
TEST("GET mylist -1 2 -3 0 ${result}" "brad;ken;bill;andy")

LIST(GET "nonexiting_list2" 1 result)
TEST("GET \"nonexiting_list2\" 1 result" "NOTFOUND")

SET(result andy)
LIST(APPEND result brad)
TEST("APPEND result brad" "andy;brad")

LIST(APPEND "nonexiting_list3" brad)
SET(result "${nonexiting_list3}")
TEST("APPEND \"nonexiting_list3\" brad" "brad")

LIST(INSERT "nonexiting_list4" 0 andy bill brad ken)
SET(result "${nonexiting_list4}")
TEST("APPEND \"nonexiting_list4\" andy bill brad ken" "andy;bill;brad;ken")

SET(result andy brad)
LIST(INSERT result -1 bill ken)
TEST("INSERT result -1 bill ken" "andy;bill;ken;brad")

SET(result andy bill brad ken bob)
LIST(REMOVE_ITEM result bob)
TEST("REMOVE_ITEM result bob" "andy;bill;brad;ken")

SET(result andy bill bob brad ken peter)
LIST(REMOVE_ITEM result peter bob)
TEST("REMOVE_ITEM result peter bob" "andy;bill;brad;ken")

SET(result bob andy bill bob brad ken bob)
LIST(REMOVE_ITEM result bob)
TEST("REMOVE_ITEM result bob" "andy;bill;brad;ken")

SET(result andy bill bob brad ken peter)
LIST(REMOVE_AT result 2 -1)
TEST("REMOVE_AT result 2 -1" "andy;bill;brad;ken")

# ken is at index 2, nobody is not in the list so -1 should be returned
SET(mylist andy bill ken brad)
LIST(FIND mylist ken result)
TEST("FIND mylist ken result" "2")

LIST(FIND mylist nobody result)
TEST("FIND mylist nobody result" "-1")

SET(result ken bill andy brad)
LIST(SORT result)
TEST("SORT result" "andy;bill;brad;ken")

SET(result andy bill brad ken)
LIST(REVERSE result)
TEST("REVERSE result" "ken;brad;bill;andy")

SET(result bill andy bill brad ken ken ken)
LIST(REMOVE_DUPLICATES result)
TEST("REMOVE_DUPLICATES result" "bill;andy;brad;ken")
