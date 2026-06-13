#pragma once

#include <string>
#include <vector>
#include <memory>
#include <any>
#include "../language_core/lexer.h"
#include "../language_core/position.h"
#include "../language_core/error.h"
#include "node.h"
#include "operation_nodes.h"
#include "variable_nodes.h"
#include "if_else_elif_nodes.h"
#include "switch_nodes.h"
#include "for_nodes.h"
#include "while_nodes.h"
#include "function_nodes.h"
#include "string_nodes.h"
#include "list_nodes.h"
#include "dict_nodes.h"
#include "jump_nodes.h"
#include "try_catch_nodes.h"
#include "class_nodes.h"
#include "fstring_nodes.h"
#include "foreach_nodes.h"
#include "summon_nodes.h"
#include "comprehension_nodes.h"

using namespace std;

inline string escape_json_string(const string& s) {
    string out = "";
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

inline string position_to_json(const Position& pos) {
    return "{\"index\":" + to_string(pos.index) +
           ",\"line\":" + to_string(pos.line) +
           ",\"col\":" + to_string(pos.col) + "}";
}

inline string position_to_json(const optional<Position>& pos) {
    if (!pos) return "null";
    return position_to_json(*pos);
}

// Emit pos_start and pos_end from a Node's base class fields
inline string node_positions_json(const shared_ptr<Node>& node) {
    return "\"pos_start\":" + position_to_json(node->pos_start) +
           ",\"pos_end\":" + position_to_json(node->pos_end);
}

inline string token_to_json(const Token& token) {
    string val_str = "null";
    if (token.value.has_value()) {
        if (token.value.type() == typeid(long long)) {
            val_str = to_string(any_cast<long long>(token.value));
        }
        else if (token.value.type() == typeid(double)) {
            val_str = to_string(any_cast<double>(token.value));
        }
        else if (token.value.type() == typeid(string)) {
            val_str = "\"" + escape_json_string(any_cast<string>(token.value)) + "\"";
        }
        else if (token.value.type() == typeid(const char*)) {
            val_str = "\"" + escape_json_string(any_cast<const char*>(token.value)) + "\"";
        }
    }
    return "{\"type\":\"" + token.type + "\",\"value\":" + val_str +
           ",\"pos_start\":" + position_to_json(token.pos_start) +
           ",\"pos_end\":" + position_to_json(token.pos_end) + "}";
}

inline string token_to_json(const optional<Token>& token) {
    if (!token) return "null";
    return token_to_json(*token);
}

inline string error_to_json(const shared_ptr<Error>& error) {
    if (!error) return "null";
    return "{\"type\":\"" + escape_json_string(error->error_name) +
           "\",\"details\":\"" + escape_json_string(error->details) +
           "\",\"pos_start\":" + position_to_json(error->pos_start) +
           ",\"pos_end\":" + position_to_json(error->pos_end) + "}";
}

// Forward declare node_to_json
inline string node_to_json(const shared_ptr<Node>& node);

inline string node_vector_to_json(const vector<shared_ptr<Node>>& nodes) {
    string out = "[";
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (i > 0) out += ",";
        out += node_to_json(nodes[i]);
    }
    out += "]";
    return out;
}

inline string token_vector_to_json(const vector<Token>& tokens) {
    string out = "[";
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) out += ",";
        out += token_to_json(tokens[i]);
    }
    out += "]";
    return out;
}

inline string node_to_json(const shared_ptr<Node>& node) {
    if (!node) return "null";

    // All nodes now include pos_start/pos_end from the base Node class
    // This ensures the frontend always has accurate position info from the C++ AST

    if (auto n = dynamic_pointer_cast<NumberNode>(node)) {
        return "{\"node_type\":\"NumberNode\"," + node_positions_json(n) +
               ",\"token\":" + token_to_json(n->token) + "}";
    }
    if (auto n = dynamic_pointer_cast<StringNode>(node)) {
        return "{\"node_type\":\"StringNode\"," + node_positions_json(n) +
               ",\"token\":" + token_to_json(n->token) + "}";
    }
    if (auto n = dynamic_pointer_cast<UnaryOperationNode>(node)) {
        return "{\"node_type\":\"UnaryOperationNode\"," + node_positions_json(n) +
               ",\"operator\":" + token_to_json(n->operator_token) +
               ",\"node\":" + node_to_json(n->node) + "}";
    }
    if (auto n = dynamic_pointer_cast<BinaryOperationNode>(node)) {
        return "{\"node_type\":\"BinaryOperationNode\"," + node_positions_json(n) +
               ",\"left\":" + node_to_json(n->left_node) +
               ",\"operator\":" + token_to_json(n->operator_token) +
               ",\"right\":" + node_to_json(n->right_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<TernaryOperationNode>(node)) {
        return "{\"node_type\":\"TernaryOperationNode\"," + node_positions_json(n) +
               ",\"condition\":" + node_to_json(n->comp_node) +
               ",\"true_branch\":" + node_to_json(n->true_node) +
               ",\"false_branch\":" + node_to_json(n->false_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<VariableUseNode>(node)) {
        return "{\"node_type\":\"VariableUseNode\"," + node_positions_json(n) +
               ",\"var_name_tok\":" + token_to_json(n->var_name_tok) +
               ",\"index_node\":" + node_vector_to_json(n->index_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<VariableAssignNode>(node)) {
        return "{\"node_type\":\"VariableAssignNode\"," + node_positions_json(n) +
               ",\"left_nodes\":" + node_vector_to_json(n->left_nodes) +
               ",\"value_nodes\":" + node_vector_to_json(n->value_nodes) + "}";
    }
    if (auto n = dynamic_pointer_cast<IndexAccessNode>(node)) {
        return "{\"node_type\":\"IndexAccessNode\"," + node_positions_json(n) +
               ",\"object_node\":" + node_to_json(n->object_node) +
               ",\"index_node\":" + node_to_json(n->index_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<ListNode>(node)) {
        return "{\"node_type\":\"ListNode\"," + node_positions_json(n) +
               ",\"element_nodes\":" + node_vector_to_json(n->element_nodes) + "}";
    }
    if (auto n = dynamic_pointer_cast<DictNode>(node)) {
        string pairs = "[";
        for (size_t i = 0; i < n->keyval_nodes.size(); ++i) {
            if (i > 0) pairs += ",";
            pairs += "{\"key\":" + node_to_json(n->keyval_nodes[i].first) +
                     ",\"value\":" + node_to_json(n->keyval_nodes[i].second) + "}";
        }
        pairs += "]";
        return "{\"node_type\":\"DictNode\"," + node_positions_json(n) +
               ",\"keyval_nodes\":" + pairs + "}";
    }
    if (auto n = dynamic_pointer_cast<IfNode>(node)) {
        string cases_str = "[";
        for (size_t i = 0; i < n->cases.size(); ++i) {
            if (i > 0) cases_str += ",";
            cases_str += "{\"condition\":" + node_to_json(get<0>(n->cases[i])) +
                         ",\"body\":" + node_to_json(get<1>(n->cases[i])) + "}";
        }
        cases_str += "]";
        string else_str = "null";
        if (n->else_case) {
            else_str = "{\"body\":" + node_to_json(n->else_case->first) + "}";
        }
        return "{\"node_type\":\"IfNode\"," + node_positions_json(n) +
               ",\"cases\":" + cases_str +
               ",\"else_case\":" + else_str + "}";
    }
    if (auto n = dynamic_pointer_cast<SwitchNode>(node)) {
        string cases_str = "[";
        for (size_t i = 0; i < n->cases.size(); ++i) {
            if (i > 0) cases_str += ",";
            string val_str = n->cases[i]->value ? node_to_json(n->cases[i]->value) : "null";
            cases_str += "{\"value\":" + val_str +
                         ",\"body\":" + node_to_json(n->cases[i]->body) + "}";
        }
        cases_str += "]";
        return "{\"node_type\":\"SwitchNode\"," + node_positions_json(n) +
               ",\"switch_value\":" + node_to_json(n->switch_value) +
               ",\"cases\":" + cases_str + "}";
    }
    if (auto n = dynamic_pointer_cast<ForNode>(node)) {
        return "{\"node_type\":\"ForNode\"," + node_positions_json(n) +
               ",\"var_name_tok\":" + token_to_json(n->var_name_tok) +
               ",\"start_value_node\":" + node_to_json(n->start_value_node) +
               ",\"end_value_node\":" + node_to_json(n->end_value_node) +
               (n->step_value_node ? (",\"step_value_node\":" + node_to_json(n->step_value_node)) : "") +
               ",\"body_node\":" + node_to_json(n->body_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<ForEachLoopNode>(node)) {
        return "{\"node_type\":\"ForEachLoopNode\"," + node_positions_json(n) +
               ",\"var_name_tokens\":" + token_vector_to_json(n->var_name_tokens) +
               ",\"collection_node\":" + node_to_json(n->collection_node) +
               ",\"body_node\":" + node_to_json(n->body_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<WhileNode>(node)) {
        return "{\"node_type\":\"WhileNode\"," + node_positions_json(n) +
               ",\"condition_node\":" + node_to_json(n->condition_node) +
               ",\"body_node\":" + node_to_json(n->body_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<FunctionDefinitionNode>(node)) {
        string args_str = "[";
        for (size_t i = 0; i < n->arg_nodes.size(); ++i) {
            if (i > 0) args_str += ",";
            string default_val = n->arg_nodes[i].second ? node_to_json(n->arg_nodes[i].second) : "null";
            args_str += "{\"arg\":" + token_to_json(n->arg_nodes[i].first) +
                        ",\"default\":" + default_val + "}";
        }
        args_str += "]";
        return "{\"node_type\":\"FunctionDefinitionNode\"," + node_positions_json(n) +
               ",\"var_name_tok\":" + token_to_json(n->var_name_tok) +
               ",\"arg_nodes\":" + args_str +
               ",\"body_node\":" + node_to_json(n->body_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<FunctionCallNode>(node)) {
        string kw_args_str = "[";
        for (size_t i = 0; i < n->keyword_arg_nodes.size(); ++i) {
            if (i > 0) kw_args_str += ",";
            kw_args_str += "{\"keyword\":" + token_to_json(n->keyword_arg_nodes[i].first) +
                           ",\"value\":" + node_to_json(n->keyword_arg_nodes[i].second) + "}";
        }
        kw_args_str += "]";
        return "{\"node_type\":\"FunctionCallNode\"," + node_positions_json(n) +
               ",\"call_node\":" + node_to_json(n->call_node) +
               ",\"positional_arg_nodes\":" + node_vector_to_json(n->positional_arg_nodes) +
               ",\"keyword_arg_nodes\":" + kw_args_str + "}";
    }
    if (auto n = dynamic_pointer_cast<ReturnNode>(node)) {
        return "{\"node_type\":\"ReturnNode\"," + node_positions_json(n) +
               ",\"node_to_return\":" + node_to_json(n->node_to_return) + "}";
    }
    if (auto n = dynamic_pointer_cast<ContinueNode>(node)) {
        return "{\"node_type\":\"ContinueNode\"," + node_positions_json(n) + "}";
    }
    if (auto n = dynamic_pointer_cast<BreakNode>(node)) {
        return "{\"node_type\":\"BreakNode\"," + node_positions_json(n) + "}";
    }
    if (auto n = dynamic_pointer_cast<TryNode>(node)) {
        string trap_str = "[";
        for (size_t i = 0; i < n->trap_nodes.size(); ++i) {
            if (i > 0) trap_str += ",";
            trap_str += node_to_json(n->trap_nodes[i]);
        }
        trap_str += "]";
        return "{\"node_type\":\"TryNode\"," + node_positions_json(n) +
               ",\"body_node\":" + node_to_json(n->body_node) +
               ",\"trap_nodes\":" + trap_str +
               ",\"clean_node\":" + node_to_json(n->clean_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<CatchNode>(node)) {
        return "{\"node_type\":\"CatchNode\"," + node_positions_json(n) +
               ",\"error_type\":" + token_to_json(n->error_type) +
               ",\"error_name\":" + token_to_json(n->error_name) +
               ",\"body_node\":" + node_to_json(n->body_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<FinallyNode>(node)) {
        return "{\"node_type\":\"FinallyNode\"," + node_positions_json(n) +
               ",\"body_node\":" + node_to_json(n->body_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<ModelNode>(node)) {
        return "{\"node_type\":\"ModelNode\"," + node_positions_json(n) +
               ",\"name_tok\":" + token_to_json(n->name_tok) +
               ",\"parent_name_toks\":" + token_vector_to_json(n->parent_name_toks) +
               ",\"body_nodes\":" + node_vector_to_json(n->body_nodes) + "}";
    }
    if (auto n = dynamic_pointer_cast<AttrNode>(node)) {
        string decl_str = "[";
        for (size_t i = 0; i < n->declarations.size(); ++i) {
            if (i > 0) decl_str += ",";
            decl_str += "{\"name\":" + token_to_json(n->declarations[i].first) +
                        ",\"value\":" + node_to_json(n->declarations[i].second) + "}";
        }
        decl_str += "]";
        return "{\"node_type\":\"AttrNode\"," + node_positions_json(n) +
               ",\"access_modifier\":\"" + escape_json_string(n->access_modifier) + "\"" +
               ",\"declarations\":" + decl_str + "}";
    }
    if (auto n = dynamic_pointer_cast<InitNode>(node)) {
        string params_str = "[";
        for (size_t i = 0; i < n->param_nodes.size(); ++i) {
            if (i > 0) params_str += ",";
            string default_val = n->param_nodes[i].second ? node_to_json(n->param_nodes[i].second) : "null";
            params_str += "{\"name\":" + token_to_json(n->param_nodes[i].first) +
                          ",\"default\":" + default_val + "}";
        }
        params_str += "]";
        return "{\"node_type\":\"InitNode\"," + node_positions_json(n) +
               ",\"param_nodes\":" + params_str +
               ",\"body_node\":" + node_to_json(n->body_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<AttrAccessNode>(node)) {
        return "{\"node_type\":\"AttrAccessNode\"," + node_positions_json(n) +
               ",\"object_node\":" + node_to_json(n->object_node) +
               ",\"attr_name_tok\":" + token_to_json(n->attr_name_tok) + "}";
    }
    if (auto n = dynamic_pointer_cast<AttrAssignNode>(node)) {
        return "{\"node_type\":\"AttrAssignNode\"," + node_positions_json(n) +
               ",\"object_node\":" + node_to_json(n->object_node) +
               ",\"attr_name_tok\":" + token_to_json(n->attr_name_tok) +
               ",\"value_node\":" + node_to_json(n->value_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<FStringNode>(node)) {
        // Simplified FString: show the literal text and expression sub-nodes only
        // Collect the full literal template as a single string for display
        string template_str = "";
        string expr_nodes_str = "[";
        bool first_expr = true;
        for (size_t i = 0; i < n->parts.size(); ++i) {
            if (n->parts[i].first == "literal") {
                template_str += any_cast<string>(n->parts[i].second);
            } else if (n->parts[i].first == "expr") {
                if (!first_expr) expr_nodes_str += ",";
                expr_nodes_str += node_to_json(any_cast<shared_ptr<Node>>(n->parts[i].second));
                first_expr = false;
                template_str += "{...}";
            }
        }
        expr_nodes_str += "]";
        return "{\"node_type\":\"FStringNode\"," + node_positions_json(n) +
               ",\"template\":\"" + escape_json_string(template_str) + "\"" +
               ",\"expressions\":" + expr_nodes_str + "}";
    }
    if (auto n = dynamic_pointer_cast<SummonNode>(node)) {
        string names_str = "[";
        for (size_t i = 0; i < n->names.size(); ++i) {
            if (i > 0) names_str += ",";
            string alias_str = n->names[i].second ? token_to_json(*(n->names[i].second)) : "null";
            names_str += "{\"name\":" + token_to_json(n->names[i].first) +
                         ",\"alias\":" + alias_str + "}";
        }
        names_str += "]";
        return "{\"node_type\":\"SummonNode\"," + node_positions_json(n) +
               ",\"module_tok\":" + token_to_json(n->module_tok) +
               ",\"names\":" + names_str +
               ",\"module_alias\":" + token_to_json(n->module_alias) +
               ",\"wildcard\":" + (n->wildcard ? "true" : "false") + "}";
    }
    if (auto n = dynamic_pointer_cast<ListComprehensionNode>(node)) {
        return "{\"node_type\":\"ListComprehensionNode\"," + node_positions_json(n) +
               ",\"expr_node\":" + node_to_json(n->expr_node) +
               ",\"loop_type\":\"" + escape_json_string(n->loop_type) + "\"" +
               ",\"var_name_tok\":" + token_to_json(n->var_name_tok) +
               ",\"start_node\":" + node_to_json(n->start_node) +
               ",\"end_node\":" + node_to_json(n->end_node) +
               ",\"step_node\":" + node_to_json(n->step_node) +
               ",\"var_name_tokens\":" + token_vector_to_json(n->var_name_tokens) +
               ",\"collection_node\":" + node_to_json(n->collection_node) +
               ",\"condition_node\":" + node_to_json(n->condition_node) + "}";
    }
    if (auto n = dynamic_pointer_cast<DictComprehensionNode>(node)) {
        return "{\"node_type\":\"DictComprehensionNode\"," + node_positions_json(n) +
               ",\"key_node\":" + node_to_json(n->key_node) +
               ",\"val_node\":" + node_to_json(n->val_node) +
               ",\"loop_type\":\"" + escape_json_string(n->loop_type) + "\"" +
               ",\"var_name_tok\":" + token_to_json(n->var_name_tok) +
               ",\"start_node\":" + node_to_json(n->start_node) +
               ",\"end_node\":" + node_to_json(n->end_node) +
               ",\"step_node\":" + node_to_json(n->step_node) +
               ",\"var_name_tokens\":" + token_vector_to_json(n->var_name_tokens) +
               ",\"collection_node\":" + node_to_json(n->collection_node) +
               ",\"condition_node\":" + node_to_json(n->condition_node) + "}";
    }

    return "{\"node_type\":\"UnknownNode\"}";
}
