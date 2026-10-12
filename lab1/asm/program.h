#pragma once

#include <string>
#include <vector>

using namespace std;

#include "../util/text.h"
#include "errors.h"

struct Line {
    long long address = 0;
    int row = 0;
    string kind;
    string mnemonic;
    string op1, op2;
    long long size = 0;
    int format = 0;
    int firstByte = 0;
    string code;
    string label;
    bool reserve = false;
};

struct Symbol {
    string name;
    long long address;
    int row;
};

struct Pass1Result {
    bool ok = false;
    vector<Error> errors;
    vector<Line> lines;
    vector<Symbol> symbols;
    string programName;
    long long start = 0;
    long long length = 0;
    long long entry = 0;
    int endRow = -1;
};

struct CodeLine {
    long long address = 0;
    long long size = 0;
    string code;
    bool reserve = false;
    bool unresolved = false;
    int row = 0;
};

struct Pass2Result {
    bool ok = false;
    vector<Error> errors;
    string programName;
    long long start = 0;
    long long length = 0;
    long long entry = 0;
    vector<CodeLine> code;
};

int findSymbol(vector<Symbol> symbols, string name) {
    for (int i = 0; i < len(symbols); i++) {
        if (sameName(symbols[i].name, name)) return i;
    }
    return -1;
}
