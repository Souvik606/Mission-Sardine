#include <bits/stdc++.h>
#include "lexer.h"

using namespace std;

pair<vector<Token>, optional<Error>> run(const string& filename, const string& text) {
    Lexer lexer(filename, text);
    auto result = lexer.enumerate_tokens();
    return result;
}

int main() {
    while (true) {
        cout << "code > ";
        string text;

        if (!getline(cin, text)) {
            break;
        }

        if (auto [tokens, error] = run("<stdin>", text); error.has_value()) {
            cout << error->to_string() << endl;
        } else {
            cout << "[";
            for (size_t i = 0; i < tokens.size(); ++i) {
                cout << tokens[i].to_string();
                if (i < tokens.size() - 1) {
                    cout << ", ";
                }
            }
            cout << "]" << endl;
        }
    }

    return 0;
}