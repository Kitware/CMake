class del {
public:
    del();
    del(int k);

    int m_k;
};

del::del()
    : del(42)
{
}

del::del(int k)
    : m_k(k)
{
}

int main()
{
    del q(41);
    del a;

    return a.m_k - q.m_k - 1;
}
