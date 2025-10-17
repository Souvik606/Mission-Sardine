#pragma once

#include <bits/stdc++.h>

using namespace std;

const string DIGITS = "0123456789";
const string LETTERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const string LETTERS_DIGITS = LETTERS + DIGITS;

const string T_INT = "INT";
const string T_FLOAT = "FLOAT";
const string T_IDENTIFIER = "IDENTIFIER";
const string T_KEYWORD = "KEYWORD";
const string T_PLUS = "PLUS";
const string T_MINUS = "MINUS";
const string T_MUL = "MUL";
const string T_DIVIDE = "DIV";
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
const string T_COLON="COLON";
const string T_EOF = "EOF";

const vector<string> KEYWORDS = {
    "define", "and", "or", "not","when","orwhen","otherwise","cycle"
};