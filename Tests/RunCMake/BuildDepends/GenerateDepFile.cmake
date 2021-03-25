file(READ "${INFILE}" INCONTENT)
file(WRITE "${OUTFILE}" "${INCONTENT}")

string(REPLACE [[ ]] [[\ ]] OUTFILE "${OUTFILE}")
string(REPLACE [[ ]] [[\ ]] INFILE "${INFILE}")
file(WRITE "${DEPFILE}" "${OUTFILE}: ${INFILE}\n")
