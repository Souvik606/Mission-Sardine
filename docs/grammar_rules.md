statements:(KEYWORD:define)? IDENTIFIER EQUAL expression

expression: comp-expression((KEYWORD:AND|OR)comp-expression)*

comp-expression: NOT comp-expression|arith-expression((EE|NEQ|LT|GT|LTE|GTE) arith-expression)*

arith-expression : term ((PLUS|MINUS) term)*

term : factor((MUL|DIV) factor )*|function-call

function-call: IDENTIFIER LPAREN (expression(COMMA expression)*)? RPAREN

factor: INT|FLOAT|IDENTIFIER|STRING|(PLUS|MINUS) factor|LPAREN expression RPAREN| if-expression| for-expression| while-expression| function-definition

while-expression: KEYWORD:during expression{(expression|statements)*}

for-expression:KEYWORD:cycle IDENTIFIER EQUAL expression COLON expression (COLON:expression)?{(expression|statements)*}

if-expression:KEYWORD:when expression{(expression|statements)*} (KEYWORD:orwhen {(expression|statements)*})*(KEYWORD:otherwise{(expression|statements)*})?

function-definition:KEYWORD:method IDENTIFIER?LPAREN (IDENTIFIER (COMMA IDENTIFIER)*)? RPAREN {(expression|statements)*}