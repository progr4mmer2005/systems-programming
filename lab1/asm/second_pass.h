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
            int i = findSymbol(pass1.symbols, line.label);
            if (i == -1) {
                result.errors.push_back({"source", line.row, "Метка «" + line.label + "» не определена - её нет в ТСИ"});
                out.code += string(digits, '?');
                out.unresolved = true;
            } else if (digits == 4 && pass1.symbols[i].address > 0xFFFF) {
                result.errors.push_back({"source", line.row,
                                         line.mnemonic + " (формат 3): адрес метки «" + line.label + "» = " +
                                             hexAddress(pass1.symbols[i].address) + " не помещается в 2 байта"});
                out.code += string(digits, '?');
                out.unresolved = true;
            } else {
                out.code += hex(pass1.symbols[i].address, digits);
            }
        }
        result.code.push_back(out);
    }

    result.ok = len(result.errors) == 0;
    return result;
}
