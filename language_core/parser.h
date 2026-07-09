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
#include "../ast_nodes/fstring_nodes.h"
#include "../ast_nodes/foreach_nodes.h"
#include "../ast_nodes/summon_nodes.h"
#include "../ast_nodes/comprehension_nodes.h"
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

    int current_depth = 0;
    int in_method_count = 0;

    struct MethodCountGuard
    {
        int& counter;
        explicit MethodCountGuard(int& cnt) : counter(cnt) { counter++; }
        ~MethodCountGuard() { counter--; }
    };

    struct ParserDepthGuard
    {
        int& depth;
        explicit ParserDepthGuard(int& d) : depth(d) { depth++; }
        ~ParserDepthGuard() { depth--; }
    };

    ParseResult _check_depth()
    {
        ParseResult res;
        if (current_depth > MAX_AST_DEPTH)
        {
            Position start = current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position();
            Position end = current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position();
            res.is_fatal = true;
            return res.failure(InvalidSyntaxError(
                start, end,
                "Expression is too complex (maximum nesting depth of " + to_string(MAX_AST_DEPTH) + " exceeded)"));
        }
        return res;
    }

    ParseResult _parse_list_comp_cycle(ParseResult& res, shared_ptr<Node> expr_node, Position pos_start)
    {
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected loop variable name after 'cycle'"));
        }
        Token var_name_tok = current_tok.value();
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

        auto start_node = res.register_node(expression());
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_COLON)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected ':'"));
        }
        res.register_advancement();
        advance();

        auto end_node = res.register_node(expression());
        if (res.error) return res;

        shared_ptr<Node> step_node = nullptr;
        if (current_tok.has_value() && current_tok->type == T_COLON)
        {
            res.register_advancement();
            advance();
            step_node = res.register_node(expression());
            if (res.error) return res;
        }

        shared_ptr<Node> condition_node = nullptr;
        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "when")
        {
            res.register_advancement();
            advance();
            condition_node = res.register_node(expression());
            if (res.error) return res;
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN3)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected ']' to close list comprehension"));
        }
        auto pos_end = current_tok->pos_end.value_or(Position());
        res.register_advancement();
        advance();

        return res.success(make_shared<ListComprehensionNode>(
            expr_node, "cycle", var_name_tok, start_node, end_node, step_node,
            vector<Token>{}, nullptr, condition_node, pos_start, pos_end
        ));
    }

    ParseResult _parse_list_comp_trace(ParseResult& res, shared_ptr<Node> expr_node, Position pos_start)
    {
        res.register_advancement();
        advance();

        vector<Token> var_name_tokens;
        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected at least one variable name after 'trace'"));
        }
        var_name_tokens.push_back(current_tok.value());
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
                    "Expected identifier after ','"));
            }
            var_name_tokens.push_back(current_tok.value());
            res.register_advancement();
            advance();
        }

        if (!current_tok.has_value() || current_tok->type != T_LARROW)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '<-'"));
        }
        res.register_advancement();
        advance();

        auto collection_node = res.register_node(expression());
        if (res.error) return res;

        shared_ptr<Node> condition_node = nullptr;
        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "when")
        {
            res.register_advancement();
            advance();
            condition_node = res.register_node(expression());
            if (res.error) return res;
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN3)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected ']' to close list comprehension"));
        }
        auto pos_end = current_tok->pos_end.value_or(Position());
        res.register_advancement();
        advance();

        return res.success(make_shared<ListComprehensionNode>(
            expr_node, "trace", nullopt, nullptr, nullptr, nullptr,
            var_name_tokens, collection_node, condition_node, pos_start, pos_end
        ));
    }

    ParseResult _parse_dict_comp_cycle(ParseResult& res, shared_ptr<Node> key_node, shared_ptr<Node> val_node, Position pos_start)
    {
        res.register_advancement();
        advance();

        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected loop variable name after 'cycle'"));
        }
        Token var_name_tok = current_tok.value();
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

        auto start_node = res.register_node(expression());
        if (res.error) return res;

        if (!current_tok.has_value() || current_tok->type != T_COLON)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected ':'"));
        }
        res.register_advancement();
        advance();

        auto end_node = res.register_node(expression());
        if (res.error) return res;

        shared_ptr<Node> step_node = nullptr;
        if (current_tok.has_value() && current_tok->type == T_COLON)
        {
            res.register_advancement();
            advance();
            step_node = res.register_node(expression());
            if (res.error) return res;
        }

        shared_ptr<Node> condition_node = nullptr;
        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "when")
        {
            res.register_advancement();
            advance();
            condition_node = res.register_node(expression());
            if (res.error) return res;
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}' to close dict comprehension"));
        }
        auto pos_end = current_tok->pos_end.value_or(Position());
        res.register_advancement();
        advance();

        return res.success(make_shared<DictComprehensionNode>(
            key_node, val_node, "cycle", var_name_tok, start_node, end_node, step_node,
            vector<Token>{}, nullptr, condition_node, pos_start, pos_end
        ));
    }

    ParseResult _parse_dict_comp_trace(ParseResult& res, shared_ptr<Node> key_node, shared_ptr<Node> val_node, Position pos_start)
    {
        res.register_advancement();
        advance();

        vector<Token> var_name_tokens;
        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected at least one variable name after 'trace'"));
        }
        var_name_tokens.push_back(current_tok.value());
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
                    "Expected identifier after ','"));
            }
            var_name_tokens.push_back(current_tok.value());
            res.register_advancement();
            advance();
        }

        if (!current_tok.has_value() || current_tok->type != T_LARROW)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '<-'"));
        }
        res.register_advancement();
        advance();

        auto collection_node = res.register_node(expression());
        if (res.error) return res;

        shared_ptr<Node> condition_node = nullptr;
        if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "when")
        {
            res.register_advancement();
            advance();
            condition_node = res.register_node(expression());
            if (res.error) return res;
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}' to close dict comprehension"));
        }
        auto pos_end = current_tok->pos_end.value_or(Position());
        res.register_advancement();
        advance();

        return res.success(make_shared<DictComprehensionNode>(
            key_node, val_node, "trace", nullopt, nullptr, nullptr, nullptr,
            var_name_tokens, collection_node, condition_node, pos_start, pos_end
        ));
    }


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

    optional<ParseResult> check_statement_separation(ParseResult& res, const vector<shared_ptr<Node>>& parsed_list)
    {
        if (!current_tok.has_value())
            return nullopt;
        if (current_tok->type == T_NEWLINE || current_tok->type == T_RPAREN2 || current_tok->type == T_EOF)
            return nullopt;
        if (!parsed_list.empty() && tok_index > 0 && tokens[tok_index - 1].type != T_NEWLINE)
        {
            string msg = "Expected newline to separate statements";
            if (current_tok->type == T_KEYWORD &&
                (any_cast<string>(current_tok->value) == "yield" ||
                 any_cast<string>(current_tok->value) == "escape" ||
                 any_cast<string>(current_tok->value) == "proceed"))
            {
                msg += ". Jump statements (like '" + any_cast<string>(current_tok->value) + "') must be on a new line.";
            }
            res.failure(InvalidSyntaxError(
                current_tok->pos_start.value_or(Position()),
                current_tok->pos_end.value_or(Position()),
                msg
            ));
            return res;
        }
        return nullopt;
    }


    ParseResult multiline()
    {
        ParseResult res;
        ParserDepthGuard guard(current_depth);
        auto depth_res = _check_depth();
        if (depth_res.error) return depth_res;

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
            int newlines_count = 0;
            while (current_tok.has_value() && current_tok->type == T_NEWLINE)
            {
                res.register_advancement();
                advance();
                newlines_count++;
            }
            if (current_tok.has_value() && current_tok->type != T_EOF && current_tok->type != T_RPAREN2 &&
                !(current_tok->type == T_KEYWORD &&
                  (any_cast<string>(current_tok->value) == "yield" ||
                   any_cast<string>(current_tok->value) == "escape" ||
                   any_cast<string>(current_tok->value) == "proceed")))
            {
                if (newlines_count == 0 && !statements_list.empty())
                {
                    auto prev_pos_end = statements_list.back()->pos_end;
                    auto curr_pos_start = current_tok->pos_start;
                    if (prev_pos_end.has_value() && curr_pos_start.has_value() && prev_pos_end->line == curr_pos_start->line)
                    {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected newline to separate statements"
                        ));
                    }
                }
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

        return res.success(make_shared<ListNode>(statements_list, pos_start, pos_end, true));
    }

    ParseResult singleline()
    {
        ParseResult res;
        ParserDepthGuard guard(current_depth);
        auto depth_res = _check_depth();
        if (depth_res.error) return depth_res;

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
        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "trace")
        {
            auto foreach_expr = res.register_node(foreach_expression());
            if (res.error)
                return res;
            return res.success(foreach_expr);
        }
        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "summon")
        {
            auto summon_stmt = res.register_node(summon_statement());
            if (res.error)
                return res;
            return res.success(summon_stmt);
        }
        if (token.type == T_KEYWORD && (any_cast<string>(token.value) == "escape" || any_cast<string>(token.value) == "proceed" || any_cast<string>(token.value) == "yield"))
        {
            auto jump_node = res.register_node(jump_statements());
            if (res.error)
                return res;
            return res.success(jump_node);
        }

        if (token.type == T_IDENTIFIER)
        {
            auto next_tok = peek();
            if (next_tok.has_value() && (next_tok->type == T_IDENTIFIER || next_tok->type == T_LPAREN2 || next_tok->type == T_INT || next_tok->type == T_FLOAT || next_tok->type == T_STRING))
            {
                return res.failure(InvalidSyntaxError(
                    current_tok->pos_start.value_or(Position()), current_tok->pos_end.value_or(Position()),
                    "Invalid keyword '" + any_cast<string>(token.value) + "'"
                ));
            }
        }

        // Try assignment first
        int saved_tok_index = tok_index;

        auto stmt = res.try_register(statements());
        if (stmt)
        {
            return res.success(stmt);
        }

        // Backtrack
        tok_index = saved_tok_index;
        update_current_tok();
        res.error = nullptr;

        // Fallback to expression
        auto expr = res.register_node(expression());
        if (res.error)
        {
            string val_str = "";
            if (current_tok.has_value()) {
                if (current_tok->value.has_value()) {
                    if (current_tok->value.type() == typeid(string)) {
                        val_str = any_cast<string>(current_tok->value);
                    } else if (current_tok->value.type() == typeid(long long)) {
                        val_str = to_string(any_cast<long long>(current_tok->value));
                    } else if (current_tok->value.type() == typeid(double)) {
                        val_str = to_string(any_cast<double>(current_tok->value));
                    }
                } else {
                    val_str = current_tok->type;
                }
            }
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Unexpected token '" + val_str + "'"
            ));
        }
        return res.success(expr);
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
            auto first_expr = res.register_node(expression());
            if (res.error)
                return res;

            if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "cycle")
            {
                return _parse_list_comp_cycle(res, first_expr, pos_start.value_or(Position()));
            }

            if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "trace")
            {
                return _parse_list_comp_trace(res, first_expr, pos_start.value_or(Position()));
            }

            element_nodes.push_back(first_expr);

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
                    "Expected ',' or ']'", "Did you forget a comma ',' between list elements?"));
            }

            pos_end = current_tok->pos_end;
            res.register_advancement();
            advance();
        }

        return res.success(make_shared<ListNode>(element_nodes, pos_start, pos_end));
    }

    ParseResult function_definition(const string &access_mod = "open")
    {
        ParseResult res;
        optional<Token> var_name_tok = nullopt;
        vector<pair<Token, shared_ptr<Node>>> arg_nodes;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "method"))
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'method'"));
        }
        res.register_advancement();
        advance();

        MethodCountGuard guard(in_method_count);

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

        bool has_seen_default = false;

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER)
        {
            Token param_tok = current_tok.value();
            res.register_advancement();
            advance();

            shared_ptr<Node> default_val = nullptr;
            if (current_tok.has_value() && current_tok->type == T_EQ)
            {
                res.register_advancement();
                advance();
                default_val = res.register_node(expression());
                if (res.error) return res;
                has_seen_default = true;
            }

            arg_nodes.push_back({param_tok, default_val});

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
                Token next_param_tok = current_tok.value();
                res.register_advancement();
                advance();

                shared_ptr<Node> next_default_val = nullptr;
                if (current_tok.has_value() && current_tok->type == T_EQ)
                {
                    res.register_advancement();
                    advance();
                    next_default_val = res.register_node(expression());
                    if (res.error) return res;
                    has_seen_default = true;
                }
                else if (has_seen_default)
                {
                    return res.failure(InvalidSyntaxError(
                        next_param_tok.pos_start.value_or(Position()),
                        next_param_tok.pos_end.value_or(Position()),
                        "Non-default argument follows default argument"));
                }

                arg_nodes.push_back({next_param_tok, next_default_val});
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
            auto sep_err = check_statement_separation(res, body_nodes);
            if (sep_err)
                return *sep_err;

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

        auto body_node = make_body_node(body_nodes, pos_start);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }
        res.register_advancement();
        advance();

        auto node = make_shared<FunctionDefinitionNode>(var_name_tok, arg_nodes, body_node, true, access_mod);
        node->pos_end = get_last_parsed_pos_end();
        return res.success(node);
    }

    ParseResult parse_arguments(vector<shared_ptr<Node>>& positional_args, vector<pair<Token, shared_ptr<Node>>>& keyword_args, optional<Position>& out_rparen_pos)
    {
        ParseResult res;
        positional_args.clear();
        keyword_args.clear();
        out_rparen_pos = nullopt;

        if (!current_tok.has_value() || current_tok->type != T_LPAREN)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '('"));
        }
        res.register_advancement();
        advance(); // consume '('

        if (current_tok.has_value() && current_tok->type == T_RPAREN)
        {
            out_rparen_pos = current_tok->pos_end;
            res.register_advancement();
            advance(); // consume ')'
            return res.success(nullptr);
        }

        bool parsing_keywords = false;

        while (true)
        {
            if (current_tok.has_value() && current_tok->type == T_IDENTIFIER && peek().has_value() && peek()->type == T_EQ)
            {
                parsing_keywords = true;
                Token name_tok = current_tok.value();
                res.register_advancement();
                advance(); // consume identifier

                res.register_advancement();
                advance(); // consume '='

                auto val_node = res.register_node(expression());
                if (res.error) return res;

                keyword_args.push_back({name_tok, val_node});
            }
            else
            {
                if (parsing_keywords)
                {
                    return res.failure(InvalidSyntaxError(
                        current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                        current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                        "Positional argument cannot follow keyword argument"));
                }

                auto val_node = res.register_node(expression());
                if (res.error) return res;

                positional_args.push_back(val_node);
            }

            if (current_tok.has_value() && current_tok->type == T_COMMA)
            {
                res.register_advancement();
                advance(); // consume ','

                if (current_tok.has_value() && current_tok->type == T_RPAREN)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if (!current_tok.has_value() || current_tok->type != T_RPAREN)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected ',' or ')'"));
        }
        out_rparen_pos = current_tok->pos_end;
        res.register_advancement();
        advance(); // consume ')'

        return res.success(nullptr);
    }

    ParseResult function_call()
    {
        ParseResult res;
        vector<shared_ptr<Node>> pos_args;
        vector<pair<Token, shared_ptr<Node>>> kw_args;
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

        optional<Position> rparen_pos = nullopt;
        res.register_node(parse_arguments(pos_args, kw_args, rparen_pos));
        if (res.error)
            return res;

        return res.success(make_shared<FunctionCallNode>(call_node, pos_args, kw_args, rparen_pos));
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

        auto node = make_shared<SwitchNode>(selection, cases, false);
        node->pos_end = get_last_parsed_pos_end();
        return res.success(node);
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

        if (current_tok.has_value() && current_tok->type == T_LPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok->pos_start.value_or(Position()),
                current_tok->pos_end.value_or(Position()),
                "Expected expression after 'choice'"));
        }

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

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
        {
            auto sep_err = check_statement_separation(res, body_nodes);
            if (sep_err)
                return *sep_err;

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
                    if (current_tok.has_value() &&
                        !(current_tok->type == T_KEYWORD &&
                          (any_cast<string>(current_tok->value) == "escape" ||
                           any_cast<string>(current_tok->value) == "proceed" ||
                           any_cast<string>(current_tok->value) == "yield")) &&
                        current_tok->type != T_RPAREN2)
                    {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected statement or '}'"
                        ));
                    }
                }
                if (multiline_node)
                {
                    if (auto lst = dynamic_pointer_cast<ListNode>(multiline_node))
                    {
                        for (const auto& item : lst->element_nodes)
                        {
                            body_nodes.push_back(item);
                        }
                    }
                    else
                    {
                        body_nodes.push_back(multiline_node);
                    }
                }
            }
        }

        auto body_node = make_body_node(body_nodes, pos_start);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }
        res.register_advancement();
        advance();

        auto node = make_shared<SwitchCaseNode>(choice_val, body_node, true);
        node->pos_end = get_last_parsed_pos_end();
        return res.success(node);
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

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
        {
            auto sep_err = check_statement_separation(res, body_nodes);
            if (sep_err)
                return *sep_err;

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
                    if (current_tok.has_value() &&
                        !(current_tok->type == T_KEYWORD &&
                          (any_cast<string>(current_tok->value) == "escape" ||
                           any_cast<string>(current_tok->value) == "proceed" ||
                           any_cast<string>(current_tok->value) == "yield")) &&
                        current_tok->type != T_RPAREN2)
                    {
                        return res.failure(InvalidSyntaxError(
                            current_tok->pos_start.value_or(Position()),
                            current_tok->pos_end.value_or(Position()),
                            "Expected statement or '}'"
                        ));
                    }
                }
                if (multiline_node)
                {
                    if (auto lst = dynamic_pointer_cast<ListNode>(multiline_node))
                    {
                        for (const auto& item : lst->element_nodes)
                        {
                            body_nodes.push_back(item);
                        }
                    }
                    else
                    {
                        body_nodes.push_back(multiline_node);
                    }
                }
            }
        }

        auto body_node = make_body_node(body_nodes, pos_start);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }
        res.register_advancement();
        advance();

        auto node = make_shared<SwitchCaseNode>(nullptr, body_node, true);
        node->pos_end = get_last_parsed_pos_end();
        return res.success(node);
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

        if (dynamic_pointer_cast<VariableAssignNode>(condition))
        {
            return res.failure(InvalidSyntaxError(
                condition->pos_start.value_or(Position()), condition->pos_end.value_or(Position()),
                "Assignment is not allowed in a 'during' condition",
                "Did you mean '==' for comparison instead of '=' for assignment?"));
        }

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            string hint = "";
            if (current_tok.has_value() && current_tok->type == T_EQ)
            {
                hint = "Did you mean '==' for comparison instead of '=' for assignment?";
            }
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'", hint));
        }
        int brace_open_line = current_tok->pos_start.value_or(Position()).line + 1;
        res.register_advancement();
        advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
        {
            auto sep_err = check_statement_separation(res, body_nodes);
            if (sep_err)
                return *sep_err;

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

        auto body_node = make_body_node(body_nodes, pos_start);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'",
                "Unexpected end of file. You opened a block '{' on line " + to_string(brace_open_line) + " that was never closed."));
        }
        res.register_advancement();
        advance();

        auto node = make_shared<WhileNode>(condition, body_node, true);
        node->pos_end = get_last_parsed_pos_end();
        return res.success(node);
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
            auto sep_err = check_statement_separation(res, body_nodes);
            if (sep_err)
                return *sep_err;

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

        auto body_node = make_body_node(body_nodes, pos_start);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }
        res.register_advancement();
        advance();

        auto node = make_shared<ForNode>(var_name, start_value, end_value, step_value, body_node, true);
        node->pos_end = get_last_parsed_pos_end();
        return res.success(node);
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

        if (dynamic_pointer_cast<VariableAssignNode>(condition))
        {
            return res.failure(InvalidSyntaxError(
                condition->pos_start.value_or(Position()), condition->pos_end.value_or(Position()),
                "Assignment is not allowed in a 'when' condition",
                "Did you mean '==' for comparison instead of '=' for assignment?"));
        }

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            string hint = "";
            if (current_tok.has_value() && current_tok->type == T_EQ)
            {
                hint = "Did you mean '==' for comparison instead of '=' for assignment?";
            }
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'", hint));
        }
        int brace_open_line = current_tok->pos_start.value_or(Position()).line + 1;
        res.register_advancement();
        advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
        {
            auto sep_err = check_statement_separation(res, body_nodes);
            if (sep_err)
                return *sep_err;

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

        auto body_node = make_body_node(body_nodes, pos_start);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'",
                "Unexpected end of file. You opened a block '{' on line " + to_string(brace_open_line) + " that was never closed."));
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

        auto node = make_shared<IfNode>(cases, else_case);
        node->pos_end = get_last_parsed_pos_end();
        return res.success(node);
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

        if (dynamic_pointer_cast<VariableAssignNode>(condition))
        {
            return res.failure(InvalidSyntaxError(
                condition->pos_start.value_or(Position()), condition->pos_end.value_or(Position()),
                "Assignment is not allowed in an 'orwhen' condition",
                "Did you mean '==' for comparison instead of '=' for assignment?"));
        }

        if (!current_tok.has_value() || current_tok->type != T_LPAREN2)
        {
            string hint = "";
            if (current_tok.has_value() && current_tok->type == T_EQ)
            {
                hint = "Did you mean '==' for comparison instead of '=' for assignment?";
            }
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '{'", hint));
        }
        int brace_open_line = current_tok->pos_start.value_or(Position()).line + 1;
        res.register_advancement();
        advance();

        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
        {
            auto sep_err = check_statement_separation(res, body_nodes);
            if (sep_err)
                return *sep_err;

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

        auto body_node = make_body_node(body_nodes, pos_start);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'",
                "Unexpected end of file. You opened a block '{' on line " + to_string(brace_open_line) + " that was never closed."));
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
                auto sep_err = check_statement_separation(res, body_nodes);
                if (sep_err)
                    return *sep_err;

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

            auto body_node = make_body_node(body_nodes, pos_start);

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

        if (token.type == T_PLUS || token.type == T_MINUS || token.type == T_BITNOT)
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
            auto num_node = make_shared<NumberNode>(token);
            return res.success(dot_access_chain(res, num_node));
        }

        if (token.type == T_STRING)
        {
            res.register_advancement();
            advance();
            auto str_node = make_shared<StringNode>(token);
            return res.success(dot_access_chain(res, str_node));
        }

        if (token.type == T_FSTRING)
        {
            res.register_advancement();
            advance();
            auto fstring_node = res.register_node(_parse_fstring(token));
            if (res.error)
                return res;
            return res.success(dot_access_chain(res, fstring_node));
        }

        if (token.type == T_KEYWORD && any_cast<string>(token.value) == "method")
        {
            auto method_expr = res.register_node(function_definition());
            if (res.error)
                return res;
            return res.success(dot_access_chain(res, method_expr));
        }

        if (token.type == T_IDENTIFIER && peek().has_value() && peek()->type == T_LPAREN)
        {
            auto call_expression = res.register_node(function_call());
            if (res.error)
                return res;
            return res.success(dot_access_chain(res, call_expression));
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

            auto obj_node = make_shared<VariableUseNode>(var_name_tok, index_node);
            return res.success(dot_access_chain(res, obj_node));
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
            return res.success(dot_access_chain(res, expr));
        }

        if (token.type == T_LPAREN3)
        {
            auto list_expr = res.register_node(list_expression());
            if (res.error)
                return res;
            return res.success(dot_access_chain(res, list_expr));
        }

        if (token.type == T_LPAREN2)
        {
            auto dict_expr = res.register_node(dict_expression());
            if (res.error)
                return res;
            return res.success(dot_access_chain(res, dict_expr));
        }

        return res.failure(InvalidSyntaxError(
            token.pos_start.value_or(Position()),
            token.pos_end.value_or(Position()),
            "Expected an expression (value, variable, '(', '[', '{', or operator)"));
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
            if (left_node->depth > MAX_AST_DEPTH) {
                return res.failure(InvalidSyntaxError(
                    left_node->pos_start.value_or(Position()),
                    left_node->pos_end.value_or(Position()),
                    "Expression is too complex (maximum AST depth exceeded)"
                ));
            }
        }

        return res.success(left_node);
    }

    ParseResult statements()
    {
        ParseResult res;
        vector<shared_ptr<Node>> left_nodes;

        while (true)
        {
            auto left_node = res.register_node(factor()); // factor() parses variables, calls, indices, dots
            if (res.error)
                return res;

            if (!dynamic_pointer_cast<VariableUseNode>(left_node) &&
                !dynamic_pointer_cast<AttrAccessNode>(left_node) &&
                !dynamic_pointer_cast<IndexAccessNode>(left_node))
            {
                return res.failure(InvalidSyntaxError(
                    left_node->pos_start.value_or(Position()),
                    left_node->pos_end.value_or(Position()),
                    "Expected variable, attribute, or index for assignment"
                ));
            }

            left_nodes.push_back(left_node);

            if (!current_tok.has_value() || current_tok->type != T_COMMA)
            {
                break;
            }
            res.register_advancement();
            advance();
        }

        optional<Token> op = nullopt;
        if (!current_tok.has_value() || current_tok->type != T_EQ)
        {
            if (current_tok.has_value() &&
                (current_tok->type == T_PLUSEQUAL || current_tok->type == T_MINUSEQUAL ||
                 current_tok->type == T_MULEQUAL || current_tok->type == T_DIVIDEEQUAL ||
                 current_tok->type == T_MODULUSEQUAL || current_tok->type == T_FLOOREQUAL ||
                 current_tok->type == T_EXPEQUAL || current_tok->type == T_BITANDEQUAL ||
                 current_tok->type == T_BITXOREQUAL || current_tok->type == T_BITOREQUAL ||
                 current_tok->type == T_LSHIFTEQUAL || current_tok->type == T_RSHIFTEQUAL))
            {
                string op_type;
                if (current_tok->type == T_PLUSEQUAL) op_type = T_PLUS;
                else if (current_tok->type == T_MINUSEQUAL) op_type = T_MINUS;
                else if (current_tok->type == T_MULEQUAL) op_type = T_MUL;
                else if (current_tok->type == T_DIVIDEEQUAL) op_type = T_DIVIDE;
                else if (current_tok->type == T_MODULUSEQUAL) op_type = T_MODULUS;
                else if (current_tok->type == T_FLOOREQUAL) op_type = T_FLOOR;
                else if (current_tok->type == T_EXPEQUAL) op_type = T_EXP;
                else if (current_tok->type == T_BITANDEQUAL) op_type = T_BITAND;
                else if (current_tok->type == T_BITXOREQUAL) op_type = T_BITXOR;
                else if (current_tok->type == T_BITOREQUAL) op_type = T_BITOR;
                else if (current_tok->type == T_LSHIFTEQUAL) op_type = T_LSHIFT;
                else if (current_tok->type == T_RSHIFTEQUAL) op_type = T_RSHIFT;

                op = Token(op_type, {}, current_tok->pos_start, current_tok->pos_end);
            }
            else
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected '=' or '+=' or '-=' or '*=' or '/=' or '%=' or '//=' or '**=' or '&=' or '^=' or '|=' or '<<=' or '>>='"));
            }
        }
        res.register_advancement();
        advance();

        vector<shared_ptr<Node>> value_nodes;
        shared_ptr<Node> val1 = nullptr;
        while (true)
        {
            auto expr = res.register_node(expression());
            if (res.error)
                return res;

            if (!op.has_value())
            {
                value_nodes.push_back(expr);
            }
            else
            {
                val1 = expr;
                size_t idx = value_nodes.size();
                if (idx < left_nodes.size())
                {
                    auto bin_op = make_shared<BinaryOperationNode>(left_nodes[idx], op.value(), expr);
                    value_nodes.push_back(bin_op);
                }
                else
                {
                    value_nodes.push_back(expr);
                }
            }

            if (!current_tok.has_value() || current_tok->type != T_COMMA)
            {
                break;
            }
            res.register_advancement();
            advance();
        }

        if (left_nodes.size() != value_nodes.size())
        {
            if (op.has_value() && val1 && value_nodes.size() == 1)
            {
                for (size_t i = 1; i < left_nodes.size(); ++i)
                {
                    auto bin_op = make_shared<BinaryOperationNode>(left_nodes[i], op.value(), val1);
                    value_nodes.push_back(bin_op);
                }
            }
        }

        return res.success(make_shared<VariableAssignNode>(left_nodes, value_nodes));
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
            auto end_pos = current_tok->pos_end.value_or(Position());
            res.register_advancement();
            advance();
            return res.success(make_shared<ContinueNode>(start_pos, end_pos));
        }

        if (any_cast<string>(current_tok->value) == "escape")
        {
            auto start_pos = current_tok->pos_start.value_or(Position());
            auto end_pos = current_tok->pos_end.value_or(Position());
            res.register_advancement();
            advance();
            return res.success(make_shared<BreakNode>(start_pos, end_pos));
        }

        if (any_cast<string>(current_tok->value) == "yield")
        {
            if (in_method_count == 0)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok->pos_start.value_or(Position()),
                    current_tok->pos_end.value_or(Position()),
                    "'yield' is only allowed inside methods/functions"
                ));
            }
            auto start_pos = current_tok->pos_start.value_or(Position());
            auto yield_end_pos = current_tok->pos_end.value_or(Position());
            res.register_advancement();
            advance();

            auto expr = res.try_register(expression());
            if (!expr)
            {
                reverse(res.to_reverse_count);
                return res.success(make_shared<ReturnNode>(nullptr, start_pos, yield_end_pos));
            }

            if (current_tok.has_value() && current_tok->type == T_COMMA)
            {
                vector<shared_ptr<Node>> element_nodes;
                element_nodes.push_back(expr);

                while (current_tok.has_value() && current_tok->type == T_COMMA)
                {
                    res.register_advancement();
                    advance();
                    auto next_expr = res.register_node(expression());
                    if (res.error) return res;
                    element_nodes.push_back(next_expr);
                }

                auto end_pos = element_nodes.back()->pos_end.value_or(Position());
                auto list_node = make_shared<ListNode>(element_nodes, start_pos, end_pos);
                return res.success(make_shared<ReturnNode>(list_node, start_pos, end_pos));
            }

            return res.success(make_shared<ReturnNode>(expr, start_pos,
                                                       current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : start_pos));
        }

        return res;
    }

    ParseResult expression()
    {
        ParseResult res;

        ParserDepthGuard guard(current_depth);
        auto depth_res = _check_depth();
        if (depth_res.error) {
            return depth_res;
        }

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
        auto left_node = res.register_node(bitwise_expression());
        if (res.error)
            return res;

        while (current_tok.has_value() && current_tok->type == T_KEYWORD &&
               (any_cast<string>(current_tok->value) == "and" || any_cast<string>(current_tok->value) == "or"))
        {
            Token operator_tok = current_tok.value();
            res.register_advancement();
            advance();

            auto right_node = res.register_node(bitwise_expression());
            if (res.error)
                return res;

            left_node = make_shared<BinaryOperationNode>(left_node, operator_tok, right_node);
            if (left_node->depth > MAX_AST_DEPTH) {
                return res.failure(InvalidSyntaxError(
                    left_node->pos_start.value_or(Position()),
                    left_node->pos_end.value_or(Position()),
                    "Expression is too complex (maximum AST depth exceeded)"
                ));
            }
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

    ParseResult bitwise_expression()
    {
        ParseResult res;
        auto left_node = res.register_node(bitwise_xor());
        if (res.error)
            return res;

        while (current_tok.has_value() && current_tok->type == T_BITOR)
        {
            Token operator_tok = current_tok.value();
            res.register_advancement();
            advance();

            auto right_node = res.register_node(bitwise_xor());
            if (res.error)
                return res;

            left_node = make_shared<BinaryOperationNode>(left_node, operator_tok, right_node);
            if (left_node->depth > MAX_AST_DEPTH) {
                return res.failure(InvalidSyntaxError(
                    left_node->pos_start.value_or(Position()),
                    left_node->pos_end.value_or(Position()),
                    "Expression is too complex (maximum AST depth exceeded)"
                ));
            }
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

    ParseResult bitwise_xor()
    {
        ParseResult res;
        auto left_node = res.register_node(bitwise_and());
        if (res.error)
            return res;

        while (current_tok.has_value() && current_tok->type == T_BITXOR)
        {
            Token operator_tok = current_tok.value();
            res.register_advancement();
            advance();

            auto right_node = res.register_node(bitwise_and());
            if (res.error)
                return res;

            left_node = make_shared<BinaryOperationNode>(left_node, operator_tok, right_node);
            if (left_node->depth > MAX_AST_DEPTH) {
                return res.failure(InvalidSyntaxError(
                    left_node->pos_start.value_or(Position()),
                    left_node->pos_end.value_or(Position()),
                    "Expression is too complex (maximum AST depth exceeded)"
                ));
            }
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

    ParseResult bitwise_and()
    {
        ParseResult res;
        auto left_node = res.register_node(comp_expression());
        if (res.error)
            return res;

        while (current_tok.has_value() && current_tok->type == T_BITAND)
        {
            Token operator_tok = current_tok.value();
            res.register_advancement();
            advance();

            auto right_node = res.register_node(comp_expression());
            if (res.error)
                return res;

            left_node = make_shared<BinaryOperationNode>(left_node, operator_tok, right_node);
            if (left_node->depth > MAX_AST_DEPTH) {
                return res.failure(InvalidSyntaxError(
                    left_node->pos_start.value_or(Position()),
                    left_node->pos_end.value_or(Position()),
                    "Expression is too complex (maximum AST depth exceeded)"
                ));
            }
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

        auto left_node = res.register_node(shift_expression());
        if (res.error)
            return res;

        while (current_tok.has_value() && (current_tok->type == T_EE || current_tok->type == T_NEQ ||
                                           current_tok->type == T_LT || current_tok->type == T_GT ||
                                           current_tok->type == T_GTE || current_tok->type == T_LTE))
        {
            Token operator_tok = current_tok.value();
            res.register_advancement();
            advance();
            auto right_node = res.register_node(shift_expression());
            if (res.error)
                return res;
            left_node = make_shared<BinaryOperationNode>(left_node, operator_tok, right_node);
            if (left_node->depth > MAX_AST_DEPTH) {
                return res.failure(InvalidSyntaxError(
                    left_node->pos_start.value_or(Position()),
                    left_node->pos_end.value_or(Position()),
                    "Expression is too complex (maximum AST depth exceeded)"
                ));
            }
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

    ParseResult shift_expression()
    {
        ParseResult res;
        auto left_node = res.register_node(arith_expression());
        if (res.error)
            return res;

        while (current_tok.has_value() && (current_tok->type == T_LSHIFT || current_tok->type == T_RSHIFT))
        {
            Token operator_tok = current_tok.value();
            res.register_advancement();
            advance();
            auto right_node = res.register_node(arith_expression());
            if (res.error)
                return res;
            left_node = make_shared<BinaryOperationNode>(left_node, operator_tok, right_node);
            if (left_node->depth > MAX_AST_DEPTH) {
                return res.failure(InvalidSyntaxError(
                    left_node->pos_start.value_or(Position()),
                    left_node->pos_end.value_or(Position()),
                    "Expression is too complex (maximum AST depth exceeded)"
                ));
            }
        }

        auto node = res.register_node(res.success(left_node));
        if (res.error)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected int,float,identifier,'+','-' or '('"));
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
            if (left_node->depth > MAX_AST_DEPTH) {
                return res.failure(InvalidSyntaxError(
                    left_node->pos_start.value_or(Position()),
                    left_node->pos_end.value_or(Position()),
                    "Expression is too complex (maximum AST depth exceeded)"
                ));
            }
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
            auto pos_end = current_tok->pos_end.value_or(Position());
            res.register_advancement();
            advance();
            return res.success(make_shared<DictNode>(keyval_nodes, pos_start, pos_end));
        }
        else
        {
            auto first_key = res.register_node(expression());
            if (res.error)
                return res;

            if (!current_tok.has_value() || current_tok->type != T_COLON)
            {
                return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected ':'"));
            }
            res.register_advancement();
            advance();

            auto first_val = res.register_node(expression());
            if (res.error)
                return res;

            // ── Dict comprehension with cycle ──────────────────────────────
            if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "cycle")
            {
                return _parse_dict_comp_cycle(res, first_key, first_val, pos_start);
            }

            // ── Dict comprehension with trace ──────────────────────────────
            if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "trace")
            {
                return _parse_dict_comp_trace(res, first_key, first_val, pos_start);
            }

            keyval_nodes.push_back({first_key, first_val});

            while (current_tok.has_value() && current_tok->type == T_COMMA)
            {
                res.register_advancement();
                advance();

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
            }

            if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
            {
                return res.failure(InvalidSyntaxError(current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(), current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(), "Expected ',' or '}'", "Did you forget a comma ',' between dictionary entries?"));
            }
            auto pos_end = current_tok->pos_end.value_or(Position());
            res.register_advancement();
            advance();

            return res.success(make_shared<DictNode>(keyval_nodes, pos_start, pos_end));
        }
    }

    ParseResult parse_block()
    {
        ParseResult res;
        vector<shared_ptr<Node>> body_nodes;
        optional<Position> pos_start = current_tok.has_value() ? current_tok->pos_start : nullopt;

        while (current_tok.has_value() && current_tok->type != T_RPAREN2 && current_tok->type != T_EOF)
        {
            auto sep_err = check_statement_separation(res, body_nodes);
            if (sep_err)
                return *sep_err;

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

        auto body_node = make_body_node(body_nodes, pos_start);
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

        if (current_tok.has_value() && (current_tok->type == T_ERROR || current_tok->type == T_IDENTIFIER))
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

        auto node = make_shared<CatchNode>(error_type, error_name, body_node);
        node->pos_end = get_last_parsed_pos_end();
        return res.success(node);
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

        auto node = make_shared<FinallyNode>(body_node);
        node->pos_end = get_last_parsed_pos_end();
        return res.success(node);
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
        while (current_tok.has_value() && (current_tok->type == T_LPAREN || current_tok->type == T_DOT || current_tok->type == T_LPAREN3))
        {
            if (current_tok->type == T_LPAREN)
            {
                vector<shared_ptr<Node>> pos_args;
                vector<pair<Token, shared_ptr<Node>>> kw_args;
                optional<Position> rparen_pos = nullopt;
                res.register_node(parse_arguments(pos_args, kw_args, rparen_pos));
                if (res.error)
                    return obj_node;
                obj_node = make_shared<FunctionCallNode>(obj_node, pos_args, kw_args, rparen_pos);
            }
            else if (current_tok->type == T_DOT)
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

                obj_node = make_shared<AttrAccessNode>(obj_node, attr_tok);
            }
            else if (current_tok->type == T_LPAREN3)
            {
                res.register_advancement();
                advance(); // consume '['

                auto index_expr = res.register_node(expression());
                if (res.error)
                    return obj_node;

                if (!current_tok.has_value() || current_tok->type != T_RPAREN3)
                {
                    res.failure(InvalidSyntaxError(
                        current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                        current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                        "Expected ']'"));
                    return obj_node;
                }

                res.register_advancement();
                advance(); // consume ']'

                obj_node = make_shared<IndexAccessNode>(obj_node, index_expr);
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
                vector<shared_ptr<Node>> pos_args;
                vector<pair<Token, shared_ptr<Node>>> kw_args;
                optional<Position> rparen_pos = nullopt;
                res.register_node(parse_arguments(pos_args, kw_args, rparen_pos));
                if (res.error)
                    return res;
                obj_node = make_shared<FunctionCallNode>(attr_access, pos_args, kw_args, rparen_pos);
            }
            else if (current_tok.has_value() && !has_more_dot &&
                     (current_tok->type == T_EQ ||
                      current_tok->type == T_PLUSEQUAL ||
                      current_tok->type == T_MINUSEQUAL ||
                      current_tok->type == T_MULEQUAL ||
                      current_tok->type == T_DIVIDEEQUAL ||
                      current_tok->type == T_MODULUSEQUAL ||
                      current_tok->type == T_FLOOREQUAL ||
                      current_tok->type == T_EXPEQUAL ||
                      current_tok->type == T_BITANDEQUAL ||
                      current_tok->type == T_BITXOREQUAL ||
                      current_tok->type == T_BITOREQUAL ||
                      current_tok->type == T_LSHIFTEQUAL ||
                      current_tok->type == T_RSHIFTEQUAL))
            {
                Token op_tok = current_tok.value();
                res.register_advancement();
                advance();
                
                auto value_node = res.register_node(expression());
                if (res.error)
                    return res;
                
                if (op_tok.type == T_EQ)
                {
                    return res.success(make_shared<AttrAssignNode>(obj_node, attr_tok, value_node));
                }
                else
                {
                    string bin_op_type;
                    if (op_tok.type == T_PLUSEQUAL) bin_op_type = T_PLUS;
                    else if (op_tok.type == T_MINUSEQUAL) bin_op_type = T_MINUS;
                    else if (op_tok.type == T_MULEQUAL) bin_op_type = T_MUL;
                    else if (op_tok.type == T_DIVIDEEQUAL) bin_op_type = T_DIVIDE;
                    else if (op_tok.type == T_MODULUSEQUAL) bin_op_type = T_MODULUS;
                    else if (op_tok.type == T_FLOOREQUAL) bin_op_type = T_FLOOR;
                    else if (op_tok.type == T_EXPEQUAL) bin_op_type = T_EXP;
                    else if (op_tok.type == T_BITANDEQUAL) bin_op_type = T_BITAND;
                    else if (op_tok.type == T_BITXOREQUAL) bin_op_type = T_BITXOR;
                    else if (op_tok.type == T_BITOREQUAL) bin_op_type = T_BITOR;
                    else if (op_tok.type == T_LSHIFTEQUAL) bin_op_type = T_LSHIFT;
                    else if (op_tok.type == T_RSHIFTEQUAL) bin_op_type = T_RSHIFT;

                    Token bin_op(bin_op_type, {}, op_tok.pos_start, op_tok.pos_end);
                    auto current_val = make_shared<AttrAccessNode>(obj_node, attr_tok);
                    auto math_node = make_shared<BinaryOperationNode>(current_val, bin_op, value_node);
                    return res.success(make_shared<AttrAssignNode>(obj_node, attr_tok, math_node));
                }
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

        vector<Token> parent_name_toks;
        if (current_tok.has_value() && current_tok->type == T_COLON)
        {
            res.register_advancement();
            advance();

            while (true)
            {
                if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
                {
                    return res.failure(InvalidSyntaxError(
                        current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                        current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                        "Expected parent class name"));
                }
                parent_name_toks.push_back(current_tok.value());
                res.register_advancement();
                advance();

                if (current_tok.has_value() && current_tok->type == T_COMMA)
                {
                    res.register_advancement();
                    advance();
                }
                else
                {
                    break;
                }
            }
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

        auto node = make_shared<ModelNode>(model_name_tok, parent_name_toks, body_nodes);
        node->pos_end = get_last_parsed_pos_end();
        return res.success(node);
    }

    ParseResult class_member()
    {
        ParseResult res;
        if (!current_tok.has_value())
        {
            return res.failure(InvalidSyntaxError(Position(), Position(), "Unexpected end of input"));
        }

        string access_mod = "open";
        if (current_tok->type == T_KEYWORD &&
            (any_cast<string>(current_tok->value) == "open" ||
             any_cast<string>(current_tok->value) == "secret" ||
             any_cast<string>(current_tok->value) == "guarded"))
        {
            access_mod = any_cast<string>(current_tok->value);
            res.register_advancement();
            advance();
        }

        if (!current_tok.has_value())
        {
            return res.failure(InvalidSyntaxError(Position(), Position(), "Expected 'attr', 'init', or 'method' after access modifier"));
        }

        if (current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "attr")
        {
            return attr_declaration(access_mod);
        }
        if (current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "init")
        {
            return constructor_definition();
        }
        if (current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "method")
        {
            return function_definition(access_mod);
        }

        return res.failure(InvalidSyntaxError(
            current_tok->pos_start.value_or(Position()),
            current_tok->pos_end.value_or(Position()),
            "Expected 'attr', 'init', or 'method'"));
    }

    ParseResult attr_declaration(const string &access_mod = "open")
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

        return res.success(make_shared<AttrNode>(declarations, access_mod, pos_start, pos_end));
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

        vector<pair<Token, shared_ptr<Node>>> param_nodes;
        bool has_seen_default = false;

        if (current_tok.has_value() && current_tok->type == T_IDENTIFIER)
        {
            Token param_tok = current_tok.value();
            res.register_advancement();
            advance();

            shared_ptr<Node> default_val = nullptr;
            if (current_tok.has_value() && current_tok->type == T_EQ)
            {
                res.register_advancement();
                advance();
                default_val = res.register_node(expression());
                if (res.error) return res;
                has_seen_default = true;
            }

            param_nodes.push_back({param_tok, default_val});

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
                Token next_param_tok = current_tok.value();
                res.register_advancement();
                advance();

                shared_ptr<Node> next_default_val = nullptr;
                if (current_tok.has_value() && current_tok->type == T_EQ)
                {
                    res.register_advancement();
                    advance();
                    next_default_val = res.register_node(expression());
                    if (res.error) return res;
                    has_seen_default = true;
                }
                else if (has_seen_default)
                {
                    return res.failure(InvalidSyntaxError(
                        next_param_tok.pos_start.value_or(Position()),
                        next_param_tok.pos_end.value_or(Position()),
                        "Non-default argument follows default argument"));
                }

                param_nodes.push_back({next_param_tok, next_default_val});
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
        auto body_node = make_body_node(body_nodes, pos_start);
        Position pos_end = current_tok->pos_end.value_or(Position());
        res.register_advancement();
        advance();
        return res.success(make_shared<InitNode>(param_nodes, body_node, pos_start, pos_end));
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

        auto left_node = make_shared<VariableUseNode>(var_name_tok);
        vector<shared_ptr<Node>> left_nodes = {left_node};
        vector<shared_ptr<Node>> value_nodes = {value_node};
        return res.success(make_shared<VariableAssignNode>(left_nodes, value_nodes));
    }

    ParseResult foreach_expression()
    {
        ParseResult res;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "trace"))
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'trace'"));
        }
        res.register_advancement();
        advance();

        vector<Token> var_name_tokens;
        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected at least one identifier"));
        }

        var_name_tokens.push_back(current_tok.value());
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
                    "Expected identifier after ','"));
            }
            var_name_tokens.push_back(current_tok.value());
            res.register_advancement();
            advance();
        }

        if (!current_tok.has_value() || current_tok->type != T_LARROW)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '<-'"));
        }
        res.register_advancement();
        advance();

        auto collection_node = res.register_node(expression());
        if (res.error)
            return res;

        while (current_tok.has_value() && current_tok->type == T_NEWLINE)
        {
            res.register_advancement();
            advance();
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
            auto sep_err = check_statement_separation(res, body_nodes);
            if (sep_err)
                return *sep_err;

            if (current_tok->type == T_KEYWORD &&
                (any_cast<string>(current_tok->value) == "yield" ||
                 any_cast<string>(current_tok->value) == "proceed" ||
                 any_cast<string>(current_tok->value) == "escape"))
            {
                auto jump_node = res.register_node(jump_statements());
                if (res.error) return res;
                body_nodes.push_back(jump_node);
            }
            else
            {
                auto multiline_node = res.try_register(multiline());
                if (res.error) return res;
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
                            "Expected statement or '}'"));
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

        auto body_node = make_body_node(body_nodes, pos_start);

        if (!current_tok.has_value() || current_tok->type != T_RPAREN2)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected '}'"));
        }
        res.register_advancement();
        advance();

        auto node = make_shared<ForEachLoopNode>(var_name_tokens, collection_node, body_node);
        node->pos_end = get_last_parsed_pos_end();
        return res.success(node);
    }

    ParseResult summon_statement()
    {
        ParseResult res;

        if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "summon"))
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected 'summon'"));
        }
        optional<Position> pos_start = current_tok->pos_start;
        res.register_advancement();
        advance();

        // Case 1: summon * from MODULE
        if (current_tok.has_value() && current_tok->type == T_MUL)
        {
            res.register_advancement();
            advance();

            if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "from"))
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected 'from' after '*'"));
            }
            res.register_advancement();
            advance();

            if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected module name after 'from'"));
            }
            Token module_tok = current_tok.value();
            optional<Position> pos_end = module_tok.pos_end;
            res.register_advancement();
            advance();

            return res.success(make_shared<SummonNode>(
                module_tok, vector<pair<Token, optional<Token>>>(), nullopt, true, pos_start, pos_end
            ));
        }

        // Must start with IDENTIFIER from here
        if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
        {
            return res.failure(InvalidSyntaxError(
                current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                "Expected module name or identifier after 'summon'"));
        }
        Token first_tok = current_tok.value();
        res.register_advancement();
        advance();

        // Case 2: summon NAME as ALIAS from MODULE (single with alias)
        // or summon NAME (',' NAME)* from MODULE (multiple names)
        bool is_from_import = false;
        if (current_tok.has_value() && (current_tok->type == T_COMMA || (current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "from")))
        {
            is_from_import = true;
        }
        else if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "as")
        {
            // Lookahead
            int saved_index = tok_index;

            res.register_advancement(); advance(); // consume 'as'
            if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected alias name after 'as'"));
            }
            Token alias_tok = current_tok.value();
            res.register_advancement(); advance(); // consume alias

            if (current_tok.has_value() && current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "from")
            {
                // Case: summon NAME as ALIAS from MODULE
                res.register_advancement(); advance(); // consume 'from'
                if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
                {
                    return res.failure(InvalidSyntaxError(
                        current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                        current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                        "Expected module name after 'from'"));
                }
                Token module_tok = current_tok.value();
                optional<Position> pos_end = module_tok.pos_end;
                res.register_advancement(); advance();
                vector<pair<Token, optional<Token>>> names = {{first_tok, make_optional(alias_tok)}};
                return res.success(make_shared<SummonNode>(
                    module_tok, names, nullopt, false, pos_start, pos_end
                ));
            }
            else
            {
                // Case: summon NAME as ALIAS (module alias)
                tok_index = saved_index;
                update_current_tok();
                res.reverse(); // undo the lookahead advancements
                res.reverse();

                res.register_advancement(); advance(); // consume 'as'
                Token alias_tok2 = current_tok.value();
                res.register_advancement(); advance();
                optional<Position> pos_end = alias_tok2.pos_end;
                return res.success(make_shared<SummonNode>(
                    first_tok, vector<pair<Token, optional<Token>>>(), make_optional(alias_tok2), false, pos_start, pos_end
                ));
            }
        }

        if (is_from_import)
        {
            vector<pair<Token, optional<Token>>> names = {{first_tok, nullopt}};
            while (current_tok.has_value() && current_tok->type == T_COMMA)
            {
                res.register_advancement(); advance();
                if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
                {
                    return res.failure(InvalidSyntaxError(
                        current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                        current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                        "Expected identifier after ','"));
                }
                names.push_back({current_tok.value(), nullopt});
                res.register_advancement(); advance();
            }

            if (!current_tok.has_value() || !(current_tok->type == T_KEYWORD && any_cast<string>(current_tok->value) == "from"))
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected 'from'"));
            }
            res.register_advancement(); advance();

            if (!current_tok.has_value() || current_tok->type != T_IDENTIFIER)
            {
                return res.failure(InvalidSyntaxError(
                    current_tok.has_value() ? current_tok->pos_start.value_or(Position()) : Position(),
                    current_tok.has_value() ? current_tok->pos_end.value_or(Position()) : Position(),
                    "Expected module name after 'from'"));
            }
            Token module_tok = current_tok.value();
            optional<Position> pos_end = module_tok.pos_end;
            res.register_advancement(); advance();

            return res.success(make_shared<SummonNode>(
                module_tok, names, nullopt, false, pos_start, pos_end
            ));
        }

        // Case 5: bare summon NAME
        optional<Position> pos_end = first_tok.pos_end;
        return res.success(make_shared<SummonNode>(
            first_tok, vector<pair<Token, optional<Token>>>(), nullopt, false, pos_start, pos_end
        ));
    }

    ParseResult _parse_fstring(const Token& token)
    {
        ParseResult res;
        string raw = any_cast<string>(token.value);
        optional<Position> pos_start = token.pos_start;
        optional<Position> pos_end = token.pos_end;
        string filename = (pos_start.has_value() && pos_start->file_name) ? *pos_start->file_name : "<fstring>";

        vector<pair<string, any>> parts;
        size_t i = 0;
        size_t n = raw.length();

        while (i < n)
        {
            size_t brace_start = raw.find('{', i);
            if (brace_start == string::npos)
            {
                string literal = raw.substr(i);
                if (!literal.empty())
                {
                    parts.push_back({"literal", literal});
                }
                break;
            }

            string literal = raw.substr(i, brace_start - i);
            if (!literal.empty())
            {
                parts.push_back({"literal", literal});
            }

            int depth = 1;
            size_t j = brace_start + 1;
            while (j < n && depth > 0)
            {
                if (raw[j] == '{')
                    depth++;
                else if (raw[j] == '}')
                    depth--;
                j++;
            }

            string expr_src = raw.substr(brace_start + 1, j - 1 - (brace_start + 1));

            Lexer sub_lexer(filename, expr_src);
            auto [sub_tokens, lex_error] = sub_lexer.enumerate_tokens();
            if (sub_error_check(lex_error, res, pos_start, pos_end, expr_src))
                return res;

            Position expr_start_pos;
            if (token.pos_start) {
                expr_start_pos = get_position_at_index(*token.pos_start, token.pos_start->index + 1 + brace_start + 1);
            } else {
                expr_start_pos = Position();
            }

            for (auto& tok : sub_tokens) {
                if (tok.pos_start) {
                    tok.pos_start = map_sub_position(expr_start_pos, *tok.pos_start);
                }
                if (tok.pos_end) {
                    tok.pos_end = map_sub_position(expr_start_pos, *tok.pos_end);
                }
            }

            Parser sub_parser(std::move(sub_tokens));
            auto sub_res = sub_parser.expression();
            if (sub_res.error)
            {
                return res.failure(InvalidSyntaxError(
                    pos_start.value_or(Position()), pos_end.value_or(Position()),
                    "Error in f-string expression {" + expr_src + "}: " + sub_res.error->details));
            }

            parts.push_back({"expr", sub_res.node});
            i = j;
        }

        return res.success(make_shared<FStringNode>(parts, pos_start, pos_end));
    }

private:
    Position get_position_at_index(const Position& base_pos, int target_index) {
        Position pos = base_pos.copy();
        if (!pos.file_text) return pos;
        const string& text = *pos.file_text;
        while (pos.index < target_index && pos.index < (int)text.length()) {
            char c = text[pos.index];
            pos.advance(c);
        }
        return pos;
    }

    Position map_sub_position(const Position& expr_start_pos, const Position& sub_pos) {
        Position mapped;
        mapped.file_name = expr_start_pos.file_name;
        mapped.file_text = expr_start_pos.file_text;
        mapped.index = expr_start_pos.index + sub_pos.index;
        mapped.line = expr_start_pos.line + sub_pos.line;
        if (sub_pos.line == 0) {
            mapped.col = expr_start_pos.col + sub_pos.col;
        } else {
            mapped.col = sub_pos.col;
        }
        return mapped;
    }

    shared_ptr<ListNode> make_body_node(const vector<shared_ptr<Node>>& body_nodes, const optional<Position>& default_pos) {
        optional<Position> start = default_pos;
        optional<Position> end = default_pos;
        if (current_tok.has_value()) {
            start = current_tok->pos_start;
            end = current_tok->pos_start;
        }
        if (!body_nodes.empty()) {
            start = body_nodes.front()->pos_start;
            end = body_nodes.back()->pos_end;
        }
        return make_shared<ListNode>(body_nodes, start, end, true);
    }

    Position get_last_parsed_pos_end() {
        if (tok_index > 0 && tok_index - 1 < (int)tokens.size()) {
            return tokens[tok_index - 1].pos_end.value_or(Position());
        }
        return Position();
    }

    bool sub_error_check(const shared_ptr<Error>& err, ParseResult& res, const optional<Position>& start, const optional<Position>& end, const string& src) {
        if (err) {
            res.failure(InvalidSyntaxError(
                start.value_or(Position()), end.value_or(Position()),
                "Error in f-string expression {" + src + "}: " + err->details));
            return true;
        }
        return false;
    }

public:
};
