include(RunCMake)

run_cmake(EmptyFilterRegex)
run_cmake(EmptyGet0)
run_cmake(EmptyRemoveAt0)
run_cmake(EmptyInsert-1)

run_cmake(NoArguments)
run_cmake(InvalidSubcommand)
run_cmake(GET-CMP0007-WARN)

run_cmake(FILTER-REGEX-InvalidRegex)
run_cmake(GET-InvalidIndex)
run_cmake(INSERT-InvalidIndex)
run_cmake(REMOVE_AT-InvalidIndex)
run_cmake(SUBLIST-InvalidIndex)

run_cmake(FILTER-REGEX-TooManyArguments)
run_cmake(JOIN-TooManyArguments)
run_cmake(LENGTH-TooManyArguments)
run_cmake(REMOVE_DUPLICATES-TooManyArguments)
run_cmake(REVERSE-TooManyArguments)
run_cmake(SUBLIST-TooManyArguments)

run_cmake(REMOVE_AT-EmptyList)

run_cmake(REMOVE_DUPLICATES-PreserveOrder)

run_cmake(FILTER-NotList)
run_cmake(REMOVE_AT-NotList)
run_cmake(REMOVE_DUPLICATES-NotList)
run_cmake(REMOVE_ITEM-NotList)
run_cmake(REMOVE_ITEM-NoItemArg)
run_cmake(REVERSE-NotList)
run_cmake(SORT-NotList)

run_cmake(FILTER-REGEX-InvalidMode)
run_cmake(FILTER-REGEX-InvalidOperator)
run_cmake(FILTER-REGEX-Valid0)
run_cmake(FILTER-REGEX-Valid1)

run_cmake(JOIN-NoArguments)
run_cmake(JOIN-NoVariable)
run_cmake(JOIN)

run_cmake(SUBLIST-NoArguments)
run_cmake(SUBLIST-NoVariable)
run_cmake(SUBLIST-InvalidLength)
run_cmake(SUBLIST)

run_cmake(TRANSFORM-NoAction)
run_cmake(TRANSFORM-InvalidAction)
# 'action' oriented tests
run_cmake(TRANSFORM-TOUPPER-TooManyArguments)
run_cmake(TRANSFORM-TOLOWER-TooManyArguments)
run_cmake(TRANSFORM-STRIP-TooManyArguments)
run_cmake(TRANSFORM-GENEX_STRIP-TooManyArguments)
run_cmake(TRANSFORM-APPEND-NoArguments)
run_cmake(TRANSFORM-APPEND-TooManyArguments)
run_cmake(TRANSFORM-PREPEND-NoArguments)
run_cmake(TRANSFORM-PREPEND-TooManyArguments)
run_cmake(TRANSFORM-REPLACE-NoArguments)
run_cmake(TRANSFORM-REPLACE-NoEnoughArguments)
run_cmake(TRANSFORM-REPLACE-TooManyArguments)
run_cmake(TRANSFORM-REPLACE-InvalidRegex)
run_cmake(TRANSFORM-REPLACE-InvalidReplace1)
run_cmake(TRANSFORM-REPLACE-InvalidReplace2)
# 'selector' oriented tests
run_cmake(TRANSFORM-Selector-REGEX-NoArguments)
run_cmake(TRANSFORM-Selector-REGEX-TooManyArguments)
run_cmake(TRANSFORM-Selector-REGEX-InvalidRegex)
run_cmake(TRANSFORM-Selector-AT-NoArguments)
run_cmake(TRANSFORM-Selector-AT-BadArgument)
run_cmake(TRANSFORM-Selector-AT-InvalidIndex)
run_cmake(TRANSFORM-Selector-FOR-NoArguments)
run_cmake(TRANSFORM-Selector-FOR-NoEnoughArguments)
run_cmake(TRANSFORM-Selector-FOR-TooManyArguments)
run_cmake(TRANSFORM-Selector-FOR-BadArgument)
run_cmake(TRANSFORM-Selector-FOR-InvalidIndex)
# 'output' oriented tests
run_cmake(TRANSFORM-Output-OUTPUT_VARIABLE-NoArguments)
run_cmake(TRANSFORM-Output-OUTPUT_VARIABLE-TooManyArguments)
# Successful tests
run_cmake(TRANSFORM-TOUPPER)
run_cmake(TRANSFORM-TOLOWER)
run_cmake(TRANSFORM-STRIP)
run_cmake(TRANSFORM-GENEX_STRIP)
run_cmake(TRANSFORM-APPEND)
run_cmake(TRANSFORM-PREPEND)
run_cmake(TRANSFORM-REPLACE)

# argument tests
run_cmake(SORT-WrongOption)
run_cmake(SORT-BadCaseOption)
run_cmake(SORT-BadCompareOption)
run_cmake(SORT-BadOrderOption)
run_cmake(SORT-DuplicateOrderOption)
run_cmake(SORT-DuplicateCompareOption)
run_cmake(SORT-DuplicateCaseOption)
run_cmake(SORT-NoCaseOption)

# Successful tests
run_cmake(SORT)

# argument tests
run_cmake(PREPEND-NoArgs)
# Successful tests
run_cmake(PREPEND)

# argument tests
run_cmake(POP_BACK-NoArgs)
run_cmake(POP_FRONT-NoArgs)
# Successful tests
run_cmake(POP_BACK)
run_cmake(POP_FRONT)
