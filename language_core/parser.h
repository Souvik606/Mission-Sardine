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

        shared_ptr<Node> statement;
        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
            auto next_tok = peek();
            if (next_tok.has_value() && next_tok->type == T_EQ) {
                statement = res.register_node(statements());
            } else {
                statement = res.register_node(expression());
            }
        } else {
            statement = res.register_node(expression());
        }

        if (res.error) return res;
        statements_list.push_back(statement);

        bool more_statements = true;

        while (true) {
            int newline_count = 0;
            while (current_tok.has_value() && current_tok->type == T_NEWLINE) {
                res.register_advancement();
                advance();
                newline_count++;
            }
            if (newline_count == 0) {
                more_statements = false;
            }

            if (!more_statements) break;

            ParseResult stmt_res;
            shared_ptr<Node> stmt_node;

            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                auto next_tok = peek();
                if (next_tok.has_value() && next_tok->type == T_EQ) {
                    stmt_node = stmt_res.register_node(statements());
                } else {
                    stmt_node = stmt_res.register_node(expression());
                }
            } else {
                stmt_node = stmt_res.register_node(expression());
            }

            if (!stmt_res.error) {
                statements_list.push_back(stmt_node);
                res.register_node(stmt_res);
            } else {
                if (stmt_res.last_registered_advance_count > 0) {
                     reverse(stmt_res.to_reverse_count);
                }
                more_statements = false;
                continue;
            }
        }

        optional<Position> pos_end;
        if (current_tok.has_value()) pos_end = current_tok->pos_end;
        else if (!statements_list.empty()) pos_end = statements_list.back()->pos_end;

        return res.success(make_shared<ListNode>(statements_list, pos_start, pos_end));
    }

    ParseResult list_expression() {
        ParseResult res;
        vector<shared_ptr<Node>> element_nodes;
        optional<Position> pos_start;

        if (current_tok.has_value()) {
            pos_start = current_tok->pos_start;
        }

        if (!current_tok.has_value() || current_tok->type != T_LPAREN3) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '['"));
        }
        res.register_advancement();
        advance();

        optional<Position> pos_end;

        if (current_tok.has_value() && current_tok->type == T_RPAREN3) {
            pos_end = current_tok->pos_end;
            res.register_advancement();
            advance();
        } else {
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
            if (!current_tok.has_value() || current_tok->type != T_LPAREN) {
                 return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '('"));
            }
        } else {
            if (!current_tok.has_value() || current_tok->type != T_LPAREN) {
                 return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '('"));
            }
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

        if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement();
            advance();
            body_node = res.register_node(multiline());
            if (res.error) return res;

             if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
            }
            res.register_advancement();
            advance();
            return res.success(make_shared<FunctionDefinitionNode>(var_name_tok, arg_name_toks, body_node, true));
        }
        else {
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                auto next_tok = peek();
                if (next_tok.has_value() && next_tok->type == T_EQ) {
                    body_node = res.register_node(statements());
                } else {
                    body_node = res.register_node(expression());
                }
            } else {
                body_node = res.register_node(expression());
            }
            if (res.error) return res;

             if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
            }
            res.register_advancement();
            advance();
            return res.success(make_shared<FunctionDefinitionNode>(var_name_tok, arg_name_toks, body_node, false));
        }
    }

    ParseResult function_call(shared_ptr<Node> node_to_call) {
        ParseResult res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '('"));
        }
        res.register_advancement();
        advance();

        vector<shared_ptr<Node>> arg_nodes;
        if (current_tok.has_value() && current_tok->type == T_RPAREN) {
            res.register_advancement();
            advance();
        } else {
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
        return res.success(make_shared<FunctionCallNode>(node_to_call, arg_nodes));
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

        if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement();
            advance();
            body_node = res.register_node(multiline());
            if (res.error) return res;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
            }
            res.register_advancement();
            advance();
            return res.success(make_shared<WhileNode>(condition, body_node, true));
        } else {
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                auto next_tok = peek();
                if (next_tok.has_value() && next_tok->type == T_EQ) {
                    body_node = res.register_node(statements());
                } else {
                    body_node = res.register_node(expression());
                }
            } else {
                body_node = res.register_node(expression());
            }
            if (res.error) return res;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
            }
            res.register_advancement();
            advance();
            return res.success(make_shared<WhileNode>(condition, body_node, false));
        }
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

        if (current_tok.has_value() && current_tok->type == T_NEWLINE) {
            res.register_advancement();
            advance();
            body_node = res.register_node(multiline());
            if (res.error) return res;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
            }
            res.register_advancement();
            advance();

            return res.success(make_shared<ForNode>(var_name, start_value, end_value, step_value, body_node, true));
        } else {
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                auto next_tok = peek();
                if (next_tok.has_value() && next_tok->type == T_EQ) {
                    body_node = res.register_node(statements());
                } else {
                    body_node = res.register_node(expression());
                }
            } else {
                body_node = res.register_node(expression());
            }
            if (res.error) return res;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
            }
            res.register_advancement();
            advance();

            return res.success(make_shared<ForNode>(var_name, start_value, end_value, step_value, body_node, false));
        }
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

            auto all_cases_res = res.register_node(elif_or_else_expression());
            if (res.error) return res;

            auto if_node = static_pointer_cast<IfNode>(all_cases_res);
            cases.insert(cases.end(), if_node->cases.begin(), if_node->cases.end());
            else_case = if_node->else_case;

        } else {
            shared_ptr<Node> expression_node;
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                auto next_tok = peek();
                if (next_tok.has_value() && next_tok->type == T_EQ) {
                    expression_node = res.register_node(statements());
                } else {
                    expression_node = res.register_node(expression());
                }
            } else {
                expression_node = res.register_node(expression());
            }
            if (res.error) return res;
            cases.emplace_back(condition, expression_node);

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
            }
            res.register_advancement();
            advance();

            auto all_cases_res = res.register_node(elif_or_else_expression());
            if (res.error) return res;

            auto if_node = static_pointer_cast<IfNode>(all_cases_res);
            cases.insert(cases.end(), if_node->cases.begin(), if_node->cases.end());
            else_case = if_node->else_case;
        }

        return res.success(make_shared<IfNode>(cases, else_case));
    }

    ParseResult elif_or_else_expression() {
        ParseResult res;
        vector<pair<shared_ptr<Node>, shared_ptr<Node>>> cases;
        shared_ptr<Node> else_case = nullptr;

        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "orwhen") {
            auto elif_res = res.register_node(elif_expression());
            if (res.error) return res;
            auto if_node = static_pointer_cast<IfNode>(elif_res);
            cases = if_node->cases;
            else_case = if_node->else_case;
        } else {
            auto else_res = res.register_node(else_expression());
            if (res.error) return res;
            if (else_res) {
                auto if_node = static_pointer_cast<IfNode>(else_res);
                else_case = if_node->else_case;
            }
        }
        return res.success(make_shared<IfNode>(cases, else_case));
    }

    ParseResult elif_expression() {
        ParseResult res;
        vector<pair<shared_ptr<Node>, shared_ptr<Node>>> cases;
        shared_ptr<Node> else_case = nullptr;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "orwhen")) {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected 'orwhen'"));
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

            auto all_cases_res = res.register_node(elif_or_else_expression());
            if (res.error) return res;

            auto if_node = static_pointer_cast<IfNode>(all_cases_res);
            cases.insert(cases.end(), if_node->cases.begin(), if_node->cases.end());
            else_case = if_node->else_case;

        } else {
            shared_ptr<Node> expression_node;
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                auto next_tok = peek();
                if (next_tok.has_value() && next_tok->type == T_EQ) {
                    expression_node = res.register_node(statements());
                } else {
                    expression_node = res.register_node(expression());
                }
            } else {
                expression_node = res.register_node(expression());
            }
            if (res.error) return res;
            cases.emplace_back(condition, expression_node);

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
            }
            res.register_advancement();
            advance();

            auto all_cases_res = res.register_node(elif_or_else_expression());
            if (res.error) return res;

            auto if_node = static_pointer_cast<IfNode>(all_cases_res);
            cases.insert(cases.end(), if_node->cases.begin(), if_node->cases.end());
            else_case = if_node->else_case;
        }

        return res.success(make_shared<IfNode>(cases, else_case));
    }

    ParseResult else_expression() {
        ParseResult res;
        shared_ptr<Node> else_case = nullptr;

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

                if (!current_tok.has_value() || current_tok->type != T_RPAREN2) {
                    return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '}'"));
                }
                res.register_advancement();
                advance();

            } else {
                if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
                    auto next_tok = peek();
                    if (next_tok.has_value() && next_tok->type == T_EQ) {
                        else_case = res.register_node(statements());
                    } else {
                        else_case = res.register_node(expression());
                    }
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
        }
        return res.success(make_shared<IfNode>(vector<pair<shared_ptr<Node>, shared_ptr<Node>>>(), else_case));
    }

    ParseResult statements() {
        ParseResult res;

        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "define") {
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

            auto expr = res.register_node(expression());
            if (res.error) return res;
            return res.success(make_shared<VariableAssignNode>(var_name, expr));
        }
        else if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
            Token var_name = current_tok.value();
            res.register_advancement();
            advance();

            if (!current_tok.has_value() || current_tok->type != T_EQ) {
                return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '='"));
            }

            res.register_advancement();
            advance();

            auto expr = res.register_node(expression());
            if (res.error) return res;
            return res.success(make_shared<VariableAssignNode>(var_name, expr));
        }

        return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected 'define' or identifier"));
    }

    ParseResult expression() {
        ParseResult res;

        if (current_tok.has_value() && current_tok->type == T_KEYWORD) {
            string key = any_cast<string>(current_tok->value);
            if (key == "return") {
                auto pos_start = current_tok->pos_start;
                res.register_advancement();
                advance();

                ParseResult expr_res = expression();
                shared_ptr<Node> expr = nullptr;

                if (!expr_res.error) {
                    expr = res.register_node(expr_res);
                } else {
                    if (expr_res.last_registered_advance_count > 0) {
                        reverse(expr_res.to_reverse_count);
                    }
                }

                auto pos_end = expr ? expr->pos_end : pos_start;
                return res.success(make_shared<ReturnNode>(expr, pos_start.value(), pos_end.value()));
            }
            else if (key == "continue") {
                auto pos_start = current_tok->pos_start;
                auto pos_end = current_tok->pos_end;
                res.register_advancement();
                advance();
                return res.success(make_shared<ContinueNode>(pos_start.value(), pos_end.value()));
            }
            else if (key == "break") {
                auto pos_start = current_tok->pos_start;
                auto pos_end = current_tok->pos_end;
                res.register_advancement();
                advance();
                return res.success(make_shared<BreakNode>(pos_start.value(), pos_end.value()));
            }
        }

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

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER) {
            auto next_tok = peek();
            if (next_tok.has_value() && next_tok->type == T_LPAREN) {
                return function_call(nullptr);
            }
        }

        auto left_node = res.register_node(factor());
        if (res.error) return res;

        while (current_tok.has_value()) {
            if (current_tok->type == T_MUL || current_tok->type == T_DIVIDE) {
                Token op_token = current_tok.value();
                res.register_advancement();
                advance();
                auto right_node = res.register_node(factor());
                if (res.error) return res;
                left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
            }
            else {
                break;
            }
        }
        return res.success(left_node);
    }

    ParseResult factor() {
        ParseResult res;
        if (!current_tok.has_value()) {
            return res.failure(InvalidSyntaxError({}, {}, "Expected expression"));
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
        else if (token.type == T_STRING) {
            res.register_advancement();
            advance();
            return res.success(make_shared<StringNode>(token));
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
        }

        return res.failure(InvalidSyntaxError(token.pos_start.value(), token.pos_end.value(), "Expected expression"));
    }
};