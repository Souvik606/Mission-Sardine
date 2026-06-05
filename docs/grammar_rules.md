```grammar
multiline: NEWLINE* (singleline)* (NEWLINE* (singleline))* NEWLINE*

singleline: expression | statements | jump-statements | if-expression | for-expression | while-expression | switch-statement | function-definition | exception-handling | class-definition | foreach-expression | summon-statement

class-definition: KEYWORD:model IDENTIFIER (COLON IDENTIFIER (COMMA IDENTIFIER)*)? LPAREN2 NEWLINE* (class-member NEWLINE*)* RPAREN2

class-member: attr-declaration | constructor-definition | method-definition

attr-declaration: (KEYWORD:open | KEYWORD:guarded | KEYWORD:secret)? KEYWORD:attr LT attr-list GT

attr-list: attr-item (COMMA attr-item)*

attr-item: IDENTIFIER (EQUAL expression)?

constructor-definition: KEYWORD:init LPAREN parameter-list? RPAREN LPAREN2 NEWLINE* (initializer-list)? (multiline | jump-statements)* NEWLINE* RPAREN2

method-definition: (KEYWORD:open | KEYWORD:guarded | KEYWORD:secret)? KEYWORD:method IDENTIFIER? LPAREN parameter-list? RPAREN LPAREN2 (multiline |jump-statements)* RPAREN2

parameter-list: parameter-item (COMMA parameter-item)*

parameter-item: IDENTIFIER (EQUAL expression)?

initializer-list: initializer-item ((COMMA NEWLINE* | NEWLINE+) initializer-item)*

initializer-item: IDENTIFIER COLON expression

jump-statements: KEYWORD:proceed | KEYWORD:escape | KEYWORD:yield expression (COMMA expression)*

statements: IDENTIFIER (LPAREN3 expression RPAREN3)* (COMMA IDENTIFIER (LPAREN3 expression RPAREN3)*)* (EQUAL | PLUSEQUAL | MINUSEQUAL | MULEQUAL | DIVIDEEQUAL | MODULUSEQUAL | FLOOREQUAL | EXPEQUAL | BITOREQUAL | BITXOREQUAL | BITANDEQUAL | LSHIFTEQUAL | RSHIFTEQUAL) expression (COMMA expression)*

switch-statement: KEYWORD:menu ternary-expression LPAREN2 NEWLINE* (case-statement* NEWLINE*)* default-statement? NEWLINE* (case-statement* NEWLINE*)* RPAREN2

case-statement: KEYWORD:choice ternary-expression LPAREN2 ((expression | statements) RPAREN2) | (NEWLINE multiline RPAREN2)

default-statement: KEYWORD:fallback LPAREN2 ((expression | statements) RPAREN2) | (NEWLINE multiline RPAREN2)

expression: ternary-expression

ternary-expression: (logical-expression | statements) (QUESTION ternary-expression COLON ternary-expression)*

logical-expression: bitwise-expression ((KEYWORD:and | KEYWORD:or) bitwise-expression)*

bitwise-expression: bitwise-xor (BITOR bitwise-xor)*

bitwise-xor: bitwise-and (BITXOR bitwise-and)*

bitwise-and: comp-expression (BITAND comp-expression)*

comp-expression: KEYWORD:not comp-expression | shift-expression ((EE | NEQ | LT | GT | LTE | GTE) shift-expression)*

shift-expression: arith-expression ((LSHIFT | RSHIFT) arith-expression)*

arith-expression: term ((PLUS | MINUS) term)*

term: unary ((MUL | DIV | MOD | FLOOR) unary)*

unary: (PLUS | MINUS | BITNOT) unary | exponent

exponent: call (EXP unary)*

call: attr-access (LPAREN (expression(COMMA expression)*)? RPAREN)*

attr-access: factor (DOT IDENTIFIER)*

factor: INT | FLOAT | STRING | FSTRING | IDENTIFIER (LPAREN3 expression RPAREN3)* | LPAREN expression RPAREN | list-expression | dict-expression

dict-expression: LPAREN2 (expression COLON expression (COMMA expression COLON expression)*)? RPAREN2 | dict-comprehension

dict-comprehension: LPAREN2 expression COLON expression (cycle-comprehension-clause | trace-comprehension-clause) RPAREN2

list-expression: LPAREN3 (expression (COMMA expression)*)? RPAREN3 | list-comprehension

list-comprehension: LPAREN3 expression (cycle-comprehension-clause | trace-comprehension-clause) RPAREN3

cycle-comprehension-clause: KEYWORD:cycle IDENTIFIER EQUAL expression COLON expression (COLON expression)? (KEYWORD:when expression)?

trace-comprehension-clause: KEYWORD:trace IDENTIFIER (COMMA IDENTIFIER)* LARROW expression (KEYWORD:when expression)?

exception-handling: try-expression NEWLINE* ( catch-expression NEWLINE* (catch-expression)* NEWLINE* finally-expression? | finally-expression)

try-expression: KEYWORD:risk LPAREN2 (multiline | jump-statements)* RPAREN2

catch-expression: KEYWORD:trap (ERROR (IDENTIFIER)?)? LPAREN2 (multiline | jump-statements)* RPAREN2

finally-expression: KEYWORD:clean LPAREN2 (multiline | jump-statements)* RPAREN2

while-expression: KEYWORD:during expression LPAREN2 (multiline | jump-statements)* RPAREN2

for-expression: KEYWORD:cycle IDENTIFIER EQUAL expression COLON expression (COLON expression)? LPAREN2 (multiline | jump-statements)* RPAREN2

foreach-expression: KEYWORD:trace IDENTIFIER (COMMA IDENTIFIER)* LARROW expression LPAREN2 (multiline | jump-statements)* RPAREN2

summon-statement: KEYWORD:summon (summon-wildcard | summon-selective | summon-alias | summon-bare)

summon-wildcard: MUL KEYWORD:from IDENTIFIER

summon-selective: (IDENTIFIER KEYWORD:as IDENTIFIER KEYWORD:from IDENTIFIER) | (IDENTIFIER (COMMA IDENTIFIER)* KEYWORD:from IDENTIFIER)

summon-alias: IDENTIFIER KEYWORD:as IDENTIFIER

summon-bare: IDENTIFIER

function-definition: KEYWORD:method IDENTIFIER? LPAREN parameter-list? RPAREN LPAREN2 (multiline |jump-statements)* RPAREN2

if-expression: KEYWORD:when expression LPAREN2 (multiline | jump-statements)* RPAREN2 NEWLINE* (elif-expression | else-expression)?

elif-expression: KEYWORD:orwhen expression LPAREN2 (multiline | jump-statements)* RPAREN2 NEWLINE* (elif-expression |else-expression)?

else-expression: KEYWORD:otherwise LPAREN2 (multiline | jump-statements)* RPAREN2
```