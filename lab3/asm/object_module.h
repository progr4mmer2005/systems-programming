#pragma once

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
    bool unresolved = false;
};

vector<ModuleRecord> objectModule(Pass1Result pass1, Pass2Result pass2) {
    vector<ModuleRecord> records;
    int startRow = len(pass1.lines) > 0 ? pass1.lines[0].row : -1;
    records.push_back({{"H", pass2.programName, hexAddress(pass2.start), hexAddress(pass2.length), ""}, startRow});
    for (Symbol name : pass2.externalNames) records.push_back({{"D", name.name, hexAddress(name.address), "", ""}, name.row});
    for (Symbol ref : pass2.externalRefs) records.push_back({{"R", ref.name, "", "", ""}, ref.row});
    for (Relocation relocation : pass2.relocations)
        records.push_back({{"M", relocation.name, hexAddress(relocation.address), "", ""}, relocation.row});
    for (CodeLine line : pass2.code) {
        ModuleRecord record = {{"T", "", hexAddress(line.address), hexLength(line.size), line.code}, line.row};
        record.reserve = line.reserve;
        record.unresolved = line.unresolved;
        records.push_back(record);
    }
    records.push_back({{"E", "", hexAddress(pass2.entry), "", ""}, pass1.endRow});
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
