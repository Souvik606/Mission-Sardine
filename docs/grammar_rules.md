```
multiline: NEWLINE* (expression|statements|jump_statements) (NEWLINE* (expression|statements|jump_statements))* NEWLINE*

jump-statements:KEYWORD:yield expression|KEYWORD:proceed | KEYWORD:escape

statements: (KEYWORD:define)? IDENTIFIER EQUAL expression

switch-statement: KEYWORD:menu ternary-expression LPAREN2 NEWLINE* (case-statement* NEWLINE*)* default-statement? NEWLINE* (case_statement* NEWLINE*)* RPAREN2

case-statement: KEYWORD:choice ternary-expression LPAREN2 ((expression|statements) RPAREN2)| (NEWLINE multiline RPAREN2)

default-statement: KEYWORD:fallback LPAREN2 ((expression|statements) RPAREN2)| (NEWLINE multiline RPAREN2)

expression: jump_statements | ternary-expression

ternary-expression: (logical-expression|statements) (QUESTION ternary-expression COLON ternary-expression)*

logical-expression: comp-expression ((KEYWORD:AND | KEYWORD:OR) comp-expression)*

comp-expression: KEYWORD:NOT comp-expression | arith-expression ((EE | NEQ | LT | GT | LTE | GTE) arith-expression)*

arith-expression: term ((PLUS | MINUS) term)*

term: unary ((MUL | DIV | MODULUS | FLOOR_DIV) unary)*

unary: (PLUS | MINUS) unary | exponent

exponent: factor (EXP unary)*

factor: INT | FLOAT | STRING | IDENTIFIER | LPAREN expression RPAREN | if-expression | for-expression | while-expression | function-definition | list-expression | function-call | switch-statement

function-call: IDENTIFIER LPAREN (expression(COMMA expression)*)? RPAREN

list-expression: LPAREN3 (expression(COMMA expression)*)? RPAREN RPAREN3

while-expression: KEYWORD:during expression LPAREN2 ((expression|statements) RPAREN2)| (NEWLINE multiline RPAREN2)

for-expression: KEYWORD:cycle IDENTIFIER EQUAL expression COLON expression (COLON:expression)?LPAREN2 ((expression|statements)RPAREN2)| (NEWLINE multiline RPAREN2)

function-definition: KEYWORD:method IDENTIFIER?LPAREN (IDENTIFIER (COMMA IDENTIFIER)*)? RPAREN LPAREN2 ((expression|statements)RPAREN2)| (NEWLINE multiline RPAREN2)

if-expression: KEYWORD:when expression LPAREN2 ((expression|statements) RPAREN2 (elif-expression|else-expression)?) | (NEWLINE multiline RPAREN2 (elif-expression|else-expression))

elif-expression: KEYWORD:orwhen expression LPAREN2 ((expression|statements) RPAREN2 (elif-expression|else-expression)?) | (NEWLINE multiline RPAREN2 (elif-expression|else-expression))

else-expression: KEYWORD:otherwise LPAREN2 (((expression|statements)RPAREN2)|NEWLINE multiline RPAREN2)
```