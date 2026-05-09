#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>

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

RunTimeResult builtin_show(const vector<shared_ptr<DataType>>& args) {
    if (args.size() != 1) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "show() takes exactly 1 argument (got " + std::to_string(args.size()) + ")",
            nullptr
        ));
    }
    cout << args[0]->to_string() << endl;
    return RunTimeResult().success(make_shared<Number>(0LL));
};

RunTimeResult builtin_listen(const vector<shared_ptr<DataType>>& args) {
    if (!args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "listen() takes 0 arguments (got " + std::to_string(args.size()) + ")",
            nullptr
        ));
    }
    string text;
    getline(cin, text);
    return RunTimeResult().success(make_shared<String>(text));
};

RunTimeResult builtin_type(const vector<shared_ptr<DataType>>& args) {
    if (args.size() != 1) {
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
};

RunTimeResult builtin_integer(const vector<shared_ptr<DataType>>& args) {
    if (args.size() != 1) {
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

RunTimeResult builtin_string(const vector<shared_ptr<DataType>>& args) {
    if (args.size() != 1) {
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

RunTimeResult builtin_size(const vector<shared_ptr<DataType>>& args) {
    if (args.size() != 1) {
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

pair<shared_ptr<DataType>, optional<Error>> run(const string& filename, const string& text) {
    Lexer lexer(filename, text);
    auto [tokens, lexer_error] = lexer.enumerate_tokens();
    if (lexer_error) {
        return { nullptr, lexer_error };
    }

    // Print tokens for debugging
    // cout << "--- Tokens ---" << endl;
    // for (const auto& token : tokens) {
    //     cout << token.to_string() << endl;
    // }
    // cout << "--------------" << endl;

    Parser parser(std::move(tokens));
    ParseResult ast = parser.parse();
    if (ast.error) {
        return { nullptr, ast.error };
    }

    // Print AST for debugging
    // cout << "--- AST ---" << endl;
    // if (ast.node) cout << ast.node->to_string() << endl;
    // cout << "-----------" << endl;

    Interpreter interpreter;
    auto context = make_shared<Context>("<program>");
    context->symbol_table = global_symbol_table;
    RunTimeResult result = interpreter.visit(ast.node, context);

    return { result.value, result.error };
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

    auto start_time = chrono::high_resolution_clock::now();
    auto [result, error] = run(filepath, file_content);
    auto end_time = chrono::high_resolution_clock::now();

    if (error) {
        cout << "Error in " << filepath << ":\n";
        cout << error->to_string() << "\n";
    }
    else if (result) {
        cout << result->to_string() << "\n";
    }

    chrono::duration<double> exec_time = end_time - start_time;
    cout << "Execution time: " << exec_time.count() << " seconds\n";
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
    global_symbol_table->set("size", make_shared<BuiltInFunction>("size", builtin_size));

    string choice;
    cout << "Enter 0 for REPL mode and 1 for file input: ";
    if (!getline(cin, choice)) return 0; // Handle EOF early

    if (choice == "0") {
        // REPL Mode
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

            if (auto [result, error] = run("<stdin>", text); error) {
                cout << error->to_string() << endl;
            }
            else if (result) {
                cout << result->to_string() << endl;
            }
        }
    }
    else {
        run_file("samples/main.sad");
    }

    return 0;
}