set(ENV{SOURCE_DATE_EPOCH} "1123456789")
string(TIMESTAMP RESULT "%Y-%m-%d %H:%M:%S.%f %A=%a %B=%b %y day=%j wd=%w week=%U w_iso=%V %%I=%I epoch=%s" UTC)
message("RESULT=${RESULT}")
