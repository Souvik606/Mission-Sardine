#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "language_core/error.h"
#include "language_core/constants.h"
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
#include "data_types/primitive_methods.h"

using namespace std;

auto global_symbol_table = make_shared<SymbolTable>();

RunTimeResult builtin_show(const Position& pos_start, const Position& pos_end, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    string separator = " ";
    string end_char = "\n";

    for (const auto &[name, value] : kw_args) {
        if (name == "sep") {
            auto str_val = dynamic_pointer_cast<String>(value);
            if (!str_val) {
                return RunTimeResult().failure(ArgumentError(
                    pos_start, pos_end,
                    "'sep' must be a string",
                    context->parent ? context->parent : context
                ));
            }
            separator = str_val->value;
        }
        else if (name == "end") {
            auto str_val = dynamic_pointer_cast<String>(value);
            if (!str_val) {
                return RunTimeResult().failure(ArgumentError(
                    pos_start, pos_end,
                    "'end' must be a string",
                    context->parent ? context->parent : context
                ));
            }
            end_char = str_val->value;
        }
        else {
            return RunTimeResult().failure(ArgumentError(
                pos_start, pos_end,
                "Unexpected keyword argument '" + name + "' for show",
                context->parent ? context->parent : context
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

RunTimeResult builtin_listen(const Position& pos_start, const Position& pos_end, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    if (!args.empty() || !kw_args.empty()) {
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
            "listen() takes 0 arguments",
            context->parent ? context->parent : context
        ));
    }
    string text;
    getline(cin, text);
    return RunTimeResult().success(make_shared<String>(text));
}

RunTimeResult builtin_type(const Position& pos_start, const Position& pos_end, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    if (args.size() != 1 || !kw_args.empty()) {
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
            "type() takes exactly one argument",
            context->parent ? context->parent : context
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

RunTimeResult builtin_integer(const Position& pos_start, const Position& pos_end, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    if (args.size() != 1 || !kw_args.empty()) {
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
            "Integer() takes exactly one argument",
            context->parent ? context->parent : context
        ));
    }

    const auto& value = args[0];

    if (const auto num = dynamic_pointer_cast<Number>(value)) {
        if (holds_alternative<double>(num->value)) {
            double d_val = get<double>(num->value);
            if (std::isnan(d_val) || std::isinf(d_val) || d_val < static_cast<double>(LLONG_MIN) || d_val > static_cast<double>(LLONG_MAX)) {
                return RunTimeResult().failure(ArgumentError(
                    pos_start, pos_end,
                    "Argument must be a value convertible to an integer",
                    context->parent ? context->parent : context
                ));
            }
        }
        long long int_val = std::visit([](auto v) { return static_cast<long long>(v); }, num->value);
        return RunTimeResult().success(make_shared<Number>(int_val));
    }

    if (const auto str = dynamic_pointer_cast<String>(value)) {
        try {
            long long int_val = stoll(str->value);
            return RunTimeResult().success(make_shared<Number>(int_val));
        }
        catch (...) {
            return RunTimeResult().failure(ArgumentError(
                pos_start, pos_end,
                "Argument must be a value convertible to an integer",
                context->parent ? context->parent : context
            ));
        }
    }

    return RunTimeResult().failure(ArgumentError(
        pos_start, pos_end,
        "Argument must be a primitive value (Number or String)",
        context->parent ? context->parent : context
    ));
}

RunTimeResult builtin_string(const Position& pos_start, const Position& pos_end, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    if (args.size() != 1 || !kw_args.empty()) {
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
            "String() takes exactly one argument",
            context->parent ? context->parent : context
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


RunTimeResult builtin_super(const Position& pos_start, const Position& pos_end, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    if (!args.empty() || !kw_args.empty()) {
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
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
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
            "super() can only be called inside a method body",
            context
        ));
    }

    if (!owner_class) {
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
            "super() could not determine the current class \u2014 make sure you are calling it inside a named method",
            context
        ));
    }

    auto proxy = make_shared<SuperProxy>(instance, owner_class);
    proxy->set_context(context);
    return RunTimeResult().success(proxy);
}

RunTimeResult builtin_is_a(const Position& pos_start, const Position& pos_end, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    if (args.size() != 2 || !kw_args.empty()) {
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
            "is_a() takes exactly 2 positional arguments: is_a(object, ModelClass)",
            context->parent ? context->parent : context
        ));
    }

    auto obj = args[0];
    auto model_class = dynamic_pointer_cast<ModelType>(args[1]);

    if (!model_class) {
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
            "Second argument to is_a() must be a model class",
            context->parent ? context->parent : context
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

class CleanExitException : public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "CleanExitException";
    }
};

bool is_integer(const shared_ptr<DataType>& arg, long long& out_val) {
    auto num = dynamic_pointer_cast<Number>(arg);
    if (!num || num->is_float) return false;
    if (holds_alternative<long long>(num->value)) {
        out_val = get<long long>(num->value);
        return true;
    }
    return false;
}

string get_type_name(const shared_ptr<DataType>& value) {
    if (dynamic_pointer_cast<Number>(value)) return "Number";
    if (dynamic_pointer_cast<String>(value)) return "String";
    if (dynamic_pointer_cast<List>(value)) return "List";
    if (dynamic_pointer_cast<Dict>(value)) return "Dict";
    if (dynamic_pointer_cast<Function>(value)) return "Function";
    if (dynamic_pointer_cast<BuiltInFunction>(value)) return "BuiltInFunction";
    if (dynamic_pointer_cast<ModelType>(value)) return "Model";
    if (dynamic_pointer_cast<ModelInstance>(value)) return "ModelInstance";
    if (dynamic_pointer_cast<SuperProxy>(value)) return "SuperProxy";
    if (dynamic_pointer_cast<Module>(value)) return "Module";
    return "Unknown";
}

RunTimeResult builtin_error(const Position& pos_start, const Position& pos_end, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    if (!kw_args.empty()) {
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
            "error() takes no keyword arguments",
            context->parent ? context->parent : context
        ));
    }
    string msg = "Illegal operation";
    if (!args.empty()) {
        auto str_val = dynamic_pointer_cast<String>(args[0]);
        if (!str_val) {
            return RunTimeResult().failure(ArgumentError(
                pos_start, pos_end,
                "Error message must be a String value",
                context->parent ? context->parent : context
            ));
        }
        msg = str_val->value;
    }
    return RunTimeResult().failure(IllegalOperationError(
        pos_start, pos_end,
        msg,
        context->parent ? context->parent : context
    ));
}

RunTimeResult builtin_len(const Position& pos_start, const Position& pos_end, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    if (args.size() != 1 || !kw_args.empty()) {
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
            "len() takes exactly 1 argument",
            context
        ));
    }

    auto arg = args[0];
    if (auto list_val = dynamic_pointer_cast<List>(arg)) {
        return RunTimeResult().success(make_shared<Number>(static_cast<long long>(list_val->elements.size())));
    }
    if (auto str_val = dynamic_pointer_cast<String>(arg)) {
        return RunTimeResult().success(make_shared<Number>(static_cast<long long>(str_val->value.length())));
    }
    if (auto dict_val = dynamic_pointer_cast<Dict>(arg)) {
        return RunTimeResult().success(make_shared<Number>(static_cast<long long>(dict_val->elements.size())));
    }
    if (auto inst_val = dynamic_pointer_cast<ModelInstance>(arg)) {
        auto [len_result, len_error] = inst_val->_call_op_method("__len__", {});
        if (len_error) {
            return RunTimeResult().failure(*len_error);
        }
        if (len_result) {
            if (auto num_val = dynamic_pointer_cast<Number>(len_result)) {
                return RunTimeResult().success(num_val);
            } else {
                return RunTimeResult().failure(IllegalOperationError(
                    pos_start, pos_end,
                    "__len__ must return a Number, not '" + get_type_name(len_result) + "'",
                    context
                ));
            }
        }
        Interpreter interp;
        auto [length_attr, attr_err] = inst_val->get_attr("length", interp, context);
        if (!attr_err && length_attr) {
            if (auto num_val = dynamic_pointer_cast<Number>(length_attr)) {
                return RunTimeResult().success(num_val);
            }
        }
        return RunTimeResult().failure(IllegalOperationError(
            pos_start, pos_end,
            "Type '" + inst_val->model->name + "' has no length",
            context
        ));
    }

    return RunTimeResult().failure(IllegalOperationError(
        pos_start, pos_end,
        "Type '" + get_type_name(arg) + "' has no length",
        context
    ));
}

string format_double_as_clean_int(double d) {
    if (d < 1e15) {
        stringstream ss;
        ss << std::fixed << std::setprecision(0) << d;
        return ss.str();
    }
    stringstream ss;
    ss << std::scientific << std::setprecision(14) << d;
    string s = ss.str();
    
    size_t e_pos = s.find('e');
    if (e_pos == string::npos) return s;
    
    string coeff = s.substr(0, e_pos);
    int exp = stoi(s.substr(e_pos + 1));
    
    size_t dot_pos = coeff.find('.');
    if (dot_pos != string::npos) {
        coeff.erase(dot_pos, 1);
        int frac_len = coeff.length() - dot_pos;
        if (exp >= frac_len) {
            coeff += string(exp - frac_len, '0');
        } else {
            coeff.insert(dot_pos + exp, ".");
        }
    } else {
        coeff += string(exp, '0');
    }
    
    if (coeff.find('.') != string::npos) {
        while (!coeff.empty() && coeff.back() == '0') coeff.pop_back();
        if (!coeff.empty() && coeff.back() == '.') coeff.pop_back();
    }
    return coeff;
}

bool get_range_arg(const shared_ptr<DataType>& arg, double& out_val) {
    auto num = dynamic_pointer_cast<Number>(arg);
    if (!num || num->is_float) return false;
    if (holds_alternative<long long>(num->value)) {
        out_val = static_cast<double>(get<long long>(num->value));
    } else {
        out_val = get<double>(num->value);
    }
    return true;
}

RunTimeResult builtin_range(const Position& pos_start, const Position& pos_end, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    if (args.empty() || args.size() > 3 || !kw_args.empty()) {
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
            "range() takes 1, 2, or 3 arguments",
            context->parent ? context->parent : context
        ));
    }

    vector<double> vals;
    for (size_t i = 0; i < args.size(); ++i) {
        double val = 0.0;
        if (!get_range_arg(args[i], val)) {
            return RunTimeResult().failure(IllegalOperationError(
                args[i]->pos_start.value_or(Position()), args[i]->pos_end.value_or(Position()),
                "Argument " + to_string(i + 1) + " to range() must be an integer Number",
                context->parent ? context->parent : context
            ));
        }
        vals.push_back(val);
    }

    double start = 0.0;
    double end = 0.0;
    double step = 1.0;

    if (vals.size() == 1) {
        end = vals[0];
    } else if (vals.size() == 2) {
        start = vals[0];
        end = vals[1];
    } else {
        start = vals[0];
        end = vals[1];
        step = vals[2];
    }

    if (step == 0.0) {
        return RunTimeResult().failure(IllegalOperationError(
            args.back()->pos_start.value_or(Position()), args.back()->pos_end.value_or(Position()),
            "range() step cannot be 0",
            context->parent ? context->parent : context
        ));
    }

    double diff = (step > 0.0) ? (end - start) : (start - end);
    double num_elements_double = 0.0;
    if ((step > 0.0 && start < end) || (step < 0.0 && start > end)) {
        num_elements_double = std::ceil(diff / std::abs(step));
    }

    if (num_elements_double > 1000000.0) {
        return RunTimeResult().failure(ValueError(
            args.front()->pos_start.value_or(Position()), args.back()->pos_end.value_or(Position()),
            "range() limit exceeded (size " + format_double_as_clean_int(num_elements_double) + " > 1,000,000 limit)",
            context->parent ? context->parent : context
        ));
    }

    // Now check if start, end, step exceed standard long long limits
    if (start < static_cast<double>(LLONG_MIN) || start > static_cast<double>(LLONG_MAX) ||
        end < static_cast<double>(LLONG_MIN) || end > static_cast<double>(LLONG_MAX) ||
        step < static_cast<double>(LLONG_MIN) || step > static_cast<double>(LLONG_MAX)) {
        return RunTimeResult().failure(ValueError(
            args.front()->pos_start.value_or(Position()), args.back()->pos_end.value_or(Position()),
            "range() boundary overflow",
            context->parent ? context->parent : context
        ));
    }

    long long start_ll = static_cast<long long>(start);
    long long end_ll = static_cast<long long>(end);
    long long step_ll = static_cast<long long>(step);

    if (step_ll > 0) {
        if (start_ll < 0 && end_ll > LLONG_MAX + start_ll) {
            return RunTimeResult().failure(ValueError(
                args.front()->pos_start.value_or(Position()), args.back()->pos_end.value_or(Position()),
                "range() boundary overflow",
                context->parent ? context->parent : context
            ));
        }
    } else {
        if (start_ll > 0 && end_ll < LLONG_MIN + start_ll) {
            return RunTimeResult().failure(ValueError(
                args.front()->pos_start.value_or(Position()), args.back()->pos_end.value_or(Position()),
                "range() boundary overflow",
                context->parent ? context->parent : context
            ));
        }
    }

    long long num_elements = 0;
    if ((step_ll > 0 && start_ll < end_ll) || (step_ll < 0 && start_ll > end_ll)) {
        long long diff_ll = (step_ll > 0) ? (end_ll - start_ll) : (start_ll - end_ll);
        long long abs_step = (step_ll > 0) ? step_ll : -step_ll;
        num_elements = diff_ll / abs_step;
        if (diff_ll % abs_step != 0) {
            num_elements += 1;
        }
    }

    vector<shared_ptr<DataType>> elements;
    elements.reserve(num_elements);
    long long current = start_ll;
    if (step_ll > 0) {
        while (current < end_ll) {
            auto num_obj = make_shared<Number>(current);
            num_obj->set_context(context);
            elements.push_back(num_obj);
            current += step_ll;
        }
    } else {
        while (current > end_ll) {
            auto num_obj = make_shared<Number>(current);
            num_obj->set_context(context);
            elements.push_back(num_obj);
            current += step_ll;
        }
    }

    auto list_obj = make_shared<List>(elements);
    list_obj->set_context(context);
    list_obj->set_pos(pos_start, pos_end);
    return RunTimeResult().success(list_obj);
}

RunTimeResult builtin_exit(const Position& pos_start, const Position& pos_end, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) {
    if (!args.empty() || !kw_args.empty()) {
        return RunTimeResult().failure(ArgumentError(
            pos_start, pos_end,
            "exit() takes no arguments",
            context->parent ? context->parent : context
        ));
    }
    throw CleanExitException();
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
    global_symbol_table->set("error", make_shared<BuiltInFunction>("error", builtin_error));
    global_symbol_table->set("len", make_shared<BuiltInFunction>("len", builtin_len));
    global_symbol_table->set("range", make_shared<BuiltInFunction>("range", builtin_range));
    global_symbol_table->set("exit", make_shared<BuiltInFunction>("exit", builtin_exit));

    vector<string> args;
    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    auto it = find(args.begin(), args.end(), "--unbounded");
    if (it != args.end()) {
        UNBOUNDED_MODE = true;
        args.erase(it);
    }

    if (args.size() > 1) {
        try {
            run_file(args[1]);
        } catch (const CleanExitException&) {
            // Clean exit
        }
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

            try {
                if (auto r = run("<stdin>", text); r.error) {
                    cout << r.error->to_string() << endl;
                }
                else if (r.value) {
                    cout << r.value->to_string() << endl;
                }
            } catch (const CleanExitException&) {
                cout << "Goodbye!\n";
                break;
            }
        }
    }
    else {
        try {
            run_file("samples/main.sad");
        } catch (const CleanExitException&) {
            // Clean exit
        }
    }

    return 0;
}