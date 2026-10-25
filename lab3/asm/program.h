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
    int addressing = 0;
    string code;
    string label;
    bool reserve = false;
};

struct Symbol {
    string name;
    long long address;
    int row;
    bool external = false;
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
    vector<Symbol> extDefs;
    vector<Symbol> extRefs;
};

struct CodeLine {
    long long address = 0;
    long long size = 0;
    string code;
    bool reserve = false;
    bool unresolved = false;
    int row = 0;
};

struct Relocation {
    long long address;
    int row;
    string name;
};

struct Pass2Result {
    bool ok = false;
    vector<Error> errors;
    string programName;
    long long start = 0;
    long long length = 0;
    long long entry = 0;
    vector<CodeLine> code;
    vector<Relocation> relocations;
    vector<Symbol> externalNames;
    vector<Symbol> externalRefs;
};

int findSymbol(vector<Symbol> symbols, string name) {
    for (int i = 0; i < len(symbols); i++) {
        if (sameName(symbols[i].name, name)) return i;
    }
    return -1;
}
