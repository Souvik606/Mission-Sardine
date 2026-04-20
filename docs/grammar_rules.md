multiline: NEWLINE* (singleline)* (NEWLINE* (singleline))* NEWLINE*

singleline: function-call | statements | if-expression | for-expression | while-expression | switch-statement | function-definition

yield-statement: KEYWORD:yield expression

jump-statements: KEYWORD:proceed | KEYWORD:escape

statements: IDENTIFIER (LPAREN3 expression RPAREN3)* (COMMA IDENTIFIER (LPAREN3 expression RPAREN3)*)* EQUAL expression (COMMA expression)*

switch-statement: KEYWORD:menu ternary-expression LPAREN2 NEWLINE* (case-statement* NEWLINE*)* default-statement? NEWLINE* (case-statement* NEWLINE*)* RPAREN2

case-statement: KEYWORD:choice ternary-expression LPAREN2 ((expression | statements) RPAREN2) | (NEWLINE multiline RPAREN2)

default-statement: KEYWORD:fallback LPAREN2 ((expression | statements) RPAREN2) | (NEWLINE multiline RPAREN2)

expression: ternary-expression

ternary-expression: (logical-expression | statements) (QUESTION ternary-expression COLON ternary-expression)*

logical-expression: comp-expression ((KEYWORD:AND | KEYWORD:OR) comp-expression)*

comp-expression: KEYWORD:NOT comp-expression | arith-expression ((EE | NEQ | LT | GT | LTE | GTE) arith-expression)*

arith-expression: term ((PLUS | MINUS) term)*

term: unary ((MUL | DIV | MODULUS | FLOOR_DIV) unary)*

unary: (PLUS | MINUS) unary | exponent

exponent: factor (EXP unary)*

factor: INT | FLOAT | STRING | IDENTIFIER (LPAREN3 expression RPAREN3)* | LPAREN expression RPAREN | list-expression | function-call

function-call: IDENTIFIER LPAREN (expression(COMMA expression)*)? RPAREN

list-expression: LPAREN3 (expression(COMMA expression)*)? RPAREN3

while-expression: KEYWORD:whenever expression LPAREN2 (multiline | jump-statements)* RPAREN2

for-expression: KEYWORD:Cycle IDENTIFIER EQUAL expression COLON expression (COLON expression)? LPAREN2 (multiline | jump-statements)* RPAREN2

function-definition: KEYWORD:method IDENTIFIER? LPAREN (IDENTIFIER (COMMA IDENTIFIER)*)? RPAREN LPAREN2 (multiline |jump-statements | yield-statement)* RPAREN2

if-expression: KEYWORD:when expression LPAREN2 (multiline | jump-statements)* RPAREN2 NEWLINE* (elif-expression | else-expression)

elif-expression: KEYWORD:orwhen expression LPAREN2 (multiline | jump-statements)* RPAREN2 NEWLINE* (elif-expression |else-expression)

else-expression: KEYWORD:otherwise LPAREN2 (multiline | jump-statements)* RPAREN2