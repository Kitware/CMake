%{
int yylex(void);
void yyerror(char const *message);
%}

%token NUMBER

%%

input:
  %empty
| input NUMBER
;

%%

void yyerror(char const *)
{
}
