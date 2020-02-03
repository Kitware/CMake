extern int AIXNotExported(void);
extern int AIXExportedSymbol(void);

int main(void)
{
  return AIXNotExported() + AIXExportedSymbol();
}
