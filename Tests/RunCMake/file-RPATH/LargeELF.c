/* Create more than 65536 ELF sections.  */

/* clang-format off */
#define C0(i) int v##i __attribute__((section("s" #i)))
#define C1(i) C0(i##0); C0(i##1); C0(i##2); C0(i##3); C0(i##4); \
              C0(i##5); C0(i##6); C0(i##7); C0(i##8); C0(i##9)
#define C2(i) C1(i##0); C1(i##1); C1(i##2); C1(i##3); C1(i##4); \
              C1(i##5); C1(i##6); C1(i##7); C1(i##8); C1(i##9)
#define C3(i) C2(i##0); C2(i##1); C2(i##2); C2(i##3); C2(i##4); \
              C2(i##5); C2(i##6); C2(i##7); C2(i##8); C2(i##9)
#define C4(i) C3(i##0); C3(i##1); C3(i##2); C3(i##3); C3(i##4); \
              C3(i##5); C3(i##6); C3(i##7); C3(i##8); C3(i##9)
/* clang-format on */

C4(1);
C4(2);
C4(3);
C4(4);
C4(5);
C4(6);
C4(7);

int main(void)
{
  return 0;
}
