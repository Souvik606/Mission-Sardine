assignment:(KEYWORD:define)? IDENTIFIER EQUAL expression

expression : term ((PLUS|MINUS) term)*

term : factor((MUL|DIV) factor )*

factor: INT|FLOAT|IDENTIFIER|(PLUS|MINUS) factor|LPAREN expression RPAREN