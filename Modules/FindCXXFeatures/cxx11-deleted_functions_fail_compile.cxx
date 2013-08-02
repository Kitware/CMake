struct A {
    A() = delete;
    ~A() = delete;
    int m;
    int ret();
};

int A::ret()
{
    return 2 * m;
}

int main(void)
{
    A bar;
    bar.m = 1;
    return bar.ret();
}
