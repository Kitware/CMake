file(WRITE "${OUTFILE}" [[int main(void)
{
  return 0;
}
]])
string(REPLACE [[ ]] [[\ ]] OUTFILE "${OUTFILE}")
string(REPLACE [[ ]] [[\ ]] INFILE "${INFILE}")
file(WRITE "${DEPFILE}" "${OUTFILE}: ${INFILE}\n")
