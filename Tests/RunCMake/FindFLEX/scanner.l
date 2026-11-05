%option noyywrap

%%

[0-9]+ { return 1; }
.|\n   { }

%%
