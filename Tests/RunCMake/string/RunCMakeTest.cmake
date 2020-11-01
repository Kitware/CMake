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
