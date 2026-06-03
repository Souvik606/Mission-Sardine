#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <algorithm>

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
#include "data_types/dict_type.h"
#include "data_types/function_type.h"
#include "data_types/builtins.h"
#include "data_types/model_type.h"
#include "data_types/module_type.h"
#include "data_types/super_proxy.h"

using namespace std;

auto global_symbol_table = make_shared<SymbolTable>();

RunTimeResult builtin_show(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
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

RunTimeResult builtin_listen(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
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

RunTimeResult builtin_type(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
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
    else if (dynamic_pointer_cast<Dict>(value)) {
        type_name = "<type Dict>";
    }
    else if (dynamic_pointer_cast<Function>(value)) {
        type_name = "<type Function>";
    }
    else if (dynamic_pointer_cast<BuiltInFunction>(value)) {
        type_name = "<type BuiltInFunction>";
    }
    else if (dynamic_pointer_cast<ModelType>(value)) {
        type_name = "<type Model>";
    }
    else if (dynamic_pointer_cast<ModelInstance>(value)) {
        type_name = "<type ModelInstance>";
    }
    else if (dynamic_pointer_cast<SuperProxy>(value)) {
        type_name = "<type SuperProxy>";
    }
    else if (dynamic_pointer_cast<Module>(value)) {
        type_name = "<type Module>";
    }

    return RunTimeResult().success(make_shared<String>(type_name));
}

RunTimeResult builtin_integer(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
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

RunTimeResult builtin_string(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
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

RunTimeResult builtin_super(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    if (!args.empty() || !kw_args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "super() takes no arguments",
            context
        ));
    }

    shared_ptr<Context> search_ctx = context;
    shared_ptr<ModelInstance> instance = nullptr;
    shared_ptr<ModelType> owner_class = nullptr;

    while (search_ctx != nullptr) {
        if (!instance) {
            auto candidate = search_ctx->symbol_table ? search_ctx->symbol_table->get("this") : nullptr;
            if (candidate) {
                instance = dynamic_pointer_cast<ModelInstance>(candidate);
            }
        }
        if (!owner_class && search_ctx->owner_class) {
            owner_class = search_ctx->owner_class;
        }
        if (instance && owner_class) {
            break;
        }
        search_ctx = search_ctx->parent;
    }

    if (!instance) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "super() can only be called inside a method body",
            context
        ));
    }

    if (!owner_class) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "super() could not determine the current class - make sure you are calling it inside a named method",
            context
        ));
    }

    auto proxy = make_shared<SuperProxy>(instance, owner_class);
    proxy->set_context(context);
    return RunTimeResult().success(proxy);
}

RunTimeResult builtin_is_a(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    if (args.size() != 2 || !kw_args.empty()) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "is_a() takes exactly 2 positional arguments: is_a(object, ModelClass)",
            context
        ));
    }

    auto obj = args[0];
    auto model_class = dynamic_pointer_cast<ModelType>(args[1]);

    if (!model_class) {
        return RunTimeResult().failure(RunTimeError(
            Position(), Position(),
            "Second argument to is_a() must be a model class",
            context
        ));
    }

    auto model_inst = dynamic_pointer_cast<ModelInstance>(obj);
    if (!model_inst) {
        // Primitive types are never instances of any user-defined model
        return RunTimeResult().success(make_shared<Number>(0LL));
    }

    bool result = model_inst->model->is_descendant_of(model_class);
    return RunTimeResult().success(make_shared<Number>(result ? 1LL : 0LL));
}

struct RunResult {
    shared_ptr<DataType> value = nullptr;
    shared_ptr<Error> error = nullptr;
};

RunResult run(const string& filename, const string& text) {
    RunResult out;

    Lexer lexer(filename, text);
    auto [tokens, lexer_error] = lexer.enumerate_tokens();
    if (lexer_error) {
        out.error = lexer_error;
        return out;
    }

    Parser parser(std::move(tokens));
    ParseResult ast = parser.parse();
    if (ast.error) {
        out.error = ast.error;
        return out;
    }

    Interpreter interpreter;
    auto context = make_shared<Context>("<program>");
    context->symbol_table = global_symbol_table;
    RunTimeResult result = interpreter.visit(ast.node, context);

    out.value = result.value;
    out.error = result.error;
    return out;
}

inline string get_relative_path(const string& file_path) {
    if (file_path.empty()) return "";
    try {
        namespace fs = std::filesystem;
        fs::path p(file_path);
        if (p.is_absolute()) {
            auto rel = fs::relative(p, fs::current_path());
            string r_str = rel.generic_string();
            return r_str;
        }
        string r_str = file_path;
        for (char& c : r_str) {
            if (c == '\\') c = '/';
        }
        return r_str;
    } catch (...) {
        string r_str = file_path;
        for (char& c : r_str) {
            if (c == '\\') c = '/';
        }
        return r_str;
    }
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

    RunResult r = run(filepath, file_content);

    if (r.error) {
        cout << "Error in " << get_relative_path(filepath) << ":\n";
        cout << r.error->to_string() << "\n";
    }
}

#ifdef __EMSCRIPTEN__
int main() {
    return 0;
}
#else
int main(int argc, char* argv[]) {
    global_symbol_table->set("None", make_shared<Number>(0LL));
    global_symbol_table->set("null", make_shared<Number>(0LL));
    global_symbol_table->set("True", make_shared<Number>(1LL));
    global_symbol_table->set("False", make_shared<Number>(0LL));

    global_symbol_table->set("show", make_shared<BuiltInFunction>("show", builtin_show));
    global_symbol_table->set("listen", make_shared<BuiltInFunction>("listen", builtin_listen));
    global_symbol_table->set("type", make_shared<BuiltInFunction>("type", builtin_type));
    global_symbol_table->set("Integer", make_shared<BuiltInFunction>("Integer", builtin_integer));
    global_symbol_table->set("String", make_shared<BuiltInFunction>("String", builtin_string));
    global_symbol_table->set("super", make_shared<BuiltInFunction>("super", builtin_super));
    global_symbol_table->set("is_a", make_shared<BuiltInFunction>("is_a", builtin_is_a));

    if (argc > 1) {
        run_file(argv[1]);
        return 0;
    }

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
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void run_interpreter(const char* raw_code) {
        // Reset and rebuild the global symbol table for a fresh execution context
        global_symbol_table = make_shared<SymbolTable>();
        global_symbol_table->set("None", make_shared<Number>(0LL));
        global_symbol_table->set("null", make_shared<Number>(0LL));
        global_symbol_table->set("True", make_shared<Number>(1LL));
        global_symbol_table->set("False", make_shared<Number>(0LL));

        global_symbol_table->set("show", make_shared<BuiltInFunction>("show", builtin_show));
        global_symbol_table->set("listen", make_shared<BuiltInFunction>("listen", builtin_listen));
        global_symbol_table->set("type", make_shared<BuiltInFunction>("type", builtin_type));
        global_symbol_table->set("Integer", make_shared<BuiltInFunction>("Integer", builtin_integer));
        global_symbol_table->set("String", make_shared<BuiltInFunction>("String", builtin_string));
        global_symbol_table->set("super", make_shared<BuiltInFunction>("super", builtin_super));
        global_symbol_table->set("is_a", make_shared<BuiltInFunction>("is_a", builtin_is_a));

        string code(raw_code);
        RunResult r = run("<stdin>", code);
        if (r.error) {
            cout << r.error->to_string() << "\n";
        }
    }
}
#endif