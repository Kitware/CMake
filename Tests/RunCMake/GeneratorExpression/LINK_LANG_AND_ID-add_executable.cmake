add_executable(empty main.c
        $<$<LINK_LANG_AND_ID:C,MSVC>:empty.c>
        $<$<LINK_LANG_AND_ID:C,GNU>:empty2.c>
        $<$<LINK_LANG_AND_ID:C,Clang>:empty3.c>
        )
