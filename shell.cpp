#include <bits/stdc++.h>

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
    } else if (dynamic_pointer_cast<String>(value)) {
        type_name = "<type String>";
    } else if (dynamic_pointer_cast<List>(value)) {
        type_name = "<type List>";
    } else if (dynamic_pointer_cast<Function>(value)) {
        type_name = "<type Function>";
    } else if (dynamic_pointer_cast<BuiltInFunction>(value)) {
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
        long long int_val = std::visit([](auto v){ return static_cast<long long>(v); }, num->value);
        return RunTimeResult().success(make_shared<Number>(int_val));
    }

    if (const auto str = dynamic_pointer_cast<String>(value)) {
        try {
            long long int_val = stoll(str->value);
            return RunTimeResult().success(make_shared<Number>(int_val));
        } catch (...) {
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
    } else {
        str_val = value->to_string();
    }

    return RunTimeResult().success(make_shared<String>(str_val));
}


pair<shared_ptr<DataType>, optional<Error>> run(const string& filename, const string& text) {
    Lexer lexer(filename, text);
    auto [tokens, lexer_error] = lexer.enumerate_tokens();
    if (lexer_error) {
        return {nullptr, lexer_error};
    }

    Parser parser(std::move(tokens));
    ParseResult ast = parser.parse();
    if (ast.error) {
        return {nullptr, ast.error};
    }

    Interpreter interpreter;
    auto context = make_shared<Context>("<program>");
    context->symbol_table = global_symbol_table;
    RunTimeResult result = interpreter.visit(ast.node, context);

    return {result.value, result.error};
}

int main() {
    global_symbol_table->set("null", make_shared<Number>(0LL));

    global_symbol_table->set("show",
        make_shared<BuiltInFunction>("show", builtin_show)
    );

    global_symbol_table->set("listen",
        make_shared<BuiltInFunction>("listen", builtin_listen)
    );

    global_symbol_table->set("type",
        make_shared<BuiltInFunction>("type", builtin_type)
    );

    global_symbol_table->set("Integer",
        make_shared<BuiltInFunction>("Integer", builtin_integer)
    );

    global_symbol_table->set("String",
        make_shared<BuiltInFunction>("String", builtin_string)
    );

    while (true) {
        cout << "code > ";
        string text;

        if (!getline(cin, text)) {
            break;
        }

        if (auto [result, error] = run("<stdin>", text); error) {
            cout << error->to_string() << endl;
        }
        else if (result) {
            cout << result->to_string() << endl;
        }
    }

    return 0;
}