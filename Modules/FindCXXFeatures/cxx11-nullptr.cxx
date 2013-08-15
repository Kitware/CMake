int func(int)
{
    return 1;
}

int func(void *)
{
    return 0;
}

int main(void)
{
    void *v = nullptr;

    if (v)
        return 1;

    return func(nullptr);
}
