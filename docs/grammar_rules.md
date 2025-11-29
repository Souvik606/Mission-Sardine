```
multiline:NEWLINE* (expression|statements) (NEWLINE* (expression|statements))* NEWLINE*

statements:(KEYWORD:define)? IDENTIFIER EQUAL expression

expression: comp-expression((KEYWORD:AND|OR)comp-expression)*

comp-expression: NOT comp-expression|arith-expression((EE|NEQ|LT|GT|LTE|GTE) arith-expression)*

arith-expression : term ((PLUS|MINUS) term)*

term : factor((MUL|DIV) factor )*|function-call

function-call: IDENTIFIER LPAREN (expression(COMMA expression)*)? RPAREN

factor: INT|FLOAT|STRING|IDENTIFIER|(PLUS|MINUS) factor|LPAREN expression RPAREN| if-expression| for-expression| while-expression| function-definition| list-expression

list-expression:LPAREN3 (expression(COMMA expression)*)? RPAREN RPAREN3

while-expression: Keyword:during expression LPAREN2 (expression|statements) RPAREN2)? NEWLINE multiline RPAREN2

for-expression:KEYWORD:cycle IDENTIFIER EQUAL expression COLON expression (COLON:expression)?LPAREN2 (expression|statements)RPAREN2)? NEWLINE multiline RPAREN2

function-definition:KEYWORD:method IDENTIFIER?LPAREN (IDENTIFIER (COMMA IDENTIFIER)*)? RPAREN LPAREN2 (expression|statements)RPAREN2)? NEWLINE multiline RPAREN2

if-expression:KEYWORD:when expression LPAREN2 ((expression|statements) RPAREN2 (elif-expression|else-expression)?)?(NEWLINE multiline RPAREN2 (elif-expression|else-expression)

elif-expression:KEYWORD:orwhen expression LPAREN2 ((expression|statements) RPAREN2 (elif-expression|else-expression)?)?(NEWLINE multiline RPAREN2 (elif-expression|else-expression)

else-expression:KEYWORD:otherwise LPAREN2 ((expression|statements)RPAREN2)? NEWLINE multiline RPAREN2
```