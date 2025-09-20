#pragma once

#include <bits/stdc++.h>

#include "lexer.h"
#include "../ast_results/parse_result.h"
#include "../ast_nodes/operation_nodes.h"
#include "../ast_nodes/variable_nodes.h"
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
        ParseResult result = {};
        if (current_tok.has_value()) {
            if (current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "define") {
                result = statements();
            } else {
                result = expression();
            }
        } else {
            return result;
        }

        if (!result.error && current_tok.has_value() && current_tok->type != T_EOF) {
            return result.failure(
                InvalidSyntaxError(
                    current_tok->pos_start.value(), current_tok->pos_end.value(),
                    "Unexpected token after statement or expression"
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
        current_tok = (tok_index < tokens.size()) ? make_optional(tokens[tok_index]) : nullopt;
    }

    [[nodiscard]] optional<Token> peek() const {
        const int next_index = tok_index + 1;
        return (next_index < tokens.size()) ? make_optional(tokens[next_index]) : nullopt;
    }

    ParseResult statements() {
        ParseResult res;
        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "define") {
            res.register_advancement();
            advance();

            if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected identifier after 'define'"));
            }

            Token var_name = current_tok.value();
            res.register_advancement();
            advance();

            if (!current_tok.has_value() || current_tok->type != T_EQ) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '=' after identifier"));
            }

            res.register_advancement();
            advance();
            auto expr = res.register_node(expression());
            if (res.error) return res;
            return res.success(make_shared<VariableAssignNode>(var_name, expr));
        }

        return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected 'define'"));
    }

    ParseResult expression() {
        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
            auto next_tok = peek();
            if (next_tok.has_value() && next_tok->type == T_EQ) {
                ParseResult res;
                Token var_name = current_tok.value();
                res.register_advancement();
                advance();
                res.register_advancement();
                advance();
                auto expr = res.register_node(expression());
                if (res.error) return res;
                return res.success(make_shared<VariableAssignNode>(var_name, expr));
            }
        }

        ParseResult res;
        auto left_node = res.register_node(comp_expression());
        if (res.error) return res;

        while (current_tok.has_value() && current_tok->type == T_KEYWORD) {
            auto keyword = any_cast<string>(current_tok->value);
            if (keyword == "and" || keyword == "or") {
                Token op_token = current_tok.value();
                res.register_advancement();
                advance();
                auto right_node = res.register_node(comp_expression());
                if (res.error) return res;
                left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
            } else {
                break;
            }
        }

        return res.success(left_node);
    }

    ParseResult comp_expression() {
        ParseResult res;
        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "not") {
            Token op_token = current_tok.value();
            res.register_advancement();
            advance();
            auto node = res.register_node(comp_expression());
            if (res.error) return res;
            return res.success(make_shared<UnaryOperationNode>(op_token, node));
        }

        auto left_node = res.register_node(arith_expression());
        if (res.error) return res;

        while (current_tok.has_value() && (current_tok->type == T_EE || current_tok->type == T_NEQ || current_tok->type == T_LT || current_tok->type == T_GT || current_tok->type == T_LTE || current_tok->type == T_GTE)) {
            Token op_token = current_tok.value();
            res.register_advancement();
            advance();
            auto right_node = res.register_node(arith_expression());
            if (res.error) return res;
            left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
        }

        return res.success(left_node);
    }

    ParseResult arith_expression() {
        ParseResult res;
        auto left_node = res.register_node(term());
        if (res.error) return res;

        while (current_tok.has_value() && (current_tok->type == T_PLUS || current_tok->type == T_MINUS)) {
            Token op_token = current_tok.value();
            res.register_advancement();
            advance();
            auto right_node = res.register_node(term());
            if (res.error) return res;
            left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
        }

        return res.success(left_node);
    }

    ParseResult term() {
        ParseResult res;
        auto left_node = res.register_node(factor());
        if (res.error) return res;

        while (current_tok.has_value() && (current_tok->type == T_MUL || current_tok->type == T_DIVIDE)) {
            Token op_token = current_tok.value();
            res.register_advancement();
            advance();
            auto right_node = res.register_node(factor());
            if (res.error) return res;
            left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
        }

        return res.success(left_node);
    }

    ParseResult factor() {
        ParseResult res;
        if (!current_tok.has_value()) {
            return res.failure(InvalidSyntaxError({}, {}, "Expected int, float, identifier, '+', '-', or '('"));
        }
        Token token = current_tok.value();

        if (token.type == T_PLUS || token.type == T_MINUS) {
            res.register_advancement();
            advance();
            auto factor_node = res.register_node(factor());
            if (res.error) return res;
            return res.success(make_shared<UnaryOperationNode>(token, factor_node));
        }
        else if (token.type == T_INT || token.type == T_FLOAT) {
            res.register_advancement();
            advance();
            return res.success(make_shared<NumberNode>(token));
        }
        else if (token.type == T_IDENTIFIER) {
            res.register_advancement();
            advance();
            return res.success(make_shared<VariableUseNode>(token));
        }
        else if (token.type == T_LPAREN) {
            res.register_advancement();
            advance();
            const auto expr_node = res.register_node(expression());
            if (res.error) return res;
            if (current_tok.has_value() && current_tok->type == T_RPAREN) {
                res.register_advancement();
                advance();
                return res.success(expr_node);
            } else {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected ')'"));
            }
        }

        return res.failure(InvalidSyntaxError(token.pos_start.value(), token.pos_end.value(), "Expected int, float, identifier, '+', '-', '(', or keyword 'not'"));
    }
};