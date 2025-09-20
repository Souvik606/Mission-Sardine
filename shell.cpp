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

using namespace std;

auto global_symbol_table = make_shared<SymbolTable>();

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

    while (true) {
        cout << "code > ";
        string text;

        if (!getline(cin, text)) {
            break;
        }

        auto [result, error] = run("<stdin>", text);

        if (error) {
            cout << error->to_string() << endl;
        }
        else if (result) {
            cout << result->to_string() << endl;
        }
    }

    return 0;
}
