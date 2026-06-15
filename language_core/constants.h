#pragma once

#include <bits/stdc++.h>

using namespace std;

const string DIGITS = "0123456789";
const string LETTERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const string LETTERS_DIGITS = LETTERS + DIGITS;

const string T_INT = "INT";
const string T_FLOAT = "FLOAT";
const string T_STRING = "STRING";
const string T_FSTRING = "FSTRING";
const string T_IDENTIFIER = "IDENTIFIER";
const string T_KEYWORD = "KEYWORD";
const string T_PLUS = "PLUS";
const string T_MINUS = "MINUS";
const string T_MUL = "MUL";
const string T_DIVIDE = "DIV";
const string T_MODULUS = "MOD";
const string T_FLOOR = "FLOOR";
const string T_EXP = "EXP";
const string T_EQ = "EQUAL";
const string T_PLUSEQUAL = "PLUSEQUAL";
const string T_MINUSEQUAL = "MINUSEQUAL";
const string T_MULEQUAL = "MULEQUAL";
const string T_DIVIDEEQUAL = "DIVIDEEQUAL";
const string T_MODULUSEQUAL = "MODULUSEQUAL";
const string T_FLOOREQUAL = "FLOOREQUAL";
const string T_EXPEQUAL = "EXPEQUAL";
const string T_BITAND = "BITAND";
const string T_BITANDEQUAL = "BITANDEQUAL";
const string T_BITXOR = "BITXOR";
const string T_BITXOREQUAL = "BITXOREQUAL";
const string T_BITOR = "BITOR";
const string T_BITOREQUAL = "BITOREQUAL";
const string T_BITNOT = "BITNOT";
const string T_LSHIFT = "LSHIFT";
const string T_LSHIFTEQUAL = "LSHIFTEQUAL";
const string T_RSHIFT = "RSHIFT";
const string T_RSHIFTEQUAL = "RSHIFTEQUAL";
const string T_ARROW = "ARROW";
const string T_LARROW = "LARROW";
const string T_NEQ = "NOTEQUAL";
const string T_EE = "DOUBLEEQUAL";
const string T_LT = "LESSTHAN";
const string T_GT = "GREATERTHAN";
const string T_LTE = "LESSERTHANEQUAL";
const string T_GTE = "GREATERTHANEQUAL";
const string T_LPAREN = "LPAREN";
const string T_RPAREN = "RPAREN";
const string T_LPAREN2 = "LPAREN2";
const string T_RPAREN2 = "RPAREN2";
const string T_LPAREN3 = "LPAREN3";
const string T_RPAREN3 = "RPAREN3";
const string T_COLON = "COLON";
const string T_QUESTION = "QUESTION";
const string T_COMMA = "COMMA";
const string T_NEWLINE = "NEWLINE";
const string T_EOF = "EOF";
const string T_ERROR = "ERROR";
const string T_DOT = "DOT";

#include "position.h"

const int MAX_AST_DEPTH = 60;
const int MAX_RECURSION_DEPTH = 100;
inline bool UNBOUNDED_MODE = false;
inline bool EDUCATIONAL_MODE = false;
inline bool JSON_OUTPUT = false;
inline string MAIN_PROGRAM_FILENAME = "";

struct ExecutionTraceVar {
    string name;
    string value;
    string type;
    vector<ExecutionTraceVar> props;
    bool is_accessed = false;
};

struct ExecutionTraceScope {
    string name;
    string parent_name;
    vector<ExecutionTraceVar> variables;
};

struct ExecutionTraceStep {
    Position pos_start;
    Position pos_end;
    string node_type;
    vector<ExecutionTraceScope> scopes;
};

inline vector<ExecutionTraceStep> EXECUTION_TRACE;


const vector<string> KEYWORDS = {
    "define", "and", "or", "not","when","orwhen","otherwise","cycle",
    "during","method","yield","escape","proceed", "menu", "choice", "fallback",
    "risk", "trap", "clean",
    "model", "attr", "init",
    "open", "secret", "guarded",
    "trace", "summon", "from", "as"
};

const vector<string> ERROR_TYPES = {
    "RunTimeError", "IllegalOperationError", "DivisionByZeroError", "IndexOutOfBoundsError",
    "NameError", "ArgumentError", "InvalidErrorTypeError", "DictKeyError", "NotImplementedError",
    "AttributeError", "TypeError", "ModuleError", "ValueError", "StackDepthExceededError",
    "FileIOError"
};