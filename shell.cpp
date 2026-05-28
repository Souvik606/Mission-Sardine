#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>

#include "language_core/error.h"
#include "language_core/lexer.h"
#include "ast_results/parse_result.h"
#include "language_core/parser.h"
#include "ast_results/runtime_result.h"
#include "language_core/interpreter.h"
#include "language_core/context.h"
#include "language_core/symbol_table.h"
#include "data_types/data_type.h"
#include "data_types/number_type.h"
#include "data_types/string_type.h"
#include "data_types/list_type.h"
#include "data_types/function_type.h"
#include "data_types/builtins.h"

using namespace std;

auto global_symbol_table = make_shared<SymbolTable>();

RunTimeResult builtin_show(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args) {
    string separator = " ";
    string end_char = "\n";

    for (const auto &[name, value] : kw_args) {
        if (name == "sep") {
            auto str_val = dynamic_pointer_cast<String>(value);
            if (!str_val) {
                return RunTimeResult().failure(RunTimeError(
                    Position(), Position(),
                    "'sep' must be a string",
                    nullptr
                ));
            }
            separator = str_val->value;
        }
        else if (name == "end") {
            auto str_val = dynamic_pointer_cast<String>(value);
            if (!str_val) {
                return RunTimeResult().failure(RunTimeError(
                    Position(), Position(),
                    "'end' must be a string",
                    nullptr
                ));
            }
            end_char = str_val->value;
        }
        else {
            return RunTimeResult().failure(RunTimeError(
                Position(), Position(),
                "Unexpected keyword argument '" + name + "' for show",
                nullptr
            ));
        }
    }

    stringstream ss;
    for (size_t i = 0; i < args.size(); ++i) {
        if (auto str_val = dynamic_pointer_cast<String>(args[i])) {
            ss << str_val->value;
        } else {
            ss << args[i]->to_string();
        }
        if (i < args.size() - 1) {
            ss << separator;
        }
    }
    ss << end_char;
    cout << ss.str();
    cout.flush();
    return RunTimeResult().success(make_shared<Number>(0LL));
}

RunTimeResult builtin_listen(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args) {
    if (!args.empty() || !kw_args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "listen() takes 0 arguments",
            nullptr
        ));
    }
    string text;
    getline(cin, text);
    return RunTimeResult().success(make_shared<String>(text));
}

RunTimeResult builtin_type(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args) {
    if (args.size() != 1 || !kw_args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "type() takes exactly 1 argument (got " + std::to_string(args.size()) + ")",
            nullptr
        ));
    }
    const auto& value = args[0];
    string type_name = "<type Unknown>";

    if (dynamic_pointer_cast<Number>(value)) {
        type_name = "<type Number>";
    }
    else if (dynamic_pointer_cast<String>(value)) {
        type_name = "<type String>";
    }
    else if (dynamic_pointer_cast<List>(value)) {
        type_name = "<type List>";
    }
    else if (dynamic_pointer_cast<Function>(value)) {
        type_name = "<type Function>";
    }
    else if (dynamic_pointer_cast<BuiltInFunction>(value)) {
        type_name = "<type BuiltInFunction>";
    }

    cout << type_name << endl;
    return RunTimeResult().success(make_shared<Number>(0LL));
}

RunTimeResult builtin_integer(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args) {
    if (args.size() != 1 || !kw_args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "Integer() takes exactly 1 argument (got " + std::to_string(args.size()) + ")",
            nullptr
        ));
    }

    const auto& value = args[0];

    if (const auto num = dynamic_pointer_cast<Number>(value)) {
        long long int_val = std::visit([](auto v) { return static_cast<long long>(v); }, num->value);
        return RunTimeResult().success(make_shared<Number>(int_val));
    }

    if (const auto str = dynamic_pointer_cast<String>(value)) {
        try {
            long long int_val = stoll(str->value);
            return RunTimeResult().success(make_shared<Number>(int_val));
        }
        catch (...) {
            return RunTimeResult().failure(RunTimeError(
                value->pos_start.value_or(Position()), value->pos_end.value_or(Position()),
                "Cannot convert '" + str->value + "' to Integer",
                value->context
            ));
        }
    }

    return RunTimeResult().failure(RunTimeError(
        value->pos_start.value_or(Position()), value->pos_end.value_or(Position()),
        "Argument to Integer() must be a Number or String",
        value->context
    ));
}

RunTimeResult builtin_string(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args) {
    if (args.size() != 1 || !kw_args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "String() takes exactly 1 argument (got " + std::to_string(args.size()) + ")",
            nullptr
        ));
    }

    const auto& value = args[0];
    string str_val;

    if (const auto str = dynamic_pointer_cast<String>(value)) {
        str_val = str->value;
    }
    else {
        str_val = value->to_string();
    }

    return RunTimeResult().success(make_shared<String>(str_val));
}

RunTimeResult builtin_size(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args) {
    if (args.size() != 1 || !kw_args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "size() takes exactly 1 argument (got " + std::to_string(args.size()) + ")",
            nullptr
        ));
    }

    const auto& value = args[0];

    if (const auto str = dynamic_pointer_cast<String>(value)) {
        return RunTimeResult().success(make_shared<Number>(static_cast<long long>(str->value.length())));
    }
    else if (const auto lst = dynamic_pointer_cast<List>(value)) {
        return RunTimeResult().success(make_shared<Number>(static_cast<long long>(lst->elements.size())));
    }

    return RunTimeResult().failure(RunTimeError(
        value->pos_start.value_or(Position()), value->pos_end.value_or(Position()),
        "illegal operation error",
        value->context
    ));
}

RunTimeResult builtin_range(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args) {
    if (args.size() != 1 || !kw_args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "range() takes exactly 1 argument (got " + std::to_string(args.size()) + ")",
            nullptr
        ));
    }
    const auto num = dynamic_pointer_cast<Number>(args[0]);
    if (!num || !holds_alternative<long long>(num->value)) {
        return RunTimeResult().failure(RunTimeError(
            args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()),
            "range() argument must be an integer",
            args[0]->context
        ));
    }
    long long n = get<long long>(num->value);
    vector<shared_ptr<DataType>> elems;
    elems.reserve(static_cast<size_t>(n > 0 ? n : 0));
    for (long long i = 0; i < n; ++i)
        elems.push_back(make_shared<Number>(i));
    return RunTimeResult().success(make_shared<List>(std::move(elems)));
}

RunTimeResult builtin_copy(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args) {
    if (args.size() != 1 || !kw_args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "copy() takes exactly 1 argument (got " + std::to_string(args.size()) + ")",
            nullptr
        ));
    }
    const auto lst = dynamic_pointer_cast<List>(args[0]);
    if (!lst) {
        return RunTimeResult().failure(RunTimeError(
            args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()),
            "copy() argument must be a List",
            args[0]->context
        ));
    }
    
    auto new_list = make_shared<List>(lst->elements);
    new_list->set_context(lst->context);
    new_list->set_pos(lst->pos_start, lst->pos_end);
    return RunTimeResult().success(new_list);
}

RunTimeResult builtin_reverse(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args) {
    if (args.size() != 3 || !kw_args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "reverse() takes exactly 3 arguments (got " + std::to_string(args.size()) + ")",
            nullptr
        ));
    }
    const auto lst = dynamic_pointer_cast<List>(args[0]);
    const auto s   = dynamic_pointer_cast<Number>(args[1]);
    const auto e   = dynamic_pointer_cast<Number>(args[2]);
    if (!lst || !s || !e ||
        !holds_alternative<long long>(s->value) ||
        !holds_alternative<long long>(e->value)) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "reverse() expects (List, int, int)",
            nullptr
        ));
    }
    
    auto new_list = make_shared<List>(lst->elements);
    new_list->set_context(lst->context);
    new_list->set_pos(lst->pos_start, lst->pos_end);
    long long lo = get<long long>(s->value);
    long long hi = get<long long>(e->value);
    long long sz = static_cast<long long>(new_list->elements.size());
    if (lo < 0 || hi >= sz) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "reverse() index out of bounds",
            nullptr
        ));
    }
    while (lo < hi) {
        std::swap(new_list->elements[lo], new_list->elements[hi]);
        ++lo; --hi;
    }
    return RunTimeResult().success(new_list);
}

RunTimeResult builtin_irev(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args) {
    if (args.size() != 3 || !kw_args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "irev() takes exactly 3 arguments (got " + std::to_string(args.size()) + ")",
            nullptr
        ));
    }
    const auto lst = dynamic_pointer_cast<List>(args[0]);
    const auto s   = dynamic_pointer_cast<Number>(args[1]);
    const auto e   = dynamic_pointer_cast<Number>(args[2]);
    if (!lst || !s || !e ||
        !holds_alternative<long long>(s->value) ||
        !holds_alternative<long long>(e->value)) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "irev() expects (List, int, int)",
            nullptr
        ));
    }
    long long lo = get<long long>(s->value);
    long long hi = get<long long>(e->value);
    long long sz = static_cast<long long>(lst->elements.size());
    if (lo < 0 || hi >= sz) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "irev() index out of bounds",
            nullptr
        ));
    }
    while (lo < hi) {
        std::swap(lst->elements[lo], lst->elements[hi]);
        ++lo; --hi;
    }
    return RunTimeResult().success(lst);
}

RunTimeResult builtin_swap(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args) {
    if (args.size() != 3 || !kw_args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "swap() takes exactly 3 arguments (got " + std::to_string(args.size()) + ")",
            nullptr
        ));
    }
    const auto lst = dynamic_pointer_cast<List>(args[0]);
    const auto ia  = dynamic_pointer_cast<Number>(args[1]);
    const auto ib  = dynamic_pointer_cast<Number>(args[2]);
    if (!lst || !ia || !ib ||
        !holds_alternative<long long>(ia->value) ||
        !holds_alternative<long long>(ib->value)) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "swap() expects (List, int, int)",
            nullptr
        ));
    }
    long long i = get<long long>(ia->value);
    long long j = get<long long>(ib->value);
    long long sz = static_cast<long long>(lst->elements.size());
    if (i < 0 || i >= sz || j < 0 || j >= sz) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "swap() index out of bounds",
            nullptr
        ));
    }
    std::swap(lst->elements[i], lst->elements[j]);
    return RunTimeResult().success(lst);  // return same list object
}

struct RunPhaseResult {
    shared_ptr<DataType> value;
    optional<Error> error;
    double lex_time   = 0.0;
    double parse_time = 0.0;
    double exec_time  = 0.0;
};

RunPhaseResult run(const string& filename, const string& text) {
    RunPhaseResult out;

    auto t0 = chrono::high_resolution_clock::now();
    Lexer lexer(filename, text);
    auto [tokens, lexer_error] = lexer.enumerate_tokens();
    auto t1 = chrono::high_resolution_clock::now();
    out.lex_time = chrono::duration<double>(t1 - t0).count();

    if (lexer_error) {
        out.error = lexer_error;
        return out;
    }

    auto t2 = chrono::high_resolution_clock::now();
    Parser parser(std::move(tokens));
    ParseResult ast = parser.parse();
    auto t3 = chrono::high_resolution_clock::now();
    out.parse_time = chrono::duration<double>(t3 - t2).count();

    if (ast.error) {
        out.error = ast.error;
        return out;
    }

    auto t4 = chrono::high_resolution_clock::now();
    Interpreter interpreter;
    auto context = make_shared<Context>("<program>");
    context->symbol_table = global_symbol_table;
    RunTimeResult result = interpreter.visit(ast.node, context);
    auto t5 = chrono::high_resolution_clock::now();
    out.exec_time = chrono::duration<double>(t5 - t4).count();

    out.value = result.value;
    out.error = result.error;
    return out;
}

void run_file(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cout << "Error: File '" << filepath << "' not found.\n";
        return;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string file_content = buffer.str();

    RunPhaseResult r = run(filepath, file_content);

    if (r.error) {
        cout << "Error in " << filepath << ":\n";
        cout << r.error->to_string() << "\n";
    }
    else if (r.value) {
        // top-level expression result (usually 0/null, suppress)
    }

    double total = r.lex_time + r.parse_time + r.exec_time;
    cout << "\n--- Phase Timings ---\n";
    cout << fixed << setprecision(6);
    cout << "  Lexer  (tokenise): " << r.lex_time   * 1000.0 << " ms\n";
    cout << "  Parser (AST):      " << r.parse_time  * 1000.0 << " ms\n";
    cout << "  Interpreter:       " << r.exec_time   * 1000.0 << " ms\n";
    cout << "  Total:             " << total         * 1000.0 << " ms\n";
    cout << "---------------------\n";
}

int main() {
    global_symbol_table->set("None", make_shared<Number>(0LL));
    global_symbol_table->set("null", make_shared<Number>(0LL));
    global_symbol_table->set("True", make_shared<Number>(1LL));
    global_symbol_table->set("False", make_shared<Number>(0LL));

    global_symbol_table->set("show", make_shared<BuiltInFunction>("show", builtin_show));
    global_symbol_table->set("listen", make_shared<BuiltInFunction>("listen", builtin_listen));
    global_symbol_table->set("type", make_shared<BuiltInFunction>("type", builtin_type));
    global_symbol_table->set("Integer", make_shared<BuiltInFunction>("Integer", builtin_integer));
    global_symbol_table->set("String", make_shared<BuiltInFunction>("String", builtin_string));
    global_symbol_table->set("size",    make_shared<BuiltInFunction>("size",    builtin_size));
    global_symbol_table->set("range",   make_shared<BuiltInFunction>("range",   builtin_range));
    global_symbol_table->set("copy",    make_shared<BuiltInFunction>("copy",    builtin_copy));
    global_symbol_table->set("reverse", make_shared<BuiltInFunction>("reverse", builtin_reverse));
    global_symbol_table->set("irev",    make_shared<BuiltInFunction>("irev",    builtin_irev));
    global_symbol_table->set("swap",    make_shared<BuiltInFunction>("swap",    builtin_swap));

    string choice;
    cout << "Enter 0 for REPL mode and 1 for file input: ";
    if (!getline(cin, choice)) return 0; // Handle EOF early

    if (choice == "0") {
        while (true) {
            cout << "code > ";
            string text;

            if (!getline(cin, text)) {
                cout << "\nGoodbye!\n";
                break;
            }

            string lower_text = text;
            transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
            if (lower_text == "exit" || lower_text == "quit") {
                cout << "Goodbye!\n";
                break;
            }

            if (text.empty()) continue;

            if (auto r = run("<stdin>", text); r.error) {
                cout << r.error->to_string() << endl;
            }
            else if (r.value) {
                cout << r.value->to_string() << endl;
            }
        }
    }
    else {
        run_file("samples/main.sad");
    }

    return 0;
}