%module my_add

%{
int add(int a, int b) {
    return a + b;
}
%}

int add(int a, int b);
