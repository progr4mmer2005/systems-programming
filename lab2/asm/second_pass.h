#pragma once

#include <string>

using namespace std;

#include "encoding.h"
#include "program.h"

Pass2Result secondPass(Pass1Result pass1) {
    Pass2Result result;
    result.programName = pass1.programName;
    result.start = pass1.start;
    result.length = pass1.length;
    result.entry = pass1.entry;

    for (Line line : pass1.lines) {
        if (line.kind == "START") continue;
        CodeLine out;
        out.address = line.address;
        out.size = line.size;
        out.code = line.code;
        out.reserve = line.reserve;
        out.row = line.row;

        if (line.label != "") {
            int digits = addressDigits(line.format);
            long long limit = digits == 4 ? 0x10000 : 0x1000000;
            string field = string(digits, '?');
            int i = findSymbol(pass1.symbols, line.label);
            if (i == -1) {
                result.errors.push_back({"source", line.row, "Метка «" + line.label + "» не определена - её нет в ТСИ"});
            } else if (line.addressing == ADDRESSING_RELATIVE) {
                long long next = line.address + line.size;
                long long offset = pass1.symbols[i].address - next;
                if (offset < -limit / 2 || offset >= limit / 2)
                    result.errors.push_back({"source", line.row,
                                             line.mnemonic + " [" + line.label + "]: смещение " + to_string(offset) +
                                                 " не помещается в " + to_string(digits / 2) + " байта"});
                else
                    field = hex((offset + limit) % limit, digits);
            } else if (pass1.symbols[i].address >= limit) {
                result.errors.push_back({"source", line.row,
                                         line.mnemonic + " (формат 3): адрес метки «" + line.label + "» = " +
                                             hexAddress(pass1.symbols[i].address) + " не помещается в 2 байта"});
            } else {
                field = hex(pass1.symbols[i].address, digits);
                result.relocations.push_back({line.address, line.row});
            }
            out.code += field;
            out.unresolved = field[0] == '?';
        }
        result.code.push_back(out);
    }

    result.ok = len(result.errors) == 0;
    return result;
}
