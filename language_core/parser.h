#pragma once

#include <bits/stdc++.h>

#include "lexer.h"
#include "../ast_results/parse_result.h"
#include "../ast_nodes/operation_nodes.h"
#include "../ast_nodes/variable_nodes.h"
#include "../ast_nodes/if_else_elif_nodes.h"
#include "../ast_nodes/switch_nodes.h"
#include "../ast_nodes/for_nodes.h"
#include "../ast_nodes/while_nodes.h"
#include "../ast_nodes/function_nodes.h"
#include "../ast_nodes/string_nodes.h"
#include "../ast_nodes/list_nodes.h"
#include "../ast_nodes/jump_nodes.h"
#include "error.h"
#include "constants.h"

using namespace std;

class ElifElseResultNode final : public Node {
public:
    vector<tuple<shared_ptr<Node>, shared_ptr<Node>, bool>> cases;
    optional<pair<shared_ptr<Node>, bool>> else_case;
    ElifElseResultNode(vector<tuple<shared_ptr<Node>, shared_ptr<Node>, bool>> cases, optional<pair<shared_ptr<Node>, bool>> else_case)
        : Node(Position(), Position()), cases(std::move(cases)), else_case(std::move(else_case)) {}
    [[nodiscard]] std::string to_string() const override { return ""; }
};

class Parser {
public:
    explicit Parser(vector<Token> tokens)
        : tokens(std::move(tokens)), tok_index(-1), current_tok(nullopt), call(0) {
        advance();
    }

    ParseResult parse() {
        ParseResult result = multiline();
        if (!result.error && current_tok.has_value() && current_tok->type != T_EOF) {
            return result.failure(
                InvalidSyntaxError(
                    current_tok->pos_start.value(), current_tok->pos_end.value(),
                    "Expected '+', '-', '*', '/'"
                )
            );
        }
        return result;
    }

private:
    vector<Token> tokens;
    int tok_index;
    optional<Token> current_tok;
    int call;

    void update_current_tok() {
        if (tok_index >= 0 && tok_index < static_cast<int>(tokens.size())) {
            current_tok = make_optional(tokens[tok_index]);
        } else {
            current_tok = nullopt;
        }
    }

    optional<Token> advance() {
        tok_index++;
        update_current_tok();
        return current_tok;
    }

    optional<Token> reverse(const int amount = 1) {
        tok_index -= amount;
        update_current_tok();
        return current_tok;
    }

    [[nodiscard]] optional<Token> peek() const {
        const int next_index = tok_index + 1;
        return (next_index < static_cast<int>(tokens.size())) ? make_optional(tokens[next_index]) : nullopt;
    }

    bool check_is_statement() {
        int current_tok_index = tok_index;
        bool is_statement = false;
        while (current_tok.has_value() && current_tok->type != T_NEWLINE && current_tok->type != T_EOF) {
            if (current_tok->type == T_EQ) {
                is_statement = true;
                break;
            }
            advance();
        }
        tok_index = current_tok_index;
        update_current_tok();
        return is_statement;
    }

    ParseResult multiline() {
        ParseResult res;
        vector<shared_ptr<Node>> statements_list;
        optional<Position> pos_start;

        if (current_tok.has_value()) pos_start = current_tok->pos_start;

        while (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement(); advance();
        }

        if (current_tok.has_value() && current_tok->type != T_EOF && current_tok->type != T_RPAREN2) {
            auto first_stmt = res.register_node(singleline());
            if (res.error) return res;
            statements_list.push_back(first_stmt);
        }

        while (true) {
            while (current_tok.has_value() && current_tok->type == T_NEWLINE) {
                res.register_advancement(); advance();
            }
            if (current_tok.has_value() && current_tok->type != T_EOF && current_tok->type != T_RPAREN2) {
                auto stmt = res.register_node(singleline());
                if (res.error) return res;
                statements_list.push_back(stmt);
            } else {
                break;
            }
        }

        while (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement(); advance();
        }

        optional<Position> pos_end;
        if (current_tok.has_value()) pos_end = current_tok->pos_end;

        return res.success(make_shared<ListNode>(statements_list, pos_start, pos_end));
    }

    ParseResult singleline() {
        ParseResult res;
        if (!current_tok.has_value()) return res;
        Token token = current_tok.value();

        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "when") {
            auto if_expr = res.register_node(if_expression());
            if (res.error) return res;
            return res.success(if_expr);
        }
        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "cycle") {
            auto for_expr = res.register_node(for_expression());
            if (res.error) return res;
            return res.success(for_expr);
        }
        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "during") {
            auto while_expr = res.register_node(while_expression());
            if (res.error) return res;
            return res.success(while_expr);
        }
        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "method") {
            auto method_expr = res.register_node(function_definition());
            if (res.error) return res;
            return res.success(method_expr);
        }
        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "menu") {
            auto switch_stmt = res.register_node(switch_statement());
            if (res.error) return res;
            return res.success(switch_stmt);
        }
        if (token.type == T_IDENTIFIER && peek().has_value() && peek()->type == T_LPAREN) {
            auto call_node = res.register_node(function_call());
            if (res.error) return res;
            return res.success(call_node);
        }

        auto statement_node = res.register_node(statements());
        if (res.error) return res;

        auto node = res.register_node(res.success(statement_node));
        if (res.error) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected int,float,identifier"
            ));
        }
        return res.success(node);
    }

    ParseResult list_expression() {
        ParseResult res;
        vector<shared_ptr<Node>> element_nodes;
        optional<Position> pos_start;

        if (current_tok.has_value()) pos_start = current_tok->pos_start;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN3) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '['"
            ));
        }

        res.register_advancement(); advance();

        optional<Position> pos_end;

        if (current_tok.has_value() && current_tok->type == T_RPAREN3) {
            pos_end = current_tok->pos_end;
            res.register_advancement(); advance();
        } else {
            element_nodes.push_back(res.register_node(expression()));
            if (res.error) return res;

            while (current_tok.has_value() && current_tok->type == T_COMMA) {
                res.register_advancement(); advance();
                element_nodes.push_back(res.register_node(expression()));
                if (res.error) return res;
            }

            if (!current_tok.has_value() || current_tok->type != T_RPAREN3) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected ',' or ']'"
                ));
            }

            pos_end = current_tok->pos_end;
            res.register_advancement(); advance();
        }

        return res.success(make_shared<ListNode>(element_nodes, pos_start, pos_end));
    }

    ParseResult function_definition() {
        ParseResult res;
        optional<Token> var_name_tok = nullopt;
        vector<Token> arg_name_toks;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "method")) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'method'"
            ));
        }
        res.register_advancement(); advance();

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
            var_name_tok = current_tok.value();
            res.register_advancement(); advance();
            if (!current_tok.has_value() || current_tok->type != T_LPAREN) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '('"
                ));
            }
        } else {
            if (!current_tok.has_value() || current_tok->type != T_LPAREN) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '('"
                ));
            }
        }

        res.register_advancement(); advance();

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
            arg_name_toks.push_back(current_tok.value());
            res.register_advancement(); advance();

            while (current_tok.has_value() && current_tok->type == T_COMMA) {
                res.register_advancement(); advance();
                if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER) {
                    return res.failure(InvalidSyntaxError(
                        current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                        current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                        "Expected identifier"
                    ));
                }
                arg_name_toks.push_back(current_tok.value());
                res.register_advancement(); advance();
            }

            if (!current_tok.has_value() || current_tok->type != T_RPAREN) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected ',' or ')'"
                ));
            }
        } else {
            if (!current_tok.has_value() || current_tok->type != T_RPAREN) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected identifier or ')'"
                ));
            }
        }

        res.register_advancement(); advance();

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"
            ));
        }
        res.register_advancement(); advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF) {
            if (current_tok->type == T_KEYWORD &&
               (any_cast<string>(current_tok->value) == "escape" ||
                any_cast<string>(current_tok->value) == "proceed" ||
                any_cast<string>(current_tok->value) == "yield")) {
                auto jump_node = res.register_node(jump_statements());
                if (res.error) return res;
                body_nodes.push_back(jump_node);
            } else {
                auto multiline_node = res.try_register(multiline());
                if (res.error) return res;
                if (!multiline_node) {
                    bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                   (any_cast<string>(current_tok->value) == "escape" ||
                                    any_cast<string>(current_tok->value) == "proceed" ||
                                    any_cast<string>(current_tok->value) == "yield");
                    if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2) {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected identifier,when,during,method or cycle"
                        ));
                    }
                }
                if (multiline_node) {
                    if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node)) {
                        body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                    }
                }
            }
        }

        auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"
            ));
        }
        res.register_advancement(); advance();

        return res.success(make_shared<FunctionDefinitionNode>(var_name_tok, arg_name_toks, body_node, false));
    }

    ParseResult function_call() {
        ParseResult res;
        vector<shared_ptr<Node>> arg_nodes;
        shared_ptr<Node> call_node;

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
            call_node = res.register_node(res.success(make_shared<VariableUseNode>(current_tok.value())));
            if (res.error) return res;
            res.register_advancement(); advance();
        } else {
            res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected identifier"
            ));
        }

        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '('"
            ));
        }

        res.register_advancement(); advance();

        if (current_tok.has_value() && current_tok->type == T_RPAREN) {
            res.register_advancement(); advance();
        } else {
            arg_nodes.push_back(res.register_node(expression()));
            if (res.error) return res;

            while (current_tok.has_value() && current_tok->type == T_COMMA) {
                res.register_advancement(); advance();
                arg_nodes.push_back(res.register_node(expression()));
                if (res.error) return res;
            }

            if (!current_tok.has_value() || current_tok->type != T_RPAREN) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected ',' or ')'"
                ));
            }

            res.register_advancement(); advance();
        }

        return res.success(make_shared<FunctionCallNode>(call_node, arg_nodes));
    }

    ParseResult switch_statement() {
        ParseResult res;
        vector<shared_ptr<SwitchCaseNode>> cases;

        if (!current_tok.has_value() || current_tok->type != T_KEYWORD || any_cast<string>(current_tok->value) != "menu") {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'menu'"
            ));
        }

        res.register_advancement(); advance();

        auto selection = res.register_node(ternary_expression());
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"
            ));
        }

        res.register_advancement(); advance();

        while (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement(); advance();
        }

        bool found_default = false;
        int count = 0;

        while (current_tok.has_value() && current_tok->type == T_KEYWORD &&
              (any_cast<string>(current_tok->value) == "choice" || any_cast<string>(current_tok->value) == "fallback")) {

            if (any_cast<string>(current_tok->value) == "choice") {
                auto case_node = res.register_node(case_statement());
                if (res.error) return res;
                cases.push_back(dynamic_pointer_cast<SwitchCaseNode>(case_node));
                while (current_tok.has_value() && current_tok->type == T_NEWLINE) {
                    res.register_advancement(); advance();
                }
            } else {
                if (found_default) {
                    return res.failure(InvalidSyntaxError(
                        current_tok->pos_start.value_or(Position()),
                        current_tok->pos_end.value_or(Position()),
                        "Multiple 'fallback' statements found"
                    ));
                }
                found_default = true;
                auto default_node = res.register_node(default_statement());
                if (res.error) return res;
                cases.push_back(dynamic_pointer_cast<SwitchCaseNode>(default_node));
                while (current_tok.has_value() && current_tok->type == T_NEWLINE) {
                    res.register_advancement(); advance();
                }
            }
            count++;
        }

        if (count == 0) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'choice' or 'fallback'"
            ));
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"
            ));
        }

        res.register_advancement(); advance();

        return res.success(make_shared<SwitchNode>(selection, cases, false));
    }

    ParseResult case_statement() {
        ParseResult res;

        if (!current_tok.has_value() || current_tok->type != T_KEYWORD || any_cast<string>(current_tok->value) != "choice") {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'choice'"
            ));
        }
        res.register_advancement(); advance();

        auto choice_val = res.register_node(ternary_expression());
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"
            ));
        }

        res.register_advancement(); advance();

        shared_ptr<Node> body_node;
        bool is_multiline = false;

        if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement(); advance();

            body_node = res.register_node(multiline());
            if (res.error) return res;

            is_multiline = true;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '}'"
                ));
            }
            res.register_advancement(); advance();
        } else {
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER && peek().has_value() && peek()->type == T_EQ) {
                body_node = res.register_node(statements());
            } else {
                body_node = res.register_node(expression());
            }
            if (res.error) return res;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '}'"
                ));
            }
            res.register_advancement(); advance();
        }

        return res.success(make_shared<SwitchCaseNode>(choice_val, body_node, is_multiline));
    }

    ParseResult default_statement() {
        ParseResult res;

        if (!current_tok.has_value() || current_tok->type != T_KEYWORD || any_cast<string>(current_tok->value) != "fallback") {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'fallback'"
            ));
        }
        res.register_advancement(); advance();

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"
            ));
        }

        res.register_advancement(); advance();

        shared_ptr<Node> body_node;
        bool is_multiline = false;

        if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement(); advance();

            body_node = res.register_node(multiline());
            if (res.error) return res;

            is_multiline = true;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '}'"
                ));
            }
            res.register_advancement(); advance();
        } else {
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER && peek().has_value() && peek()->type == T_EQ) {
                body_node = res.register_node(statements());
            } else {
                body_node = res.register_node(expression());
            }
            if (res.error) return res;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '}'"
                ));
            }
            res.register_advancement(); advance();
        }

        return res.success(make_shared<SwitchCaseNode>(nullptr, body_node, is_multiline));
    }

    ParseResult while_expression() {
        ParseResult res;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "during")) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'during'"
            ));
        }
        res.register_advancement(); advance();

        auto condition = res.register_node(expression());
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"
            ));
        }
        res.register_advancement(); advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF) {
            if (current_tok->type == T_KEYWORD &&
               (any_cast<string>(current_tok->value) == "escape" ||
                any_cast<string>(current_tok->value) == "proceed" ||
                any_cast<string>(current_tok->value) == "yield")) {
                auto jump_node = res.register_node(jump_statements());
                if (res.error) return res;
                body_nodes.push_back(jump_node);
            } else {
                auto multiline_node = res.try_register(multiline());
                if (res.error) return res;
                if (!multiline_node) {
                    bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                   (any_cast<string>(current_tok->value) == "escape" ||
                                    any_cast<string>(current_tok->value) == "proceed" ||
                                    any_cast<string>(current_tok->value) == "yield");
                    if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2) {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected identifier,when,during,method or cycle"
                        ));
                    }
                }
                if (multiline_node) {
                    if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node)) {
                        body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                    }
                }
            }
        }

        auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"
            ));
        }
        res.register_advancement(); advance();

        return res.success(make_shared<WhileNode>(condition, body_node, false));
    }

    ParseResult for_expression() {
        ParseResult res;
        shared_ptr<Node> step_value = nullptr;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "cycle")) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'cycle'"
            ));
        }
        res.register_advancement(); advance();

        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected identifier"
            ));
        }
        Token var_name = current_tok.value();
        res.register_advancement(); advance();

        if (!current_tok.has_value() || current_tok->type != T_EQ) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '='"
            ));
        }
        res.register_advancement(); advance();

        auto start_value = res.register_node(expression());
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_COLON) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected ':'"
            ));
        }
        res.register_advancement(); advance();

        auto end_value = res.register_node(expression());
        if (res.error) return res;

        if (current_tok.has_value() && current_tok->type == T_COLON) {
            res.register_advancement(); advance();
            step_value = res.register_node(expression());
            if (res.error) return res;
        }

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"
            ));
        }
        res.register_advancement(); advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF) {
            if (current_tok->type == T_KEYWORD &&
               (any_cast<string>(current_tok->value) == "escape" ||
                any_cast<string>(current_tok->value) == "proceed" ||
                any_cast<string>(current_tok->value) == "yield")) {
                auto jump_node = res.register_node(jump_statements());
                if (res.error) return res;
                body_nodes.push_back(jump_node);
            } else {
                auto multiline_node = res.try_register(multiline());
                if (res.error) return res;
                if (!multiline_node) {
                    bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                   (any_cast<string>(current_tok->value) == "escape" ||
                                    any_cast<string>(current_tok->value) == "proceed" ||
                                    any_cast<string>(current_tok->value) == "yield");
                    if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2) {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected identifier,when,during,method or cycle"
                        ));
                    }
                }
                if (multiline_node) {
                    if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node)) {
                        body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                    }
                }
            }
        }

        auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"
            ));
        }
        res.register_advancement(); advance();

        return res.success(make_shared<ForNode>(var_name, start_value, end_value, step_value, body_node, false));
    }

    ParseResult if_expression() {
        ParseResult res;
        vector<tuple<shared_ptr<Node>, shared_ptr<Node>, bool>> cases;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "when")) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'when'"
            ));
        }

        res.register_advancement(); advance();

        auto condition = res.register_node(expression());
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"
            ));
        }
        res.register_advancement(); advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF) {
            if (current_tok->type == T_KEYWORD &&
               (any_cast<string>(current_tok->value) == "escape" ||
                any_cast<string>(current_tok->value) == "proceed" ||
                any_cast<string>(current_tok->value) == "yield")) {
                auto jump_node = res.register_node(jump_statements());
                if (res.error) return res;
                body_nodes.push_back(jump_node);
            } else {
                auto multiline_node = res.try_register(multiline());
                if (res.error) return res;
                if (!multiline_node) {
                    bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                   (any_cast<string>(current_tok->value) == "escape" ||
                                    any_cast<string>(current_tok->value) == "proceed" ||
                                    any_cast<string>(current_tok->value) == "yield");
                    if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2) {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected identifier,when,during,method or cycle"
                        ));
                    }
                }
                if (multiline_node) {
                    if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node)) {
                        body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                    }
                }
            }
        }

        auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"
            ));
        }
        res.register_advancement(); advance();

        cases.emplace_back(condition, body_node, true);

        auto all_cases_node = res.register_node(elif_or_else_expression());
        if (res.error) return res;

        optional<pair<shared_ptr<Node>, bool>> else_case = nullopt;
        if (auto elif_result = dynamic_pointer_cast<ElifElseResultNode>(all_cases_node)) {
            cases.insert(cases.end(), elif_result->cases.begin(), elif_result->cases.end());
            else_case = elif_result->else_case;
        }

        return res.success(make_shared<IfNode>(cases, else_case));
    }

    ParseResult elif_or_else_expression() {
        ParseResult res;
        vector<tuple<shared_ptr<Node>, shared_ptr<Node>, bool>> cases;
        optional<pair<shared_ptr<Node>, bool>> else_case = nullopt;

        while (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement(); advance();
        }

        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "orwhen") {
            auto all_cases_node = res.register_node(elif_expression());
            if (res.error) return res;
            if (auto elif_result = dynamic_pointer_cast<ElifElseResultNode>(all_cases_node)) {
                cases = elif_result->cases;
                else_case = elif_result->else_case;
            }
        } else if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "otherwise") {
            auto else_node = res.register_node(else_expression());
            if (res.error) return res;
            if (else_node) {
                else_case = make_pair(else_node, true);
            }
        }

        return res.success(make_shared<ElifElseResultNode>(cases, else_case));
    }

    ParseResult elif_expression() {
        ParseResult res;
        vector<tuple<shared_ptr<Node>, shared_ptr<Node>, bool>> cases;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "orwhen")) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'orwhen'"
            ));
        }

        res.register_advancement(); advance();

        auto condition = res.register_node(expression());
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"
            ));
        }
        res.register_advancement(); advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF) {
            if (current_tok->type == T_KEYWORD &&
               (any_cast<string>(current_tok->value) == "escape" ||
                any_cast<string>(current_tok->value) == "proceed" ||
                any_cast<string>(current_tok->value) == "yield")) {
                auto jump_node = res.register_node(jump_statements());
                if (res.error) return res;
                body_nodes.push_back(jump_node);
            } else {
                auto multiline_node = res.try_register(multiline());
                if (res.error) return res;
                if (!multiline_node) {
                    bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                   (any_cast<string>(current_tok->value) == "escape" ||
                                    any_cast<string>(current_tok->value) == "proceed" ||
                                    any_cast<string>(current_tok->value) == "yield");
                    if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2) {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected identifier,when,during,method or cycle"
                        ));
                    }
                }
                if (multiline_node) {
                    if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node)) {
                        body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                    }
                }
            }
        }

        auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"
            ));
        }
        res.register_advancement(); advance();

        cases.emplace_back(condition, body_node, true);

        auto all_cases_node = res.register_node(elif_or_else_expression());
        if (res.error) return res;

        optional<pair<shared_ptr<Node>, bool>> else_case = nullopt;
        if (auto elif_result = dynamic_pointer_cast<ElifElseResultNode>(all_cases_node)) {
            cases.insert(cases.end(), elif_result->cases.begin(), elif_result->cases.end());
            else_case = elif_result->else_case;
        }

        return res.success(make_shared<ElifElseResultNode>(cases, else_case));
    }

    ParseResult else_expression() {
        ParseResult res;

        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "otherwise") {
            res.register_advancement(); advance();

            if (!current_tok.has_value() || current_tok->type != T_LPAREN2) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '{'"
                ));
            }
            res.register_advancement(); advance();

            vector<shared_ptr<Node>> body_nodes;
            optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

            while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF) {
                if (current_tok->type == T_KEYWORD &&
                   (any_cast<string>(current_tok->value) == "escape" ||
                    any_cast<string>(current_tok->value) == "proceed" ||
                    any_cast<string>(current_tok->value) == "yield")) {
                    auto jump_node = res.register_node(jump_statements());
                    if (res.error) return res;
                    body_nodes.push_back(jump_node);
                } else {
                    auto multiline_node = res.try_register(multiline());
                    if (res.error) return res;
                    if (!multiline_node) {
                        bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                       (any_cast<string>(current_tok->value) == "escape" ||
                                        any_cast<string>(current_tok->value) == "proceed" ||
                                        any_cast<string>(current_tok->value) == "yield");
                        if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2) {
                            return res.failure(InvalidSyntaxError(
                                current_tok->pos_start.value_or(Position()),
                                current_tok->pos_end.value_or(Position()),
                                "Expected identifier,when,during,method or cycle"
                            ));
                        }
                    }
                    if (multiline_node) {
                        if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node)) {
                            body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                        }
                    }
                }
            }

            auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '}'"
                ));
            }
            res.register_advancement(); advance();

            return res.success(body_node);
        }

        return res.success(nullptr);
    }

    ParseResult unary() {
        ParseResult res;
        if (!current_tok.has_value()) return res;
        Token token = current_tok.value();

        if (token.type == T_PLUS || token.type == T_MINUS) {
            res.register_advancement(); advance();
            auto factor_node = res.register_node(unary());
            if (res.error) return res;
            return res.success(make_shared<UnaryOperationNode>(token, factor_node));
        }

        return exponent();
    }

    ParseResult exponent() {
        ParseResult res;
        auto left_node = res.register_node(factor());
        if (res.error) return res;

        while (current_tok.has_value() && current_tok->type == T_EXP) {
            Token op_token = current_tok.value();
            res.register_advancement(); advance();
            auto right_node = res.register_node(unary());
            if (res.error) return res;
            left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
        }

        auto node = res.register_node(res.success(left_node));
        if (res.error) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'define',int,float,identifier,'+','-' or '('"
            ));
        }
        return res.success(node);
    }

    ParseResult ternary_expression() {
        ParseResult res;
        shared_ptr<Node> comp_node;

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER && peek().has_value() && peek()->type == T_EQ) {
            comp_node = res.register_node(statements());
        } else {
            comp_node = res.register_node(logical_expression());
        }

        if (res.error) return res;
        shared_ptr<Node> true_node = nullptr, false_node = nullptr;

        while (current_tok.has_value() && current_tok->type == T_QUESTION) {
            res.register_advancement(); advance();
            true_node = res.register_node(ternary_expression());
            if (res.error) return res;

            if (current_tok.has_value() && current_tok->type != T_COLON) {
                return res.failure(InvalidSyntaxError(
                    current_tok->pos_start.value_or(Position()),
                    current_tok->pos_end.value_or(Position()),
                    "Expected ':' "
                ));
            }

            res.register_advancement(); advance();
            false_node = res.register_node(ternary_expression());
            if (res.error) return res;
        }

        if (!true_node || !false_node) {
            return res.success(comp_node);
        }

        return res.success(make_shared<TernaryOperationNode>(comp_node, true_node, false_node));
    }

    ParseResult factor() {
        ParseResult res;
        if (!current_tok.has_value()) return res;
        Token token = current_tok.value();

        if (token.type == T_INT || token.type == T_FLOAT) {
            res.register_advancement(); advance();
            return res.success(make_shared<NumberNode>(token));
        }

        if (token.type == T_STRING) {
            res.register_advancement(); advance();
            return res.success(make_shared<StringNode>(token));
        }

        if (token.type == T_IDENTIFIER && peek().has_value() && peek()->type == T_LPAREN) {
            auto call_expression = res.register_node(function_call());
            if (res.error) return res;
            return res.success(call_expression);
        }

        if (token.type == T_IDENTIFIER) {
            res.register_advancement(); advance();
            Token var_name_tok = token;
            vector<shared_ptr<Node>> index_node;

            if (current_tok.has_value() && current_tok->type == T_LPAREN3) {
                while (current_tok.has_value() && current_tok->type == T_LPAREN3) {
                    res.register_advancement(); advance();
                    auto expr = res.register_node(expression());
                    if (res.error) return res;
                    index_node.push_back(expr);

                    if (!current_tok.has_value() || current_tok->type != T_RPAREN3) {
                        return res.failure(InvalidSyntaxError(
                            current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                            current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                            "Expected ']'"
                        ));
                    }
                    res.register_advancement(); advance();
                }
            }
            return res.success(make_shared<VariableUseNode>(var_name_tok, index_node));
        }

        if (token.type == T_LPAREN) {
            res.register_advancement(); advance();
            auto expr = res.register_node(expression());
            if (res.error) return res;
            if (!current_tok.has_value() || current_tok->type != T_RPAREN) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected ')'"
                ));
            }
            res.register_advancement(); advance();
            return res.success(expr);
        }

        if (token.type == T_LPAREN3) {
            auto list_expr = res.register_node(list_expression());
            if (res.error) return res;
            return res.success(list_expr);
        }

        return res.failure(InvalidSyntaxError(
            token.pos_start.value_or(Position()),
            token.pos_end.value_or(Position()),
            "Expected int, float,identifier,'+','-'or '('"
        ));
    }

    ParseResult term() {
        ParseResult res;

        auto left_node = res.register_node(unary());
        if (res.error) return res;

        while (current_tok.has_value() && (current_tok->type == T_MUL || current_tok->type == T_DIVIDE ||
               current_tok->type == T_MODULUS || current_tok->type == T_FLOOR)) {
            Token op_token = current_tok.value();
            res.register_advancement(); advance();
            auto right_node = res.register_node(unary());
            if (res.error) return res;
            left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
        }

        return res.success(left_node);
    }

    ParseResult statements() {
        ParseResult res;

        vector<Token> var_name_toks;
        vector<vector<shared_ptr<Node>>> index_nodes;
        vector<shared_ptr<Node>> value_nodes;

        while (true) {
            if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER) {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected identifier"
                ));
            }

            var_name_toks.push_back(current_tok.value());
            res.register_advancement(); advance();

            vector<shared_ptr<Node>> indices;
            while (current_tok.has_value() && current_tok->type == T_LPAREN3) {
                res.register_advancement(); advance();

                auto expr = res.register_node(expression());
                if (res.error) return res;
                indices.push_back(expr);

                if (!current_tok.has_value() || current_tok->type != T_RPAREN3) {
                    return res.failure(InvalidSyntaxError(
                        current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                        current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                        "Expected ')'"
                    ));
                }
                res.register_advancement(); advance();
            }

            index_nodes.push_back(indices.empty() ? vector<shared_ptr<Node>>() : indices);

            if (!current_tok.has_value() || current_tok->type != T_COMMA) {
                break;
            }
            res.register_advancement(); advance();
        }

        if (!current_tok.has_value() || current_tok->type != T_EQ) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '='"
            ));
        }
        res.register_advancement(); advance();

        while (true) {
            auto expr = res.register_node(expression());
            if (res.error) return res;
            value_nodes.push_back(expr);

            if (!current_tok.has_value() || current_tok->type != T_COMMA) {
                break;
            }
            res.register_advancement(); advance();
        }

        if (var_name_toks.size() != value_nodes.size()) {
            return res.failure(InvalidSyntaxError(
                var_name_toks.front().pos_start.value_or(Position()),
                value_nodes.back()->pos_end.value_or(Position()),
                "Mismatched assignment count: " + to_string(var_name_toks.size()) +
                " variables, " + to_string(value_nodes.size()) + " values"
            ));
        }

        return res.success(make_shared<VariableAssignNode>(var_name_toks, value_nodes, index_nodes));
    }

    ParseResult jump_statements() {
        ParseResult res;

        if (!current_tok.has_value() || current_tok->type != T_KEYWORD ||
            (any_cast<string>(current_tok->value) != "proceed" &&
             any_cast<string>(current_tok->value) != "escape" &&
             any_cast<string>(current_tok->value) != "yield")) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'proceed' or 'escape' or 'yield' "
            ));
        }

        if (any_cast<string>(current_tok->value) == "proceed") {
            auto start_pos = current_tok->pos_start.value_or(Position());
            res.register_advancement(); advance();
            return res.success(make_shared<ContinueNode>(start_pos, start_pos));
        }

        if (any_cast<string>(current_tok->value) == "escape") {
            auto start_pos = current_tok->pos_start.value_or(Position());
            res.register_advancement(); advance();
            return res.success(make_shared<BreakNode>(start_pos, start_pos));
        }

        if (any_cast<string>(current_tok->value) == "yield") {
            auto start_pos = current_tok->pos_start.value_or(Position());
            res.register_advancement(); advance();

            auto expr = res.try_register(expression());
            if (!expr) {
                reverse(res.to_reverse_count);
            }

            return res.success(make_shared<ReturnNode>(expr, start_pos,
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : start_pos));
        }

        return res;
    }

    ParseResult expression() {
        ParseResult res;

        auto ternary_node = res.register_node(ternary_expression());
        if (res.error) return res;

        auto node = res.register_node(res.success(ternary_node));
        if (res.error) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected int,float,identifier"
            ));
        }
        return res.success(node);
    }

    ParseResult logical_expression() {
        ParseResult res;
        auto left_node = res.register_node(comp_expression());
        if (res.error) return res;

        while (current_tok.has_value() && current_tok->type == T_KEYWORD &&
              (any_cast<string>(current_tok->value) == "and" || any_cast<string>(current_tok->value) == "or")) {
            Token operator_tok = current_tok.value();
            res.register_advancement(); advance();

            auto right_node = res.register_node(comp_expression());
            if (res.error) return res;

            left_node = make_shared<BinaryOperationNode>(left_node, operator_tok, right_node);
        }

        auto node = res.register_node(res.success(left_node));
        if (res.error) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected int,float,identifier"
            ));
        }
        return res.success(node);
    }

    ParseResult comp_expression() {
        ParseResult res;

        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "not") {
            Token operator_token = current_tok.value();
            res.register_advancement(); advance();

            auto node = res.register_node(comp_expression());
            if (res.error) return res;
            return res.success(make_shared<UnaryOperationNode>(operator_token, node));
        }

        auto left_node = res.register_node(arith_expression());
        if (res.error) return res;

        while (current_tok.has_value() && (current_tok->type == T_EE || current_tok->type == T_NEQ ||
               current_tok->type == T_LT || current_tok->type == T_GT ||
               current_tok->type == T_GTE || current_tok->type == T_LTE)) {
            Token operator_tok = current_tok.value();
            res.register_advancement(); advance();
            auto right_node = res.register_node(arith_expression());
            if (res.error) return res;
            left_node = make_shared<BinaryOperationNode>(left_node, operator_tok, right_node);
        }

        auto node = res.register_node(res.success(left_node));
        if (res.error) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected int,float,identifier,'+','-','not' or '('"
            ));
        }
        return res.success(node);
    }

    ParseResult arith_expression() {
        ParseResult res;
        auto left_node = res.register_node(term());
        if (res.error) return res;

        while (current_tok.has_value() && (current_tok->type == T_PLUS || current_tok->type == T_MINUS)) {
            Token op_token = current_tok.value();
            res.register_advancement(); advance();
            auto right_node = res.register_node(term());
            if (res.error) return res;
            left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
        }

        auto node = res.register_node(res.success(left_node));
        if (res.error) {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'define',int,float,identifier,'+','-' or '('"
            ));
        }
        return res.success(node);
    }
};