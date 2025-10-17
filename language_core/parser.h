#pragma once

#include <bits/stdc++.h>

#include "lexer.h"
#include "../ast_results/parse_result.h"
#include "../ast_nodes/operation_nodes.h"
#include "../ast_nodes/variable_nodes.h"
#include "../ast_nodes/if_else_elif_nodes.h"
#include "../ast_nodes/for_nodes.h"
#include "../ast_nodes/while_nodes.h"
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
        ParseResult result = statements();
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

    ParseResult while_expression() {
        ParseResult res;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "during")) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected 'whenever'"));
        }
        res.register_advancement();
        advance();

        auto condition = res.register_node(expression());
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '{'"));
        }
        res.register_advancement();
        advance();

        shared_ptr<Node> body_node;
        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER && peek().has_value() && peek()->type == T_EQ) {
            body_node = res.register_node(statements());
        } else {
            body_node = res.register_node(expression());
        }
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
        }
        res.register_advancement();
        advance();

        return res.success(make_shared<WhileNode>(condition, body_node));
    }

    ParseResult for_expression() {
        ParseResult res;
        shared_ptr<Node> step_value = nullptr;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "cycle")) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected 'cycle'"));
        }
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected identifier"));
        }
        Token var_name = current_tok.value();
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_EQ) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '='"));
        }
        res.register_advancement();
        advance();

        auto start_value = res.register_node(expression());
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_COLON) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected ':'"));
        }
        res.register_advancement();
        advance();

        auto end_value = res.register_node(expression());
        if (res.error) return res;

        if (current_tok.has_value() && current_tok->type == T_COLON) {
            res.register_advancement();
            advance();
            step_value = res.register_node(expression());
            if (res.error) return res;
        }

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '{'"));
        }
        res.register_advancement();
        advance();

        shared_ptr<Node> body_node;
        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER && peek().has_value() && peek()->type == T_EQ) {
            body_node = res.register_node(statements());
        } else {
            body_node = res.register_node(expression());
        }
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
        }
        res.register_advancement();
        advance();

        return res.success(make_shared<ForNode>(var_name, start_value, end_value, step_value, body_node));
    }

    ParseResult if_expression() {
        ParseResult res;
        vector<pair<shared_ptr<Node>, shared_ptr<Node>>> cases;
        shared_ptr<Node> else_case = nullptr;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "when")) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected 'when'"));
        }
        res.register_advancement();
        advance();

        auto condition = res.register_node(expression());
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '{'"));
        }
        res.register_advancement();
        advance();

        shared_ptr<Node> expr;
        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER && peek().has_value() && peek()->type == T_EQ) {
            expr = res.register_node(statements());
        } else {
            expr = res.register_node(expression());
        }
        if (res.error) return res;
        cases.emplace_back(condition, expr);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
        }
        res.register_advancement();
        advance();

        while (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "orwhen") {
            res.register_advancement();
            advance();
            condition = res.register_node(expression());
            if (res.error) return res;

            if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '{'"));
            }
            res.register_advancement();
            advance();

            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER && peek().has_value() && peek()->type == T_EQ) {
                expr = res.register_node(statements());
            } else {
                expr = res.register_node(expression());
            }
            if (res.error) return res;
            cases.emplace_back(condition, expr);

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
            }
            res.register_advancement();
            advance();
        }

        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "otherwise") {
            res.register_advancement();
            advance();

            if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '{'"));
            }
            res.register_advancement();
            advance();

            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER && peek().has_value() && peek()->type == T_EQ) {
                else_case = res.register_node(statements());
            } else {
                else_case = res.register_node(expression());
            }
            if (res.error) return res;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
            }
            res.register_advancement();
            advance();
        }

        return res.success(make_shared<IfNode>(cases, else_case));
    }

    ParseResult statements() {
        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "define") {
            ParseResult res;
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
        return expression();
    }

    ParseResult expression() {
        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
            if (auto next_tok = peek(); next_tok.has_value() && next_tok->type == T_EQ) {
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
            if (auto keyword = any_cast<string>(current_tok->value); keyword == "and" || keyword == "or") {
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
            return res.failure(InvalidSyntaxError({}, {}, "Expected int, float, identifier, '+', '-', '(', or keyword"));
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
        else if (token.type == T_KEYWORD) {
             if(const auto keyword = any_cast<string>(token.value); keyword == "when") {
                const auto if_expr = res.register_node(if_expression());
                if (res.error) return res;
                return res.success(if_expr);
            } else if (keyword == "cycle") {
                const auto for_expr = res.register_node(for_expression());
                if (res.error) return res;
                return res.success(for_expr);
            } else if (keyword == "during") {
                const auto while_expr = res.register_node(while_expression());
                if (res.error) return res;
                return res.success(while_expr);
            }
        }

        return res.failure(InvalidSyntaxError(token.pos_start.value(), token.pos_end.value(), "Expected int, float, identifier, '+', '-', '(', or keyword"));
    }
};