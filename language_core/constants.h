#pragma once

#include <bits/stdc++.h>

using namespace std;

const string DIGITS = "0123456789";
const string LETTERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const string LETTERS_DIGITS = LETTERS + DIGITS;

const string T_INT = "INT";
const string T_FLOAT = "FLOAT";
const string T_STRING = "STRING";
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

const vector<string> KEYWORDS = {
    "define", "and", "or", "not","when","orwhen","otherwise","cycle",
    "during","method","yield","escape","proceed", "menu", "choice", "fallback",
    "risk", "trap", "clean"
};

const vector<string> ERROR_TYPES = {
    "RunTimeError", "IllegalOperationError", "DivisionByZeroError", "IndexOutOfBoundsError",
    "NameError", "ArgumentError", "InvalidErrorTypeError", "DictKeyError", "NotImplementedError"
};