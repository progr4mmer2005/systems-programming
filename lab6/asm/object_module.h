#pragma once

#include <algorithm>
#include <string>
#include <vector>

using namespace std;

#include "../util/table.h"
#include "encoding.h"
#include "program.h"

struct ModuleRecord {
    Row cells;
    int sourceRow = -1;
    bool reserve = false;
    bool waiting = false;
    bool unresolved = false;
};

bool definedEarlier(Symbol a, Symbol b) { return a.row < b.row; }

vector<Symbol> externalNames(Assembly& a) {
    vector<Symbol> defined, waiting;
    for (Symbol name : a.extDefs) {
        int i = findSymbol(a.symbols, name.name);
        if (i != -1 && a.symbols[i].defined)
            defined.push_back(a.symbols[i]);
        else
            waiting.push_back(name);
    }
    stable_sort(defined.begin(), defined.end(), definedEarlier);
    for (Symbol name : waiting) defined.push_back(name);
    return defined;
}

vector<ModuleRecord> objectModule(Assembly& a) {
    vector<ModuleRecord> records;
    vector<ModuleSection> sections = a.sections;
    if (!a.ended && a.startRow >= 0) {
        ModuleSection current;
        current.name = a.programName; current.length = a.counter - a.start; current.entry = a.entry;
        current.startRow = a.startRow; current.endRow = a.endRow; current.code = a.code;
        current.symbols = a.symbols; current.extDefs = a.extDefs; current.extRefs = a.extRefs; current.relocations = a.relocations;
        sections.push_back(current);
    }
    for (ModuleSection section : sections) {
        records.push_back({{"H", section.name, "000000", hexAddress(section.length), ""}, section.startRow});
        vector<Symbol> defs, waiting;
        for (Symbol name : section.extDefs) {
            int i = findSymbol(section.symbols, name.name);
            if (i != -1 && section.symbols[i].defined) defs.push_back(section.symbols[i]);
            else waiting.push_back(name);
        }
        stable_sort(defs.begin(), defs.end(), definedEarlier);
        defs.insert(defs.end(), waiting.begin(), waiting.end());
        for (Symbol name : defs) {
            ModuleRecord record = {{"D", name.name, name.defined ? hexAddress(name.address) : "", "", ""}, name.row};
            record.waiting = !name.defined; records.push_back(record);
        }
        for (Symbol ref : section.extRefs) records.push_back({{"R", ref.name, "", "", ""}, ref.row});
        for (CodeLine line : section.code) {
            ModuleRecord record = {{"T", "", hexAddress(line.address), hexLength(line.size), line.code}, line.row};
            record.reserve = line.reserve; record.waiting = line.waiting; record.unresolved = line.unresolved; records.push_back(record);
        }
        for (Relocation relocation : section.relocations)
            records.push_back({{"M", relocation.name, hexAddress(relocation.address), "", ""}, relocation.row});
        records.push_back({{"E", "", hexAddress(section.entry), "", ""}, section.endRow});
    }
    return records;
}

string recordText(Row cells) {
    string type = cells[0], name = cells[1], address = cells[2], size = cells[3], code = cells[4];
    vector<string> parts = {type};
    if (type == "H") parts = {type, name, address, size};
    if (type == "D") parts = {type, name, address};
    if (type == "R") parts = {type, name};
    if (type == "M") parts = {type, address, name};
    if (type == "T") parts = {type, address, size, code};
    if (type == "E") parts = {type, address};
    string text;
    for (string part : parts) {
        if (part == "") continue;
        if (text != "") text += " ";
        text += part;
    }
    return text;
}

string objectModuleText(vector<ModuleRecord> records) {
    string text;
    for (ModuleRecord record : records) text += recordText(record.cells) + "\n";
    return text;
}
