#pragma once

#include <bits/stdc++.h>

#include "lexer.h"
#include "../ast_results/parse_result.h"
#include "../ast_nodes/operation_nodes.h"
#include "../ast_nodes/variable_nodes.h"
#include "../ast_nodes/if_else_elif_nodes.h"
#include "../ast_nodes/for_nodes.h"
#include "../ast_nodes/while_nodes.h"
#include "../ast_nodes/function_nodes.h"
#include "../ast_nodes/string_nodes.h"
#include "../ast_nodes/list_nodes.h"
#include "../ast_nodes/jump_nodes.h"
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
        ParseResult result = multiline();

        // Consume trailing NEWLINE tokens before EOF, matching Python's NEWLINE*
        while (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            result.register_advancement();
            advance();
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

    void update_current_tok() {
        if (tok_index >= 0 && tok_index < static_cast<int>(tokens.size())) {
            current_tok = make_optional(tokens[tok_index]);
        }
        else {
            current_tok = nullopt;
        }
    }

    void advance() {
        tok_index++;
        update_current_tok();
    }

    void reverse(int amount = 1) {
        tok_index -= amount;
        if (tok_index < -1) tok_index = -1;
        update_current_tok();
    }

    [[nodiscard]] optional<Token> peek() const {
        const int next_index = tok_index + 1;
        return (next_index < tokens.size()) ? make_optional(tokens[next_index]) : nullopt;
    }

    ParseResult multiline() {
        ParseResult res;
        vector<shared_ptr<Node>> statements_list;
        optional<Position> pos_start;

        if (current_tok.has_value()) {
            pos_start = current_tok->pos_start;
        }

        while (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement();
            advance();
        }

        if (!current_tok.has_value() || current_tok->type == T_EOF) {
            return res.failure(InvalidSyntaxError({}, {}, "Expected expression or statement"));
        }

        shared_ptr<Node> first_stmt;
        if (current_tok->type == T_IDENTIFIER) {
            auto next_tok = peek();
            if (next_tok.has_value() && next_tok->type == T_EQ) {
                first_stmt = res.register_node(statements());
            }
            else {
                first_stmt = res.register_node(expression());
            }
        }
        else {
            first_stmt = res.register_node(expression());
        }
        if (res.error) return res;
        statements_list.push_back(first_stmt);

        while (true) {
            int newline_count = 0;
            while (current_tok.has_value() && current_tok->type == T_NEWLINE) {
                res.register_advancement();
                advance();
                newline_count++;
            }
            if (newline_count == 0) break;

            ParseResult stmt_res;
            shared_ptr<Node> stmt_node;
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                auto next_tok = peek();
                if (next_tok.has_value() && next_tok->type == T_EQ) {
                    stmt_node = stmt_res.register_node(statements());
                }
                else {
                    stmt_node = stmt_res.register_node(expression());
                }
            }
            else {
                stmt_node = stmt_res.register_node(expression());
            }

            if (!stmt_res.error) {
                statements_list.push_back(stmt_node);
                res.register_node(stmt_res);
            }
            else if (!stmt_res.last_registered_advance_count) {
                reverse(stmt_res.to_reverse_count);
                break;
            }
            else {
                return res.failure(stmt_res.error.value());
            }
        }

        optional<Position> pos_end;
        if (!statements_list.empty()) {
            pos_end = statements_list.back()->pos_end;
        }

        return res.success(make_shared<ListNode>(statements_list, pos_start, pos_end));
    }

    ParseResult list_expression() {
        ParseResult res;
        vector<shared_ptr<Node>> element_nodes;
        optional<Position> pos_start;

        if (current_tok.has_value()) {
            pos_start = current_tok->pos_start;
        }
        else {
            return res.failure(InvalidSyntaxError({}, {}, "Expected '['"));
        }

        if (current_tok->type != T_LPAREN3) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '['"));
        }
        res.register_advancement();
        advance();

        optional<Position> pos_end;

        if (current_tok.has_value() && current_tok->type == T_RPAREN3) {
            pos_end = current_tok->pos_end;
            res.register_advancement();
            advance();
        }
        else {
            element_nodes.push_back(res.register_node(expression()));
            if (res.error) return res;

            while (current_tok.has_value() && current_tok->type == T_COMMA) {
                res.register_advancement();
                advance();
                element_nodes.push_back(res.register_node(expression()));
                if (res.error) return res;
            }

            if (!current_tok.has_value() || current_tok->type != T_RPAREN3) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected ',' or ']'"));
            }

            pos_end = current_tok->pos_end;
            res.register_advancement();
            advance();
        }

        return res.success(make_shared<ListNode>(element_nodes, pos_start, pos_end));
    }

    ParseResult function_definition() {
        ParseResult res;
        optional<Token> var_name_tok = nullopt;
        vector<Token> arg_name_toks;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "method")) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected 'method'"));
        }
        res.register_advancement();
        advance();

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
            var_name_tok = current_tok.value();
            res.register_advancement();
            advance();
        }

        if (!current_tok.has_value() || current_tok->type != T_LPAREN) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '('"));
        }
        res.register_advancement();
        advance();

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
            arg_name_toks.push_back(current_tok.value());
            res.register_advancement();
            advance();
            while (current_tok.has_value() && current_tok->type == T_COMMA) {
                res.register_advancement();
                advance();
                if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER) {
                    return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected identifier"));
                }
                arg_name_toks.push_back(current_tok.value());
                res.register_advancement();
                advance();
            }
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected ',' or ')'"));
        }
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '{'"));
        }
        res.register_advancement();
        advance();

        shared_ptr<Node> body_node;
        bool return_null = false;

        if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement();
            advance();
            body_node = res.register_node(multiline());
            if (res.error) return res;
            return_null = true;
        }
        else {
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                auto next_tok = peek();
                if (next_tok.has_value() && next_tok->type == T_EQ) {
                    body_node = res.register_node(statements());
                }
                else {
                    body_node = res.register_node(expression());
                }
            }
            else {
                body_node = res.register_node(expression());
            }
            if (res.error) return res;
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
        }
        res.register_advancement();
        advance();

        return res.success(make_shared<FunctionDefinitionNode>(var_name_tok, arg_name_toks, body_node, return_null));
    }

    ParseResult function_call() {
        ParseResult res;

        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected identifier"));
        }
        auto call_node = make_shared<VariableUseNode>(current_tok.value());
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_LPAREN) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '('"));
        }
        res.register_advancement();
        advance();

        vector<shared_ptr<Node>> arg_nodes;
        if (current_tok.has_value() && current_tok->type == T_RPAREN) {
            res.register_advancement();
            advance();
        }
        else {
            arg_nodes.push_back(res.register_node(expression()));
            if (res.error) return res;

            while (current_tok.has_value() && current_tok->type == T_COMMA) {
                res.register_advancement();
                advance();
                arg_nodes.push_back(res.register_node(expression()));
                if (res.error) return res;
            }

            if (!current_tok.has_value() || current_tok->type != T_RPAREN) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected ',' or ')'"));
            }
            res.register_advancement();
            advance();
        }

        return res.success(make_shared<FunctionCallNode>(call_node, arg_nodes));
    }

    ParseResult while_expression() {
        ParseResult res;
        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "during")) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected 'during'"));
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
        bool return_null = false;

        if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement();
            advance();
            body_node = res.register_node(multiline());
            if (res.error) return res;
            return_null = true;
        }
        else {
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                auto next_tok = peek();
                if (next_tok.has_value() && next_tok->type == T_EQ) {
                    body_node = res.register_node(statements());
                }
                else {
                    body_node = res.register_node(expression());
                }
            }
            else {
                body_node = res.register_node(expression());
            }
            if (res.error) return res;
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
        }
        res.register_advancement();
        advance();
        return res.success(make_shared<WhileNode>(condition, body_node, return_null));
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
        bool return_null = false;

        if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement();
            advance();
            body_node = res.register_node(multiline());
            if (res.error) return res;
            return_null = true;
        }
        else {
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                auto next_tok = peek();
                if (next_tok.has_value() && next_tok->type == T_EQ) {
                    body_node = res.register_node(statements());
                }
                else {
                    body_node = res.register_node(expression());
                }
            }
            else {
                body_node = res.register_node(expression());
            }
            if (res.error) return res;
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
        }
        res.register_advancement();
        advance();
        return res.success(make_shared<ForNode>(var_name, start_value, end_value, step_value, body_node, return_null));
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
        if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            // Multiline first when
            res.register_advancement();
            advance();

            auto statements_node = res.register_node(multiline());
            if (res.error) return res;
            cases.emplace_back(condition, statements_node);

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
            }
            res.register_advancement();
            advance();

            // Handle orwhen chain
            while (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "orwhen") {
                res.register_advancement();
                advance();

                auto or_condition = res.register_node(expression());
                if (res.error) return res;

                if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
                    return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '{'"));
                }
                res.register_advancement();
                advance();

                shared_ptr<Node> or_body;
                if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
                    res.register_advancement();
                    advance();
                    or_body = res.register_node(multiline());
                    if (res.error) return res;
                }
                else {
                    // single-line body: statements or expression
                    if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                        auto next_tok = peek();
                        if (next_tok.has_value() && next_tok->type == T_EQ) {
                            or_body = res.register_node(statements());
                        }
                        else {
                            or_body = res.register_node(expression());
                        }
                    }
                    else {
                        or_body = res.register_node(expression());
                    }
                    if (res.error) return res;
                }

                if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                    return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
                }
                res.register_advancement();
                advance();

                cases.emplace_back(or_condition, or_body);
            }

            // Optional otherwise after multiline chain
            if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "otherwise") {
                res.register_advancement();
                advance();

                if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
                    return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '{'"));
                }
                res.register_advancement();
                advance();

                if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
                    res.register_advancement();
                    advance();
                    else_case = res.register_node(multiline());
                    if (res.error) return res;
                }
                else {
                    if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                        auto next_tok = peek();
                        if (next_tok.has_value() && next_tok->type == T_EQ) {
                            else_case = res.register_node(statements());
                        }
                        else {
                            else_case = res.register_node(expression());
                        }
                    }
                    else {
                        else_case = res.register_node(expression());
                    }
                    if (res.error) return res;
                }

                if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                    return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
                }
                res.register_advancement();
                advance();
            }
        }
        else {
            // Single-line first when body
            shared_ptr<Node> expression_node;
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                auto next_tok = peek();
                if (next_tok.has_value() && next_tok->type == T_EQ) {
                    expression_node = res.register_node(statements());
                }
                else {
                    expression_node = res.register_node(expression());
                }
            }
            else {
                expression_node = res.register_node(expression());
            }
            if (res.error) return res;
            cases.emplace_back(condition, expression_node);

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
            }
            res.register_advancement();
            advance();

            // orwhen chain for single-line first when
            while (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "orwhen") {
                res.register_advancement();
                advance();

                auto or_condition = res.register_node(expression());
                if (res.error) return res;

                if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
                    return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '{'"));
                }
                res.register_advancement();
                advance();

                shared_ptr<Node> or_body;
                if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
                    res.register_advancement();
                    advance();
                    or_body = res.register_node(multiline());
                    if (res.error) return res;
                }
                else {
                    if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                        auto next_tok = peek();
                        if (next_tok.has_value() && next_tok->type == T_EQ) {
                            or_body = res.register_node(statements());
                        }
                        else {
                            or_body = res.register_node(expression());
                        }
                    }
                    else {
                        or_body = res.register_node(expression());
                    }
                    if (res.error) return res;
                }

                if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                    return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
                }
                res.register_advancement();
                advance();

                cases.emplace_back(or_condition, or_body);
            }

            // Optional otherwise after single-line chain
            if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "otherwise") {
                res.register_advancement();
                advance();

                if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
                    return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '{'"));
                }
                res.register_advancement();
                advance();

                if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
                    res.register_advancement();
                    advance();
                    else_case = res.register_node(multiline());
                    if (res.error) return res;
                }
                else {
                    if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                        auto next_tok = peek();
                        if (next_tok.has_value() && next_tok->type == T_EQ) {
                            else_case = res.register_node(statements());
                        }
                        else {
                            else_case = res.register_node(expression());
                        }
                    }
                    else {
                        else_case = res.register_node(expression());
                    }
                    if (res.error) return res;
                }

                if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                    return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
                }
                res.register_advancement();
                advance();
            }
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
            }
            else {
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
        auto left_node = res.register_node(unary());
        if (res.error) return res;

        while (current_tok.has_value()) {
            if (current_tok->type == T_MUL || current_tok->type == T_DIVIDE) {
                Token op_token = current_tok.value();
                res.register_advancement();
                advance();
                auto right_node = res.register_node(unary());
                if (res.error) return res;
                left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
            }
            else {
                break;
            }
        }
        return res.success(left_node);
    }

    ParseResult unary() {
        ParseResult res;
        if (current_tok.has_value() && (current_tok->type == T_PLUS || current_tok->type == T_MINUS)) {
            Token op_token = current_tok.value();
            res.register_advancement();
            advance();
            auto node = res.register_node(unary());
            if (res.error) return res;
            return res.success(make_shared<UnaryOperationNode>(op_token, node));
        }
        return power();
    }

    ParseResult power() {
        ParseResult res;
        auto left_node = res.register_node(factor());
        if (res.error) return res;

        while (current_tok.has_value() && current_tok->type == T_EXP) {
            Token op_token = current_tok.value();
            res.register_advancement();
            advance();
            auto right_node = res.register_node(unary());
            if (res.error) return res;
            left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
        }
        return res.success(left_node);
    }

    ParseResult factor() {
        ParseResult res;
        if (!current_tok.has_value()) {
            return res.failure(InvalidSyntaxError({}, {}, "Expected expression"));
        }
        Token token = current_tok.value();

        if (token.type == T_INT || token.type == T_FLOAT) {
            res.register_advancement();
            advance();
            return res.success(make_shared<NumberNode>(token));
        }
        else if (token.type == T_STRING) {
            res.register_advancement();
            advance();
            return res.success(make_shared<StringNode>(token));
        }
        else if (token.type == T_IDENTIFIER) {
            auto next_tok = peek();
            if (next_tok.has_value() && next_tok->type == T_LPAREN) {
                auto call_result = res.register_node(function_call());
                if (res.error) return res;
                return res.success(call_result);
            }

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
            }
            else {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected ')'"));
            }
        }
        else if (token.type == T_LPAREN3) {
            auto list_expr = res.register_node(list_expression());
            if (res.error) return res;
            return res.success(list_expr);
        }
        else if (token.type == T_KEYWORD) {
            if (const auto keyword = any_cast<string>(token.value); keyword == "when") {
                const auto if_expr = res.register_node(if_expression());
                if (res.error) return res;
                return res.success(if_expr);
            }
            else if (keyword == "cycle") {
                const auto for_expr = res.register_node(for_expression());
                if (res.error) return res;
                return res.success(for_expr);
            }
            else if (keyword == "during") {
                const auto while_expr = res.register_node(while_expression());

                if (res.error) return res;
                return res.success(while_expr);
            }
            else if (keyword == "method") {
                const auto func_def = res.register_node(function_definition());
                if (res.error) return res;
                return res.success(func_def);
            }
            else if (keyword == "yield") {
                res.register_advancement();
                advance();
                shared_ptr<Node> expr = nullptr;
                if (current_tok.has_value() && current_tok->type != T_NEWLINE && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF && current_tok->type != T_RPAREN) {
                    expr = res.register_node(expression());
                    if (res.error) return res;
                }
                return res.success(make_shared<ReturnNode>(expr, token.pos_start, current_tok.has_value() ? current_tok->pos_end : token.pos_end));
            }
            else if (keyword == "escape") {
                res.register_advancement();
                advance();
                return res.success(make_shared<BreakNode>(token.pos_start, token.pos_end));
            }
            else if (keyword == "proceed") {
                res.register_advancement();
                advance();
                return res.success(make_shared<ContinueNode>(token.pos_start, token.pos_end));
            }
        }

        return res.failure(InvalidSyntaxError(token.pos_start.value(), token.pos_end.value(), "Expected expression"));
    }
};