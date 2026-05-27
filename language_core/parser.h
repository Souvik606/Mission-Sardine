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
#include "../ast_nodes/dict_nodes.h"
#include "../ast_nodes/jump_nodes.h"
#include "../ast_nodes/try_catch_nodes.h"
#include "../ast_nodes/class_nodes.h"
#include "error.h"
#include "constants.h"

using namespace std;

class ElifElseResultNode final : public Node
{
public:
    vector<tuple<shared_ptr<Node>, shared_ptr<Node>, bool>> cases;
    optional<pair<shared_ptr<Node>, bool>> else_case;
    ElifElseResultNode(vector<tuple<shared_ptr<Node>, shared_ptr<Node>, bool>> cases, optional<pair<shared_ptr<Node>, bool>> else_case)
        : Node(Position(), Position()), cases(std::move(cases)), else_case(std::move(else_case))
    {
    }
    [[nodiscard]] std::string to_string() const override { return ""; }
};

class Parser
{
public:
    explicit Parser(vector<Token> tokens)
        : tokens(std::move(tokens)), tok_index(-1), current_tok(nullopt), call(0)
    {
        advance();
    }

    ParseResult parse()
    {
        ParseResult result = multiline();
        if (!result.error && current_tok.has_value() && current_tok->type != T_EOF)
        {
            return result.failure(
                InvalidSyntaxError(
                    current_tok->pos_start.value(), current_tok->pos_end.value(),
                    "Expected '+', '-', '*', '/'"));
        }
        return result;
    }

private:
    vector<Token> tokens;
    int tok_index;
    optional<Token> current_tok;
    int call;

    void update_current_tok()
    {
        if (tok_index >= 0 && tok_index < static_cast<int>(tokens.size()))
        {
            current_tok = make_optional(tokens[tok_index]);
        }
        else
        {
            current_tok = nullopt;
        }
    }

    optional<Token> advance()
    {
        tok_index++;
        update_current_tok();
        return current_tok;
    }

    optional<Token> reverse(const int amount = 1)
    {
        tok_index -= amount;
        update_current_tok();
        return current_tok;
    }

    [[nodiscard]] optional<Token> peek() const
    {
        const int next_index = tok_index + 1;
        return (next_index < static_cast<int>(tokens.size())) ? make_optional(tokens[next_index]) : nullopt;
    }

    bool check_is_statement()
    {
        int current_tok_index = tok_index;
        bool is_statement = false;
        while (current_tok.has_value() && current_tok->type != T_NEWLINE && current_tok->type != T_EOF)
        {
            if (current_tok->type == T_EQ)
            {
                is_statement = true;
                break;
            }
            advance();
        }
        tok_index = current_tok_index;
        update_current_tok();
        return is_statement;
    }

    ParseResult multiline()
    {
        ParseResult res;
        vector<shared_ptr<Node>> statements_list;
        optional<Position> pos_start;

        if (current_tok.has_value())
            pos_start = current_tok->pos_start;

        while (current_tok.has_value() && current_tok->type == T_NEWLINE)
        {
            res.register_advancement();
            advance();
        }

        if (current_tok.has_value() && current_tok->type != T_EOF && current_tok->type != T_RPAREN2)
        {
            auto first_stmt = res.register_node(singleline());
            if (res.error)
                return res;
            statements_list.push_back(first_stmt);
        }

        while (true)
        {
            while (current_tok.has_value() && current_tok->type == T_NEWLINE)
            {
                res.register_advancement();
                advance();
            }
            if (current_tok.has_value() && current_tok->type != T_EOF && current_tok->type != T_RPAREN2)
            {
                auto stmt = res.register_node(singleline());
                if (res.error)
                    return res;
                statements_list.push_back(stmt);
            }
            else
            {
                break;
            }
        }

        while (current_tok.has_value() && current_tok->type == T_NEWLINE)
        {
            res.register_advancement();
            advance();
        }

        optional<Position> pos_end;
        if (current_tok.has_value())
            pos_end = current_tok->pos_end;

        return res.success(make_shared<ListNode>(statements_list, pos_start, pos_end));
    }

    ParseResult singleline()
    {
        ParseResult res;
        if (!current_tok.has_value())
            return res;
        Token token = current_tok.value();

        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "when")
        {
            auto if_expr = res.register_node(if_expression());
            if (res.error)
                return res;
            return res.success(if_expr);
        }
        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "cycle")
        {
            auto for_expr = res.register_node(for_expression());
            if (res.error)
                return res;
            return res.success(for_expr);
        }
        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "during")
        {
            auto while_expr = res.register_node(while_expression());
            if (res.error)
                return res;
            return res.success(while_expr);
        }
        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "method")
        {
            auto method_expr = res.register_node(function_definition());
            if (res.error)
                return res;
            return res.success(method_expr);
        }
        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "menu")
        {
            auto switch_stmt = res.register_node(switch_statement());
            if (res.error)
                return res;
            return res.success(switch_stmt);
        }
        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "risk")
        {
            auto exception_expr = res.register_node(exception_handling());
            if (res.error)
                return res;
            return res.success(exception_expr);
        }
        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "model")
        {
            auto model_def = res.register_node(model_definition());
            if (res.error)
                return res;
            return res.success(model_def);
        }
        if (token.type == T_KEYWORD && (any_cast<string>(token.value) == "escape" || any_cast<string>(token.value) == "proceed" || any_cast<string>(token.value) == "yield"))
        {
            auto jump_node = res.register_node(jump_statements());
            if (res.error)
                return res;
            return res.success(jump_node);
        }
        // obj.attr = value  or  obj.method(args)
        if (token.type == T_IDENTIFIER && peek().has_value() && peek()->type == T_DOT)
        {
            auto dot_node = res.register_node(dot_access_statement());
            if (res.error)
                return res;
            return res.success(dot_node);
        }
        if (token.type == T_IDENTIFIER && peek().has_value() && peek()->type == T_LPAREN)
        {
            auto call_node = res.register_node(function_call());
            if (res.error)
                return res;
            return res.success(call_node);
        }

        auto statement_node = res.register_node(statements());
        if (res.error)
            return res;

        auto node = res.register_node(res.success(statement_node));
        if (res.error)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected int,float,identifier"));
        }
        return res.success(node);
    }

    ParseResult list_expression()
    {
        ParseResult res;
        vector<shared_ptr<Node>> element_nodes;
        optional<Position> pos_start;

        if (current_tok.has_value())
            pos_start = current_tok->pos_start;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN3)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '['"));
        }

        res.register_advancement();
        advance();

        optional<Position> pos_end;

        if (current_tok.has_value() && current_tok->type == T_RPAREN3)
        {
            pos_end = current_tok->pos_end;
            res.register_advancement();
            advance();
        }
        else
        {
            element_nodes.push_back(res.register_node(expression()));
            if (res.error)
                return res;

            while (current_tok.has_value() && current_tok->type == T_COMMA)
            {
                res.register_advancement();
                advance();
                element_nodes.push_back(res.register_node(expression()));
                if (res.error)
                    return res;
            }

            if (!current_tok.has_value() || current_tok->type != T_RPAREN3)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected ',' or ']'"));
            }

            pos_end = current_tok->pos_end;
            res.register_advancement();
            advance();
        }

        return res.success(make_shared<ListNode>(element_nodes, pos_start, pos_end));
    }

    ParseResult function_definition()
    {
        ParseResult res;
        optional<Token> var_name_tok = nullopt;
        vector<Token> arg_name_toks;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "method"))
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'method'"));
        }
        res.register_advancement();
        advance();

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER)
        {
            var_name_tok = current_tok.value();
            res.register_advancement();
            advance();
            if (!current_tok.has_value() || current_tok->type != T_LPAREN)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '('"));
            }
        }
        else
        {
            if (!current_tok.has_value() || current_tok->type != T_LPAREN)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '('"));
            }
        }

        res.register_advancement();
        advance();

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER)
        {
            arg_name_toks.push_back(current_tok.value());
            res.register_advancement();
            advance();

            while (current_tok.has_value() && current_tok->type == T_COMMA)
            {
                res.register_advancement();
                advance();
                if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
                {
                    return res.failure(InvalidSyntaxError(
                        current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                        current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                        "Expected identifier"));
                }
                arg_name_toks.push_back(current_tok.value());
                res.register_advancement();
                advance();
            }

            if (!current_tok.has_value() || current_tok->type != T_RPAREN)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected ',' or ')'"));
            }
        }
        else
        {
            if (!current_tok.has_value() || current_tok->type != T_RPAREN)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected identifier or ')'"));
            }
        }

        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"));
        }
        res.register_advancement();
        advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
        {
            if (current_tok->type == T_KEYWORD &&
                (any_cast<string>(current_tok->value) == "escape" ||
                 any_cast<string>(current_tok->value) == "proceed" ||
                 any_cast<string>(current_tok->value) == "yield"))
            {
                auto jump_node = res.register_node(jump_statements());
                if (res.error)
                    return res;
                body_nodes.push_back(jump_node);
            }
            else
            {
                auto multiline_node = res.try_register(multiline());
                if (res.error)
                    return res;
                if (!multiline_node)
                {
                    bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                   (any_cast<string>(current_tok->value) == "escape" ||
                                    any_cast<string>(current_tok->value) == "proceed" ||
                                    any_cast<string>(current_tok->value) == "yield");
                    if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2)
                    {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected identifier,when,during,method or cycle"));
                    }
                }
                if (multiline_node)
                {
                    if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node))
                    {
                        body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                    }
                }
            }
        }

        auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }
        res.register_advancement();
        advance();

        return res.success(make_shared<FunctionDefinitionNode>(var_name_tok, arg_name_toks, body_node, false));
    }

    ParseResult function_call()
    {
        ParseResult res;
        vector<shared_ptr<Node>> arg_nodes;
        shared_ptr<Node> call_node;

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER)
        {
            call_node = res.register_node(res.success(make_shared<VariableUseNode>(current_tok.value())));
            if (res.error)
                return res;
            res.register_advancement();
            advance();
        }
        else
        {
            res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected identifier"));
        }

        if (res.error)
            return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '('"));
        }

        res.register_advancement();
        advance();

        if (current_tok.has_value() && current_tok->type == T_RPAREN)
        {
            res.register_advancement();
            advance();
        }
        else
        {
            arg_nodes.push_back(res.register_node(expression()));
            if (res.error)
                return res;

            while (current_tok.has_value() && current_tok->type == T_COMMA)
            {
                res.register_advancement();
                advance();
                arg_nodes.push_back(res.register_node(expression()));
                if (res.error)
                    return res;
            }

            if (!current_tok.has_value() || current_tok->type != T_RPAREN)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected ',' or ')'"));
            }

            res.register_advancement();
            advance();
        }

        return res.success(make_shared<FunctionCallNode>(call_node, arg_nodes));
    }

    ParseResult switch_statement()
    {
        ParseResult res;
        vector<shared_ptr<SwitchCaseNode>> cases;

        if (!current_tok.has_value() || current_tok->type != T_KEYWORD || any_cast<string>(current_tok->value) != "menu")
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'menu'"));
        }

        res.register_advancement();
        advance();

        auto selection = res.register_node(ternary_expression());
        if (res.error)
            return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"));
        }

        res.register_advancement();
        advance();

        while (current_tok.has_value() && current_tok->type == T_NEWLINE)
        {
            res.register_advancement();
            advance();
        }

        bool found_default = false;
        int count = 0;

        while (current_tok.has_value() && current_tok->type == T_KEYWORD &&
               (any_cast<string>(current_tok->value) == "choice" || any_cast<string>(current_tok->value) == "fallback"))
        {

            if (any_cast<string>(current_tok->value) == "choice")
            {
                auto case_node = res.register_node(case_statement());
                if (res.error)
                    return res;
                cases.push_back(dynamic_pointer_cast<SwitchCaseNode>(case_node));
                while (current_tok.has_value() && current_tok->type == T_NEWLINE)
                {
                    res.register_advancement();
                    advance();
                }
            }
            else
            {
                if (found_default)
                {
                    return res.failure(InvalidSyntaxError(
                        current_tok->pos_start.value_or(Position()),
                        current_tok->pos_end.value_or(Position()),
                        "Multiple 'fallback' statements found"));
                }
                found_default = true;
                auto default_node = res.register_node(default_statement());
                if (res.error)
                    return res;
                cases.push_back(dynamic_pointer_cast<SwitchCaseNode>(default_node));
                while (current_tok.has_value() && current_tok->type == T_NEWLINE)
                {
                    res.register_advancement();
                    advance();
                }
            }
            count++;
        }

        if (count == 0)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'choice' or 'fallback'"));
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }

        res.register_advancement();
        advance();

        return res.success(make_shared<SwitchNode>(selection, cases, false));
    }

    ParseResult case_statement()
    {
        ParseResult res;

        if (!current_tok.has_value() || current_tok->type != T_KEYWORD || any_cast<string>(current_tok->value) != "choice")
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'choice'"));
        }
        res.register_advancement();
        advance();

        auto choice_val = res.register_node(ternary_expression());
        if (res.error)
            return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"));
        }

        res.register_advancement();
        advance();

        shared_ptr<Node> body_node;
        bool is_multiline = false;

        if (current_tok.has_value() && current_tok->type == T_NEWLINE)
        {
            res.register_advancement();
            advance();

            body_node = res.register_node(multiline());
            if (res.error)
                return res;

            is_multiline = true;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '}'"));
            }
            res.register_advancement();
            advance();
        }
        else
        {
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER && peek().has_value() && peek()->type == T_EQ)
            {
                body_node = res.register_node(statements());
            }
            else
            {
                body_node = res.register_node(expression());
            }
            if (res.error)
                return res;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '}'"));
            }
            res.register_advancement();
            advance();
        }

        return res.success(make_shared<SwitchCaseNode>(choice_val, body_node, is_multiline));
    }

    ParseResult default_statement()
    {
        ParseResult res;

        if (!current_tok.has_value() || current_tok->type != T_KEYWORD || any_cast<string>(current_tok->value) != "fallback")
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'fallback'"));
        }
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"));
        }

        res.register_advancement();
        advance();

        shared_ptr<Node> body_node;
        bool is_multiline = false;

        if (current_tok.has_value() && current_tok->type == T_NEWLINE)
        {
            res.register_advancement();
            advance();

            body_node = res.register_node(multiline());
            if (res.error)
                return res;

            is_multiline = true;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '}'"));
            }
            res.register_advancement();
            advance();
        }
        else
        {
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER && peek().has_value() && peek()->type == T_EQ)
            {
                body_node = res.register_node(statements());
            }
            else
            {
                body_node = res.register_node(expression());
            }
            if (res.error)
                return res;

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '}'"));
            }
            res.register_advancement();
            advance();
        }

        return res.success(make_shared<SwitchCaseNode>(nullptr, body_node, is_multiline));
    }

    ParseResult while_expression()
    {
        ParseResult res;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "during"))
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'during'"));
        }
        res.register_advancement();
        advance();

        auto condition = res.register_node(expression());
        if (res.error)
            return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"));
        }
        res.register_advancement();
        advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
        {
            if (current_tok->type == T_KEYWORD &&
                (any_cast<string>(current_tok->value) == "escape" ||
                 any_cast<string>(current_tok->value) == "proceed" ||
                 any_cast<string>(current_tok->value) == "yield"))
            {
                auto jump_node = res.register_node(jump_statements());
                if (res.error)
                    return res;
                body_nodes.push_back(jump_node);
            }
            else
            {
                auto multiline_node = res.try_register(multiline());
                if (res.error)
                    return res;
                if (!multiline_node)
                {
                    bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                   (any_cast<string>(current_tok->value) == "escape" ||
                                    any_cast<string>(current_tok->value) == "proceed" ||
                                    any_cast<string>(current_tok->value) == "yield");
                    if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2)
                    {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected identifier,when,during,method or cycle"));
                    }
                }
                if (multiline_node)
                {
                    if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node))
                    {
                        body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                    }
                }
            }
        }

        auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }
        res.register_advancement();
        advance();

        return res.success(make_shared<WhileNode>(condition, body_node, false));
    }

    ParseResult for_expression()
    {
        ParseResult res;
        shared_ptr<Node> step_value = nullptr;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "cycle"))
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'cycle'"));
        }
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected identifier"));
        }
        Token var_name = current_tok.value();
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_EQ)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '='"));
        }
        res.register_advancement();
        advance();

        auto start_value = res.register_node(expression());
        if (res.error)
            return res;

        if (!current_tok.has_value() || current_tok->type != T_COLON)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected ':'"));
        }
        res.register_advancement();
        advance();

        auto end_value = res.register_node(expression());
        if (res.error)
            return res;

        if (current_tok.has_value() && current_tok->type == T_COLON)
        {
            res.register_advancement();
            advance();
            step_value = res.register_node(expression());
            if (res.error)
                return res;
        }

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"));
        }
        res.register_advancement();
        advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
        {
            if (current_tok->type == T_KEYWORD &&
                (any_cast<string>(current_tok->value) == "escape" ||
                 any_cast<string>(current_tok->value) == "proceed" ||
                 any_cast<string>(current_tok->value) == "yield"))
            {
                auto jump_node = res.register_node(jump_statements());
                if (res.error)
                    return res;
                body_nodes.push_back(jump_node);
            }
            else
            {
                auto multiline_node = res.try_register(multiline());
                if (res.error)
                    return res;
                if (!multiline_node)
                {
                    bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                   (any_cast<string>(current_tok->value) == "escape" ||
                                    any_cast<string>(current_tok->value) == "proceed" ||
                                    any_cast<string>(current_tok->value) == "yield");
                    if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2)
                    {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected identifier,when,during,method or cycle"));
                    }
                }
                if (multiline_node)
                {
                    if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node))
                    {
                        body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                    }
                }
            }
        }

        auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }
        res.register_advancement();
        advance();

        return res.success(make_shared<ForNode>(var_name, start_value, end_value, step_value, body_node, false));
    }

    ParseResult if_expression()
    {
        ParseResult res;
        vector<tuple<shared_ptr<Node>, shared_ptr<Node>, bool>> cases;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "when"))
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'when'"));
        }

        res.register_advancement();
        advance();

        auto condition = res.register_node(expression());
        if (res.error)
            return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"));
        }
        res.register_advancement();
        advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
        {
            if (current_tok->type == T_KEYWORD &&
                (any_cast<string>(current_tok->value) == "escape" ||
                 any_cast<string>(current_tok->value) == "proceed" ||
                 any_cast<string>(current_tok->value) == "yield"))
            {
                auto jump_node = res.register_node(jump_statements());
                if (res.error)
                    return res;
                body_nodes.push_back(jump_node);
            }
            else
            {
                auto multiline_node = res.try_register(multiline());
                if (res.error)
                    return res;
                if (!multiline_node)
                {
                    bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                   (any_cast<string>(current_tok->value) == "escape" ||
                                    any_cast<string>(current_tok->value) == "proceed" ||
                                    any_cast<string>(current_tok->value) == "yield");
                    if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2)
                    {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected identifier,when,during,method or cycle"));
                    }
                }
                if (multiline_node)
                {
                    if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node))
                    {
                        body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                    }
                }
            }
        }

        auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }
        res.register_advancement();
        advance();

        cases.emplace_back(condition, body_node, true);

        auto all_cases_node = res.register_node(elif_or_else_expression());
        if (res.error)
            return res;

        optional<pair<shared_ptr<Node>, bool>> else_case = nullopt;
        if (auto elif_result = dynamic_pointer_cast<ElifElseResultNode>(all_cases_node))
        {
            cases.insert(cases.end(), elif_result->cases.begin(), elif_result->cases.end());
            else_case = elif_result->else_case;
        }

        return res.success(make_shared<IfNode>(cases, else_case));
    }

    ParseResult elif_or_else_expression()
    {
        ParseResult res;
        vector<tuple<shared_ptr<Node>, shared_ptr<Node>, bool>> cases;
        optional<pair<shared_ptr<Node>, bool>> else_case = nullopt;

        while (current_tok.has_value() && current_tok->type == T_NEWLINE)
        {
            res.register_advancement();
            advance();
        }

        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "orwhen")
        {
            auto all_cases_node = res.register_node(elif_expression());
            if (res.error)
                return res;
            if (auto elif_result = dynamic_pointer_cast<ElifElseResultNode>(all_cases_node))
            {
                cases = elif_result->cases;
                else_case = elif_result->else_case;
            }
        }
        else if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "otherwise")
        {
            auto else_node = res.register_node(else_expression());
            if (res.error)
                return res;
            if (else_node)
            {
                else_case = make_pair(else_node, true);
            }
        }

        return res.success(make_shared<ElifElseResultNode>(cases, else_case));
    }

    ParseResult elif_expression()
    {
        ParseResult res;
        vector<tuple<shared_ptr<Node>, shared_ptr<Node>, bool>> cases;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "orwhen"))
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'orwhen'"));
        }

        res.register_advancement();
        advance();

        auto condition = res.register_node(expression());
        if (res.error)
            return res;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"));
        }
        res.register_advancement();
        advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
        {
            if (current_tok->type == T_KEYWORD &&
                (any_cast<string>(current_tok->value) == "escape" ||
                 any_cast<string>(current_tok->value) == "proceed" ||
                 any_cast<string>(current_tok->value) == "yield"))
            {
                auto jump_node = res.register_node(jump_statements());
                if (res.error)
                    return res;
                body_nodes.push_back(jump_node);
            }
            else
            {
                auto multiline_node = res.try_register(multiline());
                if (res.error)
                    return res;
                if (!multiline_node)
                {
                    bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                   (any_cast<string>(current_tok->value) == "escape" ||
                                    any_cast<string>(current_tok->value) == "proceed" ||
                                    any_cast<string>(current_tok->value) == "yield");
                    if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2)
                    {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected identifier,when,during,method or cycle"));
                    }
                }
                if (multiline_node)
                {
                    if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node))
                    {
                        body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                    }
                }
            }
        }

        auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }
        res.register_advancement();
        advance();

        cases.emplace_back(condition, body_node, true);

        auto all_cases_node = res.register_node(elif_or_else_expression());
        if (res.error)
            return res;

        optional<pair<shared_ptr<Node>, bool>> else_case = nullopt;
        if (auto elif_result = dynamic_pointer_cast<ElifElseResultNode>(all_cases_node))
        {
            cases.insert(cases.end(), elif_result->cases.begin(), elif_result->cases.end());
            else_case = elif_result->else_case;
        }

        return res.success(make_shared<ElifElseResultNode>(cases, else_case));
    }

    ParseResult else_expression()
    {
        ParseResult res;

        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "otherwise")
        {
            res.register_advancement();
            advance();

            if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '{'"));
            }
            res.register_advancement();
            advance();

            vector<shared_ptr<Node>> body_nodes;
            optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

            while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
            {
                if (current_tok->type == T_KEYWORD &&
                    (any_cast<string>(current_tok->value) == "escape" ||
                     any_cast<string>(current_tok->value) == "proceed" ||
                     any_cast<string>(current_tok->value) == "yield"))
                {
                    auto jump_node = res.register_node(jump_statements());
                    if (res.error)
                        return res;
                    body_nodes.push_back(jump_node);
                }
                else
                {
                    auto multiline_node = res.try_register(multiline());
                    if (res.error)
                        return res;
                    if (!multiline_node)
                    {
                        bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                       (any_cast<string>(current_tok->value) == "escape" ||
                                        any_cast<string>(current_tok->value) == "proceed" ||
                                        any_cast<string>(current_tok->value) == "yield");
                        if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2)
                        {
                            return res.failure(InvalidSyntaxError(
                                current_tok->pos_start.value_or(Position()),
                                current_tok->pos_end.value_or(Position()),
                                "Expected identifier,when,during,method or cycle"));
                        }
                    }
                    if (multiline_node)
                    {
                        if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node))
                        {
                            body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                        }
                    }
                }
            }

            auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '}'"));
            }
            res.register_advancement();
            advance();

            return res.success(body_node);
        }

        return res.success(nullptr);
    }

    ParseResult unary()
    {
        ParseResult res;
        if (!current_tok.has_value())
            return res;
        Token token = current_tok.value();

        if (token.type == T_PLUS || token.type == T_MINUS)
        {
            res.register_advancement();
            advance();
            auto factor_node = res.register_node(unary());
            if (res.error)
                return res;
            return res.success(make_shared<UnaryOperationNode>(token, factor_node));
        }

        return exponent();
    }

    ParseResult exponent()
    {
        ParseResult res;
        auto left_node = res.register_node(factor());
        if (res.error)
            return res;

        while (current_tok.has_value() && current_tok->type == T_EXP)
        {
            Token op_token = current_tok.value();
            res.register_advancement();
            advance();
            auto right_node = res.register_node(unary());
            if (res.error)
                return res;
            left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
        }

        auto node = res.register_node(res.success(left_node));
        if (res.error)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'define',int,float,identifier,'+','-' or '('"));
        }
        return res.success(node);
    }

    ParseResult ternary_expression()
    {
        ParseResult res;
        shared_ptr<Node> comp_node;

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER && peek().has_value() && peek()->type == T_EQ)
        {
            comp_node = res.register_node(statements());
        }
        else
        {
            comp_node = res.register_node(logical_expression());
        }

        if (res.error)
            return res;
        shared_ptr<Node> true_node = nullptr, false_node = nullptr;

        while (current_tok.has_value() && current_tok->type == T_QUESTION)
        {
            res.register_advancement();
            advance();
            true_node = res.register_node(ternary_expression());
            if (res.error)
                return res;

            if (current_tok.has_value() && current_tok->type != T_COLON)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok->pos_start.value_or(Position()),
                    current_tok->pos_end.value_or(Position()),
                    "Expected ':' "));
            }

            res.register_advancement();
            advance();
            false_node = res.register_node(ternary_expression());
            if (res.error)
                return res;
        }

        if (!true_node || !false_node)
        {
            return res.success(comp_node);
        }

        return res.success(make_shared<TernaryOperationNode>(comp_node, true_node, false_node));
    }

    ParseResult factor()
    {
        ParseResult res;
        if (!current_tok.has_value())
            return res;
        Token token = current_tok.value();

        if (token.type == T_INT || token.type == T_FLOAT)
        {
            res.register_advancement();
            advance();
            return res.success(make_shared<NumberNode>(token));
        }

        if (token.type == T_STRING)
        {
            res.register_advancement();
            advance();
            return res.success(make_shared<StringNode>(token));
        }

        if (token.type == T_IDENTIFIER && peek().has_value() && peek()->type == T_LPAREN)
        {
            auto call_expression = res.register_node(function_call());
            if (res.error)
                return res;
            return res.success(call_expression);
        }

        if (token.type == T_IDENTIFIER)
        {
            res.register_advancement();
            advance();
            Token var_name_tok = token;
            vector<shared_ptr<Node>> index_node;

            if (current_tok.has_value() && current_tok->type == T_LPAREN3)
            {
                while (current_tok.has_value() && current_tok->type == T_LPAREN3)
                {
                    res.register_advancement();
                    advance();
                    auto expr = res.register_node(expression());
                    if (res.error)
                        return res;
                    index_node.push_back(expr);

                    if (!current_tok.has_value() || current_tok->type != T_RPAREN3)
                    {
                        return res.failure(InvalidSyntaxError(
                            current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                            current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                            "Expected ']'"));
                    }
                    res.register_advancement();
                    advance();
                }
            }

            if (index_node.empty() && current_tok.has_value() && current_tok->type == T_DOT)
            {
                auto obj_node = make_shared<VariableUseNode>(var_name_tok, index_node);
                return res.success(dot_access_chain(res, obj_node));
            }
            return res.success(make_shared<VariableUseNode>(var_name_tok, index_node));
        }

        if (token.type == T_LPAREN)
        {
            res.register_advancement();
            advance();
            auto expr = res.register_node(expression());
            if (res.error)
                return res;
            if (!current_tok.has_value() || current_tok->type != T_RPAREN)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected ')'"));
            }
            res.register_advancement();
            advance();
            return res.success(expr);
        }

        if (token.type == T_LPAREN3)
        {
            auto list_expr = res.register_node(list_expression());
            if (res.error)
                return res;
            return res.success(list_expr);
        }

        if (token.type == T_LPAREN2)
        {
            auto dict_expr = res.register_node(dict_expression());
            if (res.error)
                return res;
            return res.success(dict_expr);
        }

        return res.failure(InvalidSyntaxError(
            token.pos_start.value_or(Position()),
            token.pos_end.value_or(Position()),
            "Expected int, float,identifier,'+','-'or '('"));
    }

    ParseResult term()
    {
        ParseResult res;

        auto left_node = res.register_node(unary());
        if (res.error)
            return res;

        while (current_tok.has_value() && (current_tok->type == T_MUL || current_tok->type == T_DIVIDE ||
                                           current_tok->type == T_MODULUS || current_tok->type == T_FLOOR))
        {
            Token op_token = current_tok.value();
            res.register_advancement();
            advance();
            auto right_node = res.register_node(unary());
            if (res.error)
                return res;
            left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
        }

        return res.success(left_node);
    }

    ParseResult statements()
    {
        ParseResult res;

        vector<Token> var_name_toks;
        vector<vector<shared_ptr<Node>>> index_nodes;
        vector<shared_ptr<Node>> value_nodes;

        while (true)
        {
            if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected identifier"));
            }

            var_name_toks.push_back(current_tok.value());
            res.register_advancement();
            advance();

            vector<shared_ptr<Node>> indices;
            while (current_tok.has_value() && current_tok->type == T_LPAREN3)
            {
                res.register_advancement();
                advance();

                auto expr = res.register_node(expression());
                if (res.error)
                    return res;
                indices.push_back(expr);

                if (!current_tok.has_value() || current_tok->type != T_RPAREN3)
                {
                    return res.failure(InvalidSyntaxError(
                        current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                        current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                        "Expected ')'"));
                }
                res.register_advancement();
                advance();
            }

            index_nodes.push_back(indices.empty() ? vector<shared_ptr<Node>>() : indices);

            if (!current_tok.has_value() || current_tok->type != T_COMMA)
            {
                break;
            }
            res.register_advancement();
            advance();
        }

        if (!current_tok.has_value() || current_tok->type != T_EQ)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '='"));
        }
        res.register_advancement();
        advance();

        while (true)
        {
            auto expr = res.register_node(expression());
            if (res.error)
                return res;
            value_nodes.push_back(expr);

            if (!current_tok.has_value() || current_tok->type != T_COMMA)
            {
                break;
            }
            res.register_advancement();
            advance();
        }

        if (var_name_toks.size() != value_nodes.size())
        {
            return res.failure(InvalidSyntaxError(
                var_name_toks.front().pos_start.value_or(Position()),
                value_nodes.back()->pos_end.value_or(Position()),
                "Mismatched assignment count: " + to_string(var_name_toks.size()) +
                    " variables, " + to_string(value_nodes.size()) + " values"));
        }

        return res.success(make_shared<VariableAssignNode>(var_name_toks, value_nodes, index_nodes));
    }

    ParseResult jump_statements()
    {
        ParseResult res;

        if (!current_tok.has_value() || current_tok->type != T_KEYWORD ||
            (any_cast<string>(current_tok->value) != "proceed" &&
             any_cast<string>(current_tok->value) != "escape" &&
             any_cast<string>(current_tok->value) != "yield"))
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'proceed' or 'escape' or 'yield' "));
        }

        if (any_cast<string>(current_tok->value) == "proceed")
        {
            auto start_pos = current_tok->pos_start.value_or(Position());
            res.register_advancement();
            advance();
            return res.success(make_shared<ContinueNode>(start_pos, start_pos));
        }

        if (any_cast<string>(current_tok->value) == "escape")
        {
            auto start_pos = current_tok->pos_start.value_or(Position());
            res.register_advancement();
            advance();
            return res.success(make_shared<BreakNode>(start_pos, start_pos));
        }

        if (any_cast<string>(current_tok->value) == "yield")
        {
            auto start_pos = current_tok->pos_start.value_or(Position());
            res.register_advancement();
            advance();

            auto expr = res.try_register(expression());
            if (!expr)
            {
                reverse(res.to_reverse_count);
            }

            return res.success(make_shared<ReturnNode>(expr, start_pos,
                                                       current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : start_pos));
        }

        return res;
    }

    ParseResult expression()
    {
        ParseResult res;

        auto ternary_node = res.register_node(ternary_expression());
        if (res.error)
            return res;

        auto node = res.register_node(res.success(ternary_node));
        if (res.error)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected int,float,identifier"));
        }
        return res.success(node);
    }

    ParseResult logical_expression()
    {
        ParseResult res;
        auto left_node = res.register_node(comp_expression());
        if (res.error)
            return res;

        while (current_tok.has_value() && current_tok->type == T_KEYWORD &&
               (any_cast<string>(current_tok->value) == "and" || any_cast<string>(current_tok->value) == "or"))
        {
            Token operator_tok = current_tok.value();
            res.register_advancement();
            advance();

            auto right_node = res.register_node(comp_expression());
            if (res.error)
                return res;

            left_node = make_shared<BinaryOperationNode>(left_node, operator_tok, right_node);
        }

        auto node = res.register_node(res.success(left_node));
        if (res.error)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected int,float,identifier"));
        }
        return res.success(node);
    }

    ParseResult comp_expression()
    {
        ParseResult res;

        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "not")
        {
            Token operator_token = current_tok.value();
            res.register_advancement();
            advance();

            auto node = res.register_node(comp_expression());
            if (res.error)
                return res;
            return res.success(make_shared<UnaryOperationNode>(operator_token, node));
        }

        auto left_node = res.register_node(arith_expression());
        if (res.error)
            return res;

        while (current_tok.has_value() && (current_tok->type == T_EE || current_tok->type == T_NEQ ||
                                           current_tok->type == T_LT || current_tok->type == T_GT ||
                                           current_tok->type == T_GTE || current_tok->type == T_LTE))
        {
            Token operator_tok = current_tok.value();
            res.register_advancement();
            advance();
            auto right_node = res.register_node(arith_expression());
            if (res.error)
                return res;
            left_node = make_shared<BinaryOperationNode>(left_node, operator_tok, right_node);
        }

        auto node = res.register_node(res.success(left_node));
        if (res.error)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected int,float,identifier,'+','-','not' or '('"));
        }
        return res.success(node);
    }

    ParseResult arith_expression()
    {
        ParseResult res;
        auto left_node = res.register_node(term());
        if (res.error)
            return res;

        while (current_tok.has_value() && (current_tok->type == T_PLUS || current_tok->type == T_MINUS))
        {
            Token op_token = current_tok.value();
            res.register_advancement();
            advance();
            auto right_node = res.register_node(term());
            if (res.error)
                return res;
            left_node = make_shared<BinaryOperationNode>(left_node, op_token, right_node);
        }

        auto node = res.register_node(res.success(left_node));
        if (res.error)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'define',int,float,identifier,'+','-' or '('"));
        }
        return res.success(node);
    }

    ParseResult dict_expression()
    {
        ParseResult res;
        vector<pair<shared_ptr<Node>, shared_ptr<Node>>> keyval_nodes;
        auto pos_start = current_tok->pos_start.value_or(Position());

        if (current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()), "Expected '{'"));
        }
        res.register_advancement();
        advance();

        if (current_tok.has_value() && current_tok->type == T_RPAREN2)
        {
            res.register_advancement();
            advance();
        }
        else
        {
            auto key_node = res.register_node(expression());
            if (res.error)
                return res;

            if (!current_tok.has_value() || current_tok->type != T_COLON)
            {
                return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected ':'"));
            }
            res.register_advancement();
            advance();

            auto value_node = res.register_node(expression());
            if (res.error)
                return res;

            keyval_nodes.push_back({key_node, value_node});

            while (current_tok.has_value() && current_tok->type == T_COMMA)
            {
                res.register_advancement();
                advance();

                key_node = res.register_node(expression());
                if (res.error)
                    return res;

                if (!current_tok.has_value() || current_tok->type != T_COLON)
                {
                    return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected ':'"));
                }
                res.register_advancement();
                advance();

                value_node = res.register_node(expression());
                if (res.error)
                    return res;

                keyval_nodes.push_back({key_node, value_node});
            }

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
            {
                return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected '}'"));
            }
            res.register_advancement();
            advance();
        }

        return res.success(make_shared<DictNode>(keyval_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position()));
    }

    ParseResult parse_block()
    {
        ParseResult res;
        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
        {
            if (current_tok->type == T_KEYWORD &&
                (any_cast<string>(current_tok->value) == "escape" ||
                 any_cast<string>(current_tok->value) == "proceed" ||
                 any_cast<string>(current_tok->value) == "yield"))
            {
                auto jump_node = res.register_node(jump_statements());
                if (res.error)
                    return res;
                body_nodes.push_back(jump_node);
            }
            else
            {
                auto multiline_node = res.try_register(multiline());
                if (res.error)
                    return res;
                if (!multiline_node)
                {
                    bool is_jump = current_tok.has_value() && current_tok->type == T_KEYWORD &&
                                   (any_cast<string>(current_tok->value) == "escape" ||
                                    any_cast<string>(current_tok->value) == "proceed" ||
                                    any_cast<string>(current_tok->value) == "yield");
                    if (!is_jump && current_tok.has_value() && current_tok->type != T_RPAREN2)
                    {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected identifier,when,during,method,cycle or risk"));
                    }
                }
                if (multiline_node)
                {
                    if (auto list_ptr = dynamic_pointer_cast<ListNode>(multiline_node))
                    {
                        body_nodes.insert(body_nodes.end(), list_ptr->element_nodes.begin(), list_ptr->element_nodes.end());
                    }
                }
            }
        }

        auto body_node = make_shared<ListNode>(body_nodes, pos_start, current_tok.has_value() ? current_tok->pos_end : nullopt);
        return res.success(body_node);
    }

    ParseResult catch_expression()
    {
        ParseResult res;
        if (!current_tok.has_value() || current_tok->type != T_KEYWORD || any_cast<string>(current_tok->value) != "trap")
        {
            return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected 'trap'"));
        }
        res.register_advancement();
        advance();

        optional<Token> error_type = nullopt;
        optional<Token> error_name = nullopt;

        if (current_tok.has_value() && current_tok->type == T_ERROR)
        {
            error_type = current_tok;
            res.register_advancement();
            advance();
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER)
            {
                error_name = current_tok;
                res.register_advancement();
                advance();
            }
        }

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected '{'"));
        }
        res.register_advancement();
        advance();

        auto body_node = res.register_node(parse_block());
        if (res.error)
            return res;

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected '}'"));
        }
        res.register_advancement();
        advance();

        return res.success(make_shared<CatchNode>(error_type, error_name, body_node));
    }

    ParseResult finally_expression()
    {
        ParseResult res;
        if (!current_tok.has_value() || current_tok->type != T_KEYWORD || any_cast<string>(current_tok->value) != "clean")
        {
            return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected 'clean'"));
        }
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected '{'"));
        }
        res.register_advancement();
        advance();

        auto body_node = res.register_node(parse_block());
        if (res.error)
            return res;

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected '}'"));
        }
        res.register_advancement();
        advance();

        return res.success(make_shared<FinallyNode>(body_node));
    }

    ParseResult try_expression()
    {
        ParseResult res;
        if (!current_tok.has_value() || current_tok->type != T_KEYWORD || any_cast<string>(current_tok->value) != "risk")
        {
            return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected 'risk'"));
        }
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected '{'"));
        }
        res.register_advancement();
        advance();

        auto body_node = res.register_node(parse_block());
        if (res.error)
            return res;

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected '}'"));
        }
        res.register_advancement();
        advance();

        return res.success(body_node);
    }

    ParseResult exception_handling()
    {
        ParseResult res;

        auto try_node = res.register_node(try_expression());
        if (res.error)
            return res;

        while (current_tok.has_value() && current_tok->type == T_NEWLINE)
        {
            res.register_advancement();
            advance();
        }

        vector<shared_ptr<CatchNode>> trap_nodes;
        shared_ptr<FinallyNode> clean_node = nullptr;

        while (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "trap")
        {
            auto trap_node = res.register_node(catch_expression());
            if (res.error)
                return res;

            trap_nodes.push_back(dynamic_pointer_cast<CatchNode>(trap_node));

            while (current_tok.has_value() && current_tok->type == T_NEWLINE)
            {
                res.register_advancement();
                advance();
            }
        }

        while (current_tok.has_value() && current_tok->type == T_NEWLINE)
        {
            res.register_advancement();
            advance();
        }

        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "clean")
        {
            auto cn = res.register_node(finally_expression());
            if (res.error)
                return res;
            clean_node = dynamic_pointer_cast<FinallyNode>(cn);
        }

        if (trap_nodes.empty() && !clean_node)
        {
            return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected 'trap' or 'clean' after 'risk'"));
        }

        return res.success(make_shared<TryNode>(try_node, trap_nodes, clean_node));
    }

    // ─── OOP Parsing Methods ──────────────────────────────────────────────────

    // Helper: given an already-parsed object node, chains DOT attr/method accesses.
    // Returns the final node (could be AttrAccessNode or FunctionCallNode wrapping one).
    shared_ptr<Node> dot_access_chain(ParseResult &res, shared_ptr<Node> obj_node)
    {
        while (current_tok.has_value() && current_tok->type == T_DOT)
        {
            res.register_advancement();
            advance(); // consume '.'

            if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
            {
                res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected attribute name after '.'"));
                return obj_node;
            }

            Token attr_tok = current_tok.value();
            res.register_advancement();
            advance(); // consume attr name

            // Build AttrAccessNode
            obj_node = make_shared<AttrAccessNode>(obj_node, attr_tok);

            // If followed by '(', it's a method call
            if (current_tok.has_value() && current_tok->type == T_LPAREN)
            {
                res.register_advancement();
                advance(); // consume '('

                vector<shared_ptr<Node>> arg_nodes;
                if (current_tok.has_value() && current_tok->type != T_RPAREN)
                {
                    arg_nodes.push_back(res.register_node(expression()));
                    if (res.error)
                        return obj_node;
                    while (current_tok.has_value() && current_tok->type == T_COMMA)
                    {
                        res.register_advancement();
                        advance();
                        arg_nodes.push_back(res.register_node(expression()));
                        if (res.error)
                            return obj_node;
                    }
                }
                if (!current_tok.has_value() || current_tok->type != T_RPAREN)
                {
                    res.failure(InvalidSyntaxError(
                        current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                        current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                        "Expected ')'"));
                    return obj_node;
                }
                res.register_advancement();
                advance();
                obj_node = make_shared<FunctionCallNode>(obj_node, arg_nodes);
            }
        }
        return obj_node;
    }

    ParseResult dot_access_statement()
    {
        ParseResult res;

        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected identifier"));
        }

        Token var_tok = current_tok.value();
        res.register_advancement();
        advance();

        shared_ptr<Node> obj_node = make_shared<VariableUseNode>(var_tok, vector<shared_ptr<Node>>());

        while (current_tok.has_value() && current_tok->type == T_DOT)
        {
            res.register_advancement();
            advance();

            if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected attribute name after '.'"));
            }

            Token attr_tok = current_tok.value();
            res.register_advancement();
            advance();

            bool has_more_dot = current_tok.has_value() && current_tok->type == T_DOT;

            if (current_tok.has_value() && current_tok->type == T_LPAREN)
            {
                auto attr_access = make_shared<AttrAccessNode>(obj_node, attr_tok);
                res.register_advancement();
                advance();

                vector<shared_ptr<Node>> arg_nodes;
                if (current_tok.has_value() && current_tok->type != T_RPAREN)
                {
                    arg_nodes.push_back(res.register_node(expression()));
                    if (res.error)
                        return res;
                    while (current_tok.has_value() && current_tok->type == T_COMMA)
                    {
                        res.register_advancement();
                        advance();
                        arg_nodes.push_back(res.register_node(expression()));
                        if (res.error)
                            return res;
                    }
                }
                if (!current_tok.has_value() || current_tok->type != T_RPAREN)
                {
                    return res.failure(InvalidSyntaxError(
                        current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                        current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                        "Expected ')'"));
                }
                res.register_advancement();
                advance();
                obj_node = make_shared<FunctionCallNode>(attr_access, arg_nodes);
            }
            else if (current_tok.has_value() && current_tok->type == T_EQ && !has_more_dot)
            {
                res.register_advancement();
                advance();
                auto value_node = res.register_node(expression());
                if (res.error)
                    return res;
                return res.success(make_shared<AttrAssignNode>(obj_node, attr_tok, value_node));
            }
            else
            {
                obj_node = make_shared<AttrAccessNode>(obj_node, attr_tok);
            }
        }

        return res.success(obj_node);
    }

    ParseResult model_definition()
    {
        ParseResult res;

        if (!current_tok.has_value() || current_tok->type != T_KEYWORD ||
            any_cast<string>(current_tok->value) != "model")
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'model'"));
        }
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected a model name"));
        }
        Token model_name_tok = current_tok.value();
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"));
        }
        res.register_advancement();
        advance();

        while (current_tok.has_value() && current_tok->type == T_NEWLINE)
        {
            res.register_advancement();
            advance();
        }

        vector<shared_ptr<Node>> body_nodes;
        while (current_tok.has_value() && current_tok->type != T_RPAREN2)
        {
            auto member = res.register_node(class_member());
            if (res.error)
                return res;
            body_nodes.push_back(member);

            while (current_tok.has_value() && current_tok->type == T_NEWLINE)
            {
                res.register_advancement();
                advance();
            }
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }
        res.register_advancement();
        advance();

        return res.success(make_shared<ModelNode>(model_name_tok, body_nodes));
    }

    ParseResult class_member()
    {
        ParseResult res;
        if (!current_tok.has_value())
        {
            return res.failure(InvalidSyntaxError(Position(), Position(), "Unexpected end of input"));
        }

        if (current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "attr")
        {
            return attr_declaration();
        }
        if (current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "init")
        {
            return constructor_definition();
        }
        if (current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "method")
        {
            return function_definition();
        }

        return res.failure(InvalidSyntaxError(
            current_tok->pos_start.value_or(Position()),
            current_tok->pos_end.value_or(Position()),
            "Expected 'attr', 'init', or 'method'"));
    }

    ParseResult attr_declaration()
    {
        ParseResult res;
        Position pos_start = current_tok->pos_start.value_or(Position());

        if (!current_tok.has_value() || current_tok->type != T_KEYWORD ||
            any_cast<string>(current_tok->value) != "attr")
        {
            return res.failure(InvalidSyntaxError(pos_start, pos_start, "Expected 'attr'"));
        }
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_LT)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '<'"));
        }
        res.register_advancement();
        advance();

        vector<AttrDecl> declarations;

        auto parse_one_attr = [&]() -> bool
        {
            if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
            {
                res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected attribute name"));
                return false;
            }
            Token name_tok = current_tok.value();
            res.register_advancement();
            advance();

            shared_ptr<Node> default_node = nullptr;
            if (current_tok.has_value() && current_tok->type == T_EQ)
            {
                res.register_advancement();
                advance();
                default_node = res.register_node(expression());
                if (res.error)
                    return false;
            }
            declarations.push_back({name_tok, default_node});
            return true;
        };

        if (!parse_one_attr())
            return res;

        while (current_tok.has_value() && current_tok->type == T_COMMA)
        {
            res.register_advancement();
            advance();
            if (!parse_one_attr())
                return res;
        }

        if (!current_tok.has_value() || current_tok->type != T_GT)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '>'"));
        }
        Position pos_end = current_tok->pos_end.value_or(Position());
        res.register_advancement();
        advance();

        return res.success(make_shared<AttrNode>(declarations, pos_start, pos_end));
    }

    ParseResult constructor_definition()
    {
        ParseResult res;
        Position pos_start = current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position();

        if (!current_tok.has_value() || current_tok->type != T_KEYWORD ||
            any_cast<string>(current_tok->value) != "init")
        {
            return res.failure(InvalidSyntaxError(pos_start, pos_start, "Expected 'init'"));
        }
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_LPAREN)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '('"));
        }
        res.register_advancement();
        advance();

        vector<Token> param_toks;
        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER)
        {
            param_toks.push_back(current_tok.value());
            res.register_advancement();
            advance();
            while (current_tok.has_value() && current_tok->type == T_COMMA)
            {
                res.register_advancement();
                advance();
                if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
                {
                    return res.failure(InvalidSyntaxError(
                        current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                        current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                        "Expected identifier"));
                }
                param_toks.push_back(current_tok.value());
                res.register_advancement();
                advance();
            }
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected ','" + string(" or ')'")));
        }
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'"));
        }
        res.register_advancement();
        advance();

        while (current_tok.has_value() && current_tok->type == T_NEWLINE)
        {
            res.register_advancement();
            advance();
        }

        vector<shared_ptr<Node>> body_nodes;

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER &&
            peek().has_value() && peek()->type == T_COLON)
        {
            auto initializers = res.register_node(initializer_list());
            if (res.error)
                return res;
            if (auto list_node = dynamic_pointer_cast<ListNode>(initializers))
            {
                body_nodes.insert(body_nodes.end(), list_node->element_nodes.begin(), list_node->element_nodes.end());
            }
        }

        auto block = res.register_node(parse_block());
        if (res.error)
            return res;
        if (auto list_node = dynamic_pointer_cast<ListNode>(block))
        {
            body_nodes.insert(body_nodes.end(), list_node->element_nodes.begin(), list_node->element_nodes.end());
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }
        Position pos_end = current_tok->pos_end.value_or(Position());
        res.register_advancement();
        advance();

        auto body_node = make_shared<ListNode>(body_nodes, pos_start, pos_end);
        return res.success(make_shared<InitNode>(param_toks, body_node, pos_start, pos_end));
    }

    ParseResult initializer_list()
    {
        ParseResult res;
        vector<shared_ptr<Node>> initializers;

        auto first = res.register_node(initializer_item());
        if (res.error)
            return res;
        initializers.push_back(first);

        while (true)
        {
            int saved_tok_idx = tok_index;
            bool separator_found = false;

            if (current_tok.has_value() && current_tok->type == T_COMMA)
            {
                separator_found = true;
                advance();
                while (current_tok.has_value() && current_tok->type == T_NEWLINE)
                    advance();
            }
            else if (current_tok.has_value() && current_tok->type == T_NEWLINE)
            {
                separator_found = true;
                while (current_tok.has_value() && current_tok->type == T_NEWLINE)
                    advance();
            }

            if (!separator_found)
                break;

            if (!(current_tok.has_value() && current_tok->type == T_IDENTIFIER &&
                  peek().has_value() && peek()->type == T_COLON))
            {
                tok_index = saved_tok_idx;
                update_current_tok();
                break;
            }

            int advances = tok_index - saved_tok_idx;
            for (int i = 0; i < advances; ++i)
                res.register_advancement();

            auto item = res.register_node(initializer_item());
            if (res.error)
                return res;
            initializers.push_back(item);
        }

        return res.success(make_shared<ListNode>(initializers, nullopt, nullopt));
    }

    ParseResult initializer_item()
    {
        ParseResult res;

        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected identifier"));
        }

        Token var_name_tok = current_tok.value();
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_COLON)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected ':'"));
        }
        res.register_advancement();
        advance();

        auto value_node = res.register_node(expression());
        if (res.error)
            return res;

        vector<Token> vars = {var_name_tok};
        vector<shared_ptr<Node>> vals = {value_node};
        vector<vector<shared_ptr<Node>>> idxs = {{}};
        return res.success(make_shared<VariableAssignNode>(vars, vals, idxs));
    }
};
