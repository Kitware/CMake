include(RunCMake)

run_cmake(JSON)

run_cmake(JSONNoJson)
run_cmake(JSONWrongMode)
run_cmake(JSONOneArg)
run_cmake(JSONNoArgs)

run_cmake(Append)
run_cmake(AppendNoArgs)

run_cmake(Prepend)
run_cmake(PrependNoArgs)

run_cmake(Concat)
run_cmake(ConcatNoArgs)

run_cmake(Join)
run_cmake(JoinNoArgs)
run_cmake(JoinNoVar)

run_cmake(Timestamp)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "AIX" # FIXME: Needs 64-bit build
    AND NOT CMAKE_SYSTEM_NAME STREQUAL "SunOS" # FIXME: Needs 64-bit build
    AND NOT (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
    CMAKE_SYSTEM_PROCESSOR MATCHES "^(hppa|parisc64|sparc|sparc64)$" # FIXME: 32-bit time_t?
             )
    )
  run_cmake(Timestamp2038)
endif()
run_cmake(TimestampEmpty)
run_cmake(TimestampInvalid)
run_cmake(TimestampInvalid2)

run_cmake(Uuid)
run_cmake(UuidMissingNamespace)
run_cmake(UuidMissingNamespaceValue)
run_cmake(UuidBadNamespace)
run_cmake(UuidMissingNameValue)
run_cmake(UuidMissingTypeValue)
run_cmake(UuidBadType)

run_cmake(RegexClear)
run_cmake(RegexMultiMatchClear)
run_cmake(RegexEmptyMatch)
run_cmake(CMP0186)

run_cmake(UTF-16BE)
run_cmake(UTF-16LE)
run_cmake(UTF-32BE)
run_cmake(UTF-32LE)

run_cmake(Repeat)
run_cmake(RepeatNoArgs)
run_cmake(RepeatNegativeCount)

run_cmake(Hex)
run_cmake(HexTooManyArgs)
run_cmake(HexNotEnoughArgs)
