add_executable(empty main.c
        $<$<COMPILE_LANG_AND_ID:C,MSVC>:empty.c>
        $<$<COMPILE_LANG_AND_ID:C,GNU>:empty2.c>
        $<$<COMPILE_LANG_AND_ID:C,Clang>:empty3.c>
        )
