#pragma once
#include <bits/stdc++.h>
#include "lexer.h"
#include "../ast_results/parse_result.h"
#include "../ast_nodes/operation_nodes.h"
#include "error.h"
#include "constants.h"

using namespace std;

class Parser {
public:
    explicit Parser(vector<Token> tokens)
        : tokens(std::move(tokens)), tok_index(-1), current_tok(nullopt) {
        advance();
    }

    ParseResult parse() {
        ParseResult result = expression();
        if (!result.error && current_tok.has_value() && current_tok->type != T_EOF) {
            return result.failure(
                InvalidSyntaxError(
                    current_tok->pos_start.value(), current_tok->pos_end.value(),
                    "Expected '+', '-', '*', or '/'"
                )
            );
        }
        return result;
    }

private:
    vector<Token> tokens;
    int tok_index;
    optional<Token> current_tok;

    void advance() {
        tok_index++;
        if (tok_index < tokens.size()) {
            current_tok = tokens[tok_index];
        } else {
            current_tok = nullopt;
        }
    }

    ParseResult expression() {
        ParseResult res;
        auto left_node = res.register_node(term());
        if (res.error) {
            return res;
        }

        while (current_tok.has_value() && (current_tok->type == T_PLUS || current_tok->type == T_MINUS)) {
            Token op_token = current_tok.value();
            advance();
            auto right_node = res.register_node(term());
            if (res.error) {
                return res;
            }
            left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
        }

        return res.success(left_node);
    }

    ParseResult term() {
        ParseResult res;
        auto left_node = res.register_node(factor());
        if (res.error) {
            return res;
        }

        while (current_tok.has_value() && (current_tok->type == T_MUL || current_tok->type == T_DIVIDE)) {
            Token op_token = current_tok.value();
            advance();
            auto right_node = res.register_node(factor());
            if (res.error) {
                return res;
            }
            left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
        }

        return res.success(left_node);
    }

    ParseResult factor() {
        ParseResult res;
        Token token = current_tok.value();

        if (token.type == T_PLUS || token.type == T_MINUS) {
            advance();
            auto factor_node = res.register_node(factor());
            if (res.error) {
                return res;
            }
            return res.success(make_shared<UnaryOperationNode>(token, factor_node));
        } else if (token.type == T_INT || token.type == T_FLOAT) {
            advance();
            return res.success(make_shared<NumberNode>(token));
        } else if (token.type == T_LPAREN) {
            advance();
            auto expr_node = res.register_node(expression());
            if (res.error) {
                return res;
            }
            if (current_tok.has_value() && current_tok->type == T_RPAREN) {
                advance();
                return res.success(expr_node);
            } else {
                return res.failure(
                    InvalidSyntaxError(
                        current_tok->pos_start.value(), current_tok->pos_end.value(),
                        "Expected ')'"
                    )
                );
            }
        }

        return res.failure(
            InvalidSyntaxError(
                token.pos_start.value(), token.pos_end.value(),
                "Expected int, float, '+', '-', or '('"
            )
        );
    }
};
