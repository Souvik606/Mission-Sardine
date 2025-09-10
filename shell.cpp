#include <bits/stdc++.h>
#include "error.h"
#include "lexer.h"
#include "ast_nodes/operation_nodes.h"
#include "ast_results/parse_result.h"
#include "parser.h"

using namespace std;

pair<shared_ptr<Node>, optional<Error>> run(const string& filename, const string& text) {
    Lexer lexer(filename, text);
    auto [tokens, lexer_error] = lexer.enumerate_tokens();

    if (lexer_error) {
        return {nullptr, lexer_error};
    }

    Parser parser(std::move(tokens));
    ParseResult ast = parser.parse();

    return {ast.node, ast.error};
}


int main() {
    while (true) {
        cout << "code > ";
        string text;

        if (!getline(cin, text)) {
            break;
        }

        if (auto [ast, error] = run("<stdin>", text); error) {
            cout << error->to_string() << endl;
        }
        else if (ast) {
            cout << ast->to_string() << endl;
        }
    }

    return 0;
}