%module my_sub

%{
int sub(int a, int b) {
    return a - b;
}
%}

int sub(int a, int b);
