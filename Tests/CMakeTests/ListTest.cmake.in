macro(TEST command expected)
  if("x${result}" STREQUAL "x${expected}")
    #message("TEST \"${command}\" success: \"${result}\" expected: \"${expected}\"")
  else()
    message(SEND_ERROR "${CMAKE_CURRENT_LIST_LINE}: TEST \"${command}\" failed: \"${result}\" expected: \"${expected}\"")
  endif()
endmacro()

set(mylist andy bill ken brad)

list(LENGTH mylist result)
TEST("LENGTH mylist result" "4")
list(LENGTH "mylist" result)
TEST("LENGTH \"mylist\" result" "4")

list(LENGTH "nonexiting_list1" result)
TEST("LENGTH \"nonexiting_list1\" result" "0")

list(GET mylist 3 2 1 0 result)
TEST("GET mylist 3 2 1 0 result" "brad;ken;bill;andy")

list(GET mylist 0 item0)
list(GET mylist 1 item1)
list(GET mylist 2 item2)
list(GET mylist 3 item3)
set(result "${item3}" "${item0}" "${item1}" "${item2}")
TEST("GET individual 3 2 1 0 result" "brad;andy;bill;ken")

list(GET mylist -1 -2 -3 -4 result)
TEST("GET mylist -1 -2 -3 -4 result" "brad;ken;bill;andy")

list(GET mylist -1 2 -3 0 result)
TEST("GET mylist -1 2 -3 0 ${result}" "brad;ken;bill;andy")

list(GET "nonexiting_list2" 1 result)
TEST("GET \"nonexiting_list2\" 1 result" "NOTFOUND")

set(result andy)
list(APPEND result brad)
TEST("APPEND result brad" "andy;brad")

list(APPEND "nonexiting_list3" brad)
set(result "${nonexiting_list3}")
TEST("APPEND \"nonexiting_list3\" brad" "brad")

list(INSERT "nonexiting_list4" 0 andy bill brad ken)
set(result "${nonexiting_list4}")
TEST("APPEND \"nonexiting_list4\" andy bill brad ken" "andy;bill;brad;ken")

set(result andy brad)
list(INSERT result -1 bill ken)
TEST("INSERT result -1 bill ken" "andy;bill;ken;brad")

set(result andy bill brad ken bob)
list(REMOVE_ITEM result bob)
TEST("REMOVE_ITEM result bob" "andy;bill;brad;ken")

set(result andy bill bob brad ken peter)
list(REMOVE_ITEM result peter bob)
TEST("REMOVE_ITEM result peter bob" "andy;bill;brad;ken")

set(result bob andy bill bob brad ken bob)
list(REMOVE_ITEM result bob)
TEST("REMOVE_ITEM result bob" "andy;bill;brad;ken")

set(result andy bill bob brad ken peter)
list(REMOVE_AT result 2 -1)
TEST("REMOVE_AT result 2 -1" "andy;bill;brad;ken")

# ken is at index 2, nobody is not in the list so -1 should be returned
set(mylist andy bill ken brad)
list(FIND mylist ken result)
TEST("FIND mylist ken result" "2")

list(FIND mylist nobody result)
TEST("FIND mylist nobody result" "-1")

set(result ken bill andy brad)
list(SORT result)
TEST("SORT result" "andy;bill;brad;ken")

set(result andy bill brad ken)
list(REVERSE result)
TEST("REVERSE result" "ken;brad;bill;andy")

set(result bill andy bill brad ken ken ken)
list(REMOVE_DUPLICATES result)
TEST("REMOVE_DUPLICATES result" "bill;andy;brad;ken")

# these commands should just do nothing if the list is already empty
set(result "")
list(REMOVE_DUPLICATES result)
TEST("REMOVE_DUPLICATES empty result" "")

list(REVERSE result)
TEST("REVERSE empty result" "")

list(SORT result)
TEST("SORT empty result" "")
