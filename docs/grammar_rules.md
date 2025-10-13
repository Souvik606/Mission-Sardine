statements:(KEYWORD:define)? IDENTIFIER EQUAL expression

expression: comp-expression((KEYWORD:AND|OR)comp-expression)*

comp-expression: NOT comp-expression|arith-expression((EE|LT|GT|LTE|GTE) arith-expression)*

arith-expression : term ((PLUS|MINUS) term)*

term : factor((MUL|DIV) factor )*

factor: INT|FLOAT|IDENTIFIER|(PLUS|MINUS) factor|LPAREN expression RPAREN| if-expression| for-expression

for-expression:KEYWORD:cycle IDENTIFIER EQUAL expression COLON expression (COLON:expression)?{(expression|statements)*}

if-expression:KEYWORD:when expression{(expression|statements)*} (KEYWORD:orwhen {(expression|statements)*})*(KEYWORD:otherwise{(expression|statements)*})?
