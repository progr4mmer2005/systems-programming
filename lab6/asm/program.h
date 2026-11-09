#pragma once

#include <string>
#include <vector>

using namespace std;

#include "../util/table.h"
#include "../util/text.h"
#include "errors.h"
#include "optable.h"

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

struct CodeLine {
    long long address = 0;
    long long size = 0;
    string code;
    int format = 0;
    int addressing = 0;
    bool reserve = false;
    bool waiting = false;
    bool unresolved = false;
    int row = 0;
};

struct Relocation {
    long long address;
    int row;
    string name;
};

struct Reference {
    long long address;
    int line;
};

struct Symbol {
    string name;
    bool defined = false;
    long long address = 0;
    int row = -1;
    bool external = false;
    vector<Reference> waiting;
    string section;
};

struct ModuleSection {
    string name;
    long long length = 0;
    long long entry = 0;
    int startRow = -1;
    int endRow = -1;
    vector<CodeLine> code;
    vector<Symbol> symbols, extDefs, extRefs;
    vector<Relocation> relocations;
};

struct Assembly {
    Table source;
    vector<OpCode> ops;
    int mode = 0;
    int nextRow = 0;
    int lastRow = -1;
    int steps = 0;
    bool started = false;
    bool ended = false;
    bool finished = false;
    bool stopped = false;
    bool ok = false;
    vector<Error> errors;
    vector<CodeLine> code;
    vector<Symbol> symbols;
    vector<Relocation> relocations;
    vector<Symbol> extDefs;
    vector<Symbol> extRefs;
    vector<ModuleSection> sections;
    bool codeStarted = false;
    string programName;
    long long start = 0;
    long long counter = 0;
    long long length = 0;
    long long entry = 0;
    int startRow = -1;
    int endRow = -1;
    string done;
};

int findSymbol(vector<Symbol> symbols, string name) {
    for (int i = 0; i < len(symbols); i++) {
        if (sameName(symbols[i].name, name)) return i;
    }
    return -1;
}

vector<Symbol> visibleSymbols(Assembly& a) {
    vector<Symbol> result;
    for (ModuleSection section : a.sections)
        result.insert(result.end(), section.symbols.begin(), section.symbols.end());
    if (!a.ended) {
        vector<Symbol> current = a.symbols;
        for (Symbol& symbol : current) symbol.section = a.programName;
        result.insert(result.end(), current.begin(), current.end());
    }
    return result;
}

bool isDefined(Assembly& a, string name) {
    int i = findSymbol(a.symbols, name);
    return i != -1 && a.symbols[i].defined;
}
