#include <bits/stdc++.h>

#include "language_core/error.h"
#include "language_core/lexer.h"
#include "ast_results/parse_result.h"
#include "language_core/parser.h"
#include "ast_results/runtime_result.h"
#include "language_core/interpreter.h"
#include "language_core/context.h"
#include "data_types/number_type.h"

using namespace std;

pair<shared_ptr<DataType>, optional<Error> > run(const string &filename, const string &text) {
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
    RunTimeResult result = interpreter.visit(ast.node, context);

    return {result.value, result.error};
}


int main() {
    while (true) {
        cout << "code > ";
        string text;

        if (!getline(cin, text)) {
            break;
        }

        if (auto [result, error] = run("<stdin>", text); error) {
            cout << error->to_string() << endl;
        } else if (result) {
            cout << result->to_string() << endl;
        }
    }

    return 0;
}
